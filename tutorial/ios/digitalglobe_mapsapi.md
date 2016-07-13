---
title: DigitalGlobe Maps API
layout: ios-tutorial
---
*Tutorial by Ranen Ghosh*

DigitalGlobe provides [Maps API image layers](http://mapsapidocs.digitalglobe.com/) via Mapbox, including satellite imagery layers and general reference basemap layers.  Integrating these layers into the map toolkit is almost effortless.

You'll need to have done the [globe](your_first_globe.html) or [map](your_first_map.html) tutorials, either is fine.  Open your HelloEarth project and get ready.

If you haven't got one here is a suitable ViewController (for [Objective-C]({{ site.baseurl }}/tutorial/code/ViewController_globe_and_map.m) or [Swift]({{ site.baseurl }}/tutorial/code/ViewController_globe_and_map.swift)) file to start with.  This version handles both a globe and a map and makes a nice starting point.

### Maps API imagery and basemaps

Maps API provides several beautiful and high-resolution map layers:

| Name | Map ID | Description |
| ---- | ------ | ----------- |
| Recent Imagery | digitalglobe.nal0g75k | A "curated snapshot" of satellite imagery; the most recent imagery available. |
| Vivid Imagery | digitalglobe.n6ngnadl | Gorgeous and seamless satellite imagery, at 50cm resolution. |
| Street Map | digitalglobe.nako6329 | An enhanced road basemap based on OpenStreetMap. Curated by MapBox. |
| Terrain Map | digitalglobe.nako1fhg | Terrain shading and contours with OpenStreetMap reference vectors for context. Curated by MapBox. |
| Transparent Vectors | digitalglobe.nakolk5j | A street map with a transparent background. Based on OpenStreetMap; curated and enhanced by MapBox. |
| Recent Imagery with Streets | digitalglobe.nal0mpda | Hybrid map combining Recent Imagery with streets |
| Vivid Imagery with Streets | digitalglobe.n6nhclo2 | Hybrid map combining Vivid Imagery with streets |

The Map ID is used to request the desired map from the map server.

### Basic Map or Globe Setup

In this tutorial we'll setup the basic map or globe, and then add the desired DigitalGlobe map layer.

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

Also, add this additional setup for the map / globe to the end of the viewDidLoad method:

{% multiple_code %}
  {% highlight objc %}
// we want a black background for a globe, a white background for a map.
theViewC.clearColor = (globeViewC != nil) ? [UIColor blackColor] : [UIColor whiteColor];

// and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
theViewC.frameInterval = 2;

// start up over San Francisco
if (globeViewC != nil)
{
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

// start up over San Francisco
if let globeViewC = globeViewC {
    globeViewC.height = 0.8
    globeViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1.0)
}
else if let mapViewC = mapViewC {
    mapViewC.height = 1.0
    mapViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1.0)
}
  {% endhighlight %}
{% endmultiple_code %}

### Adding Maps API layers to WhirlyGlobe

Here is where we add the DigitalGlobe map layer to the map / globe.  In this example we'll use Vivid Imagery with Streets.  We create a **MaplyRemoteTileSource**, feed it to a **MaplyQuadImageTilesLayer**, and set a few attributes, and that's it.

{% multiple_code %}
  {% highlight objc %}
NSString *mapID = @"digitalglobe.n6nhclo2"; // Map ID for Vivid Imagery with Streets
NSString *accessToken = @"YOUR ACCESS TOKEN GOES HERE";

NSString *baseURL = [NSString stringWithFormat:@"https://api.tiles.mapbox.com/v4/%@/{z}/{x}/{y}.png?access_token=%@", mapID, accessToken];
MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:baseURL ext:nil minZoom:1 maxZoom:22];
NSString *baseCacheDir = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)[0];
tileSource.cacheDir = [NSString stringWithFormat:@"%@/%@", baseCacheDir, mapID];

imageLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
imageLayer.drawPriority = kMaplyImageLayerDrawPriorityDefault + 100;
imageLayer.importanceScale = 0.25;
[theViewC addLayer:imageLayer];
  {% endhighlight %}

  {----}

  {% highlight swift %}
let mapID = "digitalglobe.n6nhclo2"
let accessToken = "YOUR ACCESS TOKEN GOES HERE"

let baseURL = "https://api.tiles.mapbox.com/v4/\(mapID)/{z}/{x}/{y}.png?access_token=\(accessToken)"
let tileSource = MaplyRemoteTileSource(baseURL: baseURL, ext: nil, minZoom: 1, maxZoom: 22)
let baseCacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as String
tileSource!.cacheDir = "\(baseCacheDir)/\(mapID)/"

