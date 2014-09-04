---
title: WhirlyGlobe-Maply Binary Distribution
layout: tutorial
---

Getting the source code is the most open-source-y way to use WhirlyGlobe-Maply, but it's not required.  There's a binary framework you can download and install.  Yes, that's faster and simpler.

Setting up a WG-Maply project using a binary distribution is slightly different. First, download the [binary distribution](https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/WhirlyGlobe_Maply_Distribution_2_3.zip) and unzip it into your projects directory. It should look something like this:

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

### Component Tester App

The toolkit ships with a test app that exercises most of the good parts.  In the binary distribution you'll see it under examples.

Build and run it, preferably on some good hardware. Fool around with the various display options, and you'll get a taste of how capable WG-­Maply is.

Spending a few minutes with WhirlyGlobeComponentTester will familiarize you with many of the toolkit’s capabilities. Over the next few sections we'll use some of those ourselves, and show you how to build a simple app with a globe or a flat map that displays the world, shows country outlines, and responds to taps and selections. After that, hopefully you'll be comfortable browsing through the [WG-Maply documentation](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/) and the WhirlyGlobeComponentTester source for ideas of what to try next.

Next up, let's look at building a map or a globe.

[Globe or Map](globe_or_map.html)
