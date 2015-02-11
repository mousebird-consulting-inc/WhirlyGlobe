//
//  main.cpp
//  elev_tile_pyramid
//
//  Created by Steve Gifford on 9/5/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

#undef DEBUG
#include <iostream>
#include <string>
#include <sstream>
#include <float.h>
#include <math.h>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "cpl_minixml.h"
#include <dirent.h>
#include <vector>
#include "KompexSQLitePrerequisites.h"
#include "KompexSQLiteDatabase.h"
#include "KompexSQLiteStatement.h"
#include "KompexSQLiteException.h"
#include "KompexSQLiteStreamRedirection.h"
#include "KompexSQLiteBlob.h"
#include "KompexSQLiteException.h"
#include "ElevationPyramid.h"

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

GDALDatasetH CreateOutputDataFile(const char *pszFormat,const char *pszFilename,unsigned int pixelsX,unsigned int pixelsY,const char *outFormat)
{
    GDALDatasetH hDstDS;
    GDALDriverH hDriver;
 
    /* -------------------------------------------------------------------- */
    /*      Find the output driver.                                         */
    /* -------------------------------------------------------------------- */
    hDriver = GDALGetDriverByName( pszFormat );
    if( hDriver == NULL
       || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) == NULL )
    {
        int	iDr;
        
        printf( "Output driver `%s' not recognised or does not support\n",
               pszFormat );
        printf( "direct output file creation.  The following format drivers are configured\n"
               "and support direct output:\n" );
        
        for( iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(iDr);
            
            if( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL) != NULL )
            {
                printf( "  %s: %s\n",
                       GDALGetDriverShortName( hDriver  ),
                       GDALGetDriverLongName( hDriver ) );
            }
        }
        printf( "\n" );
        exit( 1 );
    }
    
    // Create a 1 band data file for the given type
    GDALDataType dataType = GDT_Float32;
    if (EQUAL(outFormat, "CFloat32"))
        dataType = GDT_Float32;
    else if (EQUAL(outFormat, "CInt16"))
        dataType = GDT_Int16;
    else {
        fprintf(stderr, "Unsupported data type %s",outFormat);
        return NULL;
    }
    hDstDS = GDALCreate( hDriver, pszFilename, pixelsX, pixelsY,
                        1,
                        GDT_Float32,
                        NULL );
    
    if( hDstDS == NULL )
        return NULL;
    
    return hDstDS;
}

// Maximum number of pixels we'll load at once
static int const MaxPixelLoad = 1048576;

// Look for the maximum pixel in a given area
float searchForMaxPixel(GDALRasterBandH hBand, int sx, int sy, int ex, int ey)
{
    float maxPix = -MAXFLOAT;
    
    int rowSize = ex-sx+1;
//    int colSize = ey-sy+1;
    
    // Number of rows to load at once
    int maxRows = MaxPixelLoad / rowSize;
    maxRows = MAX(1,maxRows);
    
    for (int iy=sy;iy<=ey;iy+=maxRows)
    {
        int numRows = maxRows;
        if (ey-iy<=numRows)
            numRows = ey-iy+1;
        float pixels[rowSize*numRows];
        if (GDALRasterIO( hBand, GF_Read, sx, iy, rowSize, numRows, pixels, rowSize, numRows, GDT_Float32, 0,  0) != CE_None)
        {
            fprintf(stderr,"Query failure in GDALRasterIO");
            return -1;
        }
        
        for (int which = 0; which < rowSize*numRows; which++)
            maxPix = MAX(pixels[which],maxPix);
    }
    
    return maxPix;
}

typedef enum {SampleSingle,SampleMax} SamplingType;

