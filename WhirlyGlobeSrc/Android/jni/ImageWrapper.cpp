/*
 *  ImageWrapper.cpp
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

#import "ImageWrapper.h"

namespace WhirlyKit
{

ImageWrapper::ImageWrapper(RawDataRef rawData,int width,int height)
	: rawData(rawData), width(width), height(height)
{
}

// Construct the texture
// Note: Need to handle borderSize
Texture *ImageWrapper::buildTexture(int borderSize,int width,int height)
{
	// Test code.  Off by default, obviously
//    	for (unsigned int ii=0;ii<width*height;ii++)
//    		((unsigned int *)rawData->getRawData())[ii] = 0xff0000ff;

	Texture *tex = new Texture("Tile Quad Loader",rawData,false);
	tex->setWidth(width);
	tex->setHeight(height);
	return tex;
}

/// Data type for image
LoadedImageType ImageWrapper::getType()
{
	return WKLoadedImageNSDataRawData;
}

/// This means there's nothing to display, but the children are valid
bool ImageWrapper::isPlaceholder()
{
	return false;
}

/// Return image width
int ImageWrapper::getWidth()
{
	return width;
}

/// Return image height
int ImageWrapper::getHeight()
{
	return height;
}

}
