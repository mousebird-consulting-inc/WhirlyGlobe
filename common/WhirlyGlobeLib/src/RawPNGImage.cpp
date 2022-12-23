/*  RawPNGImage.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/3/20.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "WhirlyKitLog.h"
#import "RawPNGImage.h"

// Note: These also need to be set on the compiler options for lodepng.cpp
//       in order to actually exclude the un-used code from the build.
#if !defined(LODEPNG_NO_COMPILE_ENCODER)
# define LODEPNG_NO_COMPILE_ENCODER
#endif
#if !defined(LODEPNG_NO_COMPILE_DISK)
# define LODEPNG_NO_COMPILE_DISK
#endif
// Note that this disables color profiles (ICC, gamma, whitepoint), background color for
// transparent pixels, text chunks, modification time, extension and unrecognized chunks,
// but *not* transparent color-key (tRNS)
#if !defined(LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS)
# define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#endif
#if !defined(LODEPNG_NO_COMPILE_ERROR_TEXT)
# define LODEPNG_NO_COMPILE_ERROR_TEXT
#endif
#if !defined(LODEPNG_NO_COMPILE_CPP)
# define LODEPNG_NO_COMPILE_CPP  // We'll use the C API
#endif

// Use zlib's crc32
#if defined __APPLE__
# if !defined(LODEPNG_NO_COMPILE_CRC)
#  define LODEPNG_NO_COMPILE_CRC  // We'll use the one from libz provided by the system
# endif

# import <zlib.h>
unsigned lodepng_crc32(const unsigned char* buffer, size_t length)
{
    return crc32_z(crc32(0L, Z_NULL, 0), buffer, length);
}
#else
// No `crc32_z` in Android's zlib for some reason
# if !defined(LODEPNG_COMPILE_ZLIB)
#  define LODEPNG_COMPILE_ZLIB
# endif
#endif

#import "lodepng.h"

#import <stdlib.h>
#import <string>
#import <arpa/inet.h>

#if defined __APPLE__
#import <libkern/OSByteOrder.h>
#endif

#pragma ide diagnostic ignored "ConstantConditionsOC"   // byte order is fixed at compile time

namespace WhirlyKit
{

static int getChannelCount(LodePNGColorType type)
{
    switch (type)
    {
        case LCT_GREY:       return 1;
        case LCT_GREY_ALPHA: return 2;
        case LCT_RGB:        return 3;
        case LCT_RGBA:       return 4;
        default:             return 0;
    }
}

// PNG is big-endian
#if defined(__BIG_ENDIAN__) || defined(__ORDER_BIG_ENDIAN__)
  constexpr bool needSwap = false;
#elif defined(__LITTLE_ENDIAN__) || defined(__ORDER_LITTLE_ENDIAN__)
  constexpr bool needSwap = true;
#else
# warning Inferring byte order - requires constexpr implicit ntohs()
  constexpr bool needSwap = (ntohs(1) != 1);
#endif

unsigned char *RawPNGImageLoaderInterpreter(unsigned int &width, unsigned int &height,
                                            const unsigned char * const data, const size_t length,
                                            const int valueMap[256],
                                            unsigned *outDepth, unsigned *outChannels,
                                            unsigned int *outErr, std::string* errStr)
{
    unsigned char *outData = nullptr;
    unsigned depth = 0, channels = 0, err;
    try
    {
        LodePNGState pngState;
        lodepng_state_init(&pngState);
        err = lodepng_inspect(&width, &height, &pngState, data, length);
        if (!err)
        {
            channels = getChannelCount(pngState.info_png.color.colortype);
            depth = pngState.info_png.color.bitdepth;
            if (channels < 1)
            {
                if (errStr)
                {
                    *errStr = "Unsupported image type";
                }
                err = (unsigned)-2;
            }
        }
        if (!err)
        {
            LodePNGState state;
            lodepng_state_init(&state);
            state.decoder.color_convert = 0;
            state.info_raw.colortype = pngState.info_png.color.colortype;
            state.info_raw.bitdepth = depth;
            err = lodepng_decode(&outData, &width, &height, &state, data, length);
            lodepng_state_cleanup(&state);
        }
    }
    catch (const std::exception &ex)
    {
        wkLogLevel(Error, "Exception in MaplyQuadImageLoader::dataForTile: %s", ex.what());
        if (errStr)
        {
            *errStr = ex.what();
        }
        err = (unsigned)-3;
    }
    catch (...)
    {
        wkLogLevel(Error, "Exception in MaplyQuadImageLoader::dataForTile");
        if (errStr)
        {
            *errStr = "Unknown exception";
        }
        err = (unsigned)-4;
    }

#if defined(LODEPNG_COMPILE_ERROR_TEXT)
    if ((int)err > 0 && errStr)
    {
        *errStr = lodepng_error_text(err);
    }
#endif

    // Remap data values
    if (depth == 8 && channels == 1 && valueMap && outData)
    {
        auto *p = (uint8_t*)outData;
        for (unsigned int ii=0;ii<width*height;ii++,p++)
        {
            const int newVal = valueMap[*p];
            if (newVal >= 0)
            {
                *p = newVal;
            }
        }
    }
    // PNG is big-endian
    if (needSwap && depth == 16 && outData)   //NOLINT
    {
        auto *p = (uint16_t *)outData;
        for (int i = 0; i < width * height * channels; ++i, ++p)
        {
            *p = ntohs(*p);
        }
    }

    if (outDepth)
    {
        *outDepth = depth;
    }
    if (outChannels)
    {
        *outChannels = channels;
    }
    if (outErr)
    {
        *outErr = err;
    }

    return outData;
}

}