int main(int argc, char * argv[])
{
    const char *inputFile = NULL;
    const char *targetDir = NULL;
    const char *targetDb = NULL,*updateDb=NULL;
    char outFileFormat[100] = "GTiff";
    char outFormat[100] = "CFloat32";
    char *destSRS = NULL;
    unsigned int levels = 0;
    bool teSet = false;
    double xmin,ymin,xmax,ymax;
    int pixelsX = 16, pixelsY = 16;
    bool flipY = false;
    int updateMinLevel = -1, updateMaxLevel = -2;
    double updateMinX = 0.0,updateMaxX = 0.0,updateMinY = 0.0,updateMaxY = 0.0;
    const char *updateShapeFile = NULL,*outShapeFile=NULL;
    SamplingType samplingtype = SampleSingle;

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
                fprintf(stderr,"Expecting one argument for -targetdir");
                return -1;
            }
            targetDir = argv[ii+1];
        } else if (EQUAL(argv[ii],"-t_srs"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -t_srs");
                return -1;
            }
            CPLFree(destSRS);
            destSRS = SanitizeSRS(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-levels"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting one argument for -levels");
                return -1;
            }
            levels = atoi(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-ps"))
        {
            numArgs = 3;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting two arguments for -ps");
                return -1;
            }
            pixelsX = atoi(argv[ii+1]);
            pixelsY = atoi(argv[ii+2]);
        } else if (EQUAL(argv[ii],"-te"))
        {
            numArgs = 5;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting four arguments for -te");
                return -1;
            }
            xmin = atof(argv[ii+1]);
            ymin = atof(argv[ii+2]);
            xmax = atof(argv[ii+3]);
            ymax = atof(argv[ii+4]);
            teSet = true;
        } else if (EQUAL(argv[ii],"-of"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting output format for -of");
                return -1;
            }
            strncpy(outFormat, argv[ii+1],99);
        } else if (EQUAL(argv[ii],"-flipy"))
        {
            numArgs = 1;
            flipY = true;
        } else if (EQUAL(argv[ii],"-targetdb"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting output database name for -targetdb");
                return -1;
            }
            targetDb = argv[ii+1];
        } else if (EQUAL(argv[ii],"-updatedb"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting input database name for -updatedb");
                return -1;
            }
            updateDb = argv[ii+1];
        } else if (EQUAL(argv[ii],"-ue"))
        {
            numArgs = 7;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting min/max level and extents for -ue");
                return -1;
            }
            updateMinLevel = atoi(argv[ii+1]);
            updateMaxLevel = atoi(argv[ii+2]);
            updateMinX = atof(argv[ii+3]);
            updateMinY = atof(argv[ii+4]);
            updateMaxX = atof(argv[ii+5]);
            updateMaxY = atof(argv[ii+6]);
        } else if (EQUAL(argv[ii],"-us"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting file name for shapefile with -us");
                return -1;
            }
            updateShapeFile = argv[ii+1];
        } else if (EQUAL(argv[ii],"-os"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting file name for -os");
                return -1;
            }
            outShapeFile = argv[ii+1];
        } else if (EQUAL(argv[ii],"-sample"))
        {
            numArgs = 2;
            if (ii+numArgs > argc)
            {
                fprintf(stderr,"Expecting type for -sample");
                return -1;
            }
            if (EQUAL(argv[ii+1],"single"))
                samplingtype = SampleSingle;
            else if (EQUAL(argv[ii+1],"max"))
                samplingtype = SampleMax;
            else {
                fprintf(stderr,"Expecting single or max for sampling type.");
                return -1;
            }
        } else
        {
            if (inputFile)
            {
                fprintf(stderr,"Expecting only one input file.");
                return -1;
            } else
                inputFile = argv[ii];
        }
    }
    
    if (!inputFile)
    {
        fprintf(stderr, "Need at least one input file.");
        return -1;
    }
    if (!targetDir && !targetDb && !updateDb)
    {
        fprintf(stderr, "Expecting output dir or output/update DB.");
        return -1;
    }
    if (updateDb && targetDb)
    {
        fprintf(stderr, "Expecting update or target DB, but not both.");
        return -1;
    }
    if (updateDb && teSet)
    {
        fprintf(stderr, "Not expecting both an update database (which has extents) and new extents.");
        return -1;
    }
    if (updateDb)
    {
        if (updateMinLevel > updateMaxLevel)
        {
            fprintf(stderr, "Expecting valid update min and max levels for update mode");
            return -1;
        }
        if (updateMinX >= updateMaxX || updateMinY >= updateMaxY)
        {
            fprintf(stderr,"Expecting valid update bounds for update mode.");
            return -1;
        }
    }
        
    Kompex::SQLiteDatabase *sqliteDb = NULL;
    ElevationPyramid *elevPyr = NULL;

    // Open the database for update (if there is one)
    if (updateDb)
    {
        try {
            sqliteDb = new Kompex::SQLiteDatabase(updateDb, SQLITE_OPEN_READWRITE, 0);
        } catch (Kompex::SQLiteException &exc) {
            fprintf(stderr,"Failed to open database:\n%s\n",exc.GetString().c_str());
        }
        if (!sqliteDb->GetDatabaseHandle())
        {
            fprintf(stderr, "Invalid sqlite database: %s\n",targetDb);
            return -1;
        }
        elevPyr = new ElevationPyramid(sqliteDb,updateMaxLevel);
        if (!elevPyr->isValid())
            return -1;
        elevPyr->getExtents(xmin,ymin,xmax,ymax);
        destSRS = SanitizeSRS(elevPyr->getSrs().c_str());
        elevPyr->getTileSize(pixelsX,pixelsY);
    }

    // Open the input file
    GDALDatasetH hSrcDS = NULL;
    hSrcDS = GDALOpen( inputFile, GA_ReadOnly );
    if( hSrcDS == NULL )
        return -1;
    GDALSetCacheMax64(2*1024*1024*1024);

    // Set up a coordinate transformation
    OGRCoordinateTransformationH hCT = NULL,hCTBack = NULL;
    OGRSpatialReference *hSrcSRS = NULL, *hTrgSRS = NULL;
    if (destSRS)
    {
        hSrcSRS = new OGRSpatialReference ( GDALGetProjectionRef( hSrcDS ) );
        hTrgSRS = new OGRSpatialReference ( destSRS );
        
        hCT = OCTNewCoordinateTransformation(hSrcSRS,hTrgSRS);
        hCTBack = OCTNewCoordinateTransformation(hTrgSRS,hSrcSRS);
    } else {
        // Target SRS is same as input
        hSrcSRS = new OGRSpatialReference ( GDALGetProjectionRef( hSrcDS ) );
        hTrgSRS = new OGRSpatialReference ( GDALGetProjectionRef( hSrcDS ) );
    }

    // Fill in the bounding box (for the output) if we don't already have one
    double adfGeoTransform[6], adfInvGeoTransform[6];
    if (GDALGetGeoTransform( hSrcDS, adfGeoTransform) != CE_None)
    {
        fprintf(stderr, "Unable to get geo transform for data set.");
        return -1;
    }
    GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform );
    int rasterXSize = GDALGetRasterXSize(hSrcDS);
    int rasterYSize = GDALGetRasterYSize(hSrcDS);
    if (!teSet && !elevPyr)
    {
        // Extents in the original coordinate system
        double orgGeoX = adfGeoTransform[0] + adfGeoTransform[1] * 0.0 + adfGeoTransform[2] * 0.0;
        double orgGeoY = adfGeoTransform[3] + adfGeoTransform[4] * 0.0 + adfGeoTransform[5] * 0.0;
        double destGeoX = adfGeoTransform[0] + adfGeoTransform[1] * rasterXSize + adfGeoTransform[2] * rasterYSize;
        double destGeoY = adfGeoTransform[3] + adfGeoTransform[4] * rasterXSize + adfGeoTransform[5] * rasterYSize;
        
        xmin = orgGeoX;  ymin = orgGeoY;
        xmax = destGeoX;  ymax = destGeoY;
        if (hCT)
        {
            OCTTransform(hCT, 1, &xmin, &ymin, NULL);
            OCTTransform(hCT, 1, &xmax, &ymax, NULL);
        }
        teSet = true;
    }
    
    // Look for an appropriate band
    if (GDALGetRasterCount( hSrcDS) != 1)
    {
        fprintf(stderr, "Expecting a 1 band image (elevation data)");
        return -1;
    }
    GDALRasterBandH hBand = GDALGetRasterBand( hSrcDS, 1);
    if (!hBand)
    {
        fprintf(stderr, "Failed to get input band");
        return -1;
    }

    // Shapefile we'll do inside/outside checks in
    std::vector<OGREnvelope> includeBounds;
    if (updateShapeFile)
    {
        OGRDataSource *poDS;
        poDS = OGRSFDriverRegistrar::Open( updateShapeFile, FALSE );
        if( poDS == NULL )
        {
            printf( "Open failed.\n" );
            exit( 1 );
        }
        
        // Work through the layers
        for (unsigned int ii=0;ii<poDS->GetLayerCount();ii++)
        {
            OGRLayer *layer = poDS->GetLayer(ii);
            while (OGRFeature *feat = layer->GetNextFeature())
            {
                OGRGeometry *geom = feat->GetGeometryRef();
                if (geom->transformTo(hTrgSRS) != OGRERR_NONE)
                {
                    fprintf(stderr,"Can't transform shape.");
                    return -1;
                }
                switch (geom->getGeometryType())
                {
                    case wkbPolygon:
                    {
                        OGRPolygon *poly = (OGRPolygon *)geom;
                        OGRLinearRing *outRing = poly->getExteriorRing();
                        if (outRing)
                        {
                            OGREnvelope env;
                            outRing->getEnvelope(&env);
                            includeBounds.push_back(env);
                        }
                        for (unsigned int jj=0;jj<poly->getNumInteriorRings();jj++)
                        {
                            OGRLinearRing *inRing = poly->getInteriorRing(jj);
                            if (inRing)
                            {
                                OGREnvelope env;
                                inRing->getEnvelope(&env);
                                includeBounds.push_back(env);
                            }
                        }
                    }
                        break;
                    case wkbMultiPolygon:
                    case wkbMultiPolygon25D:
                    {
                        OGRMultiPolygon *poly = (OGRMultiPolygon *)geom;
                        for (unsigned int jj=0;jj<poly->getNumGeometries();jj++)
                        {
                            OGRGeometry *thisGeom = poly->getGeometryRef(jj);
                            if (thisGeom->getGeometryType() == wkbPolygon)
                            {
                                OGRPolygon *poly = (OGRPolygon *)geom;
                                OGRLinearRing *outRing = poly->getExteriorRing();
                                if (outRing)
                                {
                                    OGREnvelope env;
                                    outRing->getEnvelope(&env);
                                    includeBounds.push_back(env);
                                }
                                for (unsigned int jj=0;jj<poly->getNumInteriorRings();jj++)
                                {
                                    OGRLinearRing *inRing = poly->getInteriorRing(jj);
                                    if (inRing)
                                    {
                                        OGREnvelope env;
                                        inRing->getEnvelope(&env);
                                        includeBounds.push_back(env);
                                    }
                                }
                            }
                        }
                    }
                        break;
                    default:
                        break;
                }
            }
        }
        
        if (includeBounds.empty())
        {
            fprintf(stderr,"Didn't find any usable areal features in the shape file.");
            return -1;
        } else
            fprintf(stderr,"Found %ld boundaries for comparison.\n",includeBounds.size());
    }
    
    // Output shape file (for tracking what tiles we wrote)
    
    OGRDataSource *outShape = NULL;
    OGRLayer *outShapeLayer = NULL;
    if (outShapeFile)
    {
        const char *pszDriverName = "ESRI Shapefile";
        OGRSFDriver *poDriver;
        poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName );

        if( poDriver == NULL )
        {
            printf( "%s driver not available.\n", pszDriverName );
            return -1;
        }

        outShape = poDriver->CreateDataSource( outShapeFile, NULL );
        if( outShape == NULL )
        {
            printf( "Creation of output file failed.\n" );
            return -1;
        }
        
        outShapeLayer = outShape->CreateLayer("boxes", hTrgSRS);
        OGRFieldDefn cellIdent("cell",OFTString);
        outShapeLayer->CreateField(&cellIdent);
        OGRFieldDefn minIndent("min",OFTReal);
        outShapeLayer->CreateField(&minIndent);
        OGRFieldDefn maxIndent("max",OFTReal);
        outShapeLayer->CreateField(&maxIndent);
    }

    if (targetDir)
        mkdir(targetDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    // Create a new database.  Blow away the old one if it's there
    if (targetDb)
    {
        if (!destSRS)
        {
            fprintf(stderr, "Need a -t_srs when creating a new elevation database.");
            return -1;
        }

        remove(targetDb);
        sqliteDb = new Kompex::SQLiteDatabase(targetDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
        if (!sqliteDb->GetDatabaseHandle())
        {
            fprintf(stderr, "Invalid sqlite database: %s\n",targetDb);
            return -1;
        }
        elevPyr = new ElevationPyramid(sqliteDb,destSRS,GDT_Int16,xmin,ymin,xmax,ymax,pixelsX,pixelsY,true,0,levels-1);
        if (!elevPyr->isValid())
            return -1;
    }
    
    // Text version of SRS so we can write it
    char *trgSrsWKT = NULL;
    OSRExportToWkt( hTrgSRS, &trgSrsWKT );

    if (targetDb)
    {
        Kompex::SQLiteStatement transactStmt(sqliteDb);
        transactStmt.SqlStatement((std::string)"BEGIN TRANSACTION");
    }

    // We might only be updating some of the levels
    int min_level = 0, max_level = levels-1;
    if (updateMaxLevel >= updateMinLevel)
    {
        min_level = updateMinLevel;
        max_level = updateMaxLevel;
    }

    // Work through the levels of detail, starting from the top
    int totalTiles = 0,zeroTiles = 0, skippedTiles = 0;
    for (int level=max_level;level>=min_level;level--)
    {
        printf("Level %d: ",level);
        fflush(stdout);
        GDALTermProgress(0.0,NULL,NULL);

        std::stringstream levelDir;
        if (targetDir)
        {
            levelDir << targetDir << "/" << level;
            mkdir(levelDir.str().c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        
        int numChunks = 1<<level;
        double sizeY = (ymax-ymin)/numChunks;
        double cellY = sizeY/(pixelsY-1);
        double sizeX = (xmax-xmin)/numChunks;
        double cellX = sizeX/(pixelsX-1);

        // We might only be updating some of the area
        int min_ix = 0, min_iy = 0, max_ix = numChunks-1, max_iy=numChunks-1;        
        if (updateMinX != updateMaxX)
        {
            min_ix = floor((updateMinX-xmin)/sizeX);
            min_iy = floor((updateMinY-ymin)/sizeY);
            max_ix = ceil((updateMaxX-xmin)/sizeX);
            max_iy = ceil((updateMaxY-ymin)/sizeY);
        }
        int numTilesToDo = (max_ix-min_ix+1)*(max_iy-min_iy+1);
        int chunksProcessed = 0;

        // Now through the individual files
        for (unsigned int ix=min_ix;ix<=max_ix;ix++)
        {
            std::stringstream xDir;
            if (targetDir)
            {
                xDir << levelDir.str() << "/" << ix;
                mkdir(xDir.str().c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            }
            
            for (unsigned int iy=min_iy;iy<=max_iy;iy++)
            {
                // Extents for this particular tile
                double tileMinX = xmin+ix*sizeX;
                double tileMinY = ymin+iy*sizeY;
                double tileMaxX = tileMinX + pixelsX * cellX;
                double tileMaxY = tileMinY + pixelsY * cellY;
               
                // Let's check this against the inclusion polygons if we have them
                bool includeTile = true;
                if (!includeBounds.empty())
                {
                    includeTile = false;
                    OGREnvelope box;
                    box.MinX = tileMinX;
                    box.MinY = tileMinY;
                    box.MaxX = tileMaxX;
                    box.MaxY = tileMaxY;
                    for (unsigned si=0;si<includeBounds.size();si++)
                    {
                        OGREnvelope &env = includeBounds[si];
                        if (env.Contains(box) || env.Intersects(box) || box.Contains(env))
                        {
                            includeTile = true;
                            break;
                        }
                    }
                }
                
                if (includeTile)
                {
                    GDALDatasetH hDestDS = NULL;
                    GDALRasterBandH hBandOut = NULL;
                    if (targetDir)
                    {
                        std::stringstream fileName;
                        int outY = (flipY ? (numChunks-1-iy) : iy);
                        fileName << xDir.str() << "/" << outY << ".tif";
                    
                        // Create the output file
                        hDestDS = CreateOutputDataFile(outFileFormat,fileName.str().c_str(),pixelsX,pixelsY,outFormat);
                        if (!hDestDS)
                            return -1;
                        hBandOut = GDALGetRasterBand(hDestDS, 1);
                        if (!hBandOut)
                        {
                            fprintf(stderr,"Failed to create output band.");
                            return -1;
                        }
                    }
                    
                    float tileData[pixelsX*pixelsY];
                    
                    // Run through and query the various cells
                    for (unsigned int cy=0;cy<pixelsY;cy++)
                        for (unsigned int cx=0;cx<pixelsX;cx++)
                        {
                            double thisX = tileMinX + cellX*cx;
                            double thisY = tileMinY + cellY*cy;
                            
                            // Project back to the original data file
                            if (hCT)
                                OCTTransform(hCTBack, 1, &thisX, &thisY, NULL);
                            
                            if (samplingtype == SampleSingle)
                            {
                                // Figure out which pixel this is
                                double pixX = adfInvGeoTransform[0] + adfInvGeoTransform[1] * thisX + adfInvGeoTransform[2] * thisY;
                                double pixY = adfInvGeoTransform[3] + adfInvGeoTransform[4] * thisX + adfInvGeoTransform[5] * thisY;
                                
                                // Note: Should do some interpolation
                                int pixXint = (int)pixX,pixYint = (int)pixY;
                                
                                double ta = pixX-pixXint;
                                double tb = pixY-pixYint;

                                // Look up the four nearby pixels
                                int pixXlookup[4],pixYlookup[4];
                                pixXlookup[0] = pixXint;  pixYlookup[0] = pixYint;
                                pixXlookup[1] = pixXint+1;  pixYlookup[1] = pixYint;
                                pixXlookup[2] = pixXint+1;  pixYlookup[2] = pixYint+1;
                                pixXlookup[3] = pixXint;  pixYlookup[3] = pixYint+1;
                                float pixVals[4];
                                for (unsigned int pi=0;pi<4;pi++)
                                {
                                    int pixXlook = pixXlookup[pi];
                                    int pixYlook = pixYlookup[pi];
                                    if (pixXlook < 0) pixXlook = 0;
                                    if (pixYlook < 0) pixYlook = 0;
                                    if (pixXlook >= rasterXSize)  pixXlook = rasterXSize-1;
                                    if (pixYlook >= rasterYSize)  pixYlook = rasterYSize-1;
                                
                                    // Fetch the pixel
                                    if (GDALRasterIO( hBand, GF_Read, pixXlook, pixYlook, 1, 1, &pixVals[pi], 1, 1, GDT_Float32, 0,  0) != CE_None)
                                    {
                                        fprintf(stderr,"Query failure in GDALRasterIO");
                                        return -1;
                                    }
                                }
                                
                                // Now do a bilinear interpolation
                                float pixA = (pixVals[1]-pixVals[0])*ta + pixVals[0];
                                float pixB = (pixVals[2]-pixVals[3])*ta + pixVals[3];
                                float pixVal = (pixB-pixA)*tb+pixA;
                                
                                tileData[cy*pixelsX+cx] = pixVal;
                            } else if (samplingtype == SampleMax)
                            {
                                // Make a bounding box and project it into the source data
                                double pixX[4],pixY[4];
                                double srcX[4],srcY[4];
                                srcX[0] = thisX-cellX/2.0;  srcY[0] = thisY-cellY/2.0;
                                srcX[1] = thisX+cellX/2.0;  srcY[1] = thisY-cellY/2.0;
                                srcX[2] = thisX+cellX/2.0;  srcY[2] = thisY+cellY/2.0;
                                srcX[3] = thisX-cellX/2.0;  srcY[3] = thisY+cellY/2.0;

                                int sx=1000000,sy=1000000,ex=-1000000,ey=-1000000;
                                for (unsigned int pi=0;pi<4;pi++)
                                {
                                    pixX[pi] = adfInvGeoTransform[0] + adfInvGeoTransform[1] * srcX[pi] + adfInvGeoTransform[2] * srcY[pi];
                                    pixY[pi] = adfInvGeoTransform[3] + adfInvGeoTransform[4] * srcX[pi] + adfInvGeoTransform[5] * srcY[pi];
                                    sx = MIN(sx,floor(pixX[pi]));
                                    sy = MIN(sy,floor(pixY[pi]));
                                    ex = MAX(ex,ceil(pixX[pi]));
                                    ey = MAX(ey,ceil(pixY[pi]));
                                }
                                sx = MAX(0,sx);
                                sy = MAX(0,sy);
                                sx = MIN(sx,rasterXSize-1);
                                sy = MIN(sy,rasterYSize-1);
                                ex = MAX(0,ex);
                                ey = MAX(0,ey);
                                ex = MIN(ex,rasterXSize-1);
                                ey = MIN(ey,rasterYSize-1);
                                
                                // Work through the pixels in the source looking for a max
                                float maxPix = -MAXFLOAT;

                                // Search for the maximum pixel in the area
                                maxPix = searchForMaxPixel(hBand, sx, sy, ex, ey);
                                
                                tileData[cy*pixelsX+cx] = maxPix;
                            }
                        }
                    
                    // Output directory
                    if (targetDir)
                    {
                        // Write all the data at once
                        if (GDALRasterIO( hBandOut, GF_Write, 0, 0, pixelsX, pixelsY, tileData, pixelsX, pixelsX, GDT_Int16, 0, 0) != CE_None)
                        {
                            fprintf(stderr,"Failed to write output data");
                            return -1;
                        }

                        // Set projection and extents
                        GDALSetProjection(hDestDS, trgSrsWKT);
                        
                        double adfOutTransform[6];
                        adfOutTransform[0] = tileMinX;
                        adfOutTransform[1] = (tileMaxX-tileMinX)/pixelsX;
                        adfOutTransform[2] = 0;
                        adfOutTransform[3] = tileMinY;
                        adfOutTransform[4] = 0;
                        adfOutTransform[5] = (tileMaxY-tileMinY)/pixelsY;
                        GDALSetGeoTransform(hDestDS, adfOutTransform);
                        
                        // Close the output file
                        GDALClose(hDestDS);
                    }
                    
                    // Output pyramid sqlite db
                    if (elevPyr)
                    {
                        // See if anything of is non-zero
                        bool nonZero = false;
                        for (unsigned int ip=0;ip<pixelsX*pixelsY;ip++)
                            if (tileData[ip] != 0)
                            {
                                nonZero = true;
                                break;
                            }
                        
                        totalTiles++;
                        if (nonZero)
                        {
                            // Need int16 data
                            short tileDataShort[pixelsX*pixelsY];
                            for (unsigned int ii=0;ii<pixelsX*pixelsY;ii++)
                                tileDataShort[ii] = tileData[ii];

                            if (!elevPyr->addElevationTile(tileDataShort, ix, iy, level))
                            {
                                fprintf(stderr, "Failed to write tile %d: %d, %d",level,ix,iy);
                                return -1;
                            }
                        } else {
                            if (!elevPyr->addElevationTile(NULL, ix, iy, level))
                            {
                                fprintf(stderr, "Failed to write empty tile %d: %d, %d",level,ix,iy);
                                return -1;
                            }
                            zeroTiles++;
                        }                    
                    }
                    
                    // Update the shape file for what we're... updating
                    if (outShapeLayer)
                    {
                        float minElev=MAXFLOAT,maxElev=-MAXFLOAT;
                        for (unsigned int it=0;it<pixelsX*pixelsY;it++)
                        {
                            minElev = MIN(minElev,tileData[it]);
                            maxElev = MAX(maxElev,tileData[it]);
                        }
                        OGRPolygon *poly = new OGRPolygon();
                        OGRLinearRing ring;
                        ring.addPoint(tileMinX, tileMinY);
                        ring.addPoint(tileMaxX, tileMinY);
                        ring.addPoint(tileMaxX, tileMaxY);
                        ring.addPoint(tileMinX, tileMaxY);
                        ring.addPoint(tileMinX, tileMinY);
                        poly->addRing(&ring);
                        OGRFeature *feat = new OGRFeature(outShapeLayer->GetLayerDefn());
                        feat->SetGeometry(poly);
                        char cellName[1024];
                        sprintf(cellName,"cell: %d: (%d,%d)",level,ix,iy);
                        feat->SetField("cell",cellName);
                        feat->SetField("min", minElev);
                        feat->SetField("max", maxElev);
                        outShapeLayer->CreateFeature(feat);
                        OGRFeature::DestroyFeature( feat );
                    }
                    
                } else
                    skippedTiles++;
                
                chunksProcessed++;
                double done = chunksProcessed/((double)numTilesToDo);
                GDALTermProgress(done,NULL,NULL);
            }
        }
        
        GDALTermProgress(1.0,NULL,NULL);
    }
    
    if (outShape)
        OGRDataSource::DestroyDataSource( outShape );
        
    printf("Flushing database...");  fflush(stdout);
    // Flush out one big one
    if (targetDb)
    {
        Kompex::SQLiteStatement transactStmt(sqliteDb);
        transactStmt.SqlStatement((std::string)"END TRANSACTION");
    }

    if (elevPyr)
    {
        elevPyr->flush();
        elevPyr->createIndex();
    }
    printf("done\n");
    
    if (sqliteDb)
    {
        try {
            sqliteDb->Close();
        }
        catch (Kompex::SQLiteException &except)
        {
            fprintf(stderr,"Failed to write blob to database:\n%s\n",except.GetString().c_str());
            return false;
        }
    }
    
    fprintf(stdout,"Wrote %d tiles, of which %d were empty and %d were skipped.\n",totalTiles,zeroTiles,skippedTiles);

    return 0;
}

