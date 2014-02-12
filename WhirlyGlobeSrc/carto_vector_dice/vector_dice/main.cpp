//
//  main.cpp
//  vector_dice
//
//  Created by Steve Gifford on 12/6/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

#undef DEBUG
#include <string>
#include <sstream>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "cpl_minixml.h"
#include "tinyxml2.h"
#include <dirent.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "MapnikConfig.h"
#include "VectorDB.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using namespace tinyxml2;

// A spatial index that uses the boost R-Tree
// The way we're chopping things up, we need a fast indexing scheme
class LayerSpatialIndex
{
public:
    // Typedefs for the R-tree
    typedef bg::model::point<double, 2, bg::cs::cartesian> point;
    typedef bg::model::box<point> box;
    typedef std::pair<box, unsigned> value;
    
    LayerSpatialIndex(OGRLayer *layer)
    : layer(layer)
    {
        int numFeatures = layer->GetFeatureCount();
        features.resize(numFeatures,NULL);
        mbrs.resize(numFeatures);
        for (unsigned int ii=0;ii<numFeatures;ii++)
        {
            features[ii] = layer->GetFeature(ii);
            OGRFeature *feature = layer->GetFeature(ii);
            OGREnvelope env;
            feature->GetGeometryRef()->getEnvelope(&env);
            mbrs[ii] = env;
            double spanX = env.MaxX - env.MinX, spanY = env.MaxY - env.MinY;
            // Note: This assumes meters-like numbers
            if (spanX == 0)                spanX = 10.0;
            if (spanY == 0)                spanY = 10.0;
            env.MinX -= spanX/10;  env.MinY -= spanY/10;
            env.MaxX += spanY/10;  env.MaxY += spanY/10;
            box b(point(env.MinX,env.MinY), point(env.MaxX,env.MaxY));
            rtree.insert(std::make_pair(b,ii));
        }
    }
    ~LayerSpatialIndex()
    {
        for (unsigned int ii=0;ii<features.size();ii++)
            OGRFeature::DestroyFeature(features[ii]);
    }
    
    // Return all the features covering a given area
    void findFeatures(double sx,double sy,double ex,double ey,std::vector<OGRFeature *> &retFeatures)
    {
        box query_box(point(sx,sy),point(ex,ey));
        std::vector<value> rets;
        rtree.query(bgi::intersects(query_box), std::back_inserter(rets));
        for (unsigned int ii=0;ii<rets.size();ii++)
            retFeatures.push_back(features[rets[ii].second]);
    }
    
    OGRLayer *layer;
    std::vector<OGRFeature *> features;
    std::vector<OGREnvelope> mbrs;
    // A max of sixteen elements per node, using a quadratic r-tree
    bgi::rtree< value, bgi::rstar<16> > rtree;
};

// Convert the given feature to the given data type
OGRGeometry *ConvertGeometryType(OGRGeometry *inGeom,MapnikConfig::SymbolDataType dataType)
{
    switch (inGeom->getGeometryType())
    {
        case wkbPoint:
        {
            OGRPoint *pt = (OGRPoint *)inGeom;
            if (dataType != MapnikConfig::SymbolDataPoint)
                return NULL;
            return pt->clone();
        }
            break;
        case wkbLineString:
        {
            OGRLineString *lin = (OGRLineString *)inGeom;
            if (dataType != MapnikConfig::SymbolDataLinear)
                return NULL;
            return lin->clone();
        }
            break;
        case wkbPolygon:
            switch (dataType)
        {
            case MapnikConfig::SymbolDataPoint:
            {
                // Note: This isn't guaranteed to be inside the area
                OGREnvelope env;
                inGeom->getEnvelope(&env);
                return new OGRPoint((env.MinX+env.MaxX)/2.0,(env.MinY+env.MaxY)/2.0);
            }
                break;
            case MapnikConfig::SymbolDataLinear:
            {
                return inGeom->Boundary();
            }
                break;
            default:
                return inGeom->clone();
                break;
        }
            break;
        case wkbMultiPoint:
        case wkbMultiLineString:
        case wkbMultiPolygon:
        {
            OGRGeometryCollection *inColl = (OGRGeometryCollection *)inGeom;
            OGRGeometryCollection *outColl = new OGRGeometryCollection();
            for (unsigned int ii=0;ii<inColl->getNumGeometries();ii++)
            {
                OGRGeometry *outGeom = ConvertGeometryType(inColl->getGeometryRef(ii), dataType);
                if (outGeom)
                    outColl->addGeometry(outGeom);
            }
            if (outColl->getNumGeometries() == 0)
            {
                delete outColl;
                outColl = NULL;
            }
            return outColl;
        }
            break;
        default:
            break;
    }
    
    return NULL;
}

