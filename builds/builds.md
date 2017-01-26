---
title: Builds
layout: builds
---

Building the toolkit from scratch can be a lot of work.  Even the Cocoapods version compiles a lot of code.  If you just want a binary version, we've got that here.

We also provide the results of our Jenkins continuous integration server.  Builds are run for iOS and Android nightly on the development branch and every time we check something in.

If you'd like a good binary build, just grab 2.5 or a *nightly* version.  If you've been sent here for a specific feature, you may want the commit builds.  Those are the ones without *nightly* in the name.

### Version 2.5

The official released version of WhirlyGlobe-Maply is 2.5.  You can get that directly right there.

| Architecture | Download |
| ------------ | -------- |
| iOS          | [iOS 2.5 framework](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_5.zip) |
| Android          | [Android 2.5 AAR](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_5.aar) |

The iOS version is a binary framework you can import into your project.  On of [our tutorials](https://mousebird.github.io/WhirlyGlobe/tutorial/ios/building_from_binary.html/) discusses that in detail.

For Android, you can use the AAR and we discuss how in [another tutorial](https://mousebird.github.io/WhirlyGlobe/tutorial/android/building-from-nightly.html).

### Cocoapods (iOS)

Get the URL of the zip file listed below and use that in your `Podfile` as follows:

{% highlight bash %}
pod 'WhirlyGlobe', :http => 'http://url/to/binary/version.zip'
{% endhighlight %}

It's too simple to be truth, isn't it?

Besides the urls listed below, you also have an special url always pointing to the lastest nightly build:

{% highlight bash %}
pod 'WhirlyGlobe', :http => 'https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/iOS_daily_builds/WhirlyGlobe-Maply_Nightly_latest.zip'
{% endhighlight %}

_Note: Cocoapods caches the file downloaded using the URL as cache key, so even though this file always contains the latest nighly build, you will have to clean the cache after the first dowload. For that, check the command `pod cache clean -all`_
