---
title: Local Image Layer
layout: tutorial
---

Despite your swelling pride, the blank screen is only so satisfying. Let's add a visible layer.  We're going to use the Geography Class example from Mapbox and we'll pull it locally, from device storage.

![Geography Class from Mapbox]({{ site.baseurl }}/images/tutorial/geography_class_mapbox.png)

You'll need to have done the [globe](your_first_globe.html) or [map](your_first_map.html) tutorials, either is fine.  Open your HelloEarth project and get ready.

![Xcode ViewController.m]({{ site.baseurl }}/images/tutorial/local_image_layer_1.png)

If you haven't got one here is a suitable ViewController (for [Objective-C]({{ site.baseurl }}/tutorial/code/ViewController_globe_and_map.m) or [Swift]({{ site.baseurl }}/tutorial/code/ViewController_globe_and_map.swift)) file to start with.  This version handles both a globe and a map and makes a nice starting point.
                                           
### Geography Class MBTiles

We need the Geography Class MBTiles file from Mapbox.  Luckily, you'll find you already have it.

If you set up WhirlyGlobe-Maply as a submodule, look in libs/WhirlyGlobeMap/resources/.  If you used the binary distribution look in BinaryDirectory/resources.  For Cocoapods [do something].  We want to the file geography-class_medres.mbtiles.

Add **geography­class_medres.mbtiles** to your project by dragging it into HelloEarth in your Project Navigator view. You can create a Resources folder there if you like, but it's not necessary. The same goes for 'Copy items into the destination group's folder' – if you want your project to have its own copy of the file, check that box. What you must do, however is check the 'Add to targets' box for HelloEarth, to ensure that the data is packaged with your app when it is built.

![Geography Class Import]({{ site.baseurl }}/images/tutorial/local_image_layer_2.png)

That will get the MBTiles file into the bundle.  Next, we display it.

### Adding a Layer

There are a few steps to displaying an MBTiles file on your globe or map.

- Create the MaplyMBTileSource to read it.
- Spin up a MaplyQuadImageTilesLayer to display it.
- Start in a useful position on the globe

If you worked through the globe or the map example, you'll need to add this little bit of code to your viewDidLoad method.  This will make the examples work with either globe or map.  If you're using the ViewController file from above, you don't need it.

{% multiple_code %}
  {% highlight objc %}
// this logic makes it work for either globe or map
WhirlyGlobeViewController *globeViewC = nil;
MaplyViewController *mapViewC = nil;
if ([theViewC isKindOfClass:[WhirlyGlobeViewController class]])
    globeViewC = (WhirlyGlobeViewController *)theViewC;
else
    mapViewC = (MaplyViewController *)theViewC;
  {% endhighlight %}

  {----}

  {% highlight swift %}
let globeViewC = theViewC as? WhirlyGlobeViewController
let mapViewC = theViewC as? MaplyViewController
  {% endhighlight %}
{% endmultiple_code %}

Here's how you open the MBTiles database, create the layer and add it to the globe or map.  These changes go in the viewDidLoad method.

{% multiple_code %}
  {% highlight objc %}
// we want a black background for a globe, a white background for a map.
theViewC.clearColor = (globeViewC != nil) ? [UIColor blackColor] : [UIColor whiteColor];

// and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
theViewC.frameInterval = 2;

// set up the data source
MaplyMBTileSource *tileSource = 
    [[MaplyMBTileSource alloc] initWithMBTiles:@"geography-class_medres"];

// set up the layer
MaplyQuadImageTilesLayer *layer = 
    [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys 
                                      tileSource:tileSource];
layer.handleEdges = (globeViewC != nil);
layer.coverPoles = (globeViewC != nil);
layer.requireElev = false;
layer.waitLoad = false;
layer.drawPriority = 0;
layer.singleLevelLoading = false;
[theViewC addLayer:layer];

// start up over San Francisco, center of the universe
if (globeViewC != nil) {
    globeViewC.height = 0.8;
    [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192,37.7793)
                time:1.0];
} else {
    mapViewC.height = 1.0;
    [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192,37.7793)
            time:1.0];
}
  {% endhighlight %}

  {----}

  {% highlight swift %}
// we want a black background for a globe, a white background for a map.
theViewC!.clearColor = (globeViewC != nil) ? UIColor.blackColor() : UIColor.whiteColor()

// and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
theViewC!.frameInterval = 2

// set up the data source
let tileSource = MaplyMBTileSource(MBTiles: "geography-class_medres")

// set up the layer
let layer = MaplyQuadImageTilesLayer(coordSystem: tileSource.coordSys, tileSource: tileSource)

layer.handleEdges = (globeViewC != nil)
layer.coverPoles = (globeViewC != nil)
layer.requireElev = false
layer.waitLoad = false
layer.drawPriority = 0
layer.singleLevelLoading = false
theViewC!.addLayer(layer)

// start up over Madrid, center of the old-world
if let globeViewC = globeViewC {
    globeViewC.height = 0.8
    globeViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
}
else if let mapViewC = mapViewC {
    mapViewC.height = 1.0
    mapViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
}
  {% endhighlight %}
{% endmultiple_code %}


Now, when you run HelloEarth, you should see a colorful set of countries looking back at you.

![iOS Simulator]({{ site.baseurl }}/images/tutorial/local_image_layer_3.png)

If you're doing a map it'll look like that.  Only, you know, flat.

### Deeper Dive

You might notice there were two interesting objects there.  First was the MaplyMBTileSource and the second a MaplyQuadImageTilesLayer.

The MaplyMBTileSource is responsible for reading the MBTiles file, which is just a big sqlite file with a bunch of images in it.  The MaplyQuadImageTilesLayer is much more complicated.  It responds to changes in viewer position and loads the most appropriate image tiles.

The MaplyQuadImageTilesLayer is pretty flexible.  Let's look at some of the properties.

- _handleEdges_ will add some extra geometry to ensure there's no gap when mismatching levels of detail abut. You'll never need this for a flat map, but for a globe or a 3D map, you might.
- _coverPoles_ will create 'caps' for the north and south poles. If you're using a globe, and your projection doesn't go all the way to the poles (this is common), you'll want to turn this on.
- _drawPriority_ sets the priority of geometry created.  You can use this to overlay image layers, with one caveat: it must be set immediately after the layer is created. You can't change it later.

There are plenty of other properties we haven't touched on here.  Browse through the documentation and you'll see a lot more.

Next up, an image layer you page over the network.
