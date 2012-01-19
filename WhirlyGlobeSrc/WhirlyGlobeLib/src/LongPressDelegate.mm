/*
 *  LongPressDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/11.
 *  Copyright 2011 mousebird consulting
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

#import "LongPressDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES1.h"
#import "GlobeMath.h"

using namespace WhirlyGlobe;

@implementation WhirlyGlobeLongPressDelegate

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
    if ((self = [super init]))
    {
        globeView = inView;
    }
    
    return self;
}

+ (WhirlyGlobeLongPressDelegate *)longPressDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
    WhirlyGlobeLongPressDelegate *pressDelegate = [[[WhirlyGlobeLongPressDelegate alloc] initWithGlobeView:globeView] autorelease];
    [view addGestureRecognizer:[[[UILongPressGestureRecognizer alloc]
                                 initWithTarget:pressDelegate action:@selector(pressAction:)]
                                autorelease]];
    return pressDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Called for a tap
- (void)pressAction:(id)sender
{
	UILongPressGestureRecognizer *press = sender;
	WhirlyGlobeEAGLView  *glView = (WhirlyGlobeEAGLView  *)press.view;
	WhirlyGlobeSceneRendererES1 *sceneRender = glView.renderer;
    WhirlyGlobe::GlobeScene *scene = sceneRender.scene;
    
    if (press.state == UIGestureRecognizerStateBegan)
    {
        // Translate that to the sphere
        // If we hit, then we'll generate a message
        Point3f hit;
        Eigen::Affine3f theTransform = [globeView calcModelMatrix];
        CGPoint touchLoc = [press locationOfTouch:0 inView:glView];
        if ([globeView pointOnSphereFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&hit])
        {
            WhirlyGlobeTapMessage *msg = [[[WhirlyGlobeTapMessage alloc] init] autorelease];
            msg.view = glView;
            msg.touchLoc = touchLoc;
            [msg setWorldLoc:hit];
            [msg setWhereGeo:scene->getCoordSystem()->geoFromPoint(hit)];
            msg.heightAboveSurface = globeView.heightAboveGlobe;
            
            [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeLongPressMsg object:msg]];
        }
    }
}

@end
