/*
 *  MaplyBaseViewController.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <UIKit/UIKit.h>
#import "MaplyCoordinate.h"
// Note: Porting
//#import "MaplyScreenMarker.h"
#import "MaplyVectorObject.h"
// Note: Porting
//#import "MaplyViewTracker.h"
#import "MaplyComponentObject.h"
#import "MaplySharedAttributes.h"
#import "MaplyViewControllerLayer.h"
//#import "MaplyLight.h"
#import "MaplyShader.h"
#import "MaplyQuadImageTilesLayer.h"
#import "MaplyTexture.h"
// Note: Porting
//#import "MaplyActiveObject.h"
//#import "MaplyElevationSource.h"

/// Where we'd like an add to be executed.  If you need immediate feedback,
///  then be on the main thread and use MaplyThreadCurrent.  Any is the default. 
typedef enum {MaplyThreadCurrent,MaplyThreadAny} MaplyThreadMode;

/** @brief Base class for the Maply and WhirlyGlobe view controllers.
    @details The Maply Base View Controller is where most of the functionality lives.  For the most part Maply and WhirlyGlobe share methods and data structures.  This view controller sets up the rendering, the threading, basically everything that makes WhirlyGlobe-Maply work.
    @details Don't create one of these directly, instead use the MaplyViewController or the WhirlyGlobeViewController.
 */
@interface MaplyBaseViewController : UIViewController

/** @brief Turn selection on or off globally.
    @details If on we'll forward selected features on to the delegate.  When off, we don't do that.  On by default.
  */
@property(nonatomic,assign) bool selection;

/** @brief Set the globe (not the UIView's) background color.
    @details This property sets the clear color used by OpenGL.  By default it's black.
  */
@property (nonatomic,strong) UIColor *clearColor;

/** @brief Set the frame interval passed to the CADisplayLink.
    @details This sets the frame rate the renderer will attempt to achieve.
 
 |value|frames per second|
 |:----|:----------------|
 |1|60fps|
 |2|30fps|
 |3|20fps|
 |4|15fps|
 |5|12fps|
 |6|Really?  No, you can do better.|
  */
@property (nonatomic,assign) int frameInterval;

// Note: Porting
///** @brief The elevation delegate that will provide elevation data per tile.
//    @details We break the image tiles out from the elevation tiles.  The data is often coming from different sources, but in the end this is a probably a hack.  It's a hack that's going to be in place for a while.
//    @details To provide elevation for your compatible MaplyTileSource objects, you fill out the MaplyElevationSourceDelegate protocol and assign the resulting object here.  When an image layer needs elevation, it will check for the delegate and then query for the respective file.
//    @details At present there is no checking for coordinate system compatibility, so be aware.
//  */
///// Fill this in to provide elevation data.  It will only work for a matching image layer,
/////  one with the same coordinate system and extents.
//@property (nonatomic,weak) NSObject<MaplyElevationSourceDelegate> *elevDelegate;

/** @brief If set we'll create a new thread for every layer the user adds.
    @details The only layers within the toolkit are for image tile paging.  So effectively this creates a thread for every image layer you add.  This is going to result in faster image paging, but higher CPU usage and a bit more memory.  On by default.
  */
@property (nonatomic,assign) bool threadPerLayer;

// Note: Porting
///** @brief Clear all the currently active lights.
//    @details There are a default set of lights, so you'll want to do this before adding your own.
//  */
//- (void)clearLights;
//
///** @brief Add the given light to the list of active lights.
//    @details This method will add the given light to our active lights.  Most shaders will recognize these lights and do the calculations.  If you have a custom shader in place, it may or may not use these.
//    @details Triangle shaders use the lights, but line shaders do not.
//  */
//- (void)addLight:(MaplyLight *)light;
//
///// @brief Remove the given light (assuming it's active) from the list of lights.
//- (void)removeLight:(MaplyLight *)light;

