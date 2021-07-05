---
title: Binary Distribution
layout: ios-tutorial
prev_next:
  prev: hello_earth.html
  next: globe_or_map.html
---

The binary distribution is one of the easiest ways to get started with WhirlyGlobe-Maply.  We distribute the toolkit as a framework for iOS device and simulator.

First, download the [binary distribution](../../builds/builds.html) and unzip it into your projects directory. It should look something like this:

{% highlight bash %}
projects/WhirlyGlobeMaplyComponent.framework
{% endhighlight %}

This guide will simply refer to it as `BinaryDirectory` for purposes of indicating paths.

You will need your HelloEarth project [from earlier](hello_earth.html) for the next part.

### Adding the Dependencies

We build WhirlyGlobe-Maply as a binary framework.  Those need to be explicitly imported into the project.

![Adding the Framework]({{ site.baseurl }}/images/tutorial/binary_dist_1.png)

To add an external library in Xcode, select the top level HelloEarth node in the project tab, click `+` at the bottom, and then `Add Files`.

Navigate to your own `BinaryDirectory` and select `WhirlyGlobeMaplyComponent.framework`, then click Open.  On the resulting item, change the Embed setting to "Embed & Sign".

Currently, you'll also need to navigate to `Build Settings`, find the `Validate Workspace` setting, and change it to `Yes`.  This works around an issue with the bundled framework.  See <a href="https://stackoverflow.com/q/65303304/135138">this StackOverflow question</a>.

We'll need to add some libraries that WG-Maply needs. Navigate to the HelloEarth target in the project settings, and in the General section click `+` to add the following.

{% highlight bash %}
+ CoreLocation
+ libz.1
+ libxml2.2
+ libsqlite3
+ libc++.1
+ libc++abi
{% endhighlight %}

![Adding the Dependent packages]({{ site.baseurl }}/images/tutorial/binary_dist_2.png)

Next, we need to indicate where to find the framework and the header files. Open up Build Settings and scroll down to `Framework Search Paths`.  Add your `BinaryDirectory` here.  This should be the level where your `WhirlyGlobeMaplyComponent.framework` directory lives.

For the header files, find the `Header Search Paths` in your Build Settings.  Add the path to your `BinaryDirectory` plus Headers, like so.

{% highlight bash %}
"BinaryDirectory/WhirlyGlobeMaplyComponent.framework/Headers/"
{% endhighlight %}

![Adding the Dependent Project]({{ site.baseurl }}/images/tutorial/binary_dist_3.png)

Last, we'll need to pull in a bridging header for Swift.  If you're not using Swift you don't need to do this.

Go to Build Settings and look for `Objective-C Bridging Header`.

![Adding the Objective-C bridging header]({{ site.baseurl }}/images/tutorial/binary_dist_4.png)

Type there the following path:

{% highlight bash %}
BinaryDirectory/WhirlyGlobe-MaplyComponent/Headers/MaplyBridge.h
{% endhighlight %}

Don’t forget to replace `BinaryDirectory` with your own.

Try and compile, just to make sure the basics are working.  But that’s it!  You should be set up properly for a project using WG-Maply.

Next up, let's look at building a map or a globe.
