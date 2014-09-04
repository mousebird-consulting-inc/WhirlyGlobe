---
title: Taps and Selection
layout: tutorial
---

Be sure to open the HelloEarth project in Xcode if it isn't already.

Tap somewhere in the HelloEarth world. Note that the display moves to center where you tapped. Nice, but what if we want some alternate or additional behavior associated with a tap?

To do so, we need to indicate that we are interested in taps and/or selections. This requires that our view controller conform to the [WhirlyGlobeViewControllerDelegate protocol](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Protocols/WhirlyGlobeViewControllerDelegate.html) (for globes), or the [MaplyViewControllerDelegate protocol](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Protocols/MaplyViewControllerDelegate.html) (for maps).  We assign ourselves as a delegate and implement any tap or selection methods we're interested in. Let's try taps first.

We're going to pop up a little annotation to show where the user tapped.

First, open up MainViewController.h. Modify the MainViewController interface to include the two protocols. (There's no harm in doing so – they're all optional.

~~~objc
@interface MainViewController : UIViewController <WhirlyGlobeViewControllerDelegate,MaplyViewControllerDelegate>
~~~

Next, open MainViewController.m. Add a new private method in the _@interface_ section to create an annotation with a given title, subtitle, and location.  We can use this with both map and globe.

~~~objc
­(void) addAnnotation:(NSString *)title withSubtitle:(NSString *)subtitle at: (MaplyCoordinate)coord;
~~~

Set your map or view controller's delegate to self after you alloc and init globeViewC or mapViewC:

~~~objc
// If you're doing a globe
globeViewC.delegate = self;

// If you're doing a map
mapViewC.delegate = self;
~~~

Now, add the following three methods after the _addCountries:_ method from the preview lesson.

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

That's it! Build and run, and each time you tap, an annotation will appear at the tapped location indicating its _latitude_ and _longitude_.

Next, let's modify HelloEarth to respond to selecting one of the countries. We'll display a different annotation with the country name. 

Add the following methods to MainViewController.m.

~~~objc
// This is the version for a globe
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

// This is the version for a map
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

Here's what we did.  We set up delegates for a globe or map, depending on which you did earlier.  Those delegate methods pop up an annotation.  The annotation will show the currently selected country or just where the user tapped, depending.

Check out the documentation for the [WhirlyGlobeViewControllerDelegate](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Protocols/WhirlyGlobeViewControllerDelegate.html) [MaplyViewControllerDelegate](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Protocols/MaplyViewControllerDelegate.html) protocols to see what other actions you can respond to.

Congratulations! You are officially a WG-­Maply novitiate. Go forth and code.
