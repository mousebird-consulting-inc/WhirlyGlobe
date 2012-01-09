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

int main (int argc, const char * argv[])
{

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    if (argc < 6)
    {
        fprintf(stderr,"syntax: %s <in.img> <outX> <outY> <outName> <outDir> [-outSize <outSize>] [-borderSize <borderSize>] [-texTool <textool path>]\n",argv[0]);
        return -1;
    }
    
    const char *inImage = argv[1];
    int outX = atoi(argv[2]);
    int outY = atoi(argv[3]);
    const char *outName = argv[4];
    const char *outDir = argv[5];

    int outSize = 512;
    int borderSize = 0;
    const char *texTool = NULL;
    
    // Work through the arguments
    int ai = 1;
    for (unsigned int ii=6;ii<argc;ii+=ai)
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
        
        fprintf(stderr,"Unrecognized argument: %s\n",argv[ii]);
        return -1;
    }
    
    if (outX < 1 || outY < 1)
    {
        fprintf(stderr,"Invalid output chunks.\n");
        return -1;
    }
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
            NSData *resultData = [imageRep TIFFRepresentation];
            char imgName[1024];
            sprintf(imgName,"%s_%dx%d",outName,ix,(outY-iy-1));
            NSString *fullName = [NSString stringWithFormat:@"%s/%s.tiff",outDir,imgName];
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
    
    // Create a little header for these images
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setValue:(texTool ? @"pvrtc" : @"tiff") forKey:@"format"];
    [dict setValue:[NSString stringWithFormat:@"%s",outName] forKey:@"baseName"];
    [dict setValue:[NSNumber numberWithInteger:outX] forKey:@"tilesInX"];
    [dict setValue:[NSNumber numberWithInteger:outY] forKey:@"tilesInY"];
    [dict setValue:[NSNumber numberWithInteger:outSize] forKey:@"pixelsSquare"];
    [dict setValue:[NSNumber numberWithInteger:borderSize] forKey:@"borderSize"];
    [dict writeToFile:[NSString stringWithFormat:@"%s/%s_info.plist",outDir,outName] atomically:NO];

    [pool drain];
    return 0;
}