imageLayer = MaplyQuadImageTilesLayer(coordSystem: tileSource!.coordSys, tileSource: tileSource!)
imageLayer.drawPriority = kMaplyImageLayerDrawPriorityDefault + 100
imageLayer.importanceScale = 0.25
theViewC!.addLayer(imageLayer)
  {% endhighlight %}
{% endmultiple_code %}

This is what the Vivid Imagery with Streets layer looks like over Anchorage, Alaska.

![Vivid Imagery with Streets over Anchorage]({{ site.baseurl }}/images/tutorial/digitalglobe_mapsapi_1.png)


To view a different layer, simply substitute the desired Map ID.  Here we'll try the Terrain Map layer.

{% multiple_code %}
  {% highlight objc %}
NSString *mapID = @"digitalglobe.nako1fhg";
  {% endhighlight %}

  {----}

  {% highlight swift %}
let mapID = "digitalglobe.nako1fhg"
  {% endhighlight %}
{% endmultiple_code %}

Here's the terrain over Grand Teton National Park in Wyoming.

![Grand Teton National Park in the Terrain Map layer]({{ site.baseurl }}/images/tutorial/digitalglobe_mapsapi_2.png)

### The Transparent Vectors Layer

As a final example, we'll demonstrate a use of DigitalGlobe's Transparent Vectors layer by overlaying it onto a color relief topographic map.  Download [the topographic map image]({{ site.baseurl }}/tutorial/resources/digitalglobe_mapsapi/sm_dem_color_relief.png) and [the legend image]({{ site.baseurl }}/tutorial/resources/digitalglobe_mapsapi/sm_dem_legend.png), and add them to your project.

Change the Map ID:

{% multiple_code %}
  {% highlight objc %}
NSString *mapID = @"digitalglobe.nakolk5j";
  {% endhighlight %}

  {----}

  {% highlight swift %}
let mapID = "digitalglobe.nakolk5j"
  {% endhighlight %}
{% endmultiple_code %}

Now add the topo map and the legend.

{% multiple_code %}
  {% highlight objc %}
MaplySticker *sticker = [[MaplySticker alloc] init];
sticker.ll = MaplyCoordinateMakeWithDegrees(-122.52, 37.7);
sticker.ur = MaplyCoordinateMakeWithDegrees(-122.35, 37.82);
sticker.image = [UIImage imageNamed:@"sf_dem_color_relief"];
[theViewC addStickers:@[sticker] desc:@{kMaplyDrawPriority: @(kMaplyImageLayerDrawPriorityDefault)}];

UIImage *legendImage = [UIImage imageNamed:@"sf_dem_legend"];
UIImageView *legend = [[UIImageView alloc] initWithImage:legendImage];
legend.frame = CGRectMake(20, 20, legendImage.size.width, legendImage.size.height);
[theViewC.view addSubview:legend];
  {% endhighlight %}

  {----}

  {% highlight swift %}
let sticker = MaplySticker()
sticker.ll = MaplyCoordinateMakeWithDegrees(-122.52, 37.7)
sticker.ur = MaplyCoordinateMakeWithDegrees(-122.35, 37.82)
sticker.image = UIImage(named:"sf_dem_color_relief")
theViewC!.addStickers([sticker], desc: [kMaplyDrawPriority : Int(kMaplyImageLayerDrawPriorityDefault)])

let legendImage = UIImage(named:"sf_dem_legend")
let legend = UIImageView(image: legendImage)
legend.frame = CGRectMake(20, 20, legendImage!.size.width, legendImage!.size.height)
theViewC!.view.addSubview(legend)
  {% endhighlight %}
{% endmultiple_code %}

Finally, lower the view height to zoom in on San Francisco at start-up.

{% multiple_code %}
  {% highlight objc %}
if (globeViewC != nil)
{
    globeViewC.height = 0.002;
    [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192,37.7793)
                             time:1.0];
} else {
    mapViewC.height = 0.002;
    [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192,37.7793)
                           time:1.0];
}
  {% endhighlight %}

  {----}

  {% highlight swift %}
if let globeViewC = globeViewC {
    globeViewC.height = 0.002
    globeViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1.0)
}
else if let mapViewC = mapViewC {
    mapViewC.height = 0.002
    mapViewC.animateToPosition(MaplyCoordinateMakeWithDegrees(-122.4192,37.7793), time: 1.0)
}
  {% endhighlight %}
{% endmultiple_code %}

The result shows the ground elevation along every street in San Francisco.

![]({{ site.baseurl }}/images/tutorial/digitalglobe_mapsapi_3.png)




