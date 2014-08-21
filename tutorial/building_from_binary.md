---
title: WhirlyGlobe-Maply Binary Distribution
layout: tutorial
---

Setting up a WG-Maply project using a binary distribution is slightly different. First, download the [binary distribution](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe-MaplyDistribution_2_3_beta2.zip) and unzip it into your projects directory. It should look something like this:

    projects/WhirlyGlobe_Maply_Distribution_2_3

This guide will simply refer to it as 'BinaryDirectory' for purposes of indicating paths.

Open Xcode and click Create a new Xcode Project. Create a new IOS Application using the Empty Application template. Call it HelloEarth, (or HelloEarthBinary if you're plowing through both of these sections) and check the Source Control box to create a local git repository for your project.

Easy.

Now that Xcode is done creating the project, we need to pull in the libraries we'll need. Go to Build Phases and expand Link Binary With Libraries. Click + and then Add Other.

Navigate to BinaryDirectory and select WhirlyGlobeMaplyComponent.framework., then click Open. Click + and Add Other again, navigate to BinaryDirectory/WhirlyGlobeMaplyComponent.framework/Versions/Current and select libWhirlyGlobeMaplyComponent.a. 

Finally, we'll need to add some libraries that WG-Maply needs. Click + and add

    CoreLocation
    libc++
    libz
    libxml2
    libsqlite3

The last thing we need to do is indicate where to search for WG-Maply header files. Open up Build Settings and scroll down to Header Search Paths. Add the path corresponding to 'BinaryDirectory/WhirlyGlobeMaplyComponent.framework/Headers/'.

That's it! You should be set up properly for a project using WG-Maply.

[Getting Start](getting_started.html) [Globe or Map](globe_or_map.html)
