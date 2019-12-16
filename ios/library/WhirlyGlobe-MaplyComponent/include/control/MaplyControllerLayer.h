/*
 *  MaplyControllerLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/25/12.
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

#import <Foundation/Foundation.h>

/** 
    The View Controller Layer is a base class for other display layers.
    
    You don't create these directly.  This is a base class for other things.  Its hooks into the rest of the system are hidden.
  */
@interface MaplyControllerLayer : NSObject

/** 
    Set the priority for drawing.
    
    This is how you control where the geometry produced by this layer shows up with respect to other layers and other geometry.  This must be set immediately after creation.  It will have undefined behavior after the layer has started.
  */
@property (nonatomic,assign) int drawPriority;

/** 
    Set as unique identifier, or group...
 
    use this property in order to localize this layer in the Globe/Map, you use in a predicate to catch as a load layer in Globe...
 */
@property (nonatomic, strong) NSString *identifier;

@end