/** @brief Set the rendering hints to control how the renderer is configured.
    @details This is a bit vestigial, but still has a few important uses.  The hints should be set right after the init call.  Any later and they'll probably be ignored.
    @details The rendering hints are as follows.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyRenderHintZBuffer|bool|If set, we'll explicitly turn on the Z buffer.  Normally it's off until a drawable requests it, allowing us to play neat tricks with overlays.  The only time you should be turning this on is if you're doing 3D elevation.  The default is off.|
 |kMaplyRenderHintCulling|bool|If set, we'll use the internal culling logic.  Texture and drawable atlases have largely made this pointless.  Leave it off unless you have a compelling reason to turn it on.|
 |kMaplyRendererLightingMode|NSString|This can be set to @"none", in which case we use optimized shaders that do no lighting or "regular".  The latter is the default.|
  */
- (void)setHints:(NSDictionary *)hintsDict;

/// @brief This calls addScreenMarkers:desc:mode: with mode set to MaplyThreadAny
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

///** @brief Add one or more screen markers to the current scene.
//    @details This method will add the given MaplyScreenMaker objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param markers An NSArray of MaplyScreenMarker objects.
//    @param desc The desciption dictionary which controls how the markers will be constructed.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyColor|UIColor|The color we'll use for the rectangle that makes up a marker. White by default.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a marker in when it appears and out when it disappears.|
// |kMaplyShader|NSString|If set, this is the name of the MaplyShader to use when rendering the screen markers.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
//
//    @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
// 
//    @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
//  */
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

/// @brief This calls addMarkers:desc:mode: with mode set to MaplyThreadAny
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

///** @brief Add one or more 3D markers to the current scene.
//    @details This method will add the given MaplyMarker objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param markers An NSArray of MaplyMarker objects.
//    @param desc The desciption dictionary which controls how the markers will be constructed.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyColor|UIColor|The color we'll use for the rectangle that makes up a marker. White by default.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a marker in when it appears and out when it disappears.|
// |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyMarkerDrawPriorityDefault.|
// |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that the geometry will draw on top of anything (respecting the kMaplyDrawPriority).|
// |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
// 
// @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
// 
// @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
// */
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

///// @brief This calls addScreenLabels:desc:mode: with mode set to MaplyThreadAny
//- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc;

// Note: Porting
///** @brief Add one or more screen labels to the current scene.
//    @details This method will add the given MaplyScreenLabel objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param labels An NSArray of MaplyScreenLabel objects.
//    @param desc The desciption dictionary which controls how the labels will be constructed.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyTextColor|UIColor|Color we'll use for the text. Black by default.|
// |kMaplyBackgroundColor|UIColor|Color we'll use for the rectangle background.  Use clearColor to make this invisible.|
// |kMaplyFont|UIFont|The font we'll use for the text.|
// |kMaplyLabelHeight|NSNumber|Height of the text in points.|
// |kMaplyLabelWidth|NSNumber|Width of the text in points.  It's best to set Height and leave this out.  That way the width will be calculated by the toolkit.|
// |kMaplyJustify|NSString|This can be set to @"middle", @"left", or @"right" to justify the text around the location.|
// |kMaplyShadowSize|NSNumber|If set, we'll draw a shadow with the kMaplyShadowColor offset by this amount.  We recommend using an outline instead.|
// |kMaplyShadowColor|UIColor|If we're drawing a shadow, this is its color.|
// |kMaplyTextOutlineSize|NSNumber|If set, we'll draw an outline around the text (really draw it twice).  The outline will be this large.|
// |kMaplyTextOutlineColor|UIColor|If we're drawing an outline, it's in this color.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a screen label in when it appears and out when it disappears.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
// 
//    @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
// 
//    @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
// */
//- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;
//
///// @brief This calls addLabels:desc:mode: with mode set to MaplyThreadAny
//- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc;

///** @brief Add one or more 3D labels to the current scene.
//    @details This method will add the given MaplyLabel objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param labels An NSArray of MaplyLabel objects.
//    @param desc The desciption dictionary which controls how the labels will be constructed.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyTextColor|UIColor|Color we'll use for the text. Black by default.|
// |kMaplyBackgroundColor|UIColor|Color we'll use for the rectangle background.  Use clearColor to make this invisible.|
// |kMaplyFont|UIFont|The font we'll use for the text.|
// |kMaplyLabelHeight|NSNumber|Height of the text in display coordinates.  For the globe these are based on radius = 1.0.|
// |kMaplyLabelWidth|NSNumber|Width of the text in display coordinates.  It's best to set Height and leave this out.  That way the width will be calculated by the toolkit.|
// |kMaplyJustify|NSString|This can be set to @"middle", @"left", or @"right" to justify the text around the location.|
// |kMaplyShadowSize|NSNumber|If set, we'll draw a shadow with the kMaplyShadowColor offset by this amount.  We recommend using an outline instead.|
// |kMaplyShadowColor|UIColor|If we're drawing a shadow, this is its color.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a label in when it appears and out when it disappears.|
// |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyLabelDrawPriorityDefault.|
// |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that the geometry will draw on top of anything (respecting the kMaplyDrawPriority).|
// |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
// 
// @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
// 
// @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
// */
//- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

