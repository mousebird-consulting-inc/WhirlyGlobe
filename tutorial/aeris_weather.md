---
title: Aeris Weather Layer
layout: tutorial
---
[Aeris Weather](http://www.aerisweather.com/) is a provider of weather content like local forecasts and time dependent radar overlays.  In this tutorial we add an animated layer of up to date weather imagery from Aeris to WhirlyGlobe-Maply.

<div class="well">
<h4><span class="label label-warning" style="margin-right:10px">Note</span>Version 2.4.1 (or better) Required</h4>

<p>
You will need WhirlyGlobe-Maply version 2.4.1 or better to use the new MaplyAeris classes.
</p>

<p>
Version 2.4.1 (or greater) can be found in the github repo or in this <a href="https://dl.dropboxusercontent.com/u/29069465/WhirlyGlobeMaplyComponent.framework_2_4_1_beta8.zip">binary distribution</a>.
</p>
</div>


You will need to have already run through the [remote image layer](remote_image_layer.html) tutorial.  Let's get started by opening your HelloEarth project.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/aeris_weather_1.png)

If you haven't got one here is a suitable [ViewController.m]({{ site.baseurl }}/tutorial/code/ViewController_aeris_weather.m) file to start with.  This version is from the previous tutorial for a [remote image layer](remote_image_layer.html).

## Setup the Aeris layer

Let's get started by adding five member variables to the ViewController implementation block.  We'll explain what these do later.

{% highlight objc %}
@implementation ViewController
{
    MaplyBaseViewController *theViewC;
    WhirlyGlobeViewController *globeViewC;
    MaplyViewController *mapViewC;

    // New member variables
    int frameCount;
    float animationPeriod;
    float importanceScale;
    NSString *aerisID;
    NSString *aerisKey;
}
{% endhighlight %}

Add a method call as the final line in viewDidLoad.

{% highlight objc %}
    [self setupAerisOverlayLayer];
{% endhighlight %}

And begin implementing it as follows:

{% highlight objc %}
- (void)setupAerisOverlayLayer {
    aerisID = @"2kDDnD7Q1XFfFm4CwH17C";
    aerisKey = @"FQmadjUccN3CnB4KG6kKeurUpxHSKM0xbCd6TlVi";
    frameCount = 6;
    animationPeriod = 3.0;
    importanceScale = 1.0/16.0;
}
{% endhighlight %}

Aeris provides quite a few time slices in their data layers, more than we can easily display.  The *frameCount* member controls how many we will show.  The *animationPeriod* is how long we'll take to run through the whole animation and the *importanceScale* controls how much data we'll load relative to the underlying map.

## WhirlyGlobe-Maply Aeris classes

There are three WhirlyGlobe-Maply Aeris-related classes needed to make animated Aeris weather happen.  **MaplyAerisTiles** provides a list of available Aeris layers.  The **MaplyAerisLayerInfo** class contains the relevant information for a particular layer.  And **MaplyAerisTileSet** provides the tile sources to add to a WhirlyGlobe-Maply layer.

Go back to the beginning of the implementation block, and add four more member variables here.  We'll need these to keep track of the Aeris layers.

{% highlight objc %}
MaplyQuadImageTilesLayer *aerisLayer;
MaplyAerisLayerInfo *layerInfo;
MaplyAerisTileSet *layerTileSet;
NSTimer *aerisRefreshTimer;
{% endhighlight %}

Now add this to setupAerisOverlayLayer:

{% highlight objc %}
NSString *layerCode = @"radar";
MaplyAerisTiles *aerisTiles = [[MaplyAerisTiles alloc] initWithAerisID:aerisID secretKey:aerisKey];
NSDictionary *layerInfoDict = [aerisTiles layerInfo];
layerInfo = layerInfoDict[layerCode];

if (!layerInfo) {
    NSLog(@"Error finding aeris radar layer parameters.");
    return;
}

layerTileSet = [[MaplyAerisTileSet alloc] initWithAerisID:aerisID secretKey:aerisKey layerInfo:layerInfo tileSetCount:frameCount];

[self refreshAerisOverlayLayer];
{% endhighlight %}

That gets the **MaplyAerisLayerInfo** object for the radar layer, and constructs a **MaplyAerisTileSet** object for it.  That tells us where to get the actual data from, but we'll need to do one more thing to find out how many time steps are available and where they are.

{% highlight objc %}
- (void)refreshAerisOverlayLayer {    
    [layerTileSet startFetchWithSuccess:^(NSArray *tileSources) {
        
        MaplyMultiplexTileSource *multiSource = [[MaplyMultiplexTileSource alloc] initWithSources:tileSources];
        
        if (aerisLayer)
            [theViewC removeLayer:aerisLayer];
        
        aerisLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:multiSource.coordSys tileSource:multiSource];
        aerisLayer.imageDepth = frameCount;
        aerisLayer.animationPeriod = animationPeriod;
        aerisLayer.imageFormat = MaplyImageUShort5551;
        aerisLayer.drawPriority = 1000;
        aerisLayer.maxTiles = kMaplyImageLayerDrawPriorityDefault+100;
        aerisLayer.importanceScale = 1.0/16.0;
        
        [theViewC addLayer:aerisLayer];
        
    } failure:^(NSError *error) {
    }];
}
{% endhighlight %}

This *refreshAerisOverlayLayer* method will query the Aeris service to figure out the end points for the timesteps in the data layer, radar in this case.  When it gets that information back it will put together a **MaplyMultiplexTileSource** which is just a tile source that deals with animated data sets.  From there it sets up a layer to fetch and draw the data and off it goes.

![Aeris Radar layer]({{ site.baseurl }}/images/tutorial/aeris_weather_2.png)

## Staying current with a periodic refresh

You'll notice that in the *refreshAerisOverlayLayer* method we first remove the aerisLayer from the view if it already exists.  This anticipates our final change, which is to periodically refresh the imagery to get the latest data.

At the end of the setupAerisOverlayLayer method, schedule a timer to refresh the imagery.

{% highlight objc %}
    aerisRefreshTimer = [NSTimer scheduledTimerWithTimeInterval:300.0 target:self selector:@selector(refreshAerisOverlayLayer) userInfo:nil repeats:YES];
{% endhighlight %}

### More Data Layers

The other layers available from the WhirlyGlobe-Maply Aeris classes are Infrared Satellite, Global Satellite, and Hi-Res Visible Satellite.  For a Global Satellite example, substitute the following assignments in setupAerisOverlayLayer:

{% highlight objc %}
//    NSString *layerCode = @"radar";
//    frameCount = 6;
//    animationPeriod = 3.0;
//    importanceScale = 1.0/16.0;
    NSString *layerCode = @"sat-global";
    frameCount = 1;
    animationPeriod = 5.0;
    importanceScale = 1.0/4.0;
{% endhighlight %}

![Aeris Radar layer]({{ site.baseurl }}/images/tutorial/aeris_weather_3.png)


