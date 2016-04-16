---
title: Aeris Weather Layer
layout: tutorial
---
Aeris Weather is a provider of weather content like local forecasts and weather imagery.  In this tutorial we add an animated layer of current weather imagery from Aeris to WhirlyGlobe-Maply.

This tutorial depends on at least the [local image layer](local_image_layer.html) tutorial.  At this time, open your HelloEarth project.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/aeris_weather_1.png)

If you haven't got one here is a suitable [ViewController.m]({{ site.baseurl }}/tutorial/code/ViewController_aeris_weather.m) file to start with.  This version is from the previous tutorial for a [remote image layer](remote_image_layer.html).

## Setup the Aeris layer

Start by adding five member variables to the ViewController implementation block.

{% highlight objc %}
@implementation ViewController
{
    MaplyBaseViewController *theViewC;
    WhirlyGlobeViewController *globeViewC;
    MaplyViewController *mapViewC;

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

## WhirlyGlobe-Maply Aeris classes

There are three WhirlyGlobe-Maply Aeris-related classes needed to make animated Aeris weather happen.  MaplyAerisTiles provides a list of available Aeris layers.  The MaplyAerisLayerInfo class contains the relevant information for a particular layer.  And MaplyAerisTileSet provides the tile sources to add to a WhirlyGlobe-Maply layer.

Go back to the beginning of the implementation block, and add four more member variables here.  (The __block keyword is explained later).

{% highlight objc %}
    __block MaplyQuadImageTilesLayer *aerisLayer;
    MaplyAerisLayerInfo *layerInfo;
    MaplyAerisTileSet *layerTileSet;
    NSTimer *aerisRefreshTimer;
}
{% endhighlight %}

Now add to setupAerisOverlayLayer:

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
}
{% endhighlight %}

This gets the MaplyAerisLayerInfo object for the radar layer, and constructs a MaplyAerisTileSet object for it.  In the refreshAerisOverlayLayer method, we will use that MaplyAerisTileSet object to feed an imagery layer.

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
        aerisLayer.maxTiles = 256;
        aerisLayer.importanceScale = 1.0/16.0;
        
        [theViewC addLayer:aerisLayer];
        
    } failure:^(NSError *error) {
    }];
}
{% endhighlight %}

The startFetchWithSuccess:failure: method of MaplyAerisTileSet takes blocks as arguments, because fetching the current frame information from Aeris is asynchronous.  In the success block, we have the code that adds the resulting tile sources to a MaplyQuadImageTilesLayer, which shows the animated weather imagery on the globe or map.

![Aeris Radar layer]({{ site.baseurl }}/images/tutorial/aeris_weather_2.png)

## Staying current with a periodic refresh

You'll notice that in the refreshAerisOverlayLayer method we first remove the aerisLayer from the view if it already exists.  This anticipates our final change, which is to periodically refresh the imagery to get the latest data.

At the end of the setupAerisOverlayLayer method, schedule a timer to refresh the imagery.

{% highlight objc %}
    aerisRefreshTimer = [NSTimer scheduledTimerWithTimeInterval:300.0 target:self selector:@selector(refreshAerisOverlayLayer) userInfo:nil repeats:YES];
{% endhighlight %}

### Code Details

The __block keyword in the MaplyQuadImageTilesLayer variable declaration ensures that this variable can be assigned from within the block in refreshAerisOverlayLayer.  Without it, the layer variable would only be available as a copy, and assignment wouldn't persist beyond the end of the block.

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


