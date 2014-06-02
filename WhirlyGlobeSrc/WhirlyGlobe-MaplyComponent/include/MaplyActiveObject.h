/*
 *  MaplyActiveObject.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/3/13.
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

@class MaplyBaseViewController;

/** @brief Active Objects are used implement animation.
    @details Active Objects work in conjuction with the renderer to make updates on the main thread.  The way they work is this.  They're called right at the beginning of a frame draw.  They can make updates to regular Maply objects via the MaplyBaseViewController add and remove calls with the MaplyThreadMode set to MaplyThreadCurrent.  This forces the changes to happen immediately on the current (main) thread.
    @details Active Objects have access to the internals of the toolkit, for historical reasons.  That makes it difficult to document them here.  Look at the AnimationTest example in the test app.  You will need to fill in the following methods at least.

|Method | Description |
|:------|:------------|
|init| The init method is up to you.  There's no specific signature you'll need, but do pass in all the information you'll want to run quickly in updateForFrame.|
|(bool)hasUpdate| Returns true if there's an update to be processed. |
|(void)updateForFrame:(id)frameInfo | This is where you do your work.  Remove the objects that need to be removed, add the objects that need to be added and be sure to use the MaplyThreadCurrent mode to make the changes happen immediately.|
|shutdown| This method is called when the active object is to be removed.  Clean up all your visible objects.|
 
    @details Active Objects are run on the main thread and you're probably going to be asking the view controller to add and remove objects on the main thread.  As such, this can be slow.  Be sure to precalculate whatever you might need to make this run faster.  Also consider implementing your changes another way.  If it can be done on another thread, do it on another thread.
 
 */
@interface MaplyActiveObject : NSObject

/** @brief Initialize with a view controller
    @details The default initializer just takes a view controller.  If you replace this with your own, be sure to pass in what you need.
  */
- (id)initWithViewController:(MaplyBaseViewController *)viewC;

/// @brief The view controller this active object is associated with
@property (nonatomic,weak,readonly) MaplyBaseViewController *viewC;

@end
