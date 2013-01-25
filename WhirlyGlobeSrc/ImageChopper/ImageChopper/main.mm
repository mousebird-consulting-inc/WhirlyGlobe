//
//  main.m
//  ImageChopper
//
//  Created by Stephen Gifford on 4/18/11.
//  Copyright 2011 mousebird consulting. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

// Copy the given row to the new location
void CopyRow(NSBitmapImageRep *imageRep,int srcRow,int destRow)
{
    unsigned char *data = [imageRep bitmapData];
    NSInteger rowSize = [imageRep bytesPerRow];
    bcopy(&data[rowSize*srcRow], &data[rowSize*destRow], rowSize);
}

// Copy the given column to the new column
void CopyColumn(NSBitmapImageRep *imageRep,int srcCol,int destCol)
{
    unsigned char *data = [imageRep bitmapData];
    NSInteger rowLen = [imageRep pixelsWide];
    NSInteger pixSize = [imageRep bitsPerPixel]/8;
    for (unsigned int ii=0;ii<[imageRep pixelsHigh];ii++)
        bcopy(&data[(rowLen*ii+srcCol)*pixSize],&data[(rowLen*ii+destCol)*pixSize], pixSize);
}

typedef enum {OutFormatTiff,OutFormatJPEG,OutFormatPNG} OutFormatType;

// Build a single level of the image grid
bool BuildLevel(int outX,int outY,const char *levelId,NSImage *img,int outSize,int borderSize,const char *outDir,const char *outName,OutFormatType outFormatType,const char *texTool)
{
    // Work through the chunks
    for (unsigned int ix=0;ix<outX;ix++)
    {
        float sx;
        sx = ix * img.size.width / outX;
        for (unsigned int iy=0;iy<outY;iy++)
        {
            float sy;
            sy = (outY-iy-1) * img.size.height / outY;
            
            NSBitmapImageRep *imageRep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:outSize pixelsHigh:outSize bitsPerSample:8 samplesPerPixel:3 hasAlpha:NO isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:4*outSize bitsPerPixel:32] autorelease];
            
            // Create an NSGraphicsContext that draws into the NSBitmapImageRep, and make it current.
            NSGraphicsContext *nsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:imageRep];
            [NSGraphicsContext saveGraphicsState];
            [NSGraphicsContext setCurrentContext:nsContext];
            
            // Clear
            [[NSColor clearColor] set];
            NSRectFill(NSMakeRect(0, 0, [imageRep pixelsWide], [imageRep pixelsHigh]));
            
            // Draw the image, but shrunk down by the border size
            [img drawInRect:NSMakeRect(borderSize,borderSize,outSize-2*borderSize,outSize-2*borderSize) 
                   fromRect:NSMakeRect(sx, sy, img.size.width / outX, img.size.height / outY) 
                  operation:NSCompositeCopy fraction:1.0];
            
            // Copy the top border upward
            for (unsigned int ib=0;ib<borderSize;ib++)
                CopyRow(imageRep,borderSize,ib);
            // And the bottom row downward
            for (unsigned int ib=outSize-borderSize;ib<outSize;ib++)
                CopyRow(imageRep,outSize-borderSize-1,ib);
            // Left border leftward
            for (unsigned int ib=0;ib<borderSize;ib++)
                CopyColumn(imageRep,borderSize,ib);
            // Right border rightward
            for (unsigned int ib=outSize-borderSize;ib<outSize;ib++)
                CopyColumn(imageRep,outSize-borderSize-1,ib);
            
            // And save it out
            char imgName[1024];
            if (levelId)
                sprintf(imgName,"%s_%sx%dx%d",outName,levelId,ix,(outY-iy-1));
            else
                sprintf(imgName,"%s_%dx%d",outName,ix,(outY-iy-1));
            NSData *resultData = nil;
            NSString *ext = nil;
            switch (outFormatType)
            {
                case OutFormatTiff:
                    ext = @"tiff";
                    resultData = [imageRep TIFFRepresentation];
                    break;
                case OutFormatJPEG:
                    ext = @"jpg";
                    resultData = [imageRep representationUsingType:NSJPEGFileType properties:nil];
                    break;
                case OutFormatPNG:
                    ext = @"png";
                    resultData = [imageRep representationUsingType:NSPNGFileType properties:nil];
                    break;
            }
            NSString *fullName = [NSString stringWithFormat:@"%s/%s.%@",outDir,imgName,ext];
            [resultData writeToFile:fullName atomically:NO];
            
            // If they gave us a path to the texture tool, invoke that
            if (texTool)
            {
                char cmd[1024];
                sprintf(cmd,"%s -e PVRTC --channel-weighting-linear --bits-per-pixel-4 -o %s/%s.pvrtc %s/%s.tiff",
                        texTool,outDir,imgName,outDir,imgName);
                if (system(cmd))
                {
                    fprintf(stderr,"Failed to convert image to pvrtc with this command:\n%s\n",cmd);
                    return -1;
                }
            }
            
            [NSGraphicsContext restoreGraphicsState];
        }
    }    
    
    return true;
}