/// @brief This calls addVectors:desc:mode: with mode set to MaplyThreadAny
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc;

/** @brief Add one or more vectors to the current scene.
   @details This method will add the given MaplyVectorObject objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
   @param vectors An NSArray of MaplyVectorObject objects.
   @param desc The desciption dictionary which controls how the vectors will look.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the vector features.|
 |kMaplyVecWidth|NSNumber|If the geometry is not filled, this is the width of the GL lines.|
 |kMaplyFilled|NSNumber boolean|If set, the areal geometry will be tesselated, taking holes into account.  The resulting triangles will be displayed instead of the vectors.|
 |kMaplySubdivType|NSString|When present, this requests that the geometry be broken up to follow the globe (really only makes sense there).  It can be set to kMaplySubdivGreatCircle or kMaplySubdivSimple which do a great circle subdivision and a simple 3-space subdivision respectively.  If the key is missing, we do no subdivision at all.|
 |kMaplySubdivEpsilon|NSNumber|If there's a kMaplySubdivType set this is the epsilon we'll pass into the subdivision routine.  The value is in display coordinates. 0.01 is a reasonable value.  Smaller results in more subdivision.|
 |kMaplyVecTexture|UIImage|If set and the kMaplyFilled attribute is set, we will apply the given texture across any areal features.  How the texture is applied can be controlled by kMaplyVecTexScaleX, kMaplyVecTexScaleY, kMaplyVecCenterX, kMaplyVecCenterY, and kMaplyVecTextureProjection|
 |kMaplyVecTexScaleX,kMaplyVecTexScaleY|NSNumber|These control the scale of the texture application.  We'll multiply by these numbers before generating texture coordinates from the vertices.|
 |kMaplyVecCenterX,kMaplyVecCenterY|NSNumber|These control the center of a texture application.  If not set we'll use the areal's centroid.  If set, we'll use these instead.  They should be in local coordinates (probably geographic radians).|
 |kMaplyVecTextureProjection|NSString|This controls how a texture is projected onto an areal feature.  By default we just use the geographic coordinates and stretch them out.  This look odd for very large features.  If you set this to kMaplyProjectionTangentPlane then we'll take the center of the feature, make a tangent plane and then project the coordinates onto that tangent plane to get texture coordinates.  This looks nice at the poles.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is below this height.  Off by default.|
 |kMaplyFade|NSNumber|The number of seconds to fade a vector in when it appears and out when it disappears.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorDrawPriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that the geometry will draw on top of anything (respecting the kMaplyDrawPriority).|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|

 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Note: Porting
///// @brief This calls addShapes:desc:mode: with mode set to MaplyThreadAny
//- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc;

// Note: Porting
///** @brief Add one or more MaplyShape children to the current scene.
//    @details This method will add the given MaplyShape derived objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param shapes An NSArray of MaplyShape derived objects.
//    @param desc The desciption dictionary which controls how the shapes will look.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyColor|UIColor|Color we'll use for the shape features.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The shapes will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The shapes will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a shape in when it appears and out when it disappears.|
// |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorShapePriorityDefault.|
// |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's on by default, meaning that the geometry can be occluded by things drawn first.|
// |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
// 
// @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
// 
// @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
// */
//- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;
//
///// @brief This calls addStickers:desc:mode: with mode set to MaplyThreadAny
//- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc;

