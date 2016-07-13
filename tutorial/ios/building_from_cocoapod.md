---
title: Cocoapods
layout: ios-tutorial
prev_next:
  prev: hello_earth.html
  next: globe_or_map.html
---

_<a href= "http://cocoapods.org/" target="_blank">CocoaPods</a> is the dependency manager for Objective-C projects. It has thousands of libraries and can help you scale your projects elegantly._  According to their web site, anyway.  Sounds good to us.

![CocoaPods]({{ site.baseurl }}/images/tutorial/cocoapodslogo.gif)

For this tutorial you'll need the [HelloEarth project]({{ site.baseurl }}/tutorial/hello_earth.html) you created in the last one.  Don't run through the binary and source distribution tutorials.  Cocoapods is an alternative to those.

### WhirlyGlobe-Maply Pod

All credit goes to <a href= "https://github.com/jcollas" target="_blank">Juan Collas</a> who put together the latest pod (and the old one).  We contributed... absolutely nothing.  Now that's what we call open source!

The pod is named WhirlyGlobe, but Maply is in there too.  If you go looking for it on cocoapods.org you'll see this.

![CocoaPods]({{ site.baseurl }}/images/tutorial/whirlyglobecocoapod.png)

### Using the Pod

Pods are pretty easy to use, which is kind of the point.  If you haven't installed the pod gem, go <a href= "http://guides.cocoapods.org/using/getting-started.html#getting-started" target="_blank">follow their instructions</a>.  You'll need that software to do the rest of this.

Every project you use Cocoapods in is going to need a Podfile.  Yours should look like this.

{% highlight bash %}
platform :ios, '8.1'

inhibit_all_warnings!

xcodeproj 'HelloEarth.xcodeproj'

target 'HelloEarth'
	pod 'WhirlyGlobe', '2.4'
	pod 'WhirlyGlobeResources'
end
{% endhighlight %}

Make sure you've closed the HelloEarth project in Xcode.  The install process is going to mess with it.  

The next step is to set up the dependencies with the podspec.  You do that in the Terminal like so.  Navigate to the directory containing your HelloEarth.xcodeproj.

{% highlight bash %}
% pod install
{% endhighlight %}

If all goes well it should say something like this.

{% highlight bash %}
Analyzing dependencies

Downloading dependencies
Installing AFNetworking (2.4.1)
Installing FMDB (2.4)
Installing GoogleProtobuf (2.5.0)
Installing KissXML (5.0)
Installing SMCalloutView (2.0.3)
Installing WhirlyGlobe (2.3)
Installing WhirlyGlobeResources (2.3)
Installing boost (1.51.0a)
Installing clipper (6.1.3a)
Installing eigen (3.1.2)
Installing glues (1.5)
Installing libjson (7.6.1)
Installing proj4 (4.8.0)
Installing shapelib (1.3)
Installing tinyxml (2.1.0)
Generating Pods project
Integrating client project
{% endhighlight %}

We love our fellow open source projects.  That's why we include so many of them!

Up next, let's compile the thing.

### Compiling With the Pod

You'll need to load the HelloEarth workspace rather than the project.  Cocoapods sets up its dependencies in there.  If it fails to compile, odds are you opened the project.  We do it all the time.

![CocoaPods]({{ site.baseurl }}/images/tutorial/cocoapods_1.png)

Just build it.  It should scamper off and do the Cocoapods portion first, consisting of all those dependent projects.  Then it'll get to your code.

If it builds, you're golden.  All the source code you need is in there, including all the projects WhirlyGlobe-Maply uses.  You can even check all this junk in.  There's a whole body of thought on that, which we won't get into here.

Go forth and do the rest of the tutorials!
