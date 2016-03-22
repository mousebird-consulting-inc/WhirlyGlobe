---
title: Map Features for Mobile Devices using WhirlyGlobe-Maply
layout: default
---

We're going to discuss some map related features of the [WhirlyGlobe-Maply toolkit](http://mousebird.github.io/WhirlyGlobe).  The toolkit is an open source, mobile centric geospatial display toolkit.  It’s used in a variety of weather, aviation, geography and map apps.

The company that makes WhirlyGlobe-Maply, mousebird consulting inc, is a member of LocationTech.  The toolkit itself predates LocationTech and so is not a formal LocationTech project.

Two of the more prominent examples are [Dark Sky](https://itunes.apple.com/us/app/dark-sky-weather-radar-hyperlocal/id517329357?mt=8), a weather predication app and [National Geographic World Atlas](https://itunes.apple.com/us/app/national-geographic-world/id364733950?mt=8), which is exactly what it sounds like.

![NatGeo World Atlas & Dark Sky]({{ site.baseurl }}/locationtech/images/natgeodarksky.jpg)

The globe based apps are best known, butWhirlyGlobe-Maply has significant support for traditional 2D map apps as well.  Let's look at some of the basic features.

### Map Apps

Map apps are one of the more popular categories in the app stores.  Both [Apple](https://developer.apple.com/library/ios/documentation/MapKit/Reference/MapKit_Framework_Reference/) and [Google](https://developers.google.com/maps/documentation/android-api/) have their own map offerings and toolkits, but what if you want to display your own data?  That’s where toolkits like WhirlyGlobe-Maply come in and we’ve built a number of features specifically for 2D map apps.

One of the more capable map apps that uses the toolkit is [Gaia GPS](https://www.gaiagps.com).  

![Gaia GPS for iOS]({{ site.baseurl }}/locationtech/images/gaiagps.jpeg)

Gaia GPS is map app that displays a variety of image and vector based data sources.  It’s popular among the backpacking and off roading communities.  As such, it can work without a network and is very careful with battery usage.

Let's look at one of the key features for nearly ever map app:  Image Tiles.

### Image Tiles

One of the more basic features required for map display is loading and rendering of image tiles.  We’ve moved well beyond [Web Map Service](http://www.opengeospatial.org/standards/wms) on mobile devices and we now expect tiles.  This might be a [Web Map Tile Service (WMTS)](http://www.opengeospatial.org/standards/wmts), but is more likely to be an ad hoc quad tree of tiles like that coming from [Google Maps](https://www.google.com/maps) or [OpenStreetMap](https://www.openstreetmap.org).

![Mapbox Satellite]({{ site.baseurl }}/locationtech/images/multiload.gif)

Efficient image tile loading is a tricky problem.  Mobile devices have network constraints and you want to make careful use of data caching.  Our toolkit can handle this for both globe and map, but there’s an interesting variant for the map case.

![Loading by the numbers]({{ site.baseurl }}/locationtech/images/multiloadnumbers.gif)

What’s going on in the figures is this.  We’re loading three levels of tiles based on the current window.  We start with an extremely low resolution tile, then overlay that with slightly higher resolution and finally toss in the *proper* resolution based on the current window.  It gives the user something to look at while the system loads the necessary data.

Tricks like these are fairly important to efficient map display on mobile, but let’s take a quick detour through some really basic functionality.

### Vectors, Labels and Markers

Any credible map toolkit should be able to display a basic set of features and WhirlyGlobe-Maply is no exception.  These include vectors for areal and linear features, labels for points and of course markers.

![Vectors + Labels + Markers]({{ site.baseurl }}/locationtech/images/vectorlabelmarker.jpg)

Those are all fairly simple to use.  Vectors, for example can be consumed from a variety of data sources, like [GeoJSON](http://geojson.org/) or (ESRI Shapefile)[https://en.wikipedia.org/wiki/Shapefile].  Labels and markers are 2D features and can interact through an adaptive layout engine.

It’s what the developers do with these features that gets interesting. That leads us to a complex modern feature: vector tiles.

### Vector Tiles

[Mapbox](https://www.mapbox.com/) has popularized [vector tiles](https://www.mapbox.com/developers/vector-tiles/) with their OpenStreetMap data.  Theirs is a [Google Protobuf](https://github.com/google/protobuf) based format, which is as complex as it sounds.  The fundamental ideas are somewhat simpler.

Vector tiles are to image tiles as vectors are to images.  They are just chopped up geometry representing what’s in each map tile.  You can use them much more flexibly than images.

![National Geographic World Atlas]({{ site.baseurl }}/locationtech/images/natgeo.jpg)

Believe it or not, National Geographic World Atlas uses vector tiles hosted by Mapbox. 

Rather than use any of the traditional styling support we built something completely custom to get that class NatGeo look.  That's a very custom example and it's technically on the globe.  But if National Geographic doesn't have some cartographic street cred, nothing does.  Here's a strictly map example.

![Gaia GPS Contour lines]({{ site.baseurl }}/locationtech/images/gaiagpsweb.jpg)

The Gaia GPS developers used vector tiles they generated themselves to represent contour lines.  They’re styled using [Mapnik XML](https://github.com/mapnik/mapnik/wiki/XMLConfigReference), which is the styling format for the [Mapnik](http://mapnik.org/) map renderer.  Of course, they’re not using the Mapnik renderer in this case.  Open source map production has gotten very interesting.

WhirlyGlobe-Maply has vector tile support down quite solid.  However, style support is still in flux and likely to be for some time.  We can support older style formats like Mapnik XML and, of course, developers can go off on their own.  However, newer formats like [Mapbox GL Style Sheets](https://www.mapbox.com/mapbox-gl-style-spec/) are not fully supported.  We also hope to add in [Styled Layer Descriptor](http://www.opengeospatial.org/standards/sld) support in the future.

Vector Tiles are a bit trippy so let’s go back to some more basic map features.

### Marker Clustering

Clustering markers is a pretty simple idea.  When you have a lot of markers on your map, it can look bad.  Best to gather them up into groups and display those groups.

![Marker Clustering]({{ site.baseurl }}/locationtech/images/clustercompare.jpg)

Of course our maps move so we have to do something more.  As the user moves in, the groups must come apart and as they move out, the groups must form.  It’s tricky to implement, but looks quite intuitive.

![Marker Clustering Animated]({{ site.baseurl }}/locationtech/images/clusteringanimation.gif)

In truth, clustering can be used on both the globe and the map even it’s nominally a map feature.  Let’s look at something purely from the 2D map.

### Infinite Scrolling

For certain types of map projections, the extreme west and east extents represent the same points. This means the user expects to move the map and have it wrap around.

![Gaia GPS for iOS]({{ site.baseurl }}/locationtech/images/infinite_scroll.gif)

For bonus points: Why can't you wrap top to bottom?

Infinite scrolling is simple in concept, but tricky to implement.  The actual rendering isn’t so bad, but when you add in feature selection and overlaid data it gets interesting.  You know what else is interesting?  Map projections.

### Map Projections

The real test of a map toolkit is whether it handles explicit map projections!  Actually, that’s not true.  Plenty of map toolkits only work in a simple map projection, known as web mercator.  But if you want to hold your head high in a room full of cartographers, you should have map projection support.

![Gaia GPS for iOS]({{ site.baseurl }}/locationtech/images/stamensphericalmercator.jpg)

[Web mercator](https://en.wikipedia.org/wiki/Web_Mercator), shown here, is the most common projection in use on mobile.  It has its problems, particularly near the poles.  We can thank Google Maps’ domination of the industry for this one, but there is data in plenty of other projections and good reason to use them.

![British National Grid]({{ site.baseurl }}/locationtech/images/androidbng.jpg)

From left to right we've got.

* A data source in [British National Grid](https://en.wikipedia.org/wiki/PROJ.4) overlaid on a web mercator map.
* The same data source overlaid on a British National Grid map.  The web mercator data source is being reprojected.
* The same thing on a globe.  Because it looks cool.

Though the vast majority of our users will always use web mercator, WhirlyGlobe-Maply can do more.  It uses the [Proj.4](https://en.wikipedia.org/wiki/PROJ.4) coordinate system internally and can handle a variety of useful map projections.


## Conclusions

Map toolkits for mobile devices are hot right now.  There are a number of excellent toolkits, though most are tied to specific data services like Apple or Google.  A few are even open source, like WhirlyGlobe-Maply.

We've discussed a few basic features and a few advanced ones we consider essential for map display on mobile.  If you're looking to build an app, we hope you'll look beyond the proprietary services to something more open, like [WhirlyGlobe-Maply](http://mousebird.github.io/WhirlyGlobe).

Special thanks to Stamen for the [Stamen Watercolor](http://maps.stamen.com/watercolor) map tiles, derived from OpenStreetMap data used in many of these examples.
