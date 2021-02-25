/*
 *  Renderer_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/19.
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

typedef JavaClassInfo<WhirlyKit::SingleVertexAttributeInfo> SingleVertexAttributeInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::ProgramGLES> OpenGLES2ProgramClassInfo;
typedef JavaClassInfo<WhirlyKit::SceneRendererGLES_Android> SceneRendererInfo;
typedef JavaClassInfo<WhirlyKit::SingleVertexAttribute> SingleVertexAttributeClassInfo;

namespace WhirlyKit {
// Image type enum, corresponds to the same thing on the Java side
typedef enum {
    MaplyImageIntRGBA,
    MaplyImageUShort565,
    MaplyImageUShort4444,
    MaplyImageUShort5551,
    MaplyImageUByteRed, MaplyImageUByteGreen, MaplyImageUByteBlue, MaplyImageUByteAlpha,
    MaplyImageUByteRGB,
    MaplyImageETC2RGB8, MaplyImageETC2RGBA8, MaplyImageETC2RGBPA8,
    MaplyImageEACR11, MaplyImageEACR11S, MaplyImageEACRG11, MaplyImageEACRG11S,
    MaplyImage4Layer8Bit
}
MaplyImageType;

// Convert the image format to an appropriate GL enum for textures
extern GLenum ImageFormatToGLenum(MaplyImageType format);

// Convert the image type format to the texture type format
TextureType ImageFormatToTexType(MaplyImageType format);
}

