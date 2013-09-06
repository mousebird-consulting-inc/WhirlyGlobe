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
        } else {
            if (sqliteFile)
            {
                fprintf(stderr,"Only expecting one sqlite output file");
                return -1;
            }
            sqliteFile = argv[ii+1];
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

    // Create a new database.  Blow away the old one if it's there
    const char *dbName = argv[6];
    remove(dbName);
    Kompex::SQLiteDatabase sqliteDb(dbName, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (!sqliteDb.GetDatabaseHandle())
    {
        fprintf(stderr, "Invalid sqlite database: %s\n",dbName);
        return -1;
    }
    
    
    
    return 0;
}

