/*
 *  TextureGroup.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011-2015 mousebird consulting
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

#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import "WhirlyVector.h"

/** The Texture Group is used to represent a large image
    that's been broken into several pieces to aid in loading
    and get around the 1k x 1k (or 2k x 2k) limit in OpenGL.
	File name: base_XxY.ext
 */
@interface WhirlyKitTextureGroup : NSObject 

/// If set, the path to the texture group.
/// Might be somewhere other than the default bundle
@property (nonatomic) NSString *basePath;
/// Base name (e.g. "worldTexture")
@property (nonatomic) NSString *baseName;
/// Extension (e.g. "png")
@property (nonatomic) NSString *ext;
/// Number of chunks in the X dimension (longitude)
@property (nonatomic,readonly) unsigned int numX;
/// Number of chunks in the Y dimension (latitude)
@property (nonatomic,readonly) unsigned int numY;
/// Number of pixels on each side
@property (nonatomic,readonly) unsigned int pixelsSquare;
/// Number of pixels on each side devoted to border
@property (nonatomic,readonly) unsigned int borderPixels;

/// Need to initialize with the the info plist file.
/// This will point us to everything else
- (id) initWithInfo:(NSString *)infoName;

/// Generate the name of the given instance (without the extension)
- (NSString *) generateFileNameX:(unsigned int)x y:(unsigned int)y;

/// Calculate the mapping that represents "full" coverage
/// This takes border pixels into account
- (void)calcTexMappingOrg:(WhirlyKit::TexCoord *)org dest:(WhirlyKit::TexCoord *)dest;

@end
