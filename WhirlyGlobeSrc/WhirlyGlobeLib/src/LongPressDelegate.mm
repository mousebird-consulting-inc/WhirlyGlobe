/*
 *  LongPressDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/11.
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

#import "LongPressDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES.h"
#import "GlobeMath.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeLongPressDelegate
{
    WhirlyGlobeView *globeView;
}

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
    WhirlyGlobeLongPressDelegate *pressDelegate = [[WhirlyGlobeLongPressDelegate alloc] initWithGlobeView:globeView];
    [view addGestureRecognizer:[[UILongPressGestureRecognizer alloc]
                                 initWithTarget:pressDelegate action:@selector(pressAction:)]];
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
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)press.view;
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
//    WhirlyKit::Scene *scene = sceneRender.scene;
    
    if (press.state == UIGestureRecognizerStateBegan)
    {
        // Translate that to the sphere
        // If we hit, then we'll generate a message
        Point3d hit;
        Eigen::Matrix4d theTransform = [globeView calcFullMatrix];
        CGPoint touchLoc = [press locationInView:press.view];
        if ([globeView pointOnSphereFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
        {
            WhirlyGlobeTapMessage *msg = [[WhirlyGlobeTapMessage alloc] init];
            msg.view = press.view;
            msg.touchLoc = touchLoc;
            [msg setWorldLocD:hit];
            Point3d geoHit = FakeGeocentricDisplayAdapter::DisplayToLocal(hit);
            [msg setWhereGeo:GeoCoord(geoHit.x(),geoHit.y())];
            msg.heightAboveSurface = globeView.heightAboveGlobe;
            
            [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeLongPressMsg object:msg]];
        }
    }
}

@end
