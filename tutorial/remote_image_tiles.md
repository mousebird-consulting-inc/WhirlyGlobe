---
title: Remote Image Tiles
layout: tutorial
---

The [geography class](http://a.tiles.mapbox.com/v3/mapbox.geography-class/page.html#4/0.00/0.00) map only has a few levels of detail. It's pretty small and restrictive.  What we'd like to have is a much bigger map, too big to fit on the device.  That means a remote tile source.

Let's add a remote tile source, and take a closer look at the Earth. We'll use the MapQuest Open Aerial tile set. If you end up wanting to use these tiles in an app that you distribute, check out the [requirements](http://developer.mapquest.com/web/products/open/map).

Start by opening the HelloEarth project if it isn't already open.  

We'll set this up to use either the local or remote tiles. Look for the following lines in your source code.

~~~objc
// Set up the layer
MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc] initWithMBTiles:@"geography­class_medres"];

MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
~~~

Now replace that with these lines instead.

~~~objc
// add the capability to use the local tiles or remote tiles
bool useLocalTiles = false;

// we'll need this layer in a second
MaplyQuadImageTilesLayer *layer;

if (useLocalTiles)
{
  MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc] initWithMBTiles:@"geography­class_medres"];
  layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
} else {
  // Because this is a remote tile set, we'll want a cache directory
  NSString *baseCacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
  NSString *aerialTilesCacheDir = [NSString stringWithFormat:@"%@/osmtiles/",baseCacheDir];
  int maxZoom = 18;

  // MapQuest Open Aerial Tiles, Courtesy Of Mapquest
  // Portions Courtesy NASA/JPL­Caltech and U.S. Depart. of Agriculture, Farm Service Agency
  MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/sat/" ext:@"png" minZoom:0 maxZoom:maxZoom];
  tileSource.cacheDir = aerialTilesCacheDir;
  layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
}
~~~

Build and run, and play with the new HelloEarth. You can zoom in to your heart's content, provided your heart doesn't desire sub­meter resolution.

Here's what's different. Previously we were using a [MaplyMBTileSource](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyMBTileSource.html) tile source.  That only knows how to read local MBTiles files.  We replaced it with a [MaplyRemoteTileSource](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyRemoteTileSource.html) which knows how to talk to servers.

The [MaplyRemoteTileSource](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/Classes/MaplyRemoteTileSource.html) has a number of interesting properties, so be sure to check out the documentation.  It can talk to a standard [Tile Map Service](http://wiki.openstreetmap.org/wiki/TMS), like the ones provided by [OpenStreetMap](http://www.openstreetmap.org/).

Next up, let’s add some vector data.

[Adding Vector Data](adding_vector_data.html)