int main (int argc, const char * argv[])
{

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    if (argc < 6)
    {
        fprintf(stderr,"syntax: %s <in.img> <outName> <outDir> [-singleres <outX> <outY>] [-multires <maxzoom>] [-outSize <outSize>] [-borderSize <borderSize>] [-texTool <textool path>] [-outformat tiff/jpeg/png]\n",argv[0]);
        return -1;
    }
    
    const char *inImage = argv[1];
    const char *outName = argv[2];
    const char *outDir = argv[3];

    int outSize = 512;
    int borderSize = 0;
    const char *texTool = NULL;
    int singleOutX = -1,singleOutY = -1;
    int maxZoom = -1;
    OutFormatType outFormatType = OutFormatTiff;
    
    // Work through the arguments
    int ai = 1;
    for (unsigned int ii=4;ii<argc;ii+=ai)
    {
        if (!strcmp(argv[ii],"-outSize"))
        {
            ai = 2;
            if (ii+ai > argc)
            {
                fprintf(stderr,"Missing argument for -outSize\n");
                return -1;
            }
            
            outSize = atoi(argv[ii+1]);
            continue;
        }

        if (!strcmp(argv[ii],"-borderSize"))
        {
            ai = 2;
            if (ii+ai > argc)
            {
                fprintf(stderr,"Missing argument for -borderSize\n");
                return -1;
            }
            
            borderSize = atoi(argv[ii+1]);
            continue;
        }
        
        if (!strcmp(argv[ii],"-texTool"))
        {
            ai = 2;
            if (ii+ai > argc)
            {
                fprintf(stderr,"Missing argument for -texTool\n");
                return -1;
            }
            
            texTool = argv[ii+1];
            continue;
        }        
        
        if (!strcmp(argv[ii],"-singleres"))
        {
            ai = 3;
            if (ii+ai > argc)
            {
                fprintf(stderr,"Missing arguments for -singleres\n");
                return -1;
            }
            
            singleOutX = atoi(argv[ii+1]);
            singleOutY = atoi(argv[ii+2]);
            continue;
        }
        
        if (!strcmp(argv[ii],"-multires"))
        {
            ai = 2;
            if (ii+ai > argc)
            {
                fprintf(stderr,"Missing argument for -multires\n");
                return -1;
            }
            
            maxZoom = atoi(argv[ii+1]);
            continue;
        }
        
        if (!strcmp(argv[ii],"-outformat"))
        {
            ai = 2;
            if (ii+ai > argc)
            {
                fprintf(stderr,"Missing argument for -outformat");
                return -1;
            }
            
            if (!strcmp(argv[ii+1],"jpg") || !strcmp(argv[ii+1],"jpeg"))
                outFormatType = OutFormatJPEG;
            else
                if (!strcmp(argv[ii+1],"png"))
                    outFormatType = OutFormatPNG;
            
            continue;
        }
        
        fprintf(stderr,"Unrecognized argument: %s\n",argv[ii]);
        return -1;
    }
    
    if (singleOutX == -1 && maxZoom == -1)
    {
        fprintf(stderr,"Must specify -singleres or -multires\n");
        return -1;
    }

    if (singleOutX > 0)
    {
        // This size needs to be a power of two
        unsigned int bits = 0;
        for (int ii=0;ii<32;ii++)
            if ((1<<ii) & outSize)
                bits++;
        if (bits != 1)
        {
            fprintf(stderr,"Output size needs to be non-zero and a power of 2.\n");
            return -1;
        }
    }
    
    NSError *error = nil;
    [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithFormat:@"%s",outDir] withIntermediateDirectories:YES attributes:nil error:&error];
    if (error)
    {
        fprintf(stderr,"Failed to create output directory.\n");
        return -1;
    }
    
    NSImage *img = [[[NSImage alloc] initByReferencingFile:[NSString stringWithFormat:@"%s",inImage]] autorelease];
    if (!img)
    {
        fprintf(stderr,"Failed to open input image.\n");
        return -1;
    }
    
    // Create a little header for these images
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setValue:(texTool ? @"pvrtc" : @"tiff") forKey:@"format"];
    [dict setValue:[NSString stringWithFormat:@"%s",outName] forKey:@"baseName"];

    // Build the images
    if (singleOutX > 0)
    {
        // Single level is easy enough
        BuildLevel(singleOutX, singleOutY, NULL, img, outSize, borderSize, outDir, outName, outFormatType, texTool);

        [dict setValue:[NSNumber numberWithInteger:singleOutX] forKey:@"tilesInX"];
        [dict setValue:[NSNumber numberWithInteger:singleOutY] forKey:@"tilesInY"];
    } else {
        for (unsigned int level=0;level<=maxZoom;level++)
        {
            char levelStr[10];
            sprintf(levelStr, "%d",level);
            BuildLevel(1<<level, 1<<level, levelStr, img, outSize, borderSize, outDir, outName, outFormatType, texTool);            
        }
        [dict setValue:[NSNumber numberWithInteger:maxZoom] forKey:@"maxLevel"];
    }
    
    [dict setValue:[NSNumber numberWithInteger:outSize] forKey:@"pixelsSquare"];
    [dict setValue:[NSNumber numberWithInteger:borderSize] forKey:@"borderSize"];
    [dict writeToFile:[NSString stringWithFormat:@"%s/%s_info.plist",outDir,outName] atomically:NO];
    

    [pool drain];
    return 0;
}

