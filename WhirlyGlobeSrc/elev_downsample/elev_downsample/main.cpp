//
//  main.cpp
//  elev_downsample
//
//  Created by Steve Gifford on 1/21/15.
//  Copyright (c) 2015 mousebird consulting. All rights reserved.
//

#undef DEBUG
#include <iostream>
#include <string>
#include <sstream>
#include "gdal_priv.h"
#include <dirent.h>

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

int main(int argc, const char * argv[])
{
    const char *inputFile = NULL;
    int numDiv;
    const char *outputFile = NULL;
    char outFileFormat[100] = "GTiff";
    char outFormat[100] = "CFloat32";

    if (argc < 4)
    {
        fprintf(stderr, "%s: in.file out.file <divisor>",argv[0]);
        return -1;
    }
    
    inputFile = argv[1];
    outputFile = argv[2];
    numDiv = atoi(argv[3]);
    
    if (numDiv <= 0)
    {
        fprintf(stderr,"Invalid divisor");
        return -1;
    }
    
    GDALAllRegister();

    // Open the input file
    GDALDatasetH hSrcDS = NULL;
    hSrcDS = GDALOpen( inputFile, GA_ReadOnly );
    if( hSrcDS == NULL )
        return -1;

    int rasterXSize = GDALGetRasterXSize(hSrcDS);
    int rasterYSize = GDALGetRasterYSize(hSrcDS);
    
    if (rasterXSize % numDiv != 0 || rasterYSize % numDiv)
    {
        fprintf(stderr,"%d does not divide (%d,%d) neatly",numDiv,rasterXSize,rasterYSize);
        return -1;
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

    // Output file size
    int outXSize = rasterXSize / numDiv;
    int outYSize = rasterYSize / numDiv;

    // Create the output file
    GDALDatasetH hDestDS = NULL;
    GDALRasterBandH hBandOut = NULL;
    hDestDS = CreateOutputDataFile(outFileFormat,outputFile,outXSize,outYSize,outFormat);
    if (!hDestDS)
        return -1;
    hBandOut = GDALGetRasterBand(hDestDS, 1);
    if (!hBandOut)
    {
        fprintf(stderr,"Failed to create output band.");
        return -1;
    }
    
    float outData[outXSize*outYSize];
    float inData[numDiv*numDiv];
    
    // Work through the output pixels
    for (int ix=0;ix<outXSize;ix++)
        for (int iy=0;iy<outYSize;iy++)
        {
            // Work through the numDiv x numDiv sample
            if (GDALRasterIO( hBand, GF_Read, ix*numDiv, iy*numDiv, numDiv, numDiv, inData, numDiv, numDiv, GDT_Float32, 0,  0) != CE_None)
            {
                fprintf(stderr,"Query failure in GDALRasterIO");
                return -1;
            }

            double maxPix = -1e10;
            for (int sample=0;sample<numDiv*numDiv;sample++)
            {
                float &pixVal = inData[sample];
                if (pixVal > maxPix)
                    maxPix = pixVal;
            }
            
            outData[iy*outXSize+ix] = maxPix;
        }
    
    // Write the output data all at once
    // Write all the data at once
    if (GDALRasterIO( hBandOut, GF_Write, 0, 0, outXSize, outYSize, outData, outXSize, outYSize, GDT_Float32, 0, 0) != CE_None)
    {
        fprintf(stderr,"Failed to write output data");
        return -1;
    }
    
    // Set projection and extents
    GDALSetProjection(hDestDS, GDALGetProjectionRef(hSrcDS));
    double adfInTransform[6];
    // Note: This is not quite right.  Need to adjust slightly.
    GDALGetGeoTransform(hSrcDS, adfInTransform);
    double minX = adfInTransform[0];
    double maxY = adfInTransform[3];
    double adfOutTransform[6];
    double cellSizeX = adfInTransform[1];
    double cellSizeY = adfInTransform[5];
    adfOutTransform[0] = minX;
    adfOutTransform[1] = cellSizeX*numDiv;
    adfOutTransform[2] = 0;
    adfOutTransform[3] = maxY;
    adfOutTransform[4] = 0;
    adfOutTransform[5] = cellSizeY*numDiv;
    GDALSetGeoTransform(hDestDS, adfOutTransform);
    
    // Close the output file
    GDALClose(hDestDS);
    
    return 0;
}
