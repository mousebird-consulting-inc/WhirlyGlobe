---
title: Source Distribution
layout: ios-tutorial
prev_next:
  prev: hello_earth.html
  next: globe_or_map.html
---

With open source projects, you expect to get all the source code.  Sure enough, that's what we do with WhirlyGlobe-Maply.  All of it is available on github, with a long history and references to other projects we use.

So the most open-source-y way to set yourself up is with submodules.  It’s also the slowest, but we like it.

### Setting Up With Submodules

Open a Terminal window, and change to the root directory of your project. That directory should contain HelloEarth.xcodeproj.

Create a _libs_ subdirectory and add WG-Maply in that path.

{% highlight bash %}
% mkdir libs
% git submodule add https://github.com/mousebird/WhirlyGlobe.git libs/WhirlyGlobeMaply
{% endhighlight %}

Next, we need to initialize and update WG-Maply's dependencies.

We've cut way back on these in recent years, but this part may take a while.  WG-Maply has a lot of data in its Resources submodule.

{% highlight bash %}
% cd libs/WhirlyGlobe­Maply
% git submodule init
% git submodule update
{% endhighlight %}

Once it's done we're done with Terminal. Close that window and open the HelloEarth project in Xcode.

### Adding the Dependencies

Now you need to add the WG-Maply library to the project. 

Select the HelloEarth project, then go to Build Phases, and expand Link Binary With Libraries.

![Adding the Dependent Project]({{ site.baseurl }}/images/tutorial/source_dist_1.png)

Click the + at the bottom, and then **Add Files** from the **Add Other** dropdown.

Navigate to the _libs/WhirlyGlobeMaply/ios/library/WhirlyGlobe-MaplyComponent/_ directory, and select _WhirlyGlobe­MaplyComponent.xcodeproj_. Click *Open*.

Click + again, and you will see a new option: _WhirlyGlobeMaplyComponent.framework_. Select that and click **Add**.

WG-Maply depends on a number of standard libraries as well, but you shouldn't need to add them.  It should pick those up.  If it doesn't, here they are.

{% highlight bash %}
+ CoreLocation
+ libc++
+ libz
+ libxml2
+ libsqlite3
{% endhighlight %}

We'll also need to embed the WG-Maply framework in any app bundle.  So go to the General tab for the project and select **Embed & Sign** for the _WhirlyGlobeMaplyComponent.framework_ entry under _Frameworks, Libraries, and Embedded Content_.

![Adding the Dependent Project]({{ site.baseurl }}/images/tutorial/source_dist_5.png)

Next, we need to add the path to the WG-Maply headers so Xcode knows where to find the includes. To do this, go to Build Settings and look for Header Search Paths. 

![Setting the Header Search Paths]({{ site.baseurl }}/images/tutorial/source_dist_3.png)

Click + and add the following path.

{% highlight bash %}
"$(SRCROOT)/libs/WhirlyGlobeMaply/ios/library/WhirlyGlobe-MaplyComponent/include/"
{% endhighlight %}

Last, we'll need to pull in a bridging header for Swift.  If you're not using Swift you don't need to do this.

Go to Build Settings and look for "Objective-C Bridging Header".

![Adding the Objective-C bridging header]({{ site.baseurl }}/images/tutorial/source_dist_4.png)

Type there the following path:

{% highlight bash %}
libs/WhirlyGlobeMaply/ios/library/WhirlyGlobe-MaplyComponent/include/MaplyBridge.h
{% endhighlight %}

### Bridging Objective C to Swift

If you're using Objective-C for your project, you can ignore this.  Also welcome back from your coma!

You're probably using Swift and the toolkit works well with it, but you have to do one more thing.

Open the HelloEarth workspace and create a new Objective C file.  I call mine "Stub" because it's not going to do anything.  It doesn't even matter what's in it.  The important bit is next.

Xcode will prompt you to create a Create Bridging Header.  Select that option.

![CocoaPods]({{ site.baseurl }}/images/tutorial/bridging_header.png)

Now open that header.  It should be called HelloEarth-Bridging-Header.h.  Add MaplyComponent like so.

![CocoaPods]({{ site.baseurl }}/images/tutorial/bridging_header2.png)

With that you should be ready to compile and use the toolkit in a Swift project.

At this point, your project should be set up properly and you’ll be linked to the github repo for updates.

Next up, let’s look at something!
