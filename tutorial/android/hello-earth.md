---
title: Hello Earth Project
layout: android-tutorial
---

### Copy and Include AAR

Copy your `Android-release.aar` that you built from source into your app's `libs` directory.

```
WhirlyGlobe/WhirlyGlobeSrc/HelloEarth/app/libs
```

Rename it to `WhirlyGlobeMaply.aar`.

Add the following `flatDir` directive to your `Build.gradle (Project: HelloEarth)` file inside of the `allprojects` > `repositories` directive.

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


Next add `compile(name:'WhirlyGlobeMaply', ext:'aar')` to the end of the `dependencies` directive in `Build.gradle (Module: app)`.

```
dependencies {
    compile fileTree(dir: 'libs', include: ['*.jar'])
    testCompile 'junit:junit:4.12'
    compile 'com.android.support:appcompat-v7:24.0.0'
    compile 'com.android.support:design:24.0.0'
    compile(name:'WhirlyGlobeMaply', ext:'aar')
}
```

