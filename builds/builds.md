---
title: Builds
layout: default
---

Building the toolkit from scratch can be a lot of work.  Even the Cocoapods version compiles a lot of code.  If you just want a binary version, we've got that here.

We also provide the results of our <a href="https://www.bitrise.io/">BitRise.io</a> continuous integration builds.  Builds are run for iOS and Android on the main and development branches every time we commit.

If you'd like a good binary build, grab 3.3 or a *latest* version.

The iOS version is a binary framework you can import into your project.  One of [our tutorials](https://mousebird-consulting-inc.github.io/WhirlyGlobe/tutorial/ios/building_from_binary.html) discusses that in detail.

For Android, you can use the AAR and we discuss how in [another tutorial](https://mousebird-consulting-inc.github.io/WhirlyGlobe/tutorial/android/building-from-binary.html).

### Version 3.3

The official released version of WhirlyGlobe-Maply is 3.3.  The downloads are as follows.

[iOS 3.3 Xframework](https://whirlyglobemaplydistribution.s3-us-west-1.amazonaws.com/WhirlyGlobe_Maply_Distribution_3_3.zip)

[Android 3.3 AAR](https://whirlyglobemaplydistribution.s3-us-west-1.amazonaws.com/WhirlyGlobe_Maply_Distribution_3_3.aar)

### Latest Versions

The most recent development branch builds:

[iOS Develop Xframework](https://whirlyglobemaplydistribution.s3-us-west-1.amazonaws.com/WhirlyGlobe_Maply_Develop_latest.zip)

[Android Develop AAR](https://whirlyglobemaplydistribution.s3-us-west-1.amazonaws.com/WhirlyGlobe_Maply_Develop_latest.aar)

The most recent main branch builds:

[iOS Main Xframework](https://whirlyglobemaplydistribution.s3-us-west-1.amazonaws.com/WhirlyGlobe_Maply_Distribution_latest.zip)

[Android Main AAR](https://whirlyglobemaplydistribution.s3-us-west-1.amazonaws.com/WhirlyGlobe_Maply_Distribution_latest.aar)

### Version 2

This is an older version of WhirlyGlobe-Maply, provided here for compatibility.  The downloads are as follows.

[iOS 2.5 framework](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_5.zip)

[Android 2.6.2 AAR](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-Maply_Distribution_2_6_2.aar)

_Note: Cocoapods caches the file downloaded using the URL as cache key, so even though this file always contains the latest nighly build, you will have to clean the cache after the first dowload. For that, check the command `pod cache clean -all`_

