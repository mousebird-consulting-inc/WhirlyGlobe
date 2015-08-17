---
title: Screen Markers
layout: tutorial
---

Screen markers are 2D markers that follow a location on the globe or map.  As the user moves, they move, but they don't get any bigger or smaller.

We'll need an XCode project here, so if you haven't done the [Hello Earth](hello_earth.html) tutorial go do that.  If you haven't got one yet, here's a suitable ViewController (for [Objective-C]({{ site.baseurl }}/tutorial/code/ViewController_screen_markers.m) or [Swift]({{ site.baseurl }}/tutorial/code/ViewController_screen_markers.swift)).

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/screen_markers_1.png)

### Marker Image

Markers don't have to have images, they can be just color, but no one does that.  You'll want a suitable image.  It so happens we have a selection available.

Look in the **maki icons** directory within the **resources**.  We're going to use the image called *alcohol-shop-24@2x*.  You can just search for that filename.

When you find it, add it to your HelloEarth project.  Be sure its part of the HelloEarth target.

![Xcode Resources]({{ site.baseurl }}/images/tutorial/screen_markers_2.png)

Now that we've got the perfect icon, let's put it somewhere.

### Placing Markers

Politicians are big drinkers, so let's toss a few of those icons on world capitals.  Add the following method to your ViewController.

{% multiple_code %}
  {% highlight objc %}
- (void)addBars
{
    // set up some locations
    MaplyCoordinate capitals[10];
    capitals[0] = MaplyCoordinateMakeWithDegrees(-77.036667, 38.895111);
    capitals[1] = MaplyCoordinateMakeWithDegrees(120.966667, 14.583333);
    capitals[2] = MaplyCoordinateMakeWithDegrees(55.75, 37.616667);
    capitals[3] = MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222);
    capitals[4] = MaplyCoordinateMakeWithDegrees(-66.916667, 10.5);
    capitals[5] = MaplyCoordinateMakeWithDegrees(139.6917, 35.689506);
    capitals[6] = MaplyCoordinateMakeWithDegrees(166.666667, -77.85);
    capitals[7] = MaplyCoordinateMakeWithDegrees(-58.383333, -34.6);
    capitals[8] = MaplyCoordinateMakeWithDegrees(-74.075833, 4.598056);
    capitals[9] = MaplyCoordinateMakeWithDegrees(-79.516667, 8.983333);

    // get the image and create the markers
    UIImage *icon = [UIImage imageNamed:@"alcohol-shop-24@2x.png"];
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=0;ii<10;ii++)
    {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = icon;
        marker.loc = capitals[ii];
        marker.size = CGSizeMake(40,40);
        [markers addObject:marker];
    }
    // add them all at once (for efficency)
    [theViewC addScreenMarkers:markers desc:nil];
}
  {% endhighlight %}

  {----}

  {% highlight swift %}
private func addBars() {
    let capitals = [
        MaplyCoordinateMakeWithDegrees(-122.4192,37.7793),
        MaplyCoordinateMakeWithDegrees(-77.036667, 38.895111),
        MaplyCoordinateMakeWithDegrees(120.966667, 14.583333),
        MaplyCoordinateMakeWithDegrees(55.75, 37.616667),
        MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222),
        MaplyCoordinateMakeWithDegrees(-66.916667, 10.5),
        MaplyCoordinateMakeWithDegrees(139.6917, 35.689506),
        MaplyCoordinateMakeWithDegrees(166.666667, -77.85),
        MaplyCoordinateMakeWithDegrees(-58.383333, -34.6),
        MaplyCoordinateMakeWithDegrees(-74.075833, 4.598056),
        MaplyCoordinateMakeWithDegrees(-79.516667, 8.983333)
    ]

    let icon = UIImage(named: "alcohol-shop-24")

    let markers = capitals.map { cap -> MaplyScreenMarker in
        let marker = MaplyScreenMarker()
        marker.image = icon
        marker.loc = cap
        marker.size = CGSizeMake(140, 140)
        return marker
    }

    theViewC?.addScreenMarkers(markers, desc: nil)
}
  {% endhighlight %}
{% endmultiple_code %}


And don't forget to run it by adding this to your viewDidLoad method.

{% multiple_code %}
  {% highlight objc %}
// add the bar icons
[self addBars];
­  {% endhighlight %}

  {----}

  {% highlight swift %}
// add the bar icons
addBars()
­  {% endhighlight %}
{% endmultiple_code %}


And thus we get... bars!

![World Bars]({{ site.baseurl }}/images/tutorial/screen_markers_3.png)

Astute observers will point out that not all of those are world capitals.  Don't bother, I'm an American and thus ignorant of geography.

### Marker Details

Let's take a closer look at what we did there.  The capitals themselves are obvious enough.  We just create a MaplyCoordinate, which is in radians from degrees, which the rest of you find more intuitive.  Weirdos.

For each marker, we just set the location, assign the image and the size.  The size is in screen pixels and won't vary.  Then we add each marker to an array and hand them over as a group.

The reason we add the markers all at once is performance.  WhirlyGlobe-Maply needs that hint to batch them all together.  There are some exceptions to this, but in general the toolkit is batching based on what you hand it.  And batching is good.