// Note: Porting
///** @brief Add one or more MaplySticker objects to the current scene.
//    @details This method will add the given MaplySticker objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param stickers An NSArray of MaplySticker derived objects.
//    @param desc The desciption dictionary which controls how the stickers will look.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyColor|UIColor|Color we'll use for the stickers.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The stickers will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The stickers will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a sticker in when it appears and out when it disappears.|
// |kMaplySampleX|NSNumber|Stickers are broken up along two dimensions to adhere to the globe.  By default this is done adaptively.  If you want to override it, this is the X dimension for the sticker.|
// |kMaplySampleY|NSNumber|If you want to override it, this is the Y dimension for the sticker.|
// |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorShapePriorityDefault.|
// |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that it will draw on top of things before it..|
// |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
// |kMaplyShader|NSString|If set, this is the name of the MaplyShader to use when rendering the sticker(s).|
// 
// @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
// 
// @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
// */
//- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Note: Porting
///** @brief Modify an existing sticker.  This only supports changing the active textures.
//    @details This method will change attributes of a sticker that's currently in use.  At present that's just the images it's displaying.  
//    @param compObj The component object representing one or more existing stickers.
//    @param desc The description dictionary for changes we're making to the sticker.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyStickerImages|NSARray|The array of images to apply to the sticker.  You can reuse old ones or introduce new ones.|
//  */
//- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Note: Porting
///** @brief Add one or more MaplyBillboard objects to the current scene.
//    @details This method will add the given MaplyBillboard objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
//    @param billboards An NSArray of MaplyBillboard objects.
//    @param desc The description dictionary that controls how the billboards will look.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyColor|UIColor|Color we'll use for the stickers.|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The billboards will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The billboards will only be visible if the user is below this height.  Off by default.|
// |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyBillboardDrawPriorityDefault.|
//
//    @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
//  */
//- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

/** @brief Add vectors that can be used for selections.
    @details These are MaplyVectorObject's that will show up in user selection, but won't be visible.  So if a user taps on one, you get the vector in your delegate.  Otherwise, no one will know it's there.
    @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
  */
- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors;

/** @brief Change the representation of the given vector features.
    @details This will change how any vector features represented by the compObj look.
    @details You can change kMaplyColor, kMaplyMinVis, kMaplyMaxVis, and kMaplyDrawPriority.
  */
- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc;

// Note: Porting
///** @brief Adds the MaplyVectorObject's passed in as lofted polygons.
//    @details Lofted polygons are filled polygons draped on top of the globe with height.  By using a transparent color, these can be used to represent selection or relative values on the globe (or map).
// 
//    @param polys An NSArray of MaplyVectorObject.
//    @param key This is part of an old caching system that's no longer necessary.  Set it to nil.
//    @param cacheDb This is part of an old caching system that's no longer necessary.  Set it to nil.
//    @param desc The desciption dictionary which controls how the lofted polys will look.  It takes the following entries.
// 
// |Key|Type|Description|
// |:--|:---|:----------|
// |kMaplyColor|UIColor|Color we'll use for the lofted polygons.  A bit of alpha looks good.|
// |kMaplyLoftedPolyHeight|NSNumber|Height of the top of the lofted polygon in display units.  For the globe display units are based on a radius of 1.0.|
// |kMaplyLoftedPolyTop|NSNumber boolean|If on we'll create the geometry for the top.  On by default.|
// |kMaplyLoftedPolySide|NSNumber boolean|If on we'll create geometry for the sides.  On by default.|
// |kMaplyLoftedPolyGridSize|NSNumber|The size of the grid (in degrees) we'll use to chop up the vector features to make them follow the sphere (for a globe).|
// |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The lofted polys will only be visible if the user is above this height.  Off by default.|
// |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The lofted polys will only be visible if the user is below this height.  Off by default.|
// |kMaplyFade|NSNumber|The number of seconds to fade a lofted poly in when it appears and out when it disappears.|
// |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyLoftedPolysShapePriorityDefault.|
// |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's on by default, meaning that it can be occluded by geometry coming before it.|
// |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
// |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
//  
// @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
//  */
//- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc;

// Note: Porting
///// @brief Add a view tracker to move a UIView around based on a geographic location.
//- (void)addViewTracker:(MaplyViewTracker *)viewTrack;
//
///// @brief Remove an existing view tracker.
//- (void)removeViewTrackForView:(UIView *)view;

