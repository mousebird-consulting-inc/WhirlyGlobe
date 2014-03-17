/*
 *  ElevationChunk.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "ElevationChunk.h"

using namespace Eigen;
using namespace WhirlyKit;

typedef enum {WhirlyKitElevationFloats,WhirlyKitElevationShorts} WhirlyKitElevationFormat;

@implementation WhirlyKitElevationChunk
{
    WhirlyKitElevationFormat dataType;
    NSData *data;
}

+ (WhirlyKitElevationChunk *)ElevationChunkWithRandomData
{
    int numX = 20;
    int numY = 20;
    float floatArray[numX*numY];
    for (unsigned int ii=0;ii<numX*numY;ii++)
        floatArray[ii] = drand48()*30000;
    NSMutableData *data = [[NSMutableData alloc] initWithBytes:floatArray length:sizeof(float)*numX*numY];
    WhirlyKitElevationChunk *chunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:data sizeX:numX sizeY:numY];
    
    return chunk;
}

- (id)initWithFloatData:(NSData *)inData sizeX:(int)sizeX sizeY:(int)sizeY
{
    self = [super init];
    if (!self)
        return nil;
    
    _numX = sizeX;
    _numY = sizeY;
    dataType = WhirlyKitElevationFloats;
    data = inData;
    _noDataValue = -10000000;
    
    return self;
}

- (id)initWithShortData:(NSData *)inData sizeX:(int)sizeX sizeY:(int)sizeY
{
    self = [super init];
    if (!self)
        return nil;
    
    _numX = sizeX;
    _numY = sizeY;
    dataType = WhirlyKitElevationShorts;
    data = inData;
    _noDataValue = -10000000;
    
    return self;    
}


/// Return a single elevation at the given location
- (float)elevationAtX:(int)x y:(int)y
{
    if (!data)
        return 0.0;
    if (x < 0)  x = 0;
    if (y < 0)  y = 0;
    if (x >= _numX)  x = _numX-1;
    if (y >= _numY)  y = _numY-1;
    
    float ret = 0.0;
    switch (dataType)
    {
        case WhirlyKitElevationShorts:
            ret = ((short *)[data bytes])[y*_numX+x];
            break;
        case WhirlyKitElevationFloats:
            ret = ((float *)[data bytes])[y*_numX+x];
            break;
    }
    
    if (ret == _noDataValue)
        ret = 0.0;
    
    return ret;
}

- (float)interpolateElevationAtX:(float)x y:(float)y
{
    if (!data)
        return 0.0;
    
    float elevs[4];
    int minX = (int)x;
    int minY = (int)y;
    elevs[0] = [self elevationAtX:minX y:minY];
    elevs[1] = [self elevationAtX:minX+1 y:minY];
    elevs[2] = [self elevationAtX:minX+1 y:minY+1];
    elevs[3] = [self elevationAtX:minX y:minY+1];
    
    // Interpolate a new value
    float ta = (x-minX);
    float tb = (y-minY);
    float elev0 = (elevs[1]-elevs[0])*ta + elevs[0];
    float elev1 = (elevs[2]-elevs[3])*ta + elevs[3];
    float ret = (elev1-elev0)*tb + elev0;
    
    return ret;
}


@end
