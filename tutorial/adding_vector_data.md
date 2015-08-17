---
title: Vector Data
layout: tutorial
---

HelloEarth is looking good, but it's still a bit plain. Almost all map or globe apps have some sort of overlay which adds additional information or context. Let's add some country outlines to get a taste of what's involved when adding vector data in WhirlyGlobe­-Maply.

This tutorial depends on at least the [local image layer](local_image_layer.html) tutorial.  Go ahead and open your HelloEarth project.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/adding_vector_data_1.png)

If you haven't got one here is a suitable ViewController (for [Objective-C]({{ site.baseurl }}/tutorial/code/ViewController_adding_vector_data.m) or [Swift]({{ site.baseurl }}/tutorial/code/ViewController_adding_vector_data.swift)) file to start with.  This version is from the previous tutorial for a [remote image layer](remote_image_layer.html).

### Vector Data

We need some vector data to overlay on the globe.  Conveniently, you've already got some.  Go to the **resources** directory from earlier.  Look for *vectors/country_json_50m*.  That's a directory full of country outlines from the Natural Earth Data project.

![Country Files]({{ site.baseurl }}/images/tutorial/adding_vector_data_2.png)

You can drag all of these .geojson files into your project or just your favorites.  They're organized by country code.  What, you don't know your ISO country codes?  Fine, drag them all in.  Be sure you're adding them to the HelloEarth target so they get copied to the bundle.

![Country Files]({{ site.baseurl }}/images/tutorial/adding_vector_data_3.png)

Great, we've got files.  Now let's do something with them.

## Add the Vectors

Here's how you add those vectors to the display. Open ViewController and add a private method. In Objective-C, this is done in the ViewController.m, modifying the *@interface* section. In Swift it's just a method with *private* modifier.

{% multiple_code %}
  {% highlight objc %}
@interface ViewController ()

- (void) addCountries;

@end
  {% endhighlight %}

  {----}

  {% highlight swift %}
class ViewController: UIViewController {
   ...

   private func addCountries() {
   }

   ...
}
  {% endhighlight %}
{% endmultiple_code %}

The WG-Maply toolkit likes to specify many of its arguments as an NSDictionary (or Dictionary in Swift).  So let's add a private dictionary member, to set our default vector characteristics.

{% multiple_code %}
  {% highlight objc %}
@interface ViewController ()
...

NSDictionary *vectorDict;

@end
  {% endhighlight %}

  {----}

  {% highlight swift %}
class ViewController: UIViewController {
   ...

   private var vectorDict: [String:AnyObject]?

   ...
}
  {% endhighlight %}
{% endmultiple_code %}


Next, add some code to set the vector properties and call the addCountries method at the end of viewDidLoad.

{% multiple_code %}
  {% highlight objc %}
// set the vector characteristics to be pretty and selectable
vectorDict = @{
  kMaplyColor: [UIColor whiteColor], 
  kMaplySelectable: @(true), 
  kMaplyVecWidth: @(4.0)};

// add the countries
[self addCountries];
  {% endhighlight %}

  {----}

  {% highlight swift %}
vectorDict = [
    kMaplyColor: UIColor.whiteColor(),
    kMaplySelectable: true,
    kMaplyVecWidth: 4.0]

// add the countries
addCountries()
  {% endhighlight %}
{% endmultiple_code %}


Finally, fill in the addCountries method itself after viewDidLoad.


{% multiple_code %}
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

  {----}

  {% highlight swift %}
private func addCountries() {
   // handle this in another thread
   let queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0)
   dispatch_async(queue) {	
      let bundle = NSBundle.mainBundle()
      let allOutlines = bundle.pathsForResourcesOfType("geojson", inDirectory: nil) as! [String]

      for outline in allOutlines {
         if let jsonData = NSData(contentsOfFile: outline) {
            let wgVecObj = MaplyVectorObject(fromGeoJSON: jsonData)
            // the admin tag from the country outline geojson has the country name ­ save
            if let attrs = wgVecObj.attributes,
                   vecName = attrs.objectForKey("ADMIN") as? NSObject {
               wgVecObj.userObject = vecName
            }

            // add the outline to our view
            let compObj = self.theViewC?.addVectors([wgVecObj], desc: self.vectorDict)

            // If you ever intend to remove these, keep track of the MaplyComponentObjects above.
         }
      }
   }
}
  {% endhighlight %}
{% endmultiple_code %}

Build and run the app. You should see the outlines of the countries you included in HelloEarth.

![Country Outlines]({{ site.baseurl }}/images/tutorial/adding_vector_data_4.png)

Neat!  That's the country outlines right on top of the globe with its base layer.  But there's a lot going on here so let's unpack it.

### Breaking it Down

First up, let's look at the <a href= "https://developer.apple.com/LIBRARY/ios/documentation/Performance/Reference/GCD_libdispatch_Ref/index.html" target="_blank">dispatch_async()</a> call.  That's an asychronous request to run a block of code in another thread.

WhirlyGlobe-Maply is very thread safe and very threaded.  For the user, this means you can call all the add methods from other threads and you probably should.  Whenever you've got a bunch of work to do, like loading all these files, do it on another thread.

The code block itself is pretty simple.  We're doing the following.

- Figuring out which geojson files are in the device bundle.
- Loading them in, one by one
- Converting them from GeoJSON to a MaplyVectorObject to MaplyBaseViewController.

The view controller returns a MaplyComponentObject.  If you ever want to remove or modify these, you'll need that object.

You might notice we're pulling a string called _"ADMIN"_ out of each vector.  This will be handly later if we want to know which country it was.

Next up, let's do some vector selection.