///** @brief Add an image as a texture and return a MaplyTexture to track it.
//    @details We reference count UIImages attached to Maply objects, but that has a couple of drawbacks.  First, it retains the UIImage and if that's large, that's a waste of memory.  Second, if you're adding and removing Maply objects you may repeatedly create and delete the same UIImage, which is a waste of CPU.
//    @details This method solves the problem by letting you create the texture associated with the UIImage and use it where you like.  You can assign these in any place a UIImage is accepted on Maply objects.
//    @details You don't have call this before using a UIImage in a MaplyScreenMarker or other object.  The system takes care of it for you.  This is purely for optimization.
//    @param image The image we wish to retain the texture for.
//    @param imageFormat If we create this image, this is the texture format we want it to use.
// 
// | Image Format | Description |
// |:-------------|:------------|
// | MaplyImageIntRGBA | 32 bit RGBA with 8 bits per channel.  The default. |
// | MaplyImageUShort565 | 16 bits with 5/6/5 for RGB and none for A. |
// | MaplyImageUShort4444 | 16 bits with 4 bits for each channel. |
// | MaplyImageUShort5551 | 16 bits with 5/5/5 bits for RGB and 1 bit for A. |
// | MaplyImageUByteRed | 8 bits, where we choose the R and ignore the rest. |
// | MaplyImageUByteGreen | 8 bits, where we choose the G and ignore the rest. |
// | MaplyImageUByteBlue | 8 bits, where we choose the B and ignore the rest. |
// | MaplyImageUByteAlpha | 8 bits, where we choose the A and ignore the rest. |
// | MaplyImageUByteRGB | 8 bits, where we average RGB for the value. |
// | MaplyImage4Layer8Bit | 32 bits, four channels of 8 bits each.  Just like MaplyImageIntRGBA, but a warning not to do anything too clever in sampling. |
//
//    @param threadMode For MaplyThreadAny we'll do the add on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the add.  MaplyThreadAny is preferred.
// 
//    @return A MaplyTexture you'll want to keep track of.  It goes out of scope, the OpenGL ES texture will be deleted.
// */
- (MaplyTexture *)addTexture:(UIImage *)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags mode:(MaplyThreadMode)threadMode;

///** @brief Remove the OpenGL ES texture associated with the given MaplyTexture.
//    @details MaplyTexture's will remove their associated OpenGL textures when they go out of scope.  This method does it expicitly and clears out the internals of the MaplyTexture.
//    @details Only call this if you're managing the texture explicitly and know you're finished with them.
//  */
- (void)removeTexture:(MaplyTexture *)image mode:(MaplyThreadMode)threadMode;

// Note: Porting
///** @brief Set the max number of objects for the layout engine to display.
//    @details The layout engine works with screen objects, such MaplyScreenLabel and MaplyScreenMaker.  If those have layoutImportance set, this will control the maximum number we can display.
//  */
//- (void)setMaxLayoutObjects:(int)maxLayoutObjects;

/// @brief Calls removeObjects:mode: with MaplyThreadAny.
- (void)removeObject:(MaplyComponentObject *)theObj;

/// @brief Calls removeObjects:mode: with MaplyThreadAny.
- (void)removeObjects:(NSArray *)theObjs;

/** @brief Remove all information associated with the given MaplyComponentObject's.
    @details Every add call returns a MaplyComponentObject.  This will remove any visible features, textures, selection data, or anything else associated with it.
    @param theObjs The MaplyComponentObject's we wish to remove.
    @param threadMode For MaplyThreadAny we'll do the removal on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the removal.  MaplyThreadAny is preferred.
 */
- (void)removeObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode;

/** @brief Disable a group of MaplyComponentObject's all at once.
    @details By default all of the geometry created for a given object will appear.  If you set kMaplyEnable to @(NO) then it will exist, but not appear.  This has the effect of setting kMaplyEnable to @(NO).
    @param theObjs The objects to disable.
    @param threadMode For MaplyThreadAny we'll do the disable on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the disable.  MaplyThreadAny is preferred.
  */
- (void)disableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode;

