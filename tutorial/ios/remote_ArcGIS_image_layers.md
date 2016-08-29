---
title: Remote ArcGIS Layers
layout: ios-tutorial
---

*Tutorial developed by Chris Lamb.*

**This tutorial requires WhirlyGlobe-Maply version 2.4 or greater.**

WhirlyGlobe-Maply is an open source toolkit, part of the vibrant FOSS4G community, albeit focused on mobile.  Even so, we interface to commercial data and systems all the time.  Let's interface to the 800lb gorilla of GIS, ESRI!

### ESIR ArcGIS Data Sets

[ESRI's ArcGIS platform](http://www.esri.com/) has a huge number of freely available data sets, as well as the ability to generate your own with ESIR tools.  Here are a few examples of what we're going to build today.

![Header pic]({{ site.baseurl }}/images/tutorial/ArcGIS_Header.jpg)

You’ll need a sample project for this tutorial. Go back and start with the [Hello Earth](http://mousebird.github.io/WhirlyGlobe/tutorial/hello_earth.html) tutorial and work thru the [CartoDB Tutorial](remote_image_layer.html).  We’ll want the vector tiling logic from that one, but if you'd rather just get started with those files, you can download them here;

- [ViewController.m]({{ site.baseurl }}/tutorial/ios/code/ViewController_cartodb.m) Your main view controller.
- [CartoDBLayer.h]({{ site.baseurl }}/tutorial/ios/code/CartoDBLayer.h) CartoDBLayer header.
- [CartoDBLayer.m]({{ site.baseurl }}/tutorial/ios/code/CartoDBLayer.m) CartoDBLayer implementation.


### Hello Earth
OK to summarize, in this app we're are going to utilize remote datasets from ESRI [ArcGIS service](https://www.arcgis.com/features/).  In this app, we are going to load one of their base maps, a National Geographic style map found in their list of example REST services [here](http://services.arcgisonline.com/arcgis/rest/services).

And as a second act we are going to access one of their vector data sets showing New York City's flood zones in a list of vector services found [here](http://services.arcgis.com/OfH668nDRN7tbJh0/ArcGIS/rest/services).

We won't get into the details of how the ESRI service works.  It's huge and they cover that in detail themselves.  We'll start with the CartoDB example and modify it as need.

First up, gather the files list above, build them and you should see something like this.

![CartoDB pic]({{ site.baseurl }}/images/tutorial/CartoDB_NYCBuildings.png)

### ArcGIS Base Map - National Geographic World Map
So first thing we're going to load up one of ArcGIS's base maps, the beautiful & revered [National Geographic World Map](http://services.arcgisonline.com/arcgis/rest/services/NatGeo_World_Map/MapServer).  All we have to do change the URL reference in the existing code & the new map should appear, as if by magic.  Find the following code in viewDidLoad, and replace the URL string in the MaplyRemoteTileSource designated initializer with the URL of the ArcGIS webpage below;

{% highlight objc %}
// Portions Courtesy NASA/JPL­Caltech and U.S. Depart. of Agriculture, Farm Service Agency
MaplyRemoteTileSource *tileSource =  [[MaplyRemoteTileSource alloc]
initWithBaseURL:@"http://services.arcgisonline.com/arcgis/rest/services/NatGeo_World_Map/MapServ/tile/{z}/{y}/{x}"
ext:@"png" minZoom:0 maxZoom:maxZoom];
{% endhighlight %}

{% highlight bash %}
http://services.arcgisonline.com/arcgis/rest/services/NatGeo_World_Map/MapServer
{% endhighlight %}

you also need to adjust the maxZoom value (in this case - 17) and make sure the ext:file is png.  The /tile/{z}/{y}/{x} string appended to the end of the URL ensures proper tiling orientation.

Run the app, zoom way out, and you should get a great rendition of everyone's favorite Nat Geo globe.  Easy Peasey!

![NatGeo globe pic]({{ site.baseurl }}/images/tutorial/NatGeoGlobe.png)

### Next up - Vector Layers
Vector layers are datasets that return polygons, attributes and other things.  The mechanics of how WhirlyGlobe handles this data is thoroughly detailed in the CartoDB tutorial, and you should review that there if you like.  Here today we are simply going to replace the CartoDB data with data from ArcGIS, specifically showing the various different flood zones in the NYC area.  We'll also make a few other changes to make the displayed data pop!  Here goes;

As discussed, we have created a CartoDBLayer object that conforms to the MaplyPagingDelegate.  This object then queries the remote data source for the data required by the tiles displayed with the method startFetchForTile:forLayer:.  This query is comprised of 2 portions, a URL for the remote server, and a SQL query that are joined together in the constructRequest method.  Let's start by changing the URL to -

{% highlight bash %}
http://services.arcgis.com/OfH668nDRN7tbJh0/ArcGIS/rest/services/NYCEvacZones2013/FeatureServer
{% endhighlight %}

in the CartoDBLayer constructRequest code.

{% highlight objc %}
- (NSURLRequest *)constructRequest:(MaplyBoundingBox)bbox {
// construct a query string
double toDeg = 180/M_PI;
NSString *query = [NSString stringWithFormat:search,bbox.ll.x*toDeg,bbox.ll.y*toDeg,bbox.ur.x*toDeg,bbox.ur.y*toDeg];
NSString *encodeQuery = [query stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
encodeQuery = [encodeQuery stringByReplacingOccurrencesOfString:@"&" withString:@"%26"];
NSString *fullUrl = [NSString stringWithFormat:@"https://pluto.cartodb.com/api/v2/sql?format=GeoJSON&q=%@",encodeQuery];.
NSURLRequest *urlReq = [NSURLRequest requestWithURL:[NSURL URLWithString:fullUrl]];

return urlReq;
}
{% endhighlight %}

And we'll also need to change the query in the ViewController's addBuildings method

{% highlight objc %}
- (void)addBuildings
{
NSString *search = @"WHERE=Zone>=1&f=pgeojson&outSR=4326";
// NSString *search = @"SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;";

CartoDBLayer *cartoLayer = [[CartoDBLayer alloc] initWithSearch:search];
cartoLayer.minZoom = 13;
cartoLayer.maxZoom = 15;
{% endhighlight %}

outSR is the ouptput spacial reference, and pgeojson also defines the output format.
Run the project, and you should see the layers for the flood zones.  Not?  OK, lets adjust a few things to see what's going on;

- Change the mni/maxZoom levels = 9 to 13
- Modify the initial zoom level to 0.008
- query for a single zone>=4
- Also let's add in a NSLog statement to see if data is being returned

{% highlight objc %}
[NSURLConnection sendAsynchronousRequest:urlReq queue:opQueue completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError)
{
NSLog(@"returned data length is %lu", (unsigned long)data.length);
{% endhighlight %}

Run the project again, and you should see some data displayed.  It's not exactly what we want, but at least we know we're receiving data, Yay!  Now lets clean up this bad boy.
 
- Color individual zones
- Adjust zoom & position

Through the magic of files fiddling, we finally end up with -

![NYC flood zones pic]({{ site.baseurl }}/images/tutorial/ArcGISProgression.png)

Very pretty.

Here are the various completed files for your programming pleasure;

- [ViewController.m]({{ site.baseurl }}/tutorial/ios/code/ViewController_ArcGIS.m)
- [ArcGISLayer.h]({{ site.baseurl }}/tutorial/ios/code/ArcGISLayer.m)
- [ArcGISLayer.m]({{ site.baseurl }}/tutorial/ios/code/ArcGISLayer.m)








