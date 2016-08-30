---
title: Building From Nightly
layout: android-tutorial
prev_next:
    prev: hello-earth.html
    next: your-first-globe-or-map.html
---

If you want the latest and greatest improvements to WhirlyGlobe-Maply you can use nightly or feature based builds.  Take a look at the [nightly versions](/WhirlyGlobe/builds/builds.html) page.  From there you can get feature branch builds or just a nightly build.  We'll show you how to use them here.


### Pick the version you need

Just go to the [Automated Builds](/WhirlyGlobe/builds/builds.html) page and choose the version you want to use.

You have two flavours:

  - _Nightly builds_: surprisngly, this is the version built every night (it's night for us, anyway). Use this version if you're sure that one specific change was included in that or previous days.
  - _Commit or Push builds_: this happens when someone pushes a change to one of the development branches. Use this version if you know the build number that includes your desired change or we pointed you here directly.

Once you've chosen the right version, just download the `.aar` file from the [builds table](/WhirlyGlobe/builds/builds.html).


### Copy and Include AAR

Copy your just downloaded `.aar` file into your app's `libs` directory.

```
WhirlyGlobe/WhirlyGlobeSrc/HelloEarth/app/libs
```

Rename it to `WhirlyGlobeMaply.aar`.

Add the following `flatDir` directive to your `Build.gradle (Project: HelloEarth)` file inside of the `allprojects > repositories` directive.

```gradle
allprojects {
    repositories {
        jcenter()
        flatDir {
            dirs 'libs'
        }
    }
}
```


Next add the following packages to the end of the `dependencies` directive in `Build.gradle (Module: app)`.

* `compile 'com.squareup.okhttp:okhttp:2.3.0'`
* `compile(name:'WhirlyGlobeMaply', ext:'aar')`

```gradle
dependencies {
    compile fileTree(dir: 'libs', include: ['*.jar'])
    testCompile 'junit:junit:4.12'
    compile 'com.android.support:appcompat-v7:24.0.0'
    compile 'com.android.support:support-v4:24.0.0'
    compile 'com.squareup.okhttp:okhttp:2.3.0'
    compile(name: 'WhirlyGlobeMaply', ext: 'aar')
}
```

Android Studio will ask you to sync Gradle. If all goes well, it will sync without complaint.

![Gradle Sync](resources/gradle-sync.png)



