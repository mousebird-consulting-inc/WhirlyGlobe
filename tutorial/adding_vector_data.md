---
title: Vector Data
layout: tutorial
---

HelloEarth is looking good, but it's still a bit plain. Almost all map or globe apps have some sort of overlay which adds additional information or context. Let's add some country outlines to get a taste of what's involved when adding vector data in WhirlyGlobe­-Maply.

This tutorial depends on at least the [local image layer](remote_image_layer.html) tutorial.  Go ahead and open your HelloEarth project.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/adding_vector_data_1.png)

If you haven't got one here is a suitable [ViewController.m]({{ site.baseurl }}/tutorial/code/ViewController_adding_vector_data.m) file to start with.  This version is from the previous tutorial for a [remote image layer](remote_image_layer.html).

### Vector Data

We need some vector data to overlay on the globe.  Conveniently, you've already got some.  Go to the **resources** directory from earlier.  Look for *vectors/country_json_50m*.  That's a directory full of country outlines from the Natural Earth Data project.

![Country Files]({{ site.baseurl }}/images/tutorial/adding_vector_data_2.png)

You can drag all of these .geojson files into your project or just your favorites.  They're organized by country code.  What, you don't know your ISO country codes?  Fine, drag them all in.  Be sure you're adding them to the HelloEarth target so they get copied to the bundle.

![Country Files]({{ site.baseurl }}/images/tutorial/adding_vector_data_3.png)

Great, we've got files.  Now let's do something with them.

## Add the Vectors

Here's how you add those vectors to the display. Open ViewController.m and modify the _@interface_ section to add a private method:

{% highlight objc %}
@interface ViewController ()

- (void) addCountries;

@end
{% endhighlight %}

The WG-Maply toolkit likes to specify many of its arguments as an NSDictionary.  So let's add a private NSDictionary member in the implementation section, to set our default vector characteristics.

{% highlight objc %}
NSDictionary *vectorDict;
{% endhighlight %}

Next, add some code to set the vector properties and call the addCountries method at the end of viewDidLoad.

{% highlight objc %}
// set the vector characteristics to be pretty and selectable
vectorDict = @{
  kMaplyColor: [UIColor whiteColor], 
  kMaplySelectable: @(true), 
  kMaplyVecWidth: @(4.0)};

// add the countries
[self addCountries];
{% endhighlight %}

Finally, fill in the addCountries method itself after viewDidLoad.

{% highlight objc %}
­- (void)addCountries
{
  // handle this in another thread
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0),
  ^{
     NSArray *allOutlines = [[NSBundle mainBundle] pathsForResourcesOfType:@"geojson" inDirectory:nil];

     for (NSString *outlineFile in allOutlines)
     {
       NSData *jsonData = [NSData dataWithContentsOfFile:outlineFile];
       if (jsonData)
       {
         MaplyVectorObject *wgVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:jsonData];

         // the admin tag from the country outline geojson has the country name ­ save
         NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
         wgVecObj.userObject = vecName;

         // add the outline to our view
         MaplyComponentObject *compObj = [theViewC addVectors:[NSArray arrayWithObject:wgVecObj] desc:vectorDict];
         // If you ever intend to remove these, keep track of the MaplyComponentObjects above.
       }
     }
  });
}
{% endhighlight %}

Build and run the app. You should see the outlines of the countries you included in HelloEarth.

![Country Outlines]({{ site.baseurl }}/images/tutorial/adding_vector_data_4.png)

Neat!  That's the country outlines right on top of the globe with its base layer.  But there's a lot going on here so let's unpack it.

### Breaking it Down

First up, let's look at the <a href= "https://developer.apple.com/LIBRARY/ios/documentation/Performance/Reference/GCD_libdispatch_Ref/index.html" target="_blank">dispatch_async()</a> call.  That's an asychronous request to run a block of code in another thread.

WhirlyGlobe-Maply is very thread safe and very threaded.  For the user, this means you can call all the add methods from other threads and you probably should.  Whenever you've got a bunch of work to do, like loading all these files, do it on another thread.

The code block itself is pretty simple.  We're doing the following.

- Figuring out which geojson files are in the device bundle.
- Loading them in, one by one
- Converting them from GeoJSON to a <a href= "{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyVectorObject.html" target="_blank">MaplyVectorObject</a>
- Adding the <a href= "{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyVectorObject.html" target="_blank">MaplyVectorObject</a> to <a href= "{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyBaseViewController.html" target="_blank">MaplyBaseViewController</a>.

The view controller returns a <a href= "{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyComponentObject.html" target="_blank">MaplyComponentObject</a>.  If you ever want to remove or modify these, you'll need that object.

You might notice we're pulling a string called _@"ADMIN"_ out of each vector.  This will be handly later if we want to know which country it was.

Next up, let's do some vector selection.

[Taps and Selection](taps_and_selection.html)

