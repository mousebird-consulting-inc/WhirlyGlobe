//
//  main.cpp
//  elev_tile_pyramid
//
//  Created by Steve Gifford on 9/5/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

#include <iostream>
#include <string>
#include <sstream>
#include "gdal.h"
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
    OGRSpatialReferenceH hSRS;
    char *pszResult = NULL;
    
    CPLErrorReset();
    
    hSRS = OSRNewSpatialReference( NULL );
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

int main(int argc, char * argv[])
{
    const char *inputFile = NULL;
    const char *targetDir = NULL;
    const char *targetDb = NULL;
    char outFileFormat[100] = "GTiff";
    char outFormat[100] = "CFloat32";
    char *destSRS = NULL;
    unsigned int levels = 0;
    bool teSet = false;
    double xmin,ymin,xmax,ymax;
    int pixelsX = 16, pixelsY = 16;
    bool flipY = false;
    GDALProgressFunc pfnProgress = GDALTermProgress;

    GDALAllRegister();
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
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting one argument for -targetdir");
                return -1;
            }
            targetDir = argv[ii+1];
        } else if (EQUAL(argv[ii],"-t_srs"))
        {
            numArgs = 2;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting one argument for -t_srs");
                return -1;
            }
            CPLFree(destSRS);
            destSRS = SanitizeSRS(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-levels"))
        {
            numArgs = 2;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting one argument for -levels");
                return -1;
            }
            levels = atoi(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-ps"))
        {
            numArgs = 3;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting two arguments for -ps");
                return -1;
            }
            pixelsX = atoi(argv[ii+1]);
            pixelsY = atoi(argv[ii+2]);
        } else if (EQUAL(argv[ii],"-te"))
        {
            numArgs = 5;
            if (ii+numArgs >= argc)
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
            if (ii+numArgs >= argc)
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
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting output database name for -targetdb");
                return -1;
            }
            targetDb = argv[ii+1];
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
    if (!targetDir && !targetDb)
    {
        fprintf(stderr, "Expecting output dir or output DB.");
        return -1;
    }
    
    // Open the input file
    GDALDatasetH hSrcDS = NULL;
    hSrcDS = GDALOpen( inputFile, GA_ReadOnly );
    if( hSrcDS == NULL )
        return -1;
    GDALSetCacheMax64(2*1024*1024*1024);

    // Set up a coordinate transformation
    OGRCoordinateTransformationH hCT = NULL,hCTBack = NULL;
    OGRSpatialReferenceH hSrcSRS = NULL, hTrgSRS = NULL;
    if (destSRS)
    {
        hSrcSRS = OSRNewSpatialReference( GDALGetProjectionRef( hSrcDS ) );
        hTrgSRS = OSRNewSpatialReference( destSRS );
        
        hCT = OCTNewCoordinateTransformation(hSrcSRS,hTrgSRS);
        hCTBack = OCTNewCoordinateTransformation(hTrgSRS,hSrcSRS);
    } else {
        // Target SRS is same as input
        hSrcSRS = OSRNewSpatialReference( GDALGetProjectionRef( hSrcDS ) );
        hTrgSRS = OSRNewSpatialReference( GDALGetProjectionRef( hSrcDS ) );
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
    if (!teSet)
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

    if (targetDir)
        mkdir(targetDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    Kompex::SQLiteDatabase *sqliteDb = NULL;
    ElevationPyramid *elevPyr = NULL;
    if (targetDb)
    {
        // Create a new database.  Blow away the old one if it's there
        remove(targetDb);
        sqliteDb = new Kompex::SQLiteDatabase(targetDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
        if (!sqliteDb->GetDatabaseHandle())
        {
            fprintf(stderr, "Invalid sqlite database: %s\n",targetDb);
            return -1;
        }
        elevPyr = new ElevationPyramid(sqliteDb,destSRS,GDT_Int16,xmin,ymin,xmax,ymax,pixelsX,pixelsY,true,0,levels-1);
    }
    
    // Text version of SRS so we can write it
    char *trgSrsWKT = NULL;
    OSRExportToWkt( hTrgSRS, &trgSrsWKT );

    if (targetDb)
    {
        Kompex::SQLiteStatement transactStmt(sqliteDb);
        transactStmt.SqlStatement((std::string)"BEGIN TRANSACTION");
    }

    int totalTiles = 0,zeroTiles = 0;
    // Work through the levels of detail, starting from the top
    for (unsigned int level=0;level<levels;level++)
    {
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

        // Now through the individual files
        for (unsigned int ix=0;ix<numChunks;ix++)
        {
            std::stringstream xDir;
            if (targetDir)
            {
                xDir << levelDir.str() << "/" << ix;
                mkdir(xDir.str().c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            }
            
            for (unsigned int iy=0;iy<numChunks;iy++)
            {
                // Extents for this particular tile
                double tileMinX = xmin+ix*sizeX;
                double tileMinY = ymin+iy*sizeY;
                
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

                        // Figure out which pixel this is
                        double pixX = adfInvGeoTransform[0] + adfInvGeoTransform[1] * thisX + adfInvGeoTransform[2] * thisY;
                        double pixY = adfInvGeoTransform[3] + adfInvGeoTransform[4] * thisX + adfInvGeoTransform[5] * thisY;

                        // Note: Should do some interpolation
                        int pixXint = (int)pixX,pixYint = (int)pixY;
                        if (pixXint < 0) pixXint = 0;
                        if (pixYint < 0) pixYint = 0;
                        if (pixXint >= rasterXSize)  pixXint = rasterXSize-1;
                        if (pixYint >= rasterYSize)  pixYint = rasterYSize-1;
                        
                        // Fetch the pixel
                        float pixVal;
                        short pixValShort;
                        if (GDALRasterIO( hBand, GF_Read, pixXint, pixYint, 1, 1, &pixValShort, 1, 1, GDT_Int16, 0,  0) != CE_None)
                        {
                            fprintf(stderr,"Query failure in GDALRasterIO");
                            return -1;
                        }
                        pixVal = pixValShort;
                        tileData[cy*pixelsX+cx] = pixVal;
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
                    double tileMaxX = tileMinX + pixelsX * cellX;
                    double tileMaxY = tileMinY + pixelsY * cellY;
                    
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
            }
        }
    }
    
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
    
    fprintf(stdout,"Wrote %d tiles, of which %d were empty.\n",totalTiles,zeroTiles);

    return 0;
}

