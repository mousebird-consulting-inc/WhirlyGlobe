//
//  WhirlyGlobeView.h
//  WhirlyGlobeLib
//
//  Created by Stephen Gifford on 1/3/11.
//  Copyright 2011 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "EAGLView.h"
#import "GlobeScene.h"

/* Whirly Globe View
	This manages the drawing of a Globe Scene.
	Just hand it the scene, tell it how often to draw and
    it'll do the rest.
 */
@interface WhirlyGlobeView : EAGLView 
{
}

// Caller is responsible for keeping it around
@property (nonatomic, assign) WhirlyGlobe::GlobeScene *scene;


// Create a Whirly Globe view like this
// The frame rate units are 60/rate.  So pass in 2 to get 30 frames/sec (at most)
+ (WhirlyGlobeView *)whirlyGlobeWithScene:(WhirlyGlobe::GlobeScene *)scene frameInterval:(unsigned int)frameInterval;


@end
