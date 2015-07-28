---
title: Binary Distribution
layout: tutorial
prev_next:
  prev: hello_earth.html
  next: globe_or_map.html
---

The binary distribution is one of the easiest ways to get started with WhirlyGlobe-Maply.  We distribute the toolkit as a framework for iOS device and simulator.

First, download the [binary distribution](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe_Maply_Distribution_2_3.zip) and unzip it into your projects directory. It should look something like this:

{% highlight bash %}
projects/WhirlyGlobe_Maply_Distribution_2_3
{% endhighlight %}

This guide will simply refer to it as **BinaryDirectory** for purposes of indicating paths.

You will need your HelloEarth project [from earlier](hello_earth.html) for the next part.

### Adding the Dependencies

To add an external library in Xcode, select the top level HelloEarth node in the project tab, pick **Build Phases** on the right and open the **Link Binary With Libraries** area.

![Adding the Framework]({{ site.baseurl }}/images/tutorial/binary_dist_1.png)

Click on the + and select **Add Other**.  Navigate to **BinaryDirectory** and select WhirlyGlobeMaplyComponent.framework., then click Open.

Finally, we'll need to add some libraries that WG-Maply needs. Click + again and add the following.

{% highlight bash %}
+ CoreLocation
+ libc++
+ libz
+ libxml2
+ libsqlite3
{% endhighlight %}

After you’ve put all those in it should look like this.

![Libraries]({{ site.baseurl }}/images/tutorial/binary_dist_2.png)

The last thing we need to do is indicate where to search for WG-Maply header files. Open up Build Settings and scroll down to Header Search Paths. Add the path as follows.  Don’t forget to replace **BinaryDirectory** with your own.

{% highlight bash %}
“BinaryDirectory/WhirlyGlobeMaplyComponent.framework/Headers/“
{% endhighlight %}

### Bridging header for Swift

Maybe you're a lucky guy and you're doing your iOS project using Swift. In such case, you need one final step. Go to Build Settings and look for "Objective-C Bridging Header".

![Adding the Objective-C bridging header]({{ site.baseurl }}/images/tutorial/source_dist_4.png)

Type there the following path:

{% highlight bash %}
$(SRCROOT)/libs/WhirlyGlobeMaply/WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/include/MaplyBridge.h
{% endhighlight %}


Try and compile, just to make sure the basic are working.  But that’s it!  You should be set up properly for a project using WG-Maply.

Next up, let's look at building a map or a globe.
