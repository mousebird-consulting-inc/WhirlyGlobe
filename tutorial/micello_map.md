---
title: Micello Indoor Maps
layout: tutorial
---
*Tutorial by Ranen Ghosh*

[Micello](http://www.micello.com/) provides indoor map data for places like shopping malls, airports, college campuses, hospitals, museums, business campuses, and conference centers.  Integration with a map toolkit like ours is a natural fit.  In this tutorial we add the multi-level structure for the Westfield Valley Fair mall to a WhirlyGlobe-Maply based map.

## Getting started

For this example we provide source code to get going.  Download [this zip archive]({{ site.baseurl }}/tutorial/resources/micello_map/micello_tutorial.zip) and copy its files to your project directory.  You should overwrite your existing ViewController.* files with the ones in the archive.  Add the other files to the project.

The new ViewController class is simplified to only use a globe, and only display remote OpenStreetMap tiles.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/micello_1.png)

## Setup the Micello map

Start by adding two imports to ViewController.m:

{% highlight objc %}
#import "MaplyMicelloMap.h"
#import "SimpleAnnotationViewController.h"
{% endhighlight %}

Add two member variables to the ViewController implementation block.  The first is a **MaplyMicelloMap** object, which we'll describe below.

{% highlight objc %}
{
    WhirlyGlobeViewController *globeVC;

    // New member variables
    MaplyMicelloMap *micelloMap;
    UISegmentedControl *segCtrl;
}
{% endhighlight %}

Initialize the Micello map variable at the end of viewDidLoad.  You'll need to give it the base URL of the map to be displayed, and the key from Micello for your account and project.  It also wants a base draw priority.  Draw priority values determine the order in which map elements are seen if they overlap.

{% highlight objc %}
    // Add Westfield Valley Fair mall map
    NSString *micelloKey = @"YOUR MICELLO KEY GOES HERE";
    NSString *baseURL = @"http://mfs.micello.com/ms/v1/mfile/map/78/mv/-/ev/-/geojson";
    micelloMap = [[MaplyMicelloMap alloc] initWithBaseURL:baseURL projectKey:micelloKey baseDrawPriority:kMaplyVectorDrawPriorityDefault+200];
{% endhighlight %}

## The **MaplyMicelloMap** class

The **MaplyMicelloMap** class is the most important class needed to display a Micello community map.  After initializing it, the way to use it is to:

* tell it to fetch the map information,
* tell it how you want the map to be drawn, and
* give it a z-level within the map to draw.

### Fetching the map information

This is done with the *startFetchMapWithSuccess:failure:* method.  If the base URL and key provided to the init method are valid, this will fetch lots of information about the community map, and store it in the object.  This includes drawings, levels, and entities.

Since doing this is asynchronous, subsequent operations on the **MaplyMicelloMap** object should go in the success block passed to this method.

### Styling the map

This can be done in two ways.  You can give the **MaplyMicelloMap** object default drawing attributes.  These are currently *fillColor*, *outlineColor*, *selectedOutlineColor*, *lineWidth*, and *selectedLineWidth*.  The basic properties for fill and outline apply to the map as it's normally displayed.  The *selected* properties apply to rooms selected by a user.

You can also add style rules, represented by the **MaplyMicelloStyleRule** class, to draw geometry features that match certain criteria.  We've provided some default style rules.

### Setting a z-level

After fetching the map information and styling the map, set the z-level using the *setZLevel:viewC:* method.  This selects one of the levels to draw and it's the one you will see.

This code, which does all of the above, goes after our **MaplyMicelloMap** initialization:

{% highlight objc %}
    [micelloMap startFetchMapWithSuccess:^() {
        [micelloMap addDefaultStyleRules];
        [globeVC setPosition:MaplyCoordinateMakeWithDegrees(micelloMap.centerLonDeg, micelloMap.centerLatDeg) height:0.0001];
        if (micelloMap.zLevels.count>0)
            [micelloMap setZLevel:((NSNumber *)micelloMap.zLevels[0]).intValue viewC:globeVC];
    } failure:^(NSError * _Nonnull error) {
    }];
{% endhighlight %}

Run this code now and you'll see the bottom level of the Westfield Valley Fair mall displayed on your globe.

![Westfield Valley Fair mall map on the globe]({{ site.baseurl }}/images/tutorial/micello_2.png)

## Changing the Map Level

If a community map has more than one level, it's simple to change.  Just call *setZLevel:viewC:* again.  Here we modify the code from above to allow the user to select a z-level:

{% highlight objc %}
    [micelloMap startFetchMapWithSuccess:^() {
        [micelloMap addDefaultStyleRules];
        [globeVC setPosition:MaplyCoordinateMakeWithDegrees(micelloMap.centerLonDeg, micelloMap.centerLatDeg) height:0.0001];

        // Add a segmented control to support switching between map levels.
        if (micelloMap.zLevels.count>1) {
            segCtrl = [[UISegmentedControl alloc] initWithItems:[micelloMap.zLevels valueForKey:@"stringValue"]];
            segCtrl.selectedSegmentIndex = 0;
            segCtrl.frame = CGRectMake(20, 20, 120, 40);
            [segCtrl addTarget:self action:@selector(onSegChange) forControlEvents:UIControlEventValueChanged];
            [globeVC.view addSubview:segCtrl];
        }

        if (micelloMap.zLevels.count>0)
            [micelloMap setZLevel:((NSNumber *)micelloMap.zLevels[0]).intValue viewC:globeVC];

    } failure:^(NSError * _Nonnull error) {
    }];
{% endhighlight %}

