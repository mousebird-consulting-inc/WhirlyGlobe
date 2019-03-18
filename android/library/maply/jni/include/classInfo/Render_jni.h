/*
 *  Astronomy_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "Maply_jni.h"
#import "WhirlyGlobe_Android.h"

/// These are the various shader programs we set up by default
#define kMaplyShaderDefaultTri "Default Triangle;lighting=yes"
#define kMaplyDefaultTriangleShader "Default Triangle;lighting=yes"

#define kMaplyShaderDefaultModelTri "Default Triangle;model=yes;lighting=yes"

#define kMaplyShaderDefaultTriNoLighting "Default Triangle;lighting=no"
#define kMaplyNoLightTriangleShader "Default Triangle;lighting=no"

#define kMaplyShaderDefaultTriScreenTex "Default Triangle;screentex=yes;lighting=yes"

#define kMaplyShaderDefaultTriMultiTex "Default Triangle;multitex=yes;lighting=yes"
#define kMaplyShaderDefaultTriMultiTexRamp "Default Triangle;multitex=yes;lighting=yes;ramp=yes"
#define kMaplyShaderDefaultTriNightDay "Default Triangle;nightday=yes;multitex=yes;lighting=yes"

#define kMaplyShaderDefaultLine "Default Line;backface=yes"
#define kMaplyDefaultLineShader "Default Line;backface=yes"

#define kMaplyShaderDefaultLineNoBackface "Default Line;backface=no"
#define kMaplyNoBackfaceLineShader "Default Line;backface=no"

#define kMaplyShaderBillboardGround "Default Billboard ground"
#define kMaplyShaderBillboardEye "Default Billboard eye"

#define kMaplyShaderDefaultWideVector "Default Wide Vector"
#define kMaplyShaderDefaultWideVectorGlobe "Default Wide Vector Globe"

#define kMaplyScreenSpaceDefaultMotionProgram "Default Screenspace Motion"
#define kMaplyScreenSpaceDefaultProgram "Default Screenspace"

#define kMaplyShaderParticleSystemPointDefault "Default Part Sys (Point)"

namespace WhirlyKit
{
// Image type enum
typedef enum
{MaplyImageIntRGBA,
    MaplyImageUShort565,
    MaplyImageUShort4444,
    MaplyImageUShort5551,
    MaplyImageUByteRed,MaplyImageUByteGreen,MaplyImageUByteBlue,MaplyImageUByteAlpha,
    MaplyImageUByteRGB,
    MaplyImageETC2RGB8,MaplyImageETC2RGBA8,MaplyImageETC2RGBPA8,
    MaplyImageEACR11,MaplyImageEACR11S,MaplyImageEACRG11,MaplyImageEACRG11S,
    MaplyImage4Layer8Bit}
    MaplyImageType;

// Convert the image format to an appropriate GL enum for textures
GLenum ImageFormatToGLenum(MaplyImageType format);

}
