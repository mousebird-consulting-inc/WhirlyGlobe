---
title: Building WhirlyGlobe-Maply With Source
layout: tutorial
---

With open source projects, you'd expect to get access to all of the source code.  Sure enough, that's what we do with WhirlyGlobe-Maply.  All the source is available on github, with references to other projects that we use.

One of the safest ways to set up your WG-Maply project is to add it as a submodule.  This ensures you get all the code and can track the version you're using.  Here's how to set that up.

### Setting up your App with Submodules

Open Xcode and click Create a new Xcode Project. Create a new IOS Application using the Empty Application template. Call it HelloEarth, and check the Source Control box to create a local git repository for your project. Done? Good. Close Xcode and let's modify the project to pull in WG-Maply.

Open a Terminal window, and change to the root directory of your project. (This directory should contain HelloEarth.xcodeproj.)

Create a libs subdirectory and add WG-Maply in that path.

    mkdir libs
    git submodule add https://github.com/mousebird/WhirlyGlobe.git libs/WhirlyGlobe­Maply

This will clone the master WG-Maply branch. Most of the time, that will be what you want.  Occasionally, you might need to use an older branch, or want to try out a beta. If you want to use an alternate branch, use the ­b option when adding the submodule.[1] For example, to clone the develop_2_3 branch:

    git submodule add ­b develop_2_3 https://github.com/mousebird/WhirlyGlobe.git libs/WhirlyGlobe­Maply

Now, we need to initialize and update WG-Maply's dependencies:

    cd libs/WhirlyGlobe­Maply
    git submodule init
    git submodule update

This will take a few minutes, as one of WG-Maply's submodules contains a fair amount of source data. Once it's done we're done with Terminal. Close that window and open the HelloEarth project in Xcode.

We now need to add the WG-Maply library to the project, along with a number of others upon which WG-Maply depends. To do that, select the HelloEarth project, then go to Build Phases, and expand Link Binary With Libraries.

Click the + at the bottom, and then Add Other.

Navigate to the libs/WhirlyGlobeSrc/WhirlyGlobe­MaplyComponent/ directory, and select WhirlyGlobe­MaplyComponent.xcodeproj. Click 'Open'. (If you are prompted to share a working copy, no is fine.)

Click + again, and you will see a new option: libWhirlyGlobe­MaplyComponent.a. Click thatand click Add. You'll need to and add the following libraries as well:

    CoreLocation
    libc++
    libz
    libxml2
    libsqlite3

Finally, we need to add the path to the WG-Maply headers so Xcode knows where to find the includes. To do this, go to Build Settings and look for Header Search Paths. Click + and add the following path:

    “$(SRCROOT)/libs/WhirlyGlobe­Maply/WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/include”

At this point, your project should be set up properly and you’ll be linked to the github distribution for updates.

### Component Tester App

Let's check out some of WG-­Maply's capabilities.  The toolkit comes with a test app that runs through the really interesting ones.  It's included with the source you cloned from github.

In libs/WhirlyGlobe-Maply/WhirlyGlobeSrc, open the WhirlyGlobeComponentTester project. Build and run it, preferably on some good hardware. Fool around with the various display options, and you'll get a taste of how capable WG-­Maply is.

Spending a few minutes with WhirlyGlobeComponentTester will familiarize you with many of the toolkit’s capabilities. Over the next few sections we'll use some of those ourselves, and show you how to build a simple app with a globe or a flat map that displays the world, shows country outlines, and responds to taps and selections. After that, hopefully you'll be comfortable browsing through the [WG-Maply documentation](http://mousebird.github.io/WhirlyGlobe/documentation/2_3/) and the WhirlyGlobeComponentTester source for ideas of what to try next.


If you’d rather use the binary distribution, select that button.  Otherwise, you can move on to the next part of the tutorial.

[Build from binary](building_from_binary.html) 

[Globe or Map](globe_or_map.html)

