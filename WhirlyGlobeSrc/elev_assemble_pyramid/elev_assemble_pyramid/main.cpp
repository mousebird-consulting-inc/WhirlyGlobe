//
//  main.cpp
//  elev_assemble_pyramid
//
//  Created by Steve Gifford on 9/6/13.
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

int main(int argc, char * argv[])
{
    const char *pyramidDir = NULL;
    const char *sqliteFile = NULL;
    char *srcSRS = NULL;
    unsigned int levels = 0;
    double minx=0.0,miny=0.0,maxx=0.0,maxy=0.0;
    
    GDALAllRegister();
    argc = GDALGeneralCmdLineProcessor( argc, &argv, 0 );
    if( argc < 1 )
        exit( -argc );
    int numArgs = 1;

    for (unsigned int ii=1;ii<argc;ii+=numArgs)
    {
        numArgs = 1;
        
        if (EQUAL(argv[ii],"-sourcedir"))
        {
            numArgs = 2;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting an argument for -sourcedir");
                return -1;
            }
            pyramidDir = argv[ii+1];
        } else if (EQUAL(argv[ii],"-s_srs"))
        {
            numArgs = 2;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting an argument for -s_srs");
                return -1;
            }
            CPLFree(srcSRS);
            srcSRS = SanitizeSRS(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-levels"))
        {
            numArgs = 2;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting an argument for -levels");
                return -1;
            }
            levels = atoi(argv[ii+1]);
        } else if (EQUAL(argv[ii],"-se"))
        {
            numArgs = 5;
            if (ii+numArgs >= argc)
            {
                fprintf(stderr,"Expecting four arguments for -se");
                return -1;
            }
            minx = atof(argv[ii+1]);
            miny = atof(argv[ii+2]);
            maxx = atof(argv[ii+3]);
            maxy = atof(argv[ii+4]);
        } else {
            if (sqliteFile)
            {
                fprintf(stderr,"Only expecting one sqlite output file");
                return -1;
            }
            sqliteFile = argv[ii];
            numArgs = 1;
        }
    }
    
    if (levels <= 0)
    {
        fprintf(stderr,"Expecting at least one level");
        return -1;
    }
    if (!pyramidDir)
    {
        fprintf(stderr,"Need a source directory for the pyramid");
        return -1;
    }
    if (!sqliteFile)
    {
        fprintf(stderr,"Need a sqlite output file.");
        return -1;
    }
    if (minx == maxx || miny == maxy)
    {
        fprintf(stderr,"Invalid extents");
        return -1;
    }
    
    // Open up the top file to get the tile size
    char topFileName[1024];
    sprintf(topFileName,"%s/0/0/0.tif",pyramidDir);
    GDALDatasetH hTopDS = GDALOpen( topFileName, GA_ReadOnly );
    if( hTopDS == NULL )
    {
        fprintf(stderr,"Couldn't find top level file.");
        return -1;
    }
    int tileSizeX = GDALGetRasterXSize(hTopDS);
    int tileSizeY = GDALGetRasterYSize(hTopDS);
    GDALClose(hTopDS);  hTopDS = NULL;

    // Create a new database.  Blow away the old one if it's there
    remove(sqliteFile);
    Kompex::SQLiteDatabase sqliteDb(sqliteFile, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (!sqliteDb.GetDatabaseHandle())
    {
        fprintf(stderr, "Invalid sqlite database: %s\n",sqliteFile);
        return -1;
    }
    ElevationPyramid elevPyr(&sqliteDb,srcSRS,GDT_Int16,minx,miny,maxx,maxy,tileSizeX,tileSizeY,true,0,levels-1);
    
    int totalTiles = 0,zeroTiles = 0;
    
    // Work through the levels
    unsigned short tileData[tileSizeX*tileSizeY];
    memset(tileData, 0, tileSizeX*tileSizeY*sizeof(unsigned short));
    for (unsigned int level=0;level<levels;level++)
    {
        int numChunks = 1<<level;
        
        for (unsigned int iy=0;iy<numChunks;iy++)
            for (unsigned int ix=0;ix<numChunks;ix++)
            {
                char fileName[1024];
                sprintf(fileName,"%s/%d/%d/%d.tif",pyramidDir,level,ix,iy);
                GDALDatasetH tileDS = GDALOpen( fileName, GA_ReadOnly );
                int thisTileSizeX = GDALGetRasterXSize(tileDS);
                int thisTileSizeY = GDALGetRasterYSize(tileDS);
                if (thisTileSizeX != tileSizeX || thisTileSizeY != tileSizeY)
                {
                    fprintf(stderr,"Tile %d: %d,%d does not match the standard tile size",level,ix,iy);
                    return -1;
                }
                
                GDALRasterBandH hBand = GDALGetRasterBand( tileDS, 1);
                if (!hBand)
                {
                    fprintf(stderr, "Failed to get input band");
                    return -1;
                }
                
                GDALRasterIO(hBand, GF_Read, 0, 0, tileSizeX, tileSizeY, &tileData[0], tileSizeX, tileSizeY, GDT_Int16, 0, 0);
                GDALClose(tileDS);
                
                // See if anything of is non-zero
                bool nonZero = false;
                for (unsigned int ip=0;ip<tileSizeX*tileSizeY;ip++)
                    if (tileData[ip] != 0)
                    {
                        nonZero = true;
                        break;
                    }
                totalTiles++;
                if (nonZero)
                {
                    if (!elevPyr.addElevationTile(tileData, ix, iy, level))
                    {
                        fprintf(stderr, "Failed to write tile %d: %d, %d",level,ix,iy);
                        return -1;
                    }
                } else {
                    if (!elevPyr.addElevationTile(NULL, ix, iy, level))
                    {
                        fprintf(stderr, "Failed to write empty tile %d: %d, %d",level,ix,iy);
                        return -1;
                    }
                    zeroTiles++;
                }
            }
    }
    
    elevPyr.createIndex();
    
    sqliteDb.Close();
    
    fprintf(stdout,"Wrote %d tiles, of which %d were empty.\n",totalTiles,zeroTiles);
    
    return 0;
}