Add the *onSegChange* callback to handle the user interaction and change the displayed z-level of the community map.

{% highlight objc %}
- (void)onSegChange {
    int zLevel = ((NSNumber *)micelloMap.zLevels[segCtrl.selectedSegmentIndex]).intValue;
    [micelloMap setZLevel:zLevel viewC:globeVC];
}
{% endhighlight %}

If you run the example now, you'll see what the upper levels of the mall look like.

![Second level of the mall]({{ site.baseurl }}/images/tutorial/micello_3.png)

## Selection of map features

Now we're going to let the user select shops and restaurants in the mall to learn more about them.  The **MaplyMicelloMap** object will return information about a selected entity using the **MaplyMicelloMapEntity** class.  Call the **MaplyMicelloMap** select:viewC: method to get a **MaplyMicelloMapEntity** object, which has a *properties* dictionary property which contains metadata we can be display.

In the example, we use a simple form view controller to display the properties (the **SimpleAnnotationViewController**, which is among the files to download that are listed above).  The **MaplyAnnotation** object anchors this view to the appropriate location in the map.

Add the following two method implementations after *onSegChange* :

{% highlight objc %}
- (void)globeViewController:(WhirlyGlobeViewController *__nonnull)viewC didSelect:(NSObject *__nonnull)selectedObj {

    MaplyMicelloMapEntity *entity = [micelloMap select:selectedObj viewC:viewC];
    if (!entity)
        return;
   
    MaplyAnnotation *annotate = [[MaplyAnnotation alloc] init];
    SimpleAnnotationViewController *svc = [[SimpleAnnotationViewController alloc] initWithName:entity.properties[@"name"] desc:entity.properties[@"description"] hours:entity.properties[@"hours"] location:entity.intAddress phone:entity.properties[@"phone"] website:entity.properties[@"url"]];

    // superview needed for Auto Layout and SMCalloutView (used in MaplyAnnotation) to play nice together.
    // https://github.com/nfarina/calloutview/issues/73
    UIView *superview = [[UIView alloc]initWithFrame:svc.view.frame];
    [superview addSubview:svc.view];
    annotate.contentView = superview;
   
    [globeVC clearAnnotations];
    [globeVC addAnnotation:annotate forPoint:MaplyCoordinateMakeWithDegrees(entity.lonDeg, entity.latDeg) offset:CGPointZero];
}

- (void)globeViewController:(WhirlyGlobeViewController *__nonnull)viewC didTapAt:(WGCoordinate)coord {
    [micelloMap clearSelectionViewC:viewC];
    [globeVC clearAnnotations];
}
{% endhighlight %}

Now if you run the example and select a store or restaurant, you'll see the entity in question highlighted, with a custom annotation displayed over it.

![Micello community map entity selection]({{ site.baseurl }}/images/tutorial/micello_4.png)

## A Mall with Two Minimums

The Westfield Valley Fair mall straddles two cities: Santa Clara and San Jose.  In 2012 San Jose raised its minimum wage from $8 to $10 per hour, while Santa Clara kept its fixed at $8.  Different stores within the mall fall within those different cities and some (such as The Gap) straddle both.  In this optional part of the tutorial let's see which!

Here we will overlay the city boundaries over top of the mall map, to display which stores have the higher minimum wage.

{% highlight objc %}
    // New member variables
    MaplyMicelloMap *micelloMap;
    UISegmentedControl *segCtrl;
    MaplyComponentObject *sanJoseCompObj, *santaClaraCompObj;
{% endhighlight %}


{% highlight objc %}
    // Add Santa Clara county boundary
    NSString *path = [[NSBundle mainBundle] pathForResource:@"SantaClaraBoundary" ofType:@"geojson"];
    MaplyVectorObject *santaClaraVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:[[NSFileManager defaultManager] contentsAtPath:path]];
    santaClaraCompObj = [globeVC addVectors:@[santaClaraVecObj] desc:@{
                                                               kMaplyVecTexture:    [UIImage imageNamed:@"bgYellow.png"],
                                                               kMaplyDrawPriority:  @(400),
                                                               kMaplyFilled:        @(YES)}];
    
    // Add San Jose boundary
    path = [[NSBundle mainBundle] pathForResource:@"SanJoseBoundary" ofType:@"geojson"];
    MaplyVectorObject *sanJoseVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:[[NSFileManager defaultManager] contentsAtPath:path]];
    sanJoseCompObj = [globeVC addVectors:@[sanJoseVecObj] desc:@{
                                                           kMaplyVecTexture:    [UIImage imageNamed:@"bgBlue.png"],
                                                           kMaplyDrawPriority:  @(500),
                                                           kMaplyFilled:        @(YES)}];
{% endhighlight %}

![San Jose and Santa Clara city boundaries overlaid upon the mall map]({{ site.baseurl }}/images/tutorial/micello_5.png)

That rounds our the tutorial.  Enjoy your building maps!


