---
title: Adding a Layer
layout: tutorial
---

Despite your swelling pride, the blank screen is only so satisfying. Let's add a visible layer.

Open HelloEarth in Xcode if it's not already open.

Now, where are you going to get some data to display? The answer is, you already have it! WG­Maply comes with some resources for its test app and for the benefit of users. If you're using WG-Maply as a submodule, the path to the resources directory is HelloEarth/libs/WhirlyGlobe­Maply/resources/. If you're using the binary distribution, the path is BinaryDirectory/resources/. We'll refer to this directory as resources from now on.

Navigate to resources/base_maps/. You will find geography­class_medres.mbtiles. Add geography­class_medres.mbtiles to your project by dragging it into HelloEarth in your Project Navigator view. You can create a Resources folder there if you like, but it's not necessary. The same goes for 'Copy items into the destination group's folder' – if you want your project to have its own copy of the file, check that box. What is necessary is to check the 'Add to targets' box for HelloEarth, to ensure that the data is packaged with your app when it is built.


Now, let's add this as a layer to theViewC. Open MainViewController.m and add the following lines to the end of the viewDidLoad method.

~~~objc
// we want a black background for a globe, a white background for a map.
theViewC.clearColor = (globeViewC != nil) ? [UIColor blackColor] : [UIColor whiteColor];

// and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
theViewC.frameInterval = 2;

// set up the layer
MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc]
initWithMBTiles:@"geography­class_medres"];

MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
layer.handleEdges = (globeViewC != nil);
layer.coverPoles = (globeViewC != nil);
layer.requireElev = false;
layer.waitLoad = false;
layer.drawPriority = 0;
layer.singleLevelLoading = false;
[theViewC addLayer:layer];

// start up over San Francisco
if (globeViewC != nil)
{
  globeViewC.height = 0.8;
  [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(­122.4192,37.7793) time:1.0];
} else {
  mapViewC.height = 1.0;
  [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(­122.4192,37.7793) time:1.0];
}
~~~

Now, when you run HelloEarth, you should see a colorful set of countries looking back at you.

The workhorse of image tile display is the [MaplyQuadImageTileLayer](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyQuadImageTilesLayer.html).  It will respond to changes in viewer position and load the most appropriate image tiles.

The [MaplyQuadImageTileLayer](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyQuadImageTilesLayer.html) needs a tile source to load data from.  It can be a remote or local source, or even something you make up.  In this case we've chosen a local [MaplyMBTileSource](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyMBTileSource.html).  That tile source knows how to read from an MBTiles file stored on the device.

[MaplyQuadImageTileLayers](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyQuadImageTilesLayer.html) are pretty flexible.  Let's look at some of the properties.

- _handleEdges_ will add some extra geometry to ensure there's no gap when mismatching levels of detail abut. You'll never need this for a flat map, but for a globe or a 3D map, you might.

- _coverPoles_ will create 'caps' for the north and south poles. If you're using a globe, and your projection doesn't go all the way to the poles (this is common), you'll want to turn this on.

- _drawPriority_ sets the priority of geometry created.  You can use this to overlay image layers, with one caveat: it must be set immediately after the layer is created. You can't change it later.

There are plenty of other properties we haven't touched on here. Browsing through the [documentation](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyQuadImageTilesLayer.html) and experimenting are encouraged.

These layers will work equally well on the globe or a flat map.  Let’s move on to remote image tiles.

[Remote Image Tiles](remote_image_tiles.html)
