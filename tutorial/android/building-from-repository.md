---
title: Building From Repository
layout: android-tutorial
---

Probably you ever heard about the beloved and hated Maven (last named Central). That's a repository. It's a place where most of the compiled Java libraries live.
After Maven, several other repositories have been founded, like JCenter and Bintray. Maven tradition comes from old Java times, meanwhile JCenter is more about Android (actually it's a superset of Maven) and Bintray is the company behind JCenter (well, sort of, I think)

The preferred way to download and use third-party library in a typical Android project is using a repository like JCenter. 

So you can expect our library to be a good citizen and live in those lands too.

### Check the repository

The best part of using JCenter is that it's included by default in every Android project. If you open the `YOUR_PROJECT_HOME/build.gradle` file, you'll see:

```gradle
allprojects {
    repositories {
        jcenter()
    }
}
```

### Add the dependency

Once the repository is configured, you just need to add the dependency. In `YOUR_PROJECT_HOME/app/build.gradle` file, you need to add one line:

```gradle
dependencies {
    compile fileTree(dir: 'libs', include: ['*.jar'])
    compile 'com.android.support:appcompat-v7:23.4.0'
    
    // other libraries will appear here

    compile 'com.mousebirdconsulting.maply:Android:0.7'
}
```

The trailing `0.7` is the version to use. You can replace that with future version numbers.


### Downloading and compiling

If everything is in place, you just need to clean and compile the project (Build > Clean) and check the Gradle Console panel to make sure the library was downloaded and included in the build.

It may happen that the specified version is not included in JCenter yet (it happens when the version is quite new, because JCenter needs some time to get synchronized). If you get this error:


```
Could not find com.mousebirdconsulting.maply:Android:0.7.
```

or maybe this one

```
Failed to resolve: com.mousebirdconsulting.maply:Android:0.7
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

This way you'll make sure the dependency will be found either in JCenter or in our own repository (inside Bintray too)



