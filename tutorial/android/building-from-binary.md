---
title: Building From Binaries
layout: android-tutorial
prev_next:
    prev: hello-earth.html
    next: your-first-globe-or-map.html
---

The simplest way to start using WhirlyGlobe-Maply is the pre-built binary.  Take a look at the [builds page](/WhirlyGlobe/builds/builds.html) page.  From there you can get the AAR file, and we'll show you how to use it here.

### Copy and Include AAR

Copy your just-downloaded `.aar` file into your app's `libs` directory:

```
HelloEarth/app/libs
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

* `implementation('com.squareup.okhttp:okhttp:2.3.0')`
* `implementation(name:'WhirlyGlobeMaply', ext:'aar')`

```gradle
dependencies {
    implementation fileTree(dir: "libs", include: ["*.jar"])
    implementation 'androidx.appcompat:appcompat:1.1.0'
    implementation 'androidx.constraintlayout:constraintlayout:1.1.3'
    testImplementation 'junit:junit:4.12'
    androidTestImplementation 'androidx.test.ext:junit:1.1.1'
    androidTestImplementation 'androidx.test.espresso:espresso-core:3.2.0'
    implementation('com.squareup.okhttp:okhttp:2.3.0')
    implementation(name:'WhirlyGlobeMaply', ext:'aar')
}
```

Android Studio will ask you to sync Gradle. If all goes well, it will sync without complaint.

![Gradle Sync](resources/gradle-sync.png)


---

*Tutorial by Nicholas Hallahan, Steve Gifford, Tim Sylvester.*
