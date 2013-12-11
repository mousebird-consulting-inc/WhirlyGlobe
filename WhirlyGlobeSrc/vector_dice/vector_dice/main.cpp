//
//  main.cpp
//  vector_dice
//
//  Created by Steve Gifford on 12/6/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

#undef DEBUG
#include <iostream>
#include <string>
#include <sstream>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "cpl_minixml.h"
#include <dirent.h>
#include <vector>

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
void TransformLayer(OGRLayer *inLayer,OGRLayer *outLayer,OGRCoordinateTransformation *transform,OGREnvelope &mbr)
{
    OGREnvelope blank;
    mbr = blank;
    
    for (unsigned int ii=0;ii<inLayer->GetFeatureCount();ii++)
    {
        OGRFeature *feature = inLayer->GetFeature(ii);
//        OGRFeature *feature = inFeature->Clone();
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
        int numFields = feature->GetFieldCount();

//        OGRFeature *feature = OGRFeature::CreateFeature(inLayer->GetLayerDefn());
//        feature->SetGeometry(geom);
        outLayer->CreateFeature(feature);
//        OGRFeature::DestroyFeature(inFeature);
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

int main(int argc, char * argv[])
{
    const char *targetDir = NULL;
    char *destSRS = NULL,*tileSRS = NULL;
    char *layerName = "";
    bool teSet = false;
    double xmin,ymin,xmax,ymax;
    int level = -1;
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
            level = atoi(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-tile_srs"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument -tile_srs\n");
                return -1;
            }
            tileSRS = SanitizeSRS(argv[ii+1]);
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
    
    if (level < 0)
    {
        fprintf(stderr, "Need a target level\n");
        return -1;
    }
    
    if (!tileSRS)
    {
        fprintf(stderr, "Need a tile SRS\n");
        return -1;
    }
    
    if (inputFiles.empty())
    {
        fprintf(stderr, "Need at least one input file.\n");
        return -1;
    }
    
    // Create the output directory
    mkdir(targetDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(((std::string)targetDir+"/"+std::to_string(level)).c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // Set up a coordinate transformation
    OGRSpatialReference *hTrgSRS = new OGRSpatialReference ( destSRS );
    OGRSpatialReference *hTileSRS = new OGRSpatialReference ( tileSRS );
    
    // Number of cells at this level
    int numCells = 1<<level;
    double cellSizeX = (xmax-xmin)/numCells;
    double cellSizeY = (ymax-ymin)/numCells;
    
    // Transform the tiles into familiar
    
    // Shapefile output driver
    OGRSFDriver *poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile" );
    OGRSFDriver *memDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("Memory");
    
    if( poDriver == NULL )
    {
        fprintf(stderr,"Shape driver not available.\n");
        return -1;
    }
    if (!memDriver)
    {
        fprintf(stderr,"Memory driver not available.");
        return -1;
    }
    
    int minFeat = 1e10,maxFeat = 0;
    double featAvg = 0.0;
    int numTiles = 0;

    // Work through the input files
    for (unsigned int ii=0;ii<inputFiles.size();ii++)
    {
        const char *inputFile = inputFiles[ii];
        OGRDataSource *poDS = OGRSFDriverRegistrar::Open( inputFile, FALSE );
        if( poDS == NULL )
        {
            fprintf(stderr, "Couldn't open file: %s\n",inputFile);
            exit( 1 );
        }

        fprintf(stdout,"Input File: %s (%d of %ld)\n",inputFile,ii,inputFiles.size());
        fprintf(stdout,"            %d layers\n",poDS->GetLayerCount());

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
                return -1;
            }
            OGRCoordinateTransformation *tileTransform = OGRCreateCoordinateTransformation(hTrgSRS,hTileSRS);
            if (!transform)
            {
                fprintf(stderr,"Can't transform from coordinate system to tile for: %s\n",inputFile);
                return -1;
            }
            
            OGRDataSource *memDS = memDriver->CreateDataSource("memory");
            OGRLayer *layer = memDS->CreateLayer("layer",this_srs);
            TransformLayer(inLayer,layer,transform,psExtent);
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
                    minFeat = std::min(minFeat,numFeat);
                    maxFeat = std::max(maxFeat,numFeat);
                    numTiles++;
                    featAvg += numFeat;
                    if (numFeat > 0)
                    {
                        std::string cellDir = (std::string)targetDir + "/" + std::to_string(level) + "/" + std::to_string(iy) + "/";
                        mkdir(cellDir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        std::string cellFileName = cellDir + std::to_string(ix) + layerName + ".shp";
                        if (!MergeIntoShapeFile(clippedFeatures,inLayer,hTrgSRS,cellFileName.c_str()))
                            return -1;
                    }
                    
                    for (unsigned int ii=0;ii<clippedFeatures.size();ii++)
                        OGRFeature::DestroyFeature(clippedFeatures[ii]);
                }
            }

            OGRDataSource::DestroyDataSource(memDS);
        }

        delete poDS;
    }
    
    if (numTiles > 0)
        fprintf(stdout,"Feature Count\n  Min = %d, Max = %d, Avg = %f\n",minFeat,maxFeat,featAvg/numTiles);
    
    return 0;
}

