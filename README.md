
## WhirlyGlobe

This is a mirror repository for WhirlyGlobe 1.2 maintained by Juan Collas.
It is not the main or official repository, which can be found here:

    http://code.google.com/p/whirlyglobe/

mousebird consulting, the developer of WhirlyGlobe keeps a blog, which
can be found here:

    http://mousebirdconsulting.blogspot.com/

If you have questions related to WhirlyGlobe send them to Steve Gifford (sjg)
at the obvious address for mousebirdconsulting (dot com).  [Trying
to avoid automatic email harvesting there].

### Building

Make sure you've retrieved all the dependencies

    git submodule update --init --recursive

This will download the following modules:

    boost
    clipper
    eigen
    shapelib

Once that's done,

    cd WhirlyGlobeLib

and build the framework used by the applications

    ./buildframework.sh

Once the framework is built, you can build and run all the applications from XCode.
