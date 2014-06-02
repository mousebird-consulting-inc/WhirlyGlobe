/*
 *  LongPressDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/11.
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

#import "LongPressDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES.h"
#import "GlobeMath.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeLongPressDelegate
{
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/LongPressDelegate.mm
    WhirlyGlobe::GlobeView *globeView;
=======
    WhirlyGlobeView *globeView;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/LongPressDelegate.mm
}

- (id)initWithGlobeView:(WhirlyGlobe::GlobeView *)inView
{
    if ((self = [super init]))
    {
        globeView = inView;
    }
    
    return self;
}

+ (WhirlyGlobeLongPressDelegate *)longPressDelegateForView:(UIView *)view globeView:(WhirlyGlobe::GlobeView *)globeView
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
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/LongPressDelegate.mm
	WhirlyKit::SceneRendererES *sceneRender = glView.renderer;
=======
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/LongPressDelegate.mm
//    WhirlyKit::Scene *scene = sceneRender.scene;
    
    if (press.state == UIGestureRecognizerStateBegan)
    {
        // Translate that to the sphere
        // If we hit, then we'll generate a message
        Point3d hit;
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/LongPressDelegate.mm
        Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
        CGPoint touchLoc = [press locationInView:press.view];
        Point2f frameSize = sceneRender->getFramebufferSize();
        if (globeView->pointOnSphereFromScreen(touchLoc,&theTransform,Point2f(frameSize.x()/glView.contentScaleFactor,frameSize.y()/glView.contentScaleFactor),&hit,true))
=======
        Eigen::Matrix4d theTransform = [globeView calcFullMatrix];
        CGPoint touchLoc = [press locationInView:press.view];
        if ([globeView pointOnSphereFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/LongPressDelegate.mm
        {
            WhirlyGlobeTapMessage *msg = [[WhirlyGlobeTapMessage alloc] init];
            msg.view = press.view;
            msg.touchLoc = touchLoc;
            [msg setWorldLocD:hit];
            Point3d geoHit = FakeGeocentricDisplayAdapter::DisplayToLocal(hit);
            [msg setWhereGeo:GeoCoord(geoHit.x(),geoHit.y())];
            msg.heightAboveSurface = globeView->heightAboveSurface();
            
            [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeLongPressMsg object:msg]];
        }
    }
}

@end
