
## WhirlyGlobe

This is an "Out of the Box" mirror repository for WhirlyGlobe 2.0 maintained by Steve Gifford.
See also https://github.com/jcollas/whirlyglobe-1.2
This means that all samples and libraries here works right after you have cloned this repo and it's submodules (see below).
I've moved resources for the sample apps to the separate repositories, they are too fat =)
Also this saved some space, because some of the resources were duplicated.
Sample apps now has relative paths to the resources.
I've upgraded all sample apps to the new WhirlyGlobeLib 2.0 framework, lots of types and method were renamed.
All samples app were tested in Simulator, iPad 2, iPad 3, works great!
But the Contributed/WhirlyGraph has some complicated compilation errors within eigen, so not working correct for now, fixes in progres�
Lots of types were renamed.
Cloned latest external libraries repositories to Git, i'll try to maintain updates.
Cloned versions: Boost 1.51.0, Clipper 4.8.8, ShapeLib 1.3.0, Eigen 3.1.1.
Fixed some "compiler happy" things in ShapeLib, and Clipper, WhirlyGlobe.
No need in manual setup of "Header Search Paths", already relative updated everywhere needed.

It is not the main or official repository, which can be found here:

    https://github.com/mousebird/WhirlyGlobe/

mousebird consulting, the developer of WhirlyGlobe keeps a blog, which
can be found here:

    http://mousebirdconsulting.blogspot.com/

If you have questions related to WhirlyGlobe send them to Steve Gifford sjg@mousebirdconsulting.com

### Building

Make sure you've retrieved all the dependencies

    git submodule update --init --recursive

This will download the following modules:

    boost
    clipper
    eigen
    shapelib
    bluemarblembtiles
    wg-resources

Once that's done,

    cd WhirlyGlobeSrc/WhirlyGlobeLib

and build the framework used by the applications

    ./buildframework.sh

Once the framework is built, you can build and run all the applications from XCode.
