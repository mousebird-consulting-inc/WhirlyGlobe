/*
 *  MaplyActiveObject.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/3/13.
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

@class MaplyBaseViewController;
@protocol MaplyRenderControllerProtocol;

/** 
    Active Objects are used implement animation.
    
    Active Objects work in conjuction with the renderer to make updates on the main thread.  The way they work is this.  They're called right at the beginning of a frame draw.  They can make updates to regular Maply objects via the MaplyBaseViewController add and remove calls with the MaplyThreadMode set to MaplyThreadCurrent.  This forces the changes to happen immediately on the current (main) thread.

    Fill in at least the hasUpdate and updateForFrameMethods.
 
    Active Objects are run on the main thread and you're probably going to be asking the view controller to add and remove objects on the main thread.  As such, this can be slow.  Be sure to precalculate whatever you might need to make this run faster.  Also consider implementing your changes another way.  If it can be done on another thread, do it on another thread.
 
 */
@interface MaplyActiveObject : NSObject

/** 
    Initialize with a view controller
    
    The default initializer just takes a view controller.  If you replace this with your own, be sure to pass in what you need.
  */
- (nonnull instancetype)initWithViewController:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/// The view controller this active object is associated with
@property (nonatomic,weak,readonly) NSObject<MaplyRenderControllerProtocol> *__nullable viewC;

/** Has Update
 
    This is called every frame to determine if the active model has an update.
    If it doesn't, we may not need to render.  So use this judiciously.
  */
- (bool)hasUpdate;

/** Update for the current frame.
 
    Run the update right now.  This should not take too long, as it's holding up
    the renderer.
 
    The frameInfo object is undefined at this point.
  */
- (void)updateForFrame:(void * __nonnull)frameInfo;

/** Teardown active model.
 
    The active model will no longer be run.  Get rid of your internal state.
  */
- (void)teardown;

@end
