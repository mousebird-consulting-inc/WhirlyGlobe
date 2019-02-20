---
title: Builds
layout: default
---

Building the toolkit from scratch can be a lot of work.  Even the Cocoapods version compiles a lot of code.  If you just want a binary version, we've got that here.

### Version 2.6

The official version of WhirlyGlobe-Maply is 2.6.2 for Android and 2.6.1 for iOS.  The former contains a few tweaks for Android and no changes for iOS.

To use WhirlyGlobe-Maply via Cocoapods, include the following in your Podfile.

{% highlight bash %}
pod 'WhirlyGlobe', :git => 'https://github.com/mousebird/WhirlyGlobe'
{% endhighlight %}

Unfortunately binary builds are currently broken for iOS.  Apple changed the signing paradigm for frameworks and I haven't been able to sort it out.  Please use the submodule or Cocoapod approach.

For Android, you can use the AAR and we discuss how in [the Android tutorial](https://mousebird.github.io/WhirlyGlobe/tutorial/android/building-from-nightly.html).

[Android 2.2 AAR](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_6_2.aar)


### Version 2.5

This is an older version of WhirlyGlobe-Maply, provided here for compatibility.  The downloads are as follows.

[iOS 2.5 framework](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_5.zip)

[Android 2.5 AAR](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_5.aar)

The iOS version is a binary framework you can import into your project.  One of [our tutorials](https://mousebird.github.io/WhirlyGlobe/tutorial/ios/building_from_binary.html/) discusses that in detail.

For Android, you can use the AAR and we discuss how in [another tutorial](https://mousebird.github.io/WhirlyGlobe/tutorial/android/building-from-nightly.html).
