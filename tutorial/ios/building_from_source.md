---
title: Source Distribution
layout: tutorial
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

Next, we need to initialize and update WG-Maply's dependencies.  We heard you liked submodules, so we put submodules in your submodules.  _No, we’re not proud of that joke._

This part will take a while.  WG-Maply uses a bunch of other projects and it has a lot of data in its Resources submodule.

{% highlight bash %}
% cd libs/WhirlyGlobe­Maply
% git submodule init
% git submodule update
{% endhighlight %}

Once it's done we're done with Terminal. Close that window and open the HelloEarth project in Xcode.

<div class="well">
<h4><span class="label label-warning" style="margin-right:10px">Note</span>Upgrading to 2.3</h4>

<p>
  In some rare cases you may have problems upgrading an existing installation to 2.3 or… other times this happens. Hey, git is a dark art.
</p>

<p>
  If you get an error message when doing the submodule init, try running the <i>update_from_2_2_to_2_3.sh</i> script.  It cleans out the submodules and tries again, basically.
</p>
</div>

### Adding the Dependencies

Now you need to add the WG-Maply library to the project, along with a number of others upon which WG-Maply depends. 

Select the HelloEarth project, then go to Build Phases, and expand Link Binary With Libraries.

![Adding the Dependent Project]({{ site.baseurl }}/images/tutorial/source_dist_1.png)

Click the + at the bottom, and then Add Other.

Navigate to the _libs/WhirlyGlobeMaply/WhirlyGlobeSrc/WhirlyGlobe-­MaplyComponent/_ directory, and select _WhirlyGlobe­MaplyComponent.xcodeproj_. Click *Open*.  If you are prompted to share a working copy, **no** is fine.

Click + again, and you will see a new option: _libWhirlyGlobe­MaplyComponent.a_. Select that and click **Add**. You'll need to and add the following libraries as well:

{% highlight bash %}
+ CoreLocation
+ libc++
+ libz
+ libxml2
+ libsqlite3
{% endhighlight %}

Once you have all those libraries added, it should look like this.

![Adding the Dependent Project]({{ site.baseurl }}/images/tutorial/source_dist_2.png)

Finally, we need to add the path to the WG-Maply headers so Xcode knows where to find the includes. To do this, go to Build Settings and look for Header Search Paths. 

![Adding the Dependent Project]({{ site.baseurl }}/images/tutorial/source_dist_3.png)

Click + and add the following path.

{% highlight bash %}
"$(SRCROOT)/libs/WhirlyGlobeMaply/WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/include/"
{% endhighlight %}

### Bridging header for Swift

Maybe you're a lucky guy and you're doing your iOS project using Swift. In such case, you need one final step. Go to Build Settings and look for "Objective-C Bridging Header".

![Adding the Objective-C bridging header]({{ site.baseurl }}/images/tutorial/source_dist_4.png)

Type there the following path:

{% highlight bash %}
$(SRCROOT)/libs/WhirlyGlobeMaply/WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/include/MaplyBridge.h
{% endhighlight %}

At this point, your project should be set up properly and you’ll be linked to the github repo for updates.

Next up, let’s look at something!
