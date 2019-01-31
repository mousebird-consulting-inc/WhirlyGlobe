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

#import "Texture.h"

namespace WhirlyKit {

/** iOS Version of texture.
 
    Adds a few convenience constructors for iOS specific data types.
  */
class Texture_iOS : public Texture
{
public:
    /// Construct emtpy
    Texture_iOS(const std::string &name);
    /// Construct with an NSData object
    Texture_iOS(const std::string &name,NSData *data,bool isPVRTC);
    /// Construct with a file name and extension
    Texture_iOS(const std::string &name,NSString *baseName,NSString *ext);
    /// Construct with a UIImage.  Expecting this to be a power of 2 on each side.
    /// If it's not we'll round up or down, depending on the flag
    Texture_iOS(const std::string &name,UIImage *inImage, bool roundUp=true);
    /// Construct by scaling the image to the given size
    Texture_iOS(const std::string &name,UIImage *inImage,int width,int height);
    
    /// Version of the PKM setter that takes NSData
    void setPKMData(NSData *data);
};

}
