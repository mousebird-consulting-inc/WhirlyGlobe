/*
 *  Texture_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/31/19.
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

#import <UIKit/UIKit.h>
#import "TextureGLES_iOS.h"
#import "RawData_NSData.h"
#import "UIImage+Stuff.h"

namespace WhirlyKit {
    
TextureGLES_iOS::TextureGLES_iOS(const std::string &name)
: TextureGLES(name), TextureBase(name), TextureBaseGLES(name)
{
}
    
TextureGLES_iOS::TextureGLES_iOS(const std::string &name,NSData *data,bool in_isPVRTC)
    : TextureGLES(name), TextureBase(name), TextureBaseGLES(name)
{
    texData = RawDataRef(new RawNSDataReader(data));
    isPVRTC = in_isPVRTC;
}

// Set up the texture from a filename
TextureGLES_iOS::TextureGLES_iOS(const std::string &name,NSString *baseName,NSString *ext)
: TextureGLES(name), TextureBase(name), TextureBaseGLES(name)
{
    if (![ext compare:@"pvrtc"])
    {
        isPVRTC = true;
        
        // Look for an absolute version or one from the bundle
        // Only for pvrtc, though
        NSString* path = [NSString stringWithFormat:@"%@.%@",baseName,ext];
        
        if (![[NSFileManager defaultManager] fileExistsAtPath:path])
            path = [[NSBundle mainBundle] pathForResource:baseName ofType:ext];
        
        if (!path)
            return;
        NSData *data = [[NSData alloc] initWithContentsOfFile:path];
        if (!data)
            return;
        texData = RawDataRef(new RawNSDataReader(data));
        if (!texData)
            return;
    } else {
        // Otherwise load it the normal way
        UIImage *image = [UIImage imageNamed:[NSString stringWithFormat:@"%@.%@",baseName,ext]];
        if (!image)
        {
            image = [[UIImage alloc] initWithContentsOfFile:[NSString stringWithFormat:@"%@.%@",baseName,ext]];
            if (!image)
                return;
        }
        NSData *data = [image rawDataRetWidth:&width height:&height roundUp:true];
        if (!data)
            return;
        texData = RawDataRef(new RawNSDataReader(data));
    }
}

// Construct with a UIImage
TextureGLES_iOS::TextureGLES_iOS(const std::string &name,UIImage *inImage,bool roundUp)
: Texture(name), TextureBase(name), TextureGLES(name), TextureBaseGLES(name)
{
    NSData *data = [inImage rawDataRetWidth:&width height:&height roundUp:roundUp];
    if (!data)
        return;
    
    texData = RawDataRef(new RawNSDataReader(data));
}

TextureGLES_iOS::TextureGLES_iOS(const std::string &name,UIImage *inImage,int inWidth,int inHeight)
: Texture(name), TextureBase(name), TextureGLES(name), TextureBaseGLES(name)
{
    NSData *data = [inImage rawDataScaleWidth:inWidth height:inHeight border:0];
    if (!data)
        return;
    width = inWidth;  height = inHeight;
    
    texData = RawDataRef(new RawNSDataReader(data));
}
 
void TextureGLES_iOS::setPKMData(NSData *data)
{
    RawDataRef dataRef = RawDataRef(new RawNSDataReader(data));
    Texture::setPKMData(dataRef);
}
    
}
