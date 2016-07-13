---
title: Hello Earth Project
layout: ios-tutorial
prev_next:
  prev: getting_started.html
  next: building_from_binary.html
---

You need to create an Xcode project.  We’ll call it **HelloEarth** and you’ll add all your tutorial code to it.

Open Xcode, select File, New, and then Project.  Use the Single View Application Template.

![Single View Application]({{ site.baseurl }}/images/tutorial/hello_earth_template.png)

Call your new app HelloEarth.  You can leave the rest as defaults.

For source control, check the Source Control box to create a local git repository for your project.  You’ll need this if you set up the project with git submodules below.

![Empty Hello Earth]({{ site.baseurl }}/images/tutorial/hello_earth_finished.png)

Done? Good. Close Xcode and let's modify the project to pull in WG-Maply.


### WhirlyGlobe-Maply Distributions

We distribute the WhirlyGlobe-Maply library in three ways. Any one of them is fine.  Pick the one you are most familiar with.  Don’t care?  Try binary.

* [Building from Source](building_from_source.html) - Use submodules for the distribution on github.
* [Build from binary](building_from_binary.html) - The self contained binary distribution.
* [Build with CocoaPods](building_from_cocoapod.html) - Use the WG-Maply CocoaPod.

It’s like a pick your own adventure book, but boring.  So just like a pick your own adventure book.
