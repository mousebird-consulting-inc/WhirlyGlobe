# Build the WhirlyGlobe-Maply binary distribution

BASENAME=WhirlyGlobe_Maply_Distribution
WG_VERSION=2_4
DIST_DIR="$BASENAME"_"$WG_VERSION"

rm -rf $DIST_DIR

mkdir $DIST_DIR
mkdir "$DIST_DIR"/examples/
mkdir "$DIST_DIR"/third-party

cp -R WhirlyGlobeSrc/WhirlyGlobeComponentTester/ $DIST_DIR/examples/WhirlyGlobeComponentTester
rm -rf $DIST_DIR/examples/WhirlyGlobeComponentTester/Build
rm -rf $DIST_DIR/examples/WhirlyGlobeComponentTester/WhirlyGlobeComponentTester.xcodeproj/project.xcworkspace
rm -rf $DIST_DIR/examples/WhirlyGlobeComponentTester/WhirlyGlobeComponentTester.xcodeproj/xcuserdata
cp -R WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/WhirlyGlobeMaplyComponent.framework $DIST_DIR

cp -R third-party/AFNetworking/ $DIST_DIR/third-party/AFNetworking
cp -R third-party/KissXML/ $DIST_DIR/third-party/KissXML
cp -R third-party/SMCalloutView/ $DIST_DIR/third-party/SMCalloutView

cp -R resources/ $DIST_DIR/resources

# Load up in Xcode
# Delete reference to WhirlyGlobe-Maply subproject
# Add WhirlyGlobe-MaplyComponent framework
# Edit header path to point to framework headers
# Take out NOTPODSPECWG define
# Turn off MapzenSource.m
