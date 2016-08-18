---
title: Builds
layout: builds
---

Building WhirlyGlobe-Maply from scratch can be a bit of work.  There are submodules and it takes a while.  The Cocoapods distribution is a bit faster, but still compiles a lot of source code.  If you just want a binary distribution we provide that for both iOS and Android.  

A stable build for iOS version 2.4 can be [downloaded directly](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe_Maply_Distribution_2_4.zip) if that's all you need.

But we also provide the results of our Jenkins continuous integration server.  Builds are run for iOS and Android nightly on the development branch and every time we check something in.

If you'd like a good binary build, just grab the *nightly* version.  If you've been sent here for a specific feature, you may want the commit builds.  Those are the ones without *nightly* in the name.

To install these development versions, just follow these steps:

### iOS

Get the URL of the zip file listed bellow and use that in your `Podfile` as follows:

{% highlight bash %}
pod 'WhirlyGlobe', :http => 'http://url/to/binary/version.zip'
{% endhighlight %}

It's too simple to be truth, isn't it?

### Android

Just download the aar file in your local machine and include that file in the Android Studio project. This [tutorial](http://mousebird.github.io/WhirlyGlobe/tutorial/android/hello-earth.html) gives a good explanation about that.


