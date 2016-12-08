---
title: Location Tracking
layout: ios-tutorial
---
*Tutorial by Ranen Ghosh*

This tutorial will show you how to track your location on the globe or map.  The basic functionality is showing a pulsing marker at your location, with a direction arrow if heading is available.  There's also the ability to lock the screen on your location, and there are a few different ways of doing that.

## Getting started

You will need to have already run through the [remote image layer](remote_image_layer.html) tutorial.  Let's get started by opening your HelloEarth project.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/location_tracking_1.png)

If you haven't got one here is a suitable ViewController file to start with (for [Objective-C]({{ site.baseurl }}/tutorial/ios/code/ViewController_location_tracking.m) or [Swift]({{ site.baseurl }}/tutorial/ios/code/ViewController_location_tracking.swift)).  This version is from the previous tutorial for a [remote image layer](remote_image_layer.html).

## Location privacy settings

In order for your app to get permission to access location services, you have to ask. In XCode, go into the settings for your project, and go to the Info tab.  Under Custom iOS Target Properties, add an entry for "Privacy - Location When In Use..." The message you enter here will be displayed when the user's prompted for location permission.

![Requesting location permission]({{ site.baseurl }}/images/tutorial/location_tracking_2.png)

## Tracking location

### Implementing the **MaplyLocationTrackerDelegate** protocol

You can start tracking location with one simple command, but first you have to have an object implementing the **MaplyLocationTrackerDelegate** protocol.  In our case, we'll use the view controller class itself.

Change the class declaration to implement the protocol:

{% multiple_code %} {% highlight objc %}
@interface ViewController : UIViewController <MaplyLocationTrackerDelegate>
{% endhighlight %}
{----}
{% highlight swift %}
class ViewController: UIViewController, MaplyLocationTrackerDelegate
{% endhighlight %} {% endmultiple_code %}

Then, implement the location event callbacks:

{% multiple_code %} {% highlight objc %}
- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error {
}
- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status {
}
{% endhighlight %}
{----}
{% highlight swift %}
    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error) {
    }
    func locationManager(_ manager: CLLocationManager, didChange status: CLAuthorizationStatus) {
    }
{% endhighlight %} {% endmultiple_code %}

For now we'll leave these empty, but they allow your app to respond to location events further up the line.

### Start location tracking

Now, call the method startLocationTrackingWithDelegate:useHeading:useCourse:simulate:

{% multiple_code %} {% highlight objc %}
    [theViewC startLocationTrackingWithDelegate:self useHeading:true useCourse:true simulate:false];
{% endhighlight %}
{----}
{% highlight swift %}
    theViewC!.startLocationTracking(with: self, useHeading: true, useCourse: true, simulate: false)
{% endhighlight %} {% endmultiple_code %}

If you now run your app, you will see your current location marked on the map or globe. If heading or course are available, they will be indicated by an arrow on the marker.

![Start tracking location]({{ site.baseurl }}/images/tutorial/location_tracking_3.png)

The first argument is the delegate, which in our case is the view controller class.  The useHeading argument is true if heading should be used when available.  The useCourse argument is true if course, when available, should be used as a fallback if heading is unavailable.  Leave the last argument false for now.

## Locking the map or globe on your location

Now, let's keep the screen locked on the current location:

{% multiple_code %} {% highlight objc %}
    [theViewC changeLocationTrackingLockType:MaplyLocationLockNorthUp];
{% endhighlight %}
{----}
{% highlight swift %}
    theViewC!.changeLocationTrackingLockType(MaplyLocationLockNorthUp)
{% endhighlight %} {% endmultiple_code %}

In this example, as you travel the map will stay centered on your position.

![Locking on]({{ site.baseurl }}/images/tutorial/location_tracking_4.png)

### Lock modes

In the previous example your position stayed at the center of the screen, with the map or globe oriented North-up.

There are actually several lock modes available:

| Lock mode | Description |
| MaplyLocationLockNone | Default behavior; screen doesn't move with travel |
| MaplyLocationLockNorthUp | Screen stays centered on user's position, oriented North-up |
| MaplyLocationLockHeadingUp | Screen stays centered on user's position, oriented heading-up |
| MaplyLocationLockHeadingUpOffset | User's position is fixed to a point above or below the view's center by an offset amount, oriented heading-up |

### Forward track and its offset

To use the *MaplyLocationLockHeadingUpOffset* locking mode, call the changeLocationTrackingLockType:forwardTrackOffset: method.

{% multiple_code %} {% highlight objc %}
    [theViewC changeLocationTrackingLockType:MaplyLocationLockHeadingUpOffset forwardTrackOffset:150];
{% endhighlight %}
{----}
{% highlight swift %}
    theViewC!.changeLocationTrackingLockType(MaplyLocationLockHeadingUpOffset, forwardTrackOffset: 150)
{% endhighlight %} {% endmultiple_code %}

The offset amount is the number of pixels below or above the center at which to fix the user's position.  A positive value is below.

![Heading up offset mode ]({{ site.baseurl }}/images/tutorial/location_tracking_5.png)

## Simulating locations with headings

You can also simulate locations with headings. (The Apple development tools allow simulating position, but not heading).

In our example, declare a *longitude* variable amoung the class variables.

{% multiple_code %} {% highlight objc %}
    float longitude;
{% endhighlight %}
{----}
{% highlight swift %}
    var longitude:Float?
{% endhighlight %} {% endmultiple_code %}

Initialize it in viewDidLoad, before the startLocationTracking... method call.

{% multiple_code %} {% highlight objc %}
    longitude = -120.0;
{% endhighlight %}
{----}
{% highlight swift %}
    longitude = -120.0
{% endhighlight %} {% endmultiple_code %}

Then implement the **MaplyLocationTrackerDelegate** protocol's getSimulationPoint method:

{% multiple_code %} {% highlight objc %}
- (MaplyLocationTrackerSimulationPoint) getSimulationPoint {
    longitude += 0.001;
    MaplyLocationTrackerSimulationPoint simPoint;
    simPoint.latDeg = 49.0;
    simPoint.lonDeg = longitude;
    simPoint.headingDeg = 90.0;
    return simPoint;
}
{% endhighlight %}
{----}
{% highlight swift %}
    func getSimulationPoint() -> MaplyLocationTrackerSimulationPoint {
        longitude = longitude! + 0.001
        return MaplyLocationTrackerSimulationPoint(lonDeg: longitude!, latDeg: 49.0, headingDeg: 90.0)
    }
{% endhighlight %} {% endmultiple_code %}

What this will do is simulate a track that starts at 120W, 49N and moves eastward over time.  (Every time the location tracker calls your getSimulationPoint method, the longitude is moved slightly eastward.)

Now change your startLocationTracking... method call so that *simulate* is set to true:

{% multiple_code %} {% highlight objc %}
    [theViewC startLocationTrackingWithDelegate:self useHeading:true useCourse:true simulate:true];
{% endhighlight %}
{----}
{% highlight swift %}
    theViewC!.startLocationTracking(with: self, useHeading: true, useCourse: true, simulate: true)
{% endhighlight %} {% endmultiple_code %}

The result in this example is a track moving eastward over the Canada-U.S. border.

![Simulating an eastward track]({{ site.baseurl }}/images/tutorial/location_tracking_6.png)

## Stop tracking location

Finally, to stop tracking location, simply call the stopLocationTracking method:

{% multiple_code %} {% highlight objc %}
    [theViewC stopLocationTracking];
{% endhighlight %}
{----}
{% highlight swift %}
    theViewC!.stopLocationTracking()
{% endhighlight %} {% endmultiple_code %}



