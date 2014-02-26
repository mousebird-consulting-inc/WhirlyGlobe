/*
 *  MaplyTexture.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/25/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "MaplyTexture_private.h"
#import "MaplyBaseViewController_private.h"
#import "UIImage+Stuff.h"

@implementation MaplyTexture

- (void)clear
{
    if (_viewC && _viewC->scene && _texID != WhirlyKit::EmptyIdentity)
    {
        _viewC->scene->addChangeRequest(new WhirlyKit::RemTextureReq(_texID));
        _viewC = nil;
        _texID = WhirlyKit::EmptyIdentity;
    }
}

- (void)dealloc
{
    [self clear];
}

@end

namespace WhirlyKit
{
    
// Set up the texture from a filename
MaplyTextureWrapper::MaplyTextureWrapper(const std::string &name,NSString *baseName,NSString *ext)
: Texture(name)
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
        texNSData = [[NSData alloc] initWithContentsOfFile:path];
        if (!texNSData)
            return;
        texData = RawDataRef(new RawDataWrapper([texNSData bytes],[texNSData length],false));
    } else {
        // Otherwise load it the normal way
        UIImage *image = [UIImage imageNamed:[NSString stringWithFormat:@"%@.%@",baseName,ext]];
        if (!image)
        {
            image = [[UIImage alloc] initWithContentsOfFile:[NSString stringWithFormat:@"%@.%@",baseName,ext]];
            if (!image)
                return;
        }
        texNSData = [image rawDataRetWidth:&width height:&height roundUp:true];
        texData = RawDataRef(new RawDataWrapper([texNSData bytes],[texNSData length],false));
    }
}

// Construct with a UIImage
MaplyTextureWrapper::MaplyTextureWrapper(const std::string &name,UIImage *inImage,bool roundUp)
: Texture(name)
{
    texNSData = [inImage rawDataRetWidth:&width height:&height roundUp:roundUp];
    texData = RawDataRef(new RawDataWrapper([texNSData bytes],[texNSData length],false));
}
    
}