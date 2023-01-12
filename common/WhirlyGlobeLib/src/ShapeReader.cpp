/*  ShapeReader.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
 *  Copyright 2011-2023 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import "ShapeReader.h"
#import "shapefil.h"
#import "GlobeMath.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

ShapeReader::ShapeReader(const std::string &fileName)
{
    const char *cFile = fileName.c_str();
    shp = SHPOpen(cFile, "rb");
    if (!shp)
        return;
    dbf = DBFOpen(cFile, "rb");
    if (!dbf)
        return;
    where = 0;
    SHPGetInfo((SHPInfo *)shp, &numEntity, &shapeType, minBound, maxBound);
}

ShapeReader::~ShapeReader()
{
    try
    {
        if (shp)
            SHPClose((SHPHandle)shp);
        if (dbf)
            DBFClose((DBFHandle)dbf);
    }
    WK_STD_DTOR_CATCH()
}

/* Shapefiles support a lot of types.  Here are the ones we recognize:
    SHPT_ARC            yes
    SHPT_POLYGON        yes
    SHPT_MULTIPOINT     yes
    SHPT_POINTZ         yes
    SHPT_ARCZ           yes
    SHPT_POLYGONZ       yes
    SHPT_MULTIPOINTZ    yes
    SHPT_POINTM         no
    SHPT_ARCM           no
    SHPT_POLYGONM       no
    SHPT_MULTIPOINTM    no
    SHPT_MULTIPATCH     no
 */ 
    
// Return a single shape by index
VectorShapeRef ShapeReader::getObjectByIndex(unsigned int vecIndex,const StringSet *filterAttrs)
{
    // Read from disk
    SHPObject *thisShape = SHPReadObject((SHPInfo *)shp, vecIndex);
    
    VectorShapeRef theShape;
    switch (shapeType)
    {
        case SHPT_POINT:
        case SHPT_POINTZ:
        {
            VectorPointsRef points = VectorPoints::createPoints();
            theShape = points;
            points->pts.emplace_back((float)WhirlyKit::DegToRad(thisShape->padfX[0]),
                                     (float)WhirlyKit::DegToRad(thisShape->padfY[0]));
            break;
        }
        case SHPT_MULTIPOINT:
        case SHPT_MULTIPOINTZ:
        {
            VectorPointsRef points = VectorPoints::createPoints();
            points->pts.reserve(thisShape->nParts);
            theShape = points;
            for (unsigned int ii=0;ii<thisShape->nParts;ii++)
            {
                points->pts.emplace_back((float)WhirlyKit::DegToRad(thisShape->padfX[ii]),
                                         (float)WhirlyKit::DegToRad(thisShape->padfY[ii]));
            }
            break;
        }
        case SHPT_ARC:
        case SHPT_ARCZ:
        {
            VectorLinearRef linear = VectorLinear::createLinear();
            linear->pts.reserve(thisShape->nVertices);
            theShape = linear;
            for (unsigned int ii=0;ii<thisShape->nVertices;ii++)
            {
                linear->pts.emplace_back((float)WhirlyKit::DegToRad(thisShape->padfX[ii]),
                                         (float)WhirlyKit::DegToRad(thisShape->padfY[ii]));
            }
            break;
        }
        case SHPT_POLYGON:
        case SHPT_POLYGONZ:
        {
            // Copy over vertices (in 2d)
            bool startOne = true;
            VectorArealRef areal = VectorAreal::createAreal();
            theShape = areal;
            VectorRing *ring = nullptr;
            for (unsigned int jj = 0, iPart = 1; jj < thisShape->nVertices; jj++)
            {
                // Add rings to the given areal until we're done
                if ( iPart < thisShape->nParts && thisShape->panPartStart[iPart] == jj)
                {
                    iPart++;
                    startOne = true;
                }
                
                if (startOne)
                {
                    areal->loops.resize(areal->loops.size()+1);
                    ring = &areal->loops.back();
                    startOne = false;
                }
                
                ring->emplace_back((float)WhirlyKit::DegToRad(thisShape->padfX[jj]),
                                   (float)WhirlyKit::DegToRad(thisShape->padfY[jj]));
            }
            areal->initGeoMbr();
            break;
        }
        default:
            return VectorShapeRef();
    }

    SHPDestroyObject(thisShape);

    // Attributes
    char attrTitle[12];
    int attrWidth, numDecimals;
    MutableDictionaryRef attrDict = theShape->getAttrDict();
    const auto dbfHandle = (DBFHandle)dbf;
    const int numDbfRecord = DBFGetRecordCount(dbfHandle);
    if (vecIndex < numDbfRecord)
    {
        for (int ii = 0; ii < DBFGetFieldCount(dbfHandle); ii++)
        {
            DBFFieldType attrType = DBFGetFieldInfo(dbfHandle, ii, attrTitle, &attrWidth, &numDecimals);
            // If we have a set of filter attrs, skip this one if it's not there
            if (filterAttrs && (filterAttrs->find(attrTitle) == filterAttrs->end()))
                continue;

            if (!DBFIsAttributeNULL(dbfHandle, (int)vecIndex, ii))
            {
                switch (attrType)
                {
                    case FTString:
                        if (const char *str = DBFReadStringAttribute(dbfHandle, (int)vecIndex, ii))
                        {
                            attrDict->setString(attrTitle, str);
                        }
                        break;
                    case FTInteger:
                        attrDict->setInt(attrTitle, DBFReadIntegerAttribute(dbfHandle, (int)vecIndex, ii));
                        break;
                    case FTDouble:
                        attrDict->setDouble(attrTitle, DBFReadDoubleAttribute(dbfHandle, (int)vecIndex, ii));
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // Let the user know what index this is
    attrDict->setInt("wgshapefileidx", (int)vecIndex);
    return theShape;
}

// Return the next shape
VectorShapeRef ShapeReader::getNextObject(const StringSet *filterAttrs)
{
    // Reached the end
    if (where >= numEntity)
        return VectorShapeRef();
    
    VectorShapeRef retShape = getObjectByIndex(where, filterAttrs);
    where++;
    
    return retShape;
}

}
