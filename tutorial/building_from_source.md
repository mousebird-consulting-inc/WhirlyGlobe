---
title: WhirlyGlobe-Maply Distribution from Source
layout: tutorial
---

Let's check out some of WG-­Maply's capabilities. To do this, we'll clone the github repository and build the test application that comes with it.

Open a Terminal window, and change directory to a good location for your WG­-Maply test.

The line below assumes this location is ~/projects.

    cd ~/projects

Now, clone the repository:

    git clone https://github.com/mousebird/WhirlyGlobe.git WhirlyGlobe­Maply

Next, initialize and update WG-­Maply's submodules. This may take a few minutes.

    cd WhirlyGlobe­Maply
    git submodule init
    git submodule update

Lastly, open the WhirlyGlobeComponentTester project in WhirlyGlobeSrc. Build and run this project. You should see screens roughly matching the following screen shots. Fool around with the various display options, and you'll get a taste of how capable WG-­Maply is.

Spending a few minutes with WhirlyGlobeComponentTester will familiarize you with many of the toolkit’s capabilities. Over the next few sections you'll learn how to set up a project to use WG-Maply, and how to build a simple app with a globe or a flat map that displays the world, shows country outlines, and responds to taps and selections. After that, hopefully you'll be comfortable browsing through the WG-Maply documentation and the WhirlyGlobeComponentTester source for ideas of what to try next.

There are two ways to incorporate WG-Maply into your project: you can include it as a submodule, or you can use the binary distribution. We'll go over the submodule option first.

### WhirlyGlobe­Maply Via Submodules

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

If you’d rather use the binary distribution, select that button.  Otherwise, you can move on to the next part of the tutorial.

[Getting Start](getting_started.html) [Build from binary](building_from_binary.html) [Globe or Map](globe_or_map.html)

