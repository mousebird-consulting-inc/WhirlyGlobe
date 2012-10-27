/*
 *  ShapeReader.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
 *  Copyright 2011-2012 mousebird consulting
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
 *
 */

#import "ShapeReader.h"
#import "shapefil.h"
#import "GlobeMath.h"

namespace WhirlyKit
{

ShapeReader::ShapeReader(NSString *fileName)
{
	const char *cFile =  [fileName cStringUsingEncoding:NSASCIIStringEncoding];
	shp = SHPOpen(cFile, "rb");
	if (!shp)
		return;
	dbf = DBFOpen(cFile, "rb");
    encoding = NSWindowsCP1251StringEncoding;
	where = 0;	
	SHPGetInfo((SHPInfo *)shp, &numEntity, &shapeType, minBound, maxBound);
}
	
ShapeReader::~ShapeReader()
{
	if (shp)
		SHPClose((SHPHandle)shp);
	if (dbf)
		DBFClose((DBFHandle)dbf);
}
	
bool ShapeReader::isValid()
{
	return shp != NULL;
}
    
unsigned int ShapeReader::getNumObjects()
{
    return numEntity;
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
            Point2f pt(WhirlyKit::DegToRad<float>(thisShape->padfX[0]),WhirlyKit::DegToRad<float>(thisShape->padfY[0]));
            points->pts.push_back(pt);
        }
            break;
        case SHPT_MULTIPOINT:
        case SHPT_MULTIPOINTZ:
        {
            VectorPointsRef points = VectorPoints::createPoints();
            theShape = points;
            for (unsigned int ii=0;ii<thisShape->nParts;ii++)
            {
                Point2f pt(WhirlyKit::DegToRad<float>(thisShape->padfX[ii]),WhirlyKit::DegToRad<float>(thisShape->padfY[ii]));
                points->pts.push_back(pt);
            }
        }
            break;
        case SHPT_ARC:
        case SHPT_ARCZ:
        {
            VectorLinearRef linear = VectorLinear::createLinear();
            theShape = linear;
            for (unsigned int ii=0;ii<thisShape->nParts;ii++)
            {
                Point2f pt(WhirlyKit::DegToRad<float>(thisShape->padfX[ii]),WhirlyKit::DegToRad<float>(thisShape->padfY[ii]));
                linear->pts.push_back(pt);
            }            
        }
            break;
        case SHPT_POLYGON:
        case SHPT_POLYGONZ:
        {
            // Copy over vertices (in 2d)
            bool startOne = true;
            VectorArealRef areal = VectorAreal::createAreal();
            theShape = areal;
            VectorRing *ring = NULL;
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
                
                Point2f pt(WhirlyKit::DegToRad<float>(thisShape->padfX[jj]),WhirlyKit::DegToRad<float>(thisShape->padfY[jj]));
                ring->push_back(pt);
            }
            areal->initGeoMbr();
        }
            break;
    }
	
	SHPDestroyObject(thisShape);
	
	// Attributes
    // Note: Probably not complete
	char attrTitle[12];
	int attrWidth, numDecimals;
	NSMutableDictionary *attrDict = [[NSMutableDictionary alloc] init];
	theShape->setAttrDict(attrDict);
	DBFHandle dbfHandle = (DBFHandle)dbf;
	int numDbfRecord = DBFGetRecordCount(dbfHandle);
	if (vecIndex < numDbfRecord)
	{
		for (unsigned int ii = 0; ii < DBFGetFieldCount(dbfHandle); ii++)
		{
			DBFFieldType attrType = DBFGetFieldInfo(dbfHandle, ii, attrTitle, &attrWidth, &numDecimals);
            // If we have a set of filter attrs, skip this one if it's not there
            if (filterAttrs && (filterAttrs->find(attrTitle) == filterAttrs->end()))
                continue;
			NSString *attrTitleStr = [NSString stringWithFormat:@"%s",attrTitle];
			
			if (!DBFIsAttributeNULL(dbfHandle, vecIndex, ii))
			{
				switch (attrType)
				{
					case FTString:
					{
						const char *str = DBFReadStringAttribute(dbfHandle, vecIndex, ii);
                        NSString *newStr = [NSString stringWithCString:str encoding:encoding];
                        if (newStr)
                            [attrDict setObject:newStr forKey:attrTitleStr];
                        //						[attrDict setObject:[NSString stringWithFormat:@"%s",str] forKey:attrTitleStr];
					}
						break;
					case FTInteger:
					{
						NSNumber *num = [NSNumber numberWithInt:DBFReadIntegerAttribute(dbfHandle, vecIndex, ii)];
						[attrDict setObject:num forKey:attrTitleStr];
					}
						break;
					case FTDouble:
					{
						NSNumber *num = [NSNumber numberWithDouble:DBFReadDoubleAttribute(dbfHandle, vecIndex, ii)];
						[attrDict setObject:num forKey:attrTitleStr];
					}
						break;
                    default:
                        break;
				}
			}
		}
	}
    
    // Let the user know what index this is
    [attrDict setObject:[NSNumber numberWithInt:vecIndex] forKey:@"wgshapefileidx"];
	
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