// Transform all the geometry in the given layer into the new coordinate system
void TransformLayer(OGRLayer *inLayer,OGRLayer *outLayer,OGRCoordinateTransformation *transform,std::vector<MapnikConfig::CompiledSymbolizerTable::SymbolizerGroup> &symGroups,MapnikConfig::SymbolDataType dataType,OGREnvelope &mbr)
{
    OGREnvelope blank;
    mbr = blank;
    
    for (unsigned int ii=0;ii<inLayer->GetFeatureCount();ii++)
    {
        OGRFeature *feature = inLayer->GetFeature(ii);
        
        // As we go through the styles and rules, these are the symbolizers that apply
        std::vector<int> symbolizers;
        std::set<std::string> attrsToKeep;
        
        // If we've got rules to apply, let's do that here
        bool approved = true;
        if (!symGroups.empty())
        {
            approved = false;

            // Work through the groups, which have their own filters
            for (unsigned int si=0;si<symGroups.size();si++)
            {
                MapnikConfig::CompiledSymbolizerTable::SymbolizerGroup &symGroup = symGroups[si];
                bool styleApproved = false;
                MapnikConfig::Filter &filter = symGroup.filter;
                // Work through the valid rules within this style
                {
                    bool ruleApproved = false;
                    // No filter means it all matches
                    if (filter.isEmpty())
                    {
                        ruleApproved = true;
                    } else {
                        if (filter.logicalOp != MapnikConfig::Filter::OperatorAND)
                        {
                            fprintf(stderr,"Can only currently handle AND in logical operator rules");
                            exit(-1);
                        }
                        
                        // Work through the comparisons
                        for (unsigned int ci=0;ci<filter.comparisons.size();ci++)
                        {
                            MapnikConfig::Filter::Comparison &comp = filter.comparisons[ci];
                            
                            // Look for the attribute we're comparing
                            int idx = feature->GetFieldIndex(comp.attrName.c_str());
                            if (idx >= 0)
                            {
                                // Compare the value.  Might be a string or a real
                                switch (comp.compareValueType)
                                {
                                    case MapnikConfig::Filter::Comparison::CompareString:
                                    {
                                        const char *strVal = feature->GetFieldAsString(idx);
                                        switch (comp.compareType)
                                        {
                                            case MapnikConfig::Filter::Comparison::CompareEqual:
                                                if (!comp.attrValStr.compare(strVal))
                                                    ruleApproved = true;
                                                break;
                                            case MapnikConfig::Filter::Comparison::CompareNotEqual:
                                                if (comp.attrValStr.compare(strVal))
                                                    ruleApproved = true;
                                                break;
                                            default:
                                                fprintf(stderr,"Not expecting value comparison for string.  Giving up.");
                                                exit(-1);
                                                break;
                                        }
                                    }
                                        break;
                                    case MapnikConfig::Filter::Comparison::CompareReal:
                                    {
                                        double val = feature->GetFieldAsDouble(idx);
                                        switch (comp.compareType)
                                        {
                                            case MapnikConfig::Filter::Comparison::CompareEqual:
                                                if (val == comp.attrValReal)
                                                    ruleApproved = true;
                                                break;
                                            case MapnikConfig::Filter::Comparison::CompareNotEqual:
                                                if (val != comp.attrValReal)
                                                    ruleApproved = true;
                                                break;
                                            case MapnikConfig::Filter::Comparison::CompareMore:
                                                if (val > comp.attrValReal)
                                                    ruleApproved = true;
                                                break;
                                            case MapnikConfig::Filter::Comparison::CompareMoreEqual:
                                                if (val >= comp.attrValReal)
                                                    ruleApproved = true;
                                                break;
                                            case MapnikConfig::Filter::Comparison::CompareLess:
                                                if (val < comp.attrValReal)
                                                    ruleApproved = true;
                                                break;
                                            case MapnikConfig::Filter::Comparison::CompareLessEqual:
                                                if (val <= comp.attrValReal)
                                                    ruleApproved = true;
                                                break;
                                        }
                                    }
                                        break;
                                }
                            }
                        }
                    }
                    
                    // Add the symbolizers since we approved this rule
                    if (ruleApproved)
                    {
                        attrsToKeep.insert(symGroup.attrs.begin(),symGroup.attrs.end());
                    }
                    styleApproved |= ruleApproved;
                    
                    approved |= styleApproved;

                    // We may just match the first rule we find
                    // Note: Everything is filter first now
                    if (ruleApproved) // && styleInst.style->filterMode == MapnikConfig::FilterFirst)
                        break;
                }
            }
        }

        if (approved)
        {
            // Copy to the output
            // Convert the geometry type, if needed
            OGRGeometry *geom = ConvertGeometryType(feature->GetGeometryRef(), dataType);
            if (!geom)
                continue;
            
            OGRErr err = geom->transform(transform);
            if (err != OGRERR_NONE)
            {
                fprintf(stderr, "Error transforming feature.");
                exit(-1);
            }
            OGREnvelope thisEnv;
            geom->getEnvelope(&thisEnv);
            mbr.Merge(thisEnv);
//            int numFields = feature->GetFieldCount();

            // Need attributes for the various styles
            for (unsigned int ig=0;ig<symGroups.size();ig++)
            {
                std::string styleIndex = (std::string)"style" + std::to_string(ig);
                // Make sure the field definition is there
                OGRFieldDefn fieldDef(styleIndex.c_str(),OFTString);
                if (outLayer->GetLayerDefn()->GetFieldIndex(styleIndex.c_str()) == -1)
                    if (outLayer->CreateField(&fieldDef) != OGRERR_NONE)
                    {
                        fprintf(stderr,"Unable to create field in output file");
                        exit(-1);
                    }
            }
            
            // And for the attributes the styles expect
            for (std::set<std::string>::iterator it = attrsToKeep.begin();
                 it != attrsToKeep.end(); ++it)
            {
                // Note: Should check the type;
                OGRFieldDefn fieldDef(it->c_str(),OFTString);
                if (outLayer->GetLayerDefn()->GetFieldIndex(it->c_str()) == -1)
                    if (outLayer->CreateField(&fieldDef) != OGRERR_NONE)
                    {
                        fprintf(stderr,"Unable to create field in output file");
                        exit(-1);
                    }
            }
            
//            int whichFeature = outLayer->GetFeatureCount();
//            outLayer->CreateFeature(feature);
//            OGRFeature *newFeature = outLayer->GetFeature(whichFeature);
            OGRFeature *newFeature = new OGRFeature(outLayer->GetLayerDefn());
            newFeature->SetGeometry(geom);

            // Now apply the symbolizers (they'll be styles in the final output)
//            int numNewFields = newFeature->GetFieldCount();
//                std::string styleIndex = (std::string)"style" + std::to_string(si);
            for (unsigned int ig=0;ig<symGroups.size();ig++)
            {
                std::string styleIndex = (std::string)"style" + std::to_string(ig);
                std::string uuid = boost::uuids::to_string(symGroups[ig].uuid);
                newFeature->SetField(styleIndex.c_str(), uuid.c_str());
            }
            
            // And don't forget the attributes they care about
            for (std::set<std::string>::iterator it = attrsToKeep.begin();
                 it != attrsToKeep.end(); ++it)
            {
                std::string fieldName = *it;
                int idx = feature->GetFieldIndex(fieldName.c_str());
                if (idx != -1)
                {
                    // Note: Should deal with other types
                    const char *fieldVal = feature->GetFieldAsString(idx);
                    if (fieldVal)
                        newFeature->SetField(fieldName.c_str(), fieldVal);
                }
            }
            
            outLayer->CreateFeature(newFeature);
            OGRFeature::DestroyFeature(newFeature);
        }
        OGRFeature::DestroyFeature(feature);
    }
}

