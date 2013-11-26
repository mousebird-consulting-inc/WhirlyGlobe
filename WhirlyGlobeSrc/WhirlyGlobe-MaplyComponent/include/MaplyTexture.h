/*
 *  MaplyTexture.h
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

#import <Foundation/Foundation.h>

/** @brief Represents an OpenGL texture.
    @details The MaplyTexture is an opaque object used to track OpenGL ES textures.  You create one from the MaplyBaseViewController's addImage call.  Then that texture will live until this object is released.
    @details These can be used in place of UIImages in the various objects (e.g. MaplyScreenMarker).
  */
@interface MaplyTexture : NSObject
@end