/** @brief Enable a group of MaplyComponentObject's all at once.
    @details By default all of the geometry created for a given object will appear.  If you set kMaplyEnable to @(NO) then it will exist, but not appear.  This has the effect of setting kMaplyEnable to @(YES).
    @param theObjs The objects to enable.
    @param threadMode For MaplyThreadAny we'll do the enable on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the enable.  MaplyThreadAny is preferred.
 */
- (void)enableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode;

/** @brief Call this to start journaling changes for this thread.
    @details Your can collect up your add/remove/enable changes on the current thread.  Call startChanges to start collecting and endChanges to flush the changes.
    @details This has no real meaning on the main thread and don't collect too many changes.  They take memory.
  */
- (void)startChanges;

/** @brief Call this to flush your journal changes out ot the scene.
    @details This is the other end of startChanges.
  */
- (void)endChanges;

// Note: Porting
///** @brief Add the given active object to the scene.
//    @details Active objects are used for immediate, frame based updates.  They're fairly expensive, so be careful.  After you create one, you add it to the scene here.
//  */
//- (void)addActiveObject:(MaplyActiveObject *)theObj;
//
///// @brief Remove an active object from the scene.
//- (void)removeActiveObject:(MaplyActiveObject *)theObj;
//
///// @brief Remove an array of active objects from the scene
//- (void)removeActiveObjects:(NSArray *)theObjs;

/** @brief Add a MaplyViewControllerLayer to the globe or map.
    @details At present, layers are for paged geometry such as image tiles or vector tiles.  You can create someting like a MaplyQuadImageTilesLayer, set it up and then hand it to addLayer: to add to the scene.
  */
- (bool)addLayer:(MaplyViewControllerLayer *)layer;

/// @brief Remove a MaplyViewControllerLayer from the globe or map.
- (void)removeLayer:(MaplyViewControllerLayer *)layer;

/// @brief Remove zero or more MaplyViewControllerLayer objects from the globe or map.
- (void)removeLayers:(NSArray *)layers;

/// @brief Remove all the user created MaplyViewControllerLayer objects from the globe or map.
- (void)removeAllLayers;

/** @brief Utility routine to convert from a lat/lon (in radians) to display coordinates
    @details This is a simple routine to get display coordinates from geocoordinates.  Display coordinates for the globe are based on a radius of 1.0 and an origin of (0,0,0).
    @return The input coordinate in display coordinates.
  */
- (MaplyCoordinate3d)displayPointFromGeo:(MaplyCoordinate)geoCoord;

/** @brief If you've paused the animation earlier, this will start it again.
    @details The renderer relies on a CADisplayLink.  If it's paused, this will unpause it.
  */
- (void)startAnimation;

/** @brief Pause the animation. 
    @details The renderer relies on a CADisplayLink.  This will pause it.  You'll want to do this if your app is going into the background or if you generally want the OpenGL ES code to stop doing anything.
  */
- (void)stopAnimation;

/** @brief Add a compiled shader.  We'll refer to it by the scene name.
    @details Once you've create a MaplyShader, you'll need to add it to the scene to use it.
    @param shader The working shader (be sure valid is true) to add to the scene.
    @param sceneName How we'll refer to it in the scene. If you use one of the well known scene names, you can replace the default shader.  The well known scene names are as follows.
 
 |Scene Name|Purpose|
 |:---------|:------|
 |kMaplyShaderDefaultTri|The shader used on triangles by default when there is lighting.|
 |kMaplyShaderDefaultTriNoLighting|The shader used when lighting is explicitly turned off.|
 |kMaplyShaderDefaultTriMultiTex|The shader used when drawables have more than one texture.|
 |kMaplyShaderDefaultLine|The shader used for line drawing on the globe.  This does a tricky bit of backface culling.|
 |kMaplyShaderDefaultLineNoBackface|The shader used for line drawing on the map.  This does no backface culling.|
  */
- (void)addShaderProgram:(MaplyShader *)shader sceneName:(NSString *)sceneName;

/** @brief Look for a shader with the given name.  
    @details This is the shader's own name as specified in the init call, not the scene name as might be specified in addShaderProgram:sceneName:
    @return Returns the registered shader if it found one.
  */
- (MaplyShader *)getShaderByName:(NSString *)name;

/// @brief Turn on/off performance output (goes to the log periodically).
@property (nonatomic,assign) bool performanceOutput;

@end
