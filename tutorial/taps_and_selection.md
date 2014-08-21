---
title: Taps and Selection
layout: tutorial
---

Tap somewhere in the HelloEarth world. Note that the display moves to center where you tapped. Nice, but what if we want some alternate or additional behavior associated with a tap?

To do so, we need to indicate that we are interested in taps and/or selections. This requires that our view controller conform to the WhirlyGlobeViewControllerDelegate protocol (for globes), or the MaplyViewControllerDelegate protocol (for maps). Then we assign ourselves as a delegate and implement any tap or selection methods we're interested in. Let's try taps first.

First, open up MainViewController.h. Modify the MainViewController interface to include the two protocols. (There's no harm in doing so – implement only the methods you want and ignore the rest.

~~~objc
@interface MainViewController : UIViewController <WhirlyGlobeViewControllerDelegate,MaplyViewControllerDelegate>
~~~

Next, open MainViewController.m. Add a new private method to add an annotation with a given title, subtitle, and location:

~~~objc
­(void) addAnnotation:(NSString *)title withSubtitle:(NSString *)subtitle at: (MaplyCoordinate)coord;
~~~

Set your map or view controller's delegate to self after you alloc and init globeViewC or mapViewC:

~~~objc
globeViewC.delegate = self;
~~~

or

~~~objc
mapViewC.delegate = self;
~~~

*for git project: have logic in here. i.e. if (globeViewC), etc.*

Now, add the following three methods after addCountries:

~~~objc
­(void)addAnnotation:(NSString *)title withSubtitle:(NSString *)subtitle at:(MaplyCoordinate)coord
{
[theViewC clearAnnotations];
MaplyAnnotation *annotation = [[MaplyAnnotation alloc] init];
annotation.title = title;
annotation.subTitle = subtitle;
[theViewC addAnnotation:annotation forPoint:coord offset:CGPointZero];
}

­(void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
NSString *title = @"Tap Location:";
NSString *subtitle = [NSString stringWithFormat:@"(%.2fN, %.2fE)", coord.y*57.296,coord.x*57.296];
[self addAnnotation:title withSubtitle:subtitle at:coord];
}

­(void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
NSString *title = @"Tap Location:";
NSString *subtitle = [NSString stringWithFormat:@"(%.2fN, %.2fE)", coord.y*57.296,coord.x*57.296];
[self addAnnotation:title withSubtitle:subtitle at:coord];
}
~~~

That's it! Build and run, and each time you tap, an annotation will appear at the tapped location indicating its latitude and longitude.

Now, let's modify HelloEarth to respond to selecting one of the countries. We'll display a different annotation with the country name. Add the following methods to MainViewController.m:

~~~objc
­(void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj
{
// ensure it's a MaplyVectorObject. It should be one of our outlines.
if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
{
MaplyVectorObject *theVector = (MaplyVectorObject *)selectedObj;
MaplyCoordinate location;
if ([theVector centroid:&location])
{
NSString *title = @"Selected:";
NSString *subtitle = (NSString *)theVector.userObject;
[self addAnnotation:title withSubtitle:subtitle at:location];
}
}
}

­(void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj
{
// ensure it's a MaplyVectorObject. It should be one of our outlines.
if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
{
MaplyVectorObject *theVector = (MaplyVectorObject *)selectedObj;
MaplyCoordinate location;
if ([theVector centroid:&location])
{
NSString *title = @"Selected:";
NSString *subtitle = (NSString *)theVector.userObject;
[self addAnnotation:title withSubtitle:subtitle at:location];
}
}
}
~~~

Check out the documentation for the WhirlyGlobeViewControllerDelegate MaplyViewControllerDelegate protocols to see what other actions you can respond to.

Congratulations! You are officially a WG­Maply novitiate. Go forth and code.