// Clip the input layer to the given box
void ClipInputToBox(LayerSpatialIndex *layer,double llX,double llY,double urX,double urY,OGRSpatialReference *out_srs,std::vector<OGRFeature *> &outFeatures,OGRCoordinateTransformation *tileTransform)
{
    // Set up the clipping layer
    OGRPolygon poly;
    OGRLinearRing ring;
    ring.addPoint(llX,llY);
    ring.addPoint(llX,urY);
    ring.addPoint(urX,urY);
    ring.addPoint(urX,llY);
    ring.addPoint(llX,llY);
    poly.addRing(&ring);
    OGREnvelope mbr;
    mbr.MinX = llX;  mbr.MinY = llY;
    mbr.MaxX = urX;  mbr.MaxY = urY;
    
    std::vector<OGRFeature *> features;
    layer->findFeatures(llX,llY,urX,urY,features);
    for (unsigned int ii=0;ii<features.size();ii++)
    {
        OGRFeature *inFeature = features[ii];
//            int numFields = inFeature->GetFieldCount();
        OGRGeometry *geom = inFeature->GetGeometryRef();
        try {
            OGRGeometry *clipGeom = geom->Intersection(&poly);
            if (clipGeom && !clipGeom->IsEmpty())
            {
                if (tileTransform)
                    clipGeom->transform(tileTransform);
                OGRFeature *feature = inFeature->Clone();
                feature->SetGeometryDirectly(clipGeom);
                outFeatures.push_back(feature);
            } else if (clipGeom)
                delete clipGeom;
        }
        catch (...)
        {
            fprintf(stderr,"Unhappy intersection call.  Punting geometry.");
        }
    }
}

/************************************************************************/
/*                             SanitizeSRS                              */
/************************************************************************/
// Borrowed from http://svn.osgeo.org/gdal/tags/1.10.0/gdal/apps/gdallocationinfo.cpp
char *SanitizeSRS( const char *pszUserInput )

{
    OGRSpatialReference *hSRS;
    char *pszResult = NULL;
    
    CPLErrorReset();
    
    hSRS = new OGRSpatialReference();
    if( OSRSetFromUserInput( hSRS, pszUserInput ) == OGRERR_NONE )
        OSRExportToWkt( hSRS, &pszResult );
    else
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                 "Translating source or target SRS failed:\n%s",
                 pszUserInput );
        exit( 1 );
    }
    
    OSRDestroySpatialReference( hSRS );
    
    return pszResult;
}

// Get the driver once to save time
OGRSFDriver *shpDriver = NULL;
OGRSFDriver *memDriver = NULL;

