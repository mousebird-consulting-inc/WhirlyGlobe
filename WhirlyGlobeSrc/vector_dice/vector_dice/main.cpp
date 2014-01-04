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
#include "MapnikConfig.h"

using namespace tinyxml2;

// Used to store and calculate feature MBRs
class LayerMBRs
{
public:
    LayerMBRs(OGRLayer *layer)
    : layer(layer)
    {
        features.reserve(layer->GetFeatureCount());
        mbrs.reserve(layer->GetFeatureCount());
        for (unsigned int ii=0;ii<layer->GetFeatureCount();ii++)
        {
            OGRFeature *feature = layer->GetFeature(ii);
            OGRGeometry *geom = feature->GetGeometryRef();
            OGREnvelope mbr;
            geom->getEnvelope(&mbr);
            features.push_back(feature);
            mbrs.push_back(mbr);
        }
    }
    ~LayerMBRs()
    {
        for (unsigned int ii=0;ii<features.size();ii++)
        {
            OGRFeature::DestroyFeature(features[ii]);
        }
    }
    OGRLayer *layer;
    std::vector<OGRFeature *> features;
    std::vector<OGREnvelope> mbrs;
};

// Transform all the geometry in the given layer into the new coordinate system
void TransformLayer(OGRLayer *inLayer,OGRLayer *outLayer,OGRCoordinateTransformation *transform,std::vector<MapnikConfig::Style> *styles,OGREnvelope &mbr)
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
        if (styles)
        {
            approved = false;

            // Work through the styles
            for (unsigned int si=0;si<styles->size();si++)
            {
                MapnikConfig::Style &style = styles->at(si);
                bool styleApproved = false;
                // Work through the valid rules within this style
                for (unsigned int ri=0;ri<style.rules.size();ri++)
                {
                    MapnikConfig::Rule &rule = style.rules[ri];
                    bool ruleApproved = false;
                    // No filter means it all matches
                    if (rule.filter.isEmpty())
                    {
                        ruleApproved = true;
                    } else {
                        // Look for the attribute we're comparing
                        int idx = feature->GetFieldIndex(rule.filter.attrName.c_str());
                        if (idx >= 0)
                        {
                            // Compare the value.  Just string comparison at the moment
                            const char *strVal = feature->GetFieldAsString(idx);
                            if (!rule.filter.attrVal.compare(strVal))
                                ruleApproved = true;
                        }
                    }
                    
                    // Add the symbolizers since we approved this rule
                    if (ruleApproved)
                    {
                        symbolizers.insert(symbolizers.end(), rule.symbolizers.begin(), rule.symbolizers.end());
                        attrsToKeep.insert(rule.attrs.begin(),rule.attrs.end());
                    }
                    styleApproved |= ruleApproved;
                    
                    // We may just match the first rule we find
                    if (ruleApproved && style.filterMode == MapnikConfig::FilterFirst)
                        break;
                }

                approved |= styleApproved;
            }
        }

        if (approved)
        {
            // Copy to the output
            OGRGeometry *geom = feature->GetGeometryRef();
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

            // Need attributges for the various styles
            for (unsigned int si=0;si<symbolizers.size();si++)
            {
                std::string styleIndex = (std::string)"style" + std::to_string(si);
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
            for (unsigned int si=0;si<symbolizers.size();si++)
            {
                std::string styleIndex = (std::string)"style" + std::to_string(si);
                
                newFeature->SetField(styleIndex.c_str(), symbolizers[si]);
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
void ClipInputToBox(LayerMBRs *layer,double llX,double llY,double urX,double urY,OGRSpatialReference *out_srs,std::vector<OGRFeature *> &outFeatures,OGRCoordinateTransformation *tileTransform)
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
    
    for (unsigned int ii=0;ii<layer->features.size();ii++)
    {
        if (mbr.Intersects(layer->mbrs[ii]))
        {
            OGRFeature *inFeature = layer->features[ii];
            int numFields = inFeature->GetFieldCount();
            OGRGeometry *geom = inFeature->GetGeometryRef();
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

// Merge the given features into an existing shapefile or create a new one
bool MergeIntoShapeFile(std::vector<OGRFeature *> &features,OGRLayer *srcLayer,OGRSpatialReference *out_srs,const char *fileName)
{
    // Look for an existing shapefile
    OGRLayer *destLayer = NULL;
    OGRDataSource *poCDS = OGRSFDriverRegistrar::Open( fileName, true );
    if (poCDS)
    {
        // Use the layer in the data file
        destLayer = poCDS->GetLayer(0);
    } else {
        // Create a data file and a layer
        OGRSFDriver *poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile" );
        poCDS = poDriver->CreateDataSource(fileName);
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
void ChopShapefile(const char *layerName,const char *inputFile,std::vector<MapnikConfig::Style> *styles,const char *targetDir,double xmin,double ymin,double xmax,double ymax,int level,OGRSpatialReference *hTrgSRS,OGRSpatialReference *hTileSRS,BuildStats &stats)
{
    // Number of cells at this level
    int numCells = 1<<level;
    double cellSizeX = (xmax-xmin)/numCells;
    double cellSizeY = (ymax-ymin)/numCells;

    // Shapefile output driver
    OGRSFDriver *poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile" );
    OGRSFDriver *memDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("Memory");
    if( poDriver == NULL )
    {
        fprintf(stderr,"Shape driver not available.\n");
        exit(-1);
    }
    if (!memDriver)
    {
        fprintf(stderr,"Memory driver not available.");
        exit(-1);
    }

    OGRDataSource *poDS = OGRSFDriverRegistrar::Open( inputFile, FALSE );
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
        fprintf(stdout,"            Layer: %s, %d features\n",inLayer->GetName(),inLayer->GetFeatureCount());
        
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
        if (!transform)
        {
            fprintf(stderr,"Can't transform from coordinate system to tile for: %s\n",inputFile);
            exit(-1);
        }
        
        OGRDataSource *memDS = memDriver->CreateDataSource("memory");
        OGRLayer *layer = memDS->CreateLayer("layer",this_srs);
        // We'll also apply any rules and attribute changes at this point
        TransformLayer(inLayer,layer,transform,styles,psExtent);
        LayerMBRs layerMBRs(layer);
        
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
                double llX = ix*cellSizeX+xmin;
                double llY = iy*cellSizeY+ymin;
                double urX = (ix+1)*cellSizeX+xmin;
                double urY = (iy+1)*cellSizeY+ymin;
                std::vector<OGRFeature *> clippedFeatures;
                ClipInputToBox(&layerMBRs,llX,llY,urX,urY,hTrgSRS,clippedFeatures,tileTransform);
                
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
                    std::string cellFileName = cellDir + std::to_string(ix) + layerName + ".shp";
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

// This is the map scale for the given level.
// These are sort of made up to match what TileMill is producing
int ScaleForLevel(int level)
{
    int exp = 22-level;
    return (1<<exp) * 300;
}

int main(int argc, char * argv[])
{
    const char *targetDir = NULL;
    char *destSRS = NULL,*tileSRS = NULL;
    char *layerName = NULL;
    bool teSet = false;
    char *xmlConfig = NULL;
    double xmin,ymin,xmax,ymax;
    int minLevel = -1,maxLevel = -1;
    std::vector<const char *> inputFiles;
    
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
        } else if (EQUAL(argv[ii],"-name"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -name\n");
                return -1;
            }
            layerName = argv[ii+1];
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
        } else if (EQUAL(argv[ii],"-level"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -level\n");
                return -1;
            }
            minLevel = maxLevel = atoi(argv[ii+1]);
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
        } else
        {
            inputFiles.push_back(argv[ii]);
        }
    }

    if (!targetDir)
    {
        fprintf(stderr, "Need a target directory.\n");
        return -1;
    }
    
    if (!destSRS)
    {
        fprintf(stderr, "Need a target SRS.\n");
        return -1;
    }
    
    if (!teSet)
    {
        fprintf(stderr, "Need a target bounding box.\n");
        return -1;
    }
    
    if (minLevel < 0 || maxLevel < 0)
    {
        fprintf(stderr, "Need to set target levels\n");
        return -1;
    }
    
    if (!tileSRS)
    {
        fprintf(stderr, "Need a tile SRS\n");
        return -1;
    }
    
    if (inputFiles.empty() && !xmlConfig)
    {
        fprintf(stderr, "Need at least one input file.\n");
        return -1;
    }
    
    if (!inputFiles.empty() && xmlConfig)
    {
        fprintf(stderr, "Expecting XML config or input files, but not both.\n");
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
    
    // Create the output directory
    mkdir(targetDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    for (int level = minLevel;level<=maxLevel;level++)
        mkdir(((std::string)targetDir+"/"+std::to_string(level)).c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // Set up a coordinate transformation
    OGRSpatialReference *hTrgSRS = new OGRSpatialReference ( destSRS );
    OGRSpatialReference *hTileSRS = new OGRSpatialReference ( tileSRS );
    
    // Keep track of what we've built
    BuildStats buildStats;
    
    if (mapnikConfig)
    {
        // Work through the levels
        for (int li=minLevel;li<=maxLevel;li++)
        {
            printf("==== Level %d ====\n",li);
            int scale = ScaleForLevel(li);
            
            // Work through the layers, looking for rules that match this level
            for (unsigned int ii=0;ii<mapnikConfig->layers.size();ii++)
            {
                MapnikConfig::Layer &layer = mapnikConfig->layers[ii];
                
                // We need a list of all the rules that apply, sorted by styles
                std::vector<MapnikConfig::Style> outStyles;
                std::vector<std::string> styleNames;
                for (unsigned int si=0;si<layer.styleNames.size();si++)
                {
                    const MapnikConfig::Style *style = mapnikConfig->findStyle(layer.styleNames[si]);
                    MapnikConfig::Style outStyle = *style;
                    outStyle.rules.clear();
                    if (!style)
                    {
                        fprintf(stderr, "Dangling style name %s in layer %s",layer.styleNames[si].c_str(),layer.name.c_str());
                        return -1;
                    }
                    
                    // Look through the rules that might apply to this level
                    for (unsigned int ri=0;ri<style->rules.size();ri++)
                    {
                        const MapnikConfig::Rule &rule = style->rules[ri];
                        int maxScale = 1e20;
                        if (rule.maxScale > -1)
                            maxScale = rule.maxScale;
                        int minScale = 0;
                        if (rule.minScale > -1)
                            minScale = rule.minScale;

                        // Should be in this level
                        if (minScale < scale && scale < maxScale)
                        {
                            bool approved = true;
                            // However, there are rules that apply at low levels that would be redundant higher up
                            if (rule.minScale == -1)
                            {
                                if (li!=minLevel)
                                {
                                    int lastScale = ScaleForLevel(li-1);
                                    if (minScale < lastScale && lastScale < maxScale)
                                        approved = false;
                                }
                            }
                                
                            if (approved)
                            {
                                outStyle.rules.push_back(rule);
                                styleNames.push_back(style->name);
                            }
                        }
                    }
                    
                    // This style had applicable rules
                    if (!outStyle.rules.empty())
                        outStyles.push_back(outStyle);
                }
                
                // Some of the rules apply here, so let's work through the data files
                if (!outStyles.empty())
                {
                    int numRules = 0;
                    for (unsigned int si=0;si<outStyles.size();si++)
                        numRules += outStyles[si].rules.size();
                    printf(" %d styles, %d rules: %s\n",(int)outStyles.size(),numRules,layer.dataSources[0].c_str());
                    
                    std::string thisLayerName = layer.name;
                    mapnikConfig->symbolizerTable.layerNames.insert(thisLayerName);
                        
                    ChopShapefile(thisLayerName.c_str(), layer.dataSources[0].c_str(), &outStyles, targetDir, xmin, ymin, xmax, ymax, li, hTrgSRS, hTileSRS, buildStats);
                }
            }
        }
        
        // Write the symbolizers out as JSON
        std::string styleJson;
        mapnikConfig->symbolizerTable.minLevel = minLevel;
        mapnikConfig->symbolizerTable.maxLevel = maxLevel;
        if (!mapnikConfig->symbolizerTable.writeJSON(styleJson))
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
    } else {
        // Work through the input files
        for (unsigned int ii=0;ii<inputFiles.size();ii++)
            for (int level=minLevel;level<=maxLevel;level++)
                ChopShapefile(layerName,inputFiles[ii], NULL, targetDir,xmin,ymin,xmax,ymax,level,hTrgSRS,hTileSRS,buildStats);
    }
    
    if (buildStats.numTiles > 0)
        fprintf(stdout,"Feature Count\n  Min = %d, Max = %d, Avg = %f\n",buildStats.minFeat,buildStats.maxFeat,buildStats.featAvg/buildStats.numTiles);
    
    if (mapnikConfig)
        delete mapnikConfig;
    
    return 0;
}

