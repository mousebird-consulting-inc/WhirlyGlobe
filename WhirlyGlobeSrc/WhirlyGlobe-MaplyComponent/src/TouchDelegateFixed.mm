//
//  TouchDelegateFixed.mm
//  WhirlyGlobe-MaplyComponent
//
//  Created by John Kokkinidis on 9/23/13.
//
//


#import "TouchDelegateFixed.h"

@implementation TouchDelegateFixed 
{
    WhirlyGlobeView * __weak view;
}


- (id)initOnView:(UIView *)glView withGlobeView:globeView
{
	if ((self = [super init]))
	{
		view = globeView;


        if([glView isKindOfClass:[WhirlyKitEAGLView class]])
        {
            ((WhirlyKitEAGLView*)glView).delegate = self;
        }
	}
	
	return self;
}


//Run whatever you want when the user first touches the screen
-(void)userTouchBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [view cancelAnimation];
    
}


+ (TouchDelegateFixed *)touchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
    TouchDelegateFixed *touchDelegate =
    [[TouchDelegateFixed alloc]
     initOnView:(UIView *)view
     withGlobeView:globeView];

    
//    UIPanGestureRecognizer *touchRecog = [[UITouchGestureRecognizer alloc] initWithTarget:touchDelegate action:@selector(touchAction:)];
//    touchRecog.delegate = self;
//	[view addGestureRecognizer:touchRecog];
    
	return touchDelegate;
}

@end
