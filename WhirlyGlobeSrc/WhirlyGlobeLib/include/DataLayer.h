/*
 *  DataLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import <math.h>
#import <Foundation/Foundation.h>
#import "Scene.h"

/// @cond
@class WhirlyKitLayerThread;
/// @endcond

/** A WhirlyGlobe Layer is just an objective C object that
    responds to a certain protocol.  All you have to do is
    fill in the one routine.  If you want to be called at
    regular intervals, you need to schedule yourself in the
    run loop provided with the thread.
    Layers will do things like overlay data on the globe or
    respond to user input.
    Layers do not have to be particularly fast.  At least not
    as fast as they'd have to be in the main run loop.
 */
@protocol WhirlyKitLayer

/// This is called after the layer thread kicks off.
/// Open your files and such here and then insert yourself in the run loop
///  for further processing.
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called by the layer thread to shut a layer down.
/// Clean all your stuff out of the scenegraph and so forth.
- (void)shutdown;

@optional

/// Dump logging information out to the console
- (void)log;

@end
