---
title: Remote Image Tiles
layout: tutorial
---

Note that the built­in map only has a few levels of detail. Let's add a remote tile source, and take a closer look at the Earth. We'll use the MapQuest Open Aerial tile set. (If you end up wanting to use these tiles in an app that you distribute, check out the requirements here:

http://developer.mapquest.com/web/products/open/map).

We'll set this up to use either the local or remote tiles. To do so, replace these lines:

~~~objc
// set up the layer
MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc] initWithMBTiles:@"geography­class_medres"];

MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
~~~

with this code snippet:

~~~objc
// add the capability to use the local tiles or remote tiles
bool useLocalTiles = false;

// we'll need this layer in a second
MaplyQuadImageTilesLayer *layer;

if (useLocalTiles)
{
MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc]
initWithMBTiles:@"geography­class_medres"];
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

Build and run, and play with the new HelloEarth. Now you can zoom in to your heart's content, provided your heart doesn't desire sub­meter resolution.

So what's different? Previously we were using a MaplyMBTileSource object, now we've added a MaplyRemoteTileSource alternative. Apart from some minor housekeeping such as setting up the cache, everything else remains the same. Check out the WG­Maply documentation to explore the other types of supported tile sources. (Note that currently WG-Maply caches are unlimited in size, so you'll want to implement some sort of cache management if you use remote tiles and caching in your app.)

Next up, let’s add some vector data.

[Adding Vector Data](adding_vector_data.html)
