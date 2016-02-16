/*
 *  ImageWrapper.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/21/15.
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

#ifdef __ANDROID__
#import <jni.h>
#import <android/bitmap.h>
#import "Maply_jni.h"
#endif
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

// Interfaces between our image and what the toolkit is expecting
class ImageWrapper : public LoadedImage
{
public:
	ImageWrapper(RawDataRef rawData,int width,int height);
    ImageWrapper();

	// Construct the texture
	// Note: Need to handle borderSize
    virtual Texture *buildTexture(int borderSize,int width,int height);

    /// Data type for image
    virtual LoadedImageType getType();

    /// This means there's nothing to display, but the children are valid
    virtual bool isPlaceholder();

    /// Return image width
    virtual int getWidth();

    /// Return image height
    virtual int getHeight();

    bool placeholder;
    int width,height;
    RawDataRef rawData;
};

}
