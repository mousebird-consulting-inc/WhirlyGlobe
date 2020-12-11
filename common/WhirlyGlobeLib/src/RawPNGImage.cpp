/*
 *  RawPNGImage.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/3/20.
 *  Copyright 2011-2020 mousebird consulting
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

#include <stdlib.h>
#include <string>
#import "WhirlyKitLog.h"
#import "RawPNGImage.h"
#import "lodepng.h"

namespace WhirlyKit
{

unsigned char *RawPNGImageLoaderInterpreter(unsigned int &width,unsigned int &height,
                                          const unsigned char *data,size_t length,
                                          const std::vector<int> &valueMap,
                                          int &byteWidth,
                                          unsigned int &err)
{
    unsigned char *outData = NULL;

    try {
        LodePNGState pngState;
        lodepng_state_init(&pngState);
        err = lodepng_inspect(&width, &height, &pngState, data, length);
        if (pngState.info_png.color.colortype == LCT_GREY) {
            byteWidth = 1;
            err = lodepng_decode_memory(&outData, &width, &height, data, length, LCT_GREY, 8);
        } else {
            byteWidth = 4;
            err = lodepng_decode_memory(&outData, &width, &height, data, length, LCT_RGBA, 8);
        }
    }
    catch (const std::exception &ex) {
        wkLogLevel(Error, "Exception in MaplyQuadImageLoader::dataForTile: %s", ex.what());
        err = -1;
    }
    catch (...) {
        wkLogLevel(Error, "Exception in MaplyQuadImageLoader::dataForTile");
        err = -1;
    }
    
    // Remap data values
    if (byteWidth == 1 && !valueMap.empty()) {
        unsigned char *data = outData;
        for (unsigned int ii=0;ii<width*height;ii++) {
            int newVal = valueMap[*data];
            if (newVal >= 0)
                *data = newVal;
            data++;
        }
    }
    
    return outData;
}

}

