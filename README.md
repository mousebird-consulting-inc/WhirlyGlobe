The master branch is deprecated in favor of [main](https://github.com/mousebird-consulting-inc/WhirlyGlobe/tree/main).
![WhirlyGlobe-Maply](/common/images/banner.jpg)

[![Apache v2](https://img.shields.io/badge/License-Apache%202-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0.html)
[![Release 3.5](https://img.shields.io/badge/Release-3.5-blue.svg)](https://github.com/mousebird-consulting-inc/WhirlyGlobe/releases)
![Platform iOS+Android](https://img.shields.io/badge/Platform-%20iOS%20%7c%20Android-blue.svg)
[![Open Issues](https://img.shields.io/github/issues/mousebird-consulting-inc/WhirlyGlobe.svg?color=blue)](https://github.com/mousebird-consulting-inc/WhirlyGlobe/issues)
[![Closed Issues](https://img.shields.io/github/issues-closed/mousebird-consulting-inc/WhirlyGlobe.svg?color=blue)](https://github.com/mousebird-consulting-inc/WhirlyGlobe/issues?q=is%3Aissue+is%3Aclosed)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/mousebird-consulting-inc/WhirlyGlobe/pulls)

What is WhirlyGlobe-Maply?
---

WhirlyGlobe-Maply is a mapping toolkit with two parts, hence the dash. The WhirlyGlobe part is an interactive 3D globe. The Maply part is an interactive 2D map. There are separate view controllers (on iOS) for each, but otherwise they share 95% of their code.

This toolkit is used in [many apps](http://mousebird-consulting-inc.github.io/WhirlyGlobe/apps/apps.html) from big, complicated apps to even more smaller, simpler apps. Feel free to use it in yours.

WhirlyGlobe-Maply is available for both iOS and Android.

Getting Started
---

If you're new to WhirlyGlobe-Maply, please [read the main page](https://mousebird-consulting-inc.github.io/WhirlyGlobe/). See More Information below for useful links and resources.

There’s a tutorial for both iOS and Android:

- [Getting started with iOS](http://mousebird-consulting-inc.github.io/WhirlyGlobe/tutorial/ios/getting_started.html) 
- [Getting started with Android](http://mousebird-consulting-inc.github.io/WhirlyGlobe/tutorial/android/getting-started.html) 

Builds
---

[![iOS Build Status](https://app.bitrise.io/app/73efc488840cb3bf.svg?token=-_RGRNloG-bCA9O5ml-U8Q)](https://app.bitrise.io/app/73efc488840cb3bf) | iOS Main
-: | :-
[![Android Build Status](https://app.bitrise.io/app/36f069a6fbd58b11.svg?token=XD5YnMiUwnj0169yhIOkPQ)](https://app.bitrise.io/app/36f069a6fbd58b11) | Android Main

This is the main branch of the WhirlyGlobe-Maply Component and API version 3.5. It should be easy to compile, as all the crazy dependencies are in submodules. You can also get a [precompiled version](https://mousebird-consulting-inc.github.io/WhirlyGlobe/builds/builds.html).

WhirlyGlobe-Maply uses a bunch of submodules, which you'll need to get. Like so:

```
git submodule init
git submodule update
```

Get comfortable. The data is contained in a submodule and it's large.

Once you get all this synced, try to build AutoTester. If it builds, you're good to go. Enjoy.

Want more detail? Go read the [Tutorials](http://mousebird-consulting-inc.github.io/WhirlyGlobe/tutorial/) on the WhirlyGlobe-Maply site.

Breaking Changes
---

If you're upgrading from 3.3, you will need to make some adjustments due to the switch to xcframework and modular headers:

- When embedding the framework as is done in AutoTester, Swift no longer automatically imports the module because it doesn't match the project name.  You will need an additional `import WhirlyGlobe` at the top of Swift files using the API.
- If you're including header files from Obj-C++ code, you'll need to use the modular include style, `#import <WhirlyGlobe/Header.h>`, and using quoted includes will produce a compile error.

More Information
---

There's a [mailing list](http://eepurl.com/D30CD) for periodic announcements. Join!

Check out the [mousebird consulting blog](http://mousebirdconsulting.blogspot.com) for current progress.

Follow us on Twitter: [@whrlyglobemaply](https://twitter.com/whrlyglobemaply)

Email questions to: contact@mousebirdconsulting.com
