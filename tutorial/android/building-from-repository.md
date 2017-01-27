---
title: Building From Repository
layout: android-tutorial
prev_next:
    prev: hello-earth.html
    next: your-first-globe-or-map.html
---

Probably the simplest way to build your WhirlyGlobe-Maply app on Android is to use a central repository.  A repo is where you are probably getting the rest of your libraries, including ones provided by Google and it's a place where most *compiled* Android libraries live.

Popular repositories include Maven, JCenter, and Bintray.  We're somewhat confused by their relationships with each other, but we do publish our library out there.

### Check the repository

JCenter is an easy repository to use because it's included by default in every Android project. If you open the `YOUR_PROJECT_HOME/build.gradle` file, you'll see:

```gradle
allprojects {
    repositories {
        jcenter()
    }
}
```

### Add the dependency

Once the repository is configured, you just need to add the dependency. In `YOUR_PROJECT_HOME/app/build.gradle` file, you need to add this line:

```gradle
dependencies {
    compile fileTree(dir: 'libs', include: ['*.jar'])
    compile 'com.android.support:appcompat-v7:23.4.0'
    
    // other libraries will appear here

    compile 'com.mousebirdconsulting.maply:Android:2.5'
}
```

The trailing `0.7` is the version to use. You can replace that with future version numbers.


### Downloading and compiling

If everything is in place, you just need to clean and compile the project (Build > Clean) and check the Gradle Console panel to make sure the library was downloaded and included in the build.

It may happen that the specified version is not included in JCenter yet (it happens when the version is quite new, because JCenter needs some time to get synchronized). If you get this error:


```
Could not find com.mousebirdconsulting.maply:Android:2.5.
```

or maybe this one

```
Failed to resolve: com.mousebirdconsulting.maply:Android:2.5
```

then add the following "maven" section to your `YOUR_PROJECT_HOME/build.gradle` file

```gradle
allprojects {
    repositories {
        jcenter()
        maven {
            url "https://dl.bintray.com/mousebirdconsulting/WhirlyGlobe/"
        }
    }
}
```

This way you'll make sure the dependency will be found either in JCenter or in our own repository (inside Bintray too).