// Merge the given features into an existing shapefile or create a new one
bool MergeIntoShapeFile(std::vector<OGRFeature *> &features,OGRLayer *srcLayer,OGRSpatialReference *out_srs,const char *fileName)
{
    // Look for an existing shapefile
    OGRLayer *destLayer = NULL;
    OGRDataSource *poCDS = shpDriver->Open(fileName,true);
    if (poCDS)
    {
        // Use the layer in the data file
        destLayer = poCDS->GetLayer(0);
    } else {
        // Create a data file and a layer
        poCDS = shpDriver->CreateDataSource(fileName);
        if (!poCDS)
        {
            fprintf(stderr,"Unable to create output file: %s\n",fileName);
            return false;
        }
        destLayer = poCDS->CreateLayer("features",out_srs);
        if (!destLayer)
        {
            fprintf(stderr,"Unable to create output file: %s\n",fileName);
            return false;
        }
    }
    
    // Add the various fields from one layer into another
    OGRFeatureDefn *featureDfn = srcLayer->GetLayerDefn();
    if (featureDfn)
        for (unsigned int ii=0;ii<featureDfn->GetFieldCount();ii++)
        {
            OGRFieldDefn fieldDefn = featureDfn->GetFieldDefn(ii);
            switch (fieldDefn.GetType())
            {
                case OFTInteger:
                case OFTReal:
                case OFTString:
                case OFTWideString:
                    if (destLayer->GetLayerDefn()->GetFieldIndex(fieldDefn.GetNameRef()) == -1)
                        destLayer->CreateField(&fieldDefn);
                    break;
                default:
                    break;
            }
        }
    poCDS->SyncToDisk();
    
    for (unsigned int ii=0;ii<features.size();ii++)
    {
        OGRFeature *feature = features[ii];
        OGRFeature *newFeature = OGRFeature::CreateFeature(destLayer->GetLayerDefn());
        newFeature->SetFrom(feature);
        destLayer->CreateFeature(newFeature);
    }
    
    OGRDataSource::DestroyDataSource(poCDS);
    
    return true;
}

// Used for tracking what we build
class BuildStats
{
public:
    BuildStats()
    {
        minFeat = 1e10,maxFeat = 0;
        featAvg = 0.0;
        numTiles = 0;
    }
    
    int minFeat,maxFeat;
    double featAvg;
    int numTiles;
};

