git checkout develop_2_3
git submodule init
git submodule update
git submodule sync
rm -rf resources
rm -rf third-party/eigen
rm -rf third-party/clipper
git submodule update
