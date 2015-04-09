/*
 *  UIImage+Stuff.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/11/11.
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

#import <UIKit/UIKit.h>


@interface UIImage(Stuff)

/// Construct with raw pixel data (row)+(row)+(row)....  32 bit RGBA
+ (UIImage *)imageWithRawData:(NSData *)data width:(unsigned int)width height:(unsigned int)height;

/// Pull the raw data, width, and height out of a UIImage
-(NSData *)rawDataRetWidth:(unsigned int *)width height:(unsigned int *)height roundUp:(bool)roundUp;

/// Generate raw data for the image at the given resolution, with the given replicated border
-(NSData *)rawDataScaleWidth:(unsigned int)width height:(unsigned int)height border:(int)border;

@end

// A function we can call to force the linker to bring in categories
#ifdef __cplusplus
extern "C"
#endif
void UIImageDummyFunc();