// Chop up a shapefile into little bits
void ChopShapefile(const char *layerName,const char *inputFile,std::vector<MapnikConfig::CompiledSymbolizerTable::SymbolizerGroup> &symGroups, MapnikConfig::SymbolDataType dataType,const char *targetDir,double xmin,double ymin,double xmax,double ymax,int level,OGRSpatialReference *hTrgSRS,OGRSpatialReference *hTileSRS,BuildStats &stats,OGREnvelope &totalEnv,OGREnvelope *clipEnv,int &copiedFeatures)
{
    // Number of cells at this level
    int numCells = 1<<level;
    double cellSizeX = (xmax-xmin)/numCells;
    double cellSizeY = (ymax-ymin)/numCells;

    OGRDataSource *poDS = shpDriver->Open(inputFile);
    if( poDS == NULL )
    {
        fprintf(stderr, "Couldn't open file: %s\n",inputFile);
        exit( 1 );
    }
    
//    fprintf(stdout,"Input File: %s\n",inputFile);
//    fprintf(stdout,"            %d layers\n",poDS->GetLayerCount());
    
    // Work through the layers
    for (unsigned int jj=0;jj<poDS->GetLayerCount();jj++)
    {
        // Copy the input layer into memory and reproject it
        OGRLayer *inLayer = poDS->GetLayer(jj);
        OGRSpatialReference *this_srs = inLayer->GetSpatialRef();
//        fprintf(stdout,"            Layer: %s, %d features\n",inLayer->GetName(),inLayer->GetFeatureCount());
        
        OGREnvelope psExtent;
        inLayer->GetExtent(&psExtent);
        
        // Transform and copy features
        OGRCoordinateTransformation *transform = OGRCreateCoordinateTransformation(this_srs,hTrgSRS);
        if (!transform)
        {
            fprintf(stderr,"Can't transform from coordinate system to destination for: %s\n",inputFile);
            exit(-1);
        }
        OGRCoordinateTransformation *tileTransform = OGRCreateCoordinateTransformation(hTrgSRS,hTileSRS);
        if (!tileTransform)
        {
            fprintf(stderr,"Can't transform from coordinate system to tile for: %s\n",inputFile);
            exit(-1);
        }
        
        OGRDataSource *memDS = memDriver->CreateDataSource("memory");
        OGRLayer *layer = memDS->CreateLayer("layer",this_srs);
        // We'll also apply any rules and attribute changes at this point
        TransformLayer(inLayer,layer,transform,symGroups,dataType,psExtent);
        LayerSpatialIndex layerIndex(layer);
        copiedFeatures += layer->GetFeatureCount();
        
        // Which cells might we be covering
        int sx = floor((psExtent.MinX - xmin) / cellSizeX);
        int sy = floor((psExtent.MinY - ymin) / cellSizeY);
        int ex = ceil((psExtent.MaxX - xmin) / cellSizeX);
        int ey = ceil((psExtent.MaxY - ymin) / cellSizeY);
        sx = std::max(sx,0);  sy = std::max(sy,0);
        ex = std::min(ex,numCells-1);  ey = std::min(ey,numCells-1);
        
        // Work through the possible cells
        for (unsigned int iy=sy;iy<=ey;iy++)
        {
            for (unsigned int ix=sx;ix<=ex;ix++)
            {
                std::string cellDir = (std::string)targetDir + "/" + std::to_string(iy) + "/";
                std::string cellFileName = cellDir + std::to_string(ix) + ".shp";
                
                // Clip the input geometry to this cell
                OGREnvelope cellEnv;
                cellEnv.MinX = ix*cellSizeX+xmin;
                cellEnv.MinY = iy*cellSizeY+ymin;
                cellEnv.MaxX = (ix+1)*cellSizeX+xmin;
                cellEnv.MaxY = (iy+1)*cellSizeY+ymin;
                
                // Check against clip bounds
                if (clipEnv && !(clipEnv->Intersects(cellEnv) || clipEnv->Contains(cellEnv)))
                    continue;
                
                totalEnv.Merge(cellEnv);
                
                std::vector<OGRFeature *> clippedFeatures;
                ClipInputToBox(&layerIndex,cellEnv.MinX,cellEnv.MinY,cellEnv.MaxX,cellEnv.MaxY,hTrgSRS,clippedFeatures,tileTransform);
                
                //                    fprintf(stdout, "            Cell (%d,%d):  %d features\n",ix,iy,cellLayer->GetFeatureCount());
                
                // Clean up and flush output data
                int numFeat = (int)clippedFeatures.size();
                stats.minFeat = std::min(stats.minFeat,numFeat);
                stats.maxFeat = std::max(stats.maxFeat,numFeat);
                stats.numTiles++;
                stats.featAvg += numFeat;
                if (numFeat > 0)
                {
                    std::string cellDir = (std::string)targetDir + "/" + std::to_string(level) + "/" + std::to_string(iy) + "/";
                    mkdir(cellDir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    const char *typeName = (dataType == MapnikConfig::SymbolDataPoint ? "_p" : ((dataType == MapnikConfig::SymbolDataLinear) ? "_l" : ((dataType == MapnikConfig::SymbolDataAreal) ? "_a" : "_u")));
                    std::string cellFileName = cellDir + std::to_string(ix) + layerName + typeName + ".shp";
                    if (!MergeIntoShapeFile(clippedFeatures,layer,hTrgSRS,cellFileName.c_str()))
                        exit(-1);
                }
                
                for (unsigned int ii=0;ii<clippedFeatures.size();ii++)
                    OGRFeature::DestroyFeature(clippedFeatures[ii]);
            }
        }
        
        OGRDataSource::DestroyDataSource(memDS);
    }
    
    delete poDS;
}

// Merge vectors from the source into the dest
void MergeDataIntoLayer(OGRLayer *destLayer,OGRLayer *srcLayer)
{
    
}

// This is the map scale for the given level.
// These are sort of made up to match what TileMill is producing
int ScaleForLevel(int level)
{
    int exp = 22-level;
    return (1<<exp) * 150;
}

int main(int argc, char * argv[])
{
    const char *targetDir = NULL;
    const char *targetDb = NULL;
    const char *shapeCacheDir = NULL;
    char *destSRS = NULL,*tileSRS = NULL;
    bool teSet = false;
    char *xmlConfig = NULL;
    double xmin=-20037508.34,ymin=-20037508.34,xmax=20037508.34,ymax=20037508.3;
    bool clipBoundsSet = false;
    double clipXmin,clipYmin,clipXmax,clipYmax;
    int minLevel = -1,maxLevel = -1;
    
    GDALAllRegister();
    OGRRegisterAll();
    argc = GDALGeneralCmdLineProcessor( argc, &argv, 0 );
    if( argc < 1 )
        exit( -argc );
    int numArgs = 1;

    for (unsigned int ii=1;ii<argc;ii+=numArgs)
    {
        numArgs = 1;
        
        if (EQUAL(argv[ii],"-targetdir"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -targetdir\n");
                return -1;
            }
            targetDir = argv[ii+1];
        } else if (EQUAL(argv[ii],"-cachedir"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -cachedir\n");
                return -1;
            }
            shapeCacheDir = argv[ii+1];
        } else if (EQUAL(argv[ii],"-targetdb"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -targetdb\n");
                return -1;
            }
            targetDb = argv[ii+1];
        } else if (EQUAL(argv[ii],"-t_srs"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -t_srs\n");
                return -1;
            }
            CPLFree(destSRS);
            destSRS = SanitizeSRS(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-te"))
        {
            numArgs = 5;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting four arguments for -te\n");
                return -1;
            }
            xmin = atof(argv[ii+1]);
            ymin = atof(argv[ii+2]);
            xmax = atof(argv[ii+3]);
            ymax = atof(argv[ii+4]);
            teSet = true;
        } else if (EQUAL(argv[ii],"-levels"))
        {
            numArgs = 3;
            if (ii+numArgs > argc)
            {
                fprintf(stderr, "Expecting two arguments for -levels\n");
                return -1;
            }
            minLevel = atoi(argv[ii+1]);
            maxLevel = atoi(argv[ii+2]);
        } else if (EQUAL(argv[ii],"-tile_srs"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -tile_srs\n");
                return -1;
            }
            tileSRS = SanitizeSRS(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-config"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -config");
                return -1;
            }
            xmlConfig = argv[ii+1];
        } else if (EQUAL(argv[ii],"-clip"))
        {
            numArgs = 5;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -clip");
                return -1;
            }
            clipBoundsSet = true;
            clipXmin = atof(argv[ii+1]);
            clipYmin = atof(argv[ii+2]);
            clipXmax = atof(argv[ii+3]);
            clipYmax = atof(argv[ii+4]);
        }
    }

    if (!targetDir)
    {
        fprintf(stderr, "Need a target directory.\n");
        return -1;
    }
    
    if (!destSRS)
        fprintf(stdout,"Missing target SRS.  Defaulting to EPSG:3857.\n");
    
    if (!teSet)
        fprintf(stdout,"Missing bounding box.  Defaulting to full extent of web mercator.\n");
    
    if (minLevel < 0 || maxLevel < 0)
    {
        fprintf(stderr, "Need to set target levels\n");
        return -1;
    }
    
    if (!tileSRS)
        fprintf(stdout,"Missing tile SRS.  Defaulting to EPSG:4326.\n");
    
    if (!xmlConfig)
    {
        fprintf(stderr, "Need a mapnik config file.\n");
        return -1;
    }
    
    if (shapeCacheDir)
    {
        mkdir(shapeCacheDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    
    shpDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile" );
    if (!shpDriver)
    {
        fprintf(stderr, "Couldn't get shape file driver");
        return -1;
    }
    memDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("Memory");
    if (!memDriver)
    {
        fprintf(stderr, "Couldn't get memory driver");
        return -1;
    }
    
    // If the XML config file is here, try to parse it
    MapnikConfig *mapnikConfig = NULL;
    XMLDocument doc;
    if (xmlConfig)
    {
        if (doc.LoadFile(xmlConfig) != XML_NO_ERROR)
        {
            fprintf(stderr, "Failed to parse Mapnik config file\n");
            return -1;
        }
        mapnikConfig = new MapnikConfig();
        std::string errorStr;
        if (!mapnikConfig->parseXML(&doc, errorStr))
        {
            fprintf(stderr,"Failed to parse Mapnik config file because:\n%s\n",errorStr.c_str());
            return -1;
        }
    }
    
    // We won't bother with anything outside this boundary
    OGREnvelope clipBounds;
    if (clipBoundsSet)
    {
        clipBounds.MinX = clipXmin;
        clipBounds.MinY = clipYmin;
        clipBounds.MaxX = clipXmax;
        clipBounds.MaxY = clipYmax;
    }
    
    // Clear out the target directory
    system(((std::string) "rm -rf " + targetDir).c_str() );
    
    // Create the output directory
    mkdir(targetDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    for (int level = minLevel;level<=maxLevel;level++)
        mkdir(((std::string)targetDir+"/"+std::to_string(level)).c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // Set up a coordinate transformation
    OGRSpatialReference *hTrgSRS = new OGRSpatialReference ( destSRS ? destSRS : SanitizeSRS("EPSG:3857") );
    OGRSpatialReference *hTileSRS = new OGRSpatialReference ( tileSRS ? tileSRS : SanitizeSRS("EPSG:4326") );
    
    // Keep track of what we've built
    BuildStats buildStats;
    std::set<std::string> layerNames;
    OGREnvelope fullExtents;
    
    if (mapnikConfig)
    {
        // Work through the layers
        for (unsigned int ii=0;ii<mapnikConfig->layers.size();ii++)
        {
            MapnikConfig::Layer &inLayer = mapnikConfig->layers[ii];
            MapnikConfig::SortedLayer layer(mapnikConfig,inLayer);
            if (!layer.isValid())
            {
                fprintf(stderr,"Problem parsing layer definition: %s\n",inLayer.name.c_str());
                exit(-1);
            }
            std::vector<bool> sortedStylesDone(layer.sortStyles.size(),false);
            
            fprintf(stdout,"Data Source: %s\n",inLayer.name.c_str());
            fflush(stdout);
            
            // Work through the levels
            for (int li=minLevel;li<=maxLevel;li++)
            {
                fprintf(stdout, "  Level %d\n",li);
                int scale = ScaleForLevel(li);
                
                // Look through the sorted styles that might apply and aren't done
                for (unsigned int ssi=0;ssi<layer.sortStyles.size();ssi++)
                {
                    MapnikConfig::SortedLayer::SortedStyle *style = &layer.sortStyles[ssi];
                    if (!sortedStylesDone[ssi] && (style->maxScale == 0 || style->maxScale >= scale))
                    {
                        sortedStylesDone[ssi] = true;
                        // Compile these styles, which gives us the UUIDs we need below for the groups
                        // Individual features point to these groups
                        std::vector<MapnikConfig::CompiledSymbolizerTable::SymbolizerGroup> symGroups;
                        mapnikConfig->compiledSymTable.addSymbolizerGroup(mapnikConfig, style, symGroups);
                        
                        // This is one specific filter, so we need to chop this version right here
                        // We'll work through the data type.  Chop will actually merge, so we can hit the same layer multiple times.
                        int totalFeatures = 0;
                        for (unsigned int di=0;di<MapnikConfig::SymbolDataUnknown;di++)
                        {
                            const char *symbolType = NULL;
                            switch (di)
                            {
                                case MapnikConfig::SymbolDataPoint:
                                    symbolType = "Point";
                                    break;
                                case MapnikConfig::SymbolDataLinear:
                                    symbolType = "Linear";
                                    break;
                                case MapnikConfig::SymbolDataAreal:
                                    symbolType = "Areal";
                                    break;
                            }
                            int numRules = 0;
                            for (unsigned int si=0;si<style->styleInstances.size();si++)
                                numRules += style->styleInstances[si].rules.size();
                            if (style->filter.filter.empty())
                                fprintf(stdout, "\tData Type = %s; Empty Filter, %d rules\n",symbolType,numRules);
                            else
                                fprintf(stdout,"\tData Type = %s; Filter = %s, %d rules\n",symbolType,style->filter.filter.c_str(),numRules);
                            
                            int copiedFeatures = 0;
                            std::string thisLayerName = inLayer.name;
                            ChopShapefile(thisLayerName.c_str(), inLayer.dataSources[0]->getShapefileName(&inLayer,(shapeCacheDir ? shapeCacheDir : targetDir)).c_str(), symGroups, (MapnikConfig::SymbolDataType)di, targetDir, xmin, ymin, xmax, ymax, li, hTrgSRS, hTileSRS, buildStats,fullExtents, (clipBoundsSet ? &clipBounds : NULL),copiedFeatures);
                            
                            if (copiedFeatures > 0)
                            {
                                mapnikConfig->compiledSymTable.layerNames.insert(thisLayerName);

                                fprintf(stdout,"\t\tChopped %d %s features\n",copiedFeatures,symbolType);
                                totalFeatures += copiedFeatures;
                            }
                        }
                        
                        // Something used this style, so writ it out
                        if (totalFeatures > 0)
                        {
                        }
                    }
                    
                    // We know something came over from this layer
                    layerNames.insert(inLayer.name);
                }
            }
        }
        
        // Write the symbolizers out as JSON
        std::string styleJson;
        mapnikConfig->compiledSymTable.minLevel = minLevel;
        mapnikConfig->compiledSymTable.maxLevel = maxLevel;
        if (!mapnikConfig->compiledSymTable.writeJSON(styleJson))
        {
            fprintf(stderr, "Failed to convert styles to JSON");
            return -1;
        }
        try {
            std::ofstream outStream((std::string)targetDir+"/styles.json");
            outStream << styleJson;
            
        } catch (...) {
            fprintf(stderr,"Could not write styles.json file");
            return -1;
        }
    }
    
    if (buildStats.numTiles > 0)
        fprintf(stdout,"Feature Count\n  Min = %d, Max = %d, Avg = %f\n",buildStats.minFeat,buildStats.maxFeat,buildStats.featAvg/buildStats.numTiles);
    
    // If there's a target DB we'll build a sqlite database from the shapefiles
    if (targetDb && mapnikConfig)
    {
//        OGRSFDriver *memDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("Memory");

        fprintf(stdout,"Compiling to vector DB: %s\n",targetDb);
        GDALTermProgress(0.0,NULL,NULL);
        
        // Total number of chunks to process
        int totalNumCells = 0;
        for (unsigned int level = minLevel; level <= maxLevel; level++)
        {
            // We don't do the whole span of each level
            int numCells = 1<<level;
            double cellSizeX = (xmax-xmin)/numCells;
            double cellSizeY = (ymax-ymin)/numCells;
            int sx = floor((fullExtents.MinX - xmin) / cellSizeX);
            int sy = floor((fullExtents.MinY - ymin) / cellSizeY);
            int ex = ceil((fullExtents.MaxX - xmin) / cellSizeX);
            int ey = ceil((fullExtents.MaxY - ymin) / cellSizeY);
            sx = std::max(sx,0);  sy = std::max(sy,0);
            ex = std::min(ex,numCells-1);  ey = std::min(ey,numCells-1);

            totalNumCells += (ex-sx+1)*(ey-sy+1);
        }
        
        remove(targetDb);
        Kompex::SQLiteDatabase *sqliteDb = NULL;
        sqliteDb = new Kompex::SQLiteDatabase(targetDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
        if (!sqliteDb->GetDatabaseHandle())
        {
            fprintf(stderr, "Invalid sqlite database: %s\n",targetDb);
            return -1;
        }
        
        // Do one big transaction
        {
            Kompex::SQLiteStatement transactStmt(sqliteDb);
            transactStmt.SqlStatement((std::string)"BEGIN TRANSACTION");
        }
        
        int cellsProcessed = 0;
        try {
            // Set up the vector DB
            Maply::VectorDatabase *vectorDb = new Maply::VectorDatabase();
            vectorDb->setupDatabase(sqliteDb, destSRS, tileSRS, xmin, ymin, ymax, ymax, minLevel, maxLevel, true);
            
            // Write out the styles
            for (unsigned int ii=0;ii<mapnikConfig->compiledSymTable.symGroups.size();ii++)
            {
                std::string styleStr;
                MapnikConfig::CompiledSymbolizerTable::SymbolizerGroup &symGroup = mapnikConfig->compiledSymTable.symGroups[ii];
                symGroup.toString(styleStr);
                std::string name = "style " + std::to_string(ii);
                vectorDb->addStyle(name.c_str(), styleStr.c_str());
            }
            
            // Set up the layer tables
            std::vector<int> layerIDs;
            std::vector<std::string> localLayerNames;
            for (std::set<std::string>::iterator it = layerNames.begin();it != layerNames.end(); ++it)
            {
                const std::string &layerName = *it;
                int layerID = vectorDb->addVectorLayer(layerName.c_str());
                localLayerNames.push_back(layerName);
                layerIDs.push_back(layerID);
            }
            
            // Work through the levels
            for (unsigned int level = minLevel; level <= maxLevel; level++)
            {
                int numCells = 1<<level;
                double cellSizeX = (xmax-xmin)/numCells;
                double cellSizeY = (ymax-ymin)/numCells;

                // Work through the cells
                int sx = floor((fullExtents.MinX - xmin) / cellSizeX);
                int sy = floor((fullExtents.MinY - ymin) / cellSizeY);
                int ex = ceil((fullExtents.MaxX - xmin) / cellSizeX);
                int ey = ceil((fullExtents.MaxY - ymin) / cellSizeY);
                sx = std::max(sx,0);  sy = std::max(sy,0);
                ex = std::min(ex,numCells-1);  ey = std::min(ey,numCells-1);
                
                for (unsigned int iy=sy;iy<=ey;iy++)
                {
                    // If there's nothing in the row we can skip that
                    std::string yDir = (std::string)targetDir + "/" + std::to_string(level) + "/" + std::to_string(iy);
                    boost::filesystem::path yDirPath(yDir);
                    if (!boost::filesystem::exists(yDirPath))
                    {
                        cellsProcessed += (ex-sx+1);
                        double done = cellsProcessed/((double)totalNumCells);
                        GDALTermProgress(done,NULL,NULL);
                        continue;
                    }
                    
                    // Collect up all the filenames for the row, to save us from opening files one by one
                    std::set<std::string> yFileNames;
                    for (boost::filesystem::directory_iterator dirIter = boost::filesystem::directory_iterator(yDir);
                         dirIter != boost::filesystem::directory_iterator(); ++dirIter)
                    {
                        boost::filesystem::path p = dirIter->path();
                        std::string ext = p.extension().string();
                        if (!ext.compare(".shp"))
                        {
                            yFileNames.insert(p.string());
                        }
                    }

                    for (unsigned int ix=sx;ix<=ex;ix++)
                    {
                        // Set up an in memory data source
//                        OGRDataSource *memDS = memDriver->CreateDataSource("memory");
//                        OGRLayer *memLayer = memDS->CreateLayer("layer",hTileSRS);
                        
                        // Work through the layers at this level
                        for (unsigned int li=0;li<layerNames.size();li++)
                        {
                            // Load all the data types together into memory at once
                            std::string &layerName = localLayerNames[li];
                            std::string cellDir = (std::string)targetDir + "/" + std::to_string(level) + "/" + std::to_string(iy) + "/";

                            std::vector<OGRDataSource *> dataSources;
                            std::vector<OGRFeature *> layerFeatures;
                            for (int di=0;di<MapnikConfig::SymbolDataUnknown;di++)
                            {
                                const char *typeName = (di == MapnikConfig::SymbolDataPoint ? "_p" : ((di == MapnikConfig::SymbolDataLinear) ? "_l" : ((di == MapnikConfig::SymbolDataAreal) ? "_a" : "_u")));
                                std::string cellFileName = cellDir + std::to_string(ix) + layerName + typeName + ".shp";
                                
                                // Look for the filename in the name cache.  Faster.
                                if (yFileNames.find(cellFileName) == yFileNames.end())
                                    continue;

                                // Open the shapefile and get pointers to the features
                                OGRDataSource *poCDS = shpDriver->Open(cellFileName.c_str());
                                if (poCDS)
                                {
                                    OGRLayer *srcLayer = poCDS->GetLayer(0);
                                    for (unsigned int fi=0;fi<srcLayer->GetFeatureCount();fi++)
                                        layerFeatures.push_back(srcLayer->GetFeature(fi));
                                    dataSources.push_back(poCDS);
                                }
                            }
                            
                            // Write the data out in our custom format
                            try {
                                if (!layerFeatures.empty())
                                {
                                    std::vector<unsigned char> vecData;
                                    vectorDb->vectorToDBFormat(layerFeatures, vecData);
                                    if (!vecData.empty())
                                        vectorDb->addVectorTile(ix, iy, level, layerIDs[li], (const char *)&vecData[0], (int)vecData.size());
                                }
                            }
                            catch (std::string &errorStr)
                            {
                                fprintf(stderr,"Unable to write tile %d: (%d,%d)\nBecause: %s\n",level,ix,iy,errorStr.c_str());
                                return -1;
                            }
                            
                            // Clean up the data sources
                            for (unsigned int si=0;si<dataSources.size();si++)
                                OGRDataSource::DestroyDataSource(dataSources[si]);
                        }
                        
                        cellsProcessed++;
                        double done = cellsProcessed/((double)totalNumCells);
                        GDALTermProgress(done,NULL,NULL);
                    }
                }
            }
        }
        catch (const std::string &what)
        {
            fprintf(stderr,"Failed to write to target DB because:\n%s\n",what.c_str());
            return -1;
        }
        
        // End the big transaction
        {
            Kompex::SQLiteStatement transactStmt(sqliteDb);
            transactStmt.SqlStatement((std::string)"END TRANSACTION");
        }

        GDALTermProgress(1.0,NULL,NULL);
    }
    
    if (mapnikConfig)
        delete mapnikConfig;
    
    return 0;
}

