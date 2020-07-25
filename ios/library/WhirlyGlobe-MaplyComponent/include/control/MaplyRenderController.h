/*
 *  MaplyRenderController.h
 *  WhirlyGlobeMaplyComponent
 *
 *  Created by Stephen Gifford on 1/19/18.
 *  Copyright 2012-2018 Saildrone Inc.
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

#import "math/MaplyCoordinate.h"
#import "visual_objects/MaplyScreenMarker.h"
#import "visual_objects/MaplyVectorObject.h"
#import "visual_objects/MaplyComponentObject.h"
#import "MaplySharedAttributes.h"
#import "rendering/MaplyLight.h"
#import "rendering/MaplyShader.h"
#import "visual_objects/MaplyTexture.h"
#import "visual_objects/MaplyParticleSystem.h"
#import "visual_objects/MaplyPoints.h"
#import "visual_objects/MaplyCluster.h"
#import "rendering/MaplyRenderTarget.h"
#import "control/MaplyActiveObject.h"
#import "control/MaplyControllerLayer.h"

@class MaplyRemoteTileFetcher;

/// Where we'd like an add to be executed.  If you need immediate feedback,
///  then be on the main thread and use MaplyThreadCurrent.  Any is the default.
typedef NS_ENUM(NSInteger, MaplyThreadMode) {
    MaplyThreadCurrent,
    MaplyThreadAny,
};

/// The various image formats we support.  RGBA is the default, and most expensive.
typedef NS_ENUM(NSInteger, MaplyQuadImageFormat) {
    MaplyImageIntRGBA,
    MaplyImageUShort565,
    MaplyImageUShort4444,
    MaplyImageUShort5551,
    MaplyImageUByteRed,MaplyImageUByteGreen,MaplyImageUByteBlue,MaplyImageUByteAlpha,
    MaplyImageUByteRG,
    MaplyImageUByteRGB,
    MaplyImageETC2RGB8,MaplyImageETC2RGBA8,MaplyImageETC2RGBPA8,
    MaplyImageEACR11,MaplyImageEACR11S,MaplyImageEACRG11,MaplyImageEACRG11S,
    MaplyImage4Layer8Bit,
    // Metal only
    MaplyImageSingleFloat16,MaplyImageSingleFloat32,MaplyImageDoubleFloat16,MaplyImageDoubleFloat32,MaplyImageQuadFloat16,MaplyImageQuadFloat32,
    MaplyImageUInt32,MaplyImageDoubleUInt32,MaplyImageQuadUInt32
};

/// Wrap values for certain types of textures
#define MaplyImageWrapNone (0)
#define MaplyImageWrapX (1<<0)
#define MaplyImageWrapY (1<<1)

@class MaplyRenderController;

/// The system can set up as either GL or Metal
typedef NS_ENUM(NSInteger, MaplyRenderType) {
    MaplyRenderGLES,
    MaplyRenderMetal,
    MaplyRenderUnknown
};

/**
    Render Controler Protocol defines the methods required of a render controller.
 
    The view controllers and offscreen rendere implement this protocol.
  */
@protocol MaplyRenderControllerProtocol

/**
 Set the offset for the screen space objects.
 
 In general you want the screen space objects to appear on top of everything else.  There used to be structural versions for this, but now you can mix and match where everything appears.  This controls the offset that's used to push screen space objects behind everything else in the list (and thus, on top).
 
 If you set this to 0, you can control the ordering of everything more precisely.
 */
@property (nonatomic,assign) int screenObjectDrawPriorityOffset;

/**
 Clear all the currently active lights.
 
 There are a default set of lights, so you'll want to do this before adding your own.
 */
- (void)clearLights;

/**
 Reset the lighting back to its default state at startup.
 
 This clears out all the lights and adds in the default starting light source.
 */
- (void)resetLights;

/**
 Add the given light to the list of active lights.
 
 This method will add the given light to our active lights.  Most shaders will recognize these lights and do the calculations.  If you have a custom shader in place, it may or may not use these.
 
 Triangle shaders use the lights, but line shaders do not.
 */
- (void)addLight:(MaplyLight *__nonnull)light;

/// Remove the given light (assuming it's active) from the list of lights.
- (void)removeLight:(MaplyLight *__nonnull)light;

/**
 Set the rendering hints to control how the renderer is configured.
 
 This is a bit vestigial, but still has a few important uses.  The hints should be set right after the init call.  Any later and they'll probably be ignored.
 
 The rendering hints are as follows.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyRenderHintZBuffer|bool|If set, we'll explicitly turn on the Z buffer.  Normally it's off until a drawable requests it, allowing us to play neat tricks with overlays.  The only time you should be turning this on is if you're doing 3D elevation.  The default is off.|
 |kMaplyRenderHintCulling|bool|If set, we'll use the internal culling logic.  Texture and drawable atlases have largely made this pointless.  Leave it off unless you have a compelling reason to turn it on.|
 |kMaplyRendererLightingMode|NSString|This can be set to @"none", in which case we use optimized shaders that do no lighting or "regular".  The latter is the default.|
 */
- (void)setHints:(NSDictionary *__nonnull)hintsDict;

/**
    Add a cluster generator for making clustered marker images on demand.
    
    When the layout system clusters a bunch of markers or labels together, it needs new images to represent the cluster.
    
    You can provide a custom image for each group of markers by filling in one of these generates and passing it in.
  */
- (void)addClusterGenerator:(NSObject <MaplyClusterGenerator> *__nonnull)clusterGen;

/**
 Add one or more screen markers to the current scene.
 
 This method will add the given MaplyScreenMaker objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param markers An NSArray of MaplyScreenMarker objects.
 
 @param desc The desciption dictionary which controls how the markers will be constructed.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|The color we'll use for the rectangle that makes up a marker. White by default.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyDrawPriority|NSNumber|If set, the markers are sorted by this number.  Larger numbers will be sorted later.|
 |kMaplyFade|NSNumber|The number of seconds to fade a marker in when it appears and out when it disappears.|
 |kMaplyFadeIn|NSNumber|The number of seconds to fade a marker in when it appears.  This overrides kMaplyFade.|
 |kMaplyFadeOut|NSNumber|The number of seconds to fade a marker out when it disappears.  This override kMaplyFade.|
 |kMaplyFadeOutTime|NSNumber|If you want to create an object, just to have it fade out at a specific time, this is what you set.|
 |kMaplyShader|NSString|If set, this is the name of the MaplyShader to use when rendering the screen markers.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 |kMaplyEnableStart|NSNumber|If set, this controls when the resulting objects will be activated.|
 |kMaplyEnableEnd|NSNumber|If set, this controls when the resulting objects will be deactivated.|
 |kMaplyClusterGroup|NSNumber|If set, the screen markers will be clustered together according to the given group ID.  Off by default, but 0 is the default cluster.|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addScreenMarkers:(NSArray *__nonnull)markers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more 3D markers to the current scene.
 
 This method will add the given MaplyMarker objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param markers An NSArray of MaplyMarker objects.
 
 @param desc The desciption dictionary which controls how the markers will be constructed.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|The color we'll use for the rectangle that makes up a marker. White by default.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The marker will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a marker in when it appears and out when it disappears.|
 |kMaplyFadeIn|NSNumber|The number of seconds to fade a marker in when it appears.  This overrides kMaplyFade.|
 |kMaplyFadeOut|NSNumber|The number of seconds to fade a marker out when it disappears.  This override kMaplyFade.|
 |kMaplyFadeOutTime|NSNumber|If you want to create an object, just to have it fade out at a specific time, this is what you set.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyMarkerDrawPriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that the geometry will draw on top of anything (respecting the kMaplyDrawPriority).|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addMarkers:(NSArray *__nonnull)markers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more screen labels to the current scene.
 
 This method will add the given MaplyScreenLabel objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param labels An NSArray of MaplyScreenLabel objects.
 
 @param desc The desciption dictionary which controls how the labels will be constructed.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyTextColor|UIColor|Color we'll use for the text. Black by default.|
 |kMaplyBackgroundColor|UIColor|Color we'll use for the rectangle background.  Use clearColor to make this invisible.|
 |kMaplyFont|UIFont|The font we'll use for the text.|
 |kMaplyLabelHeight|NSNumber|Height of the text in points.|
 |kMaplyLabelWidth|NSNumber|Width of the text in points.  It's best to set Height and leave this out.  That way the width will be calculated by the toolkit.|
 |kMaplyJustify|NSString|This can be set to @"middle", @"left", or @"right" to justify the text around the location.|
 |kMaplyTextJustify|NSString|This can be kMaplyTextJustifyRight, kMaplyTextJustifyCenter, or kMaplyTextJustifyLeft|
 |kMaplyShadowSize|NSNumber|If set, we'll draw a shadow with the kMaplyShadowColor offset by this amount.  We recommend using an outline instead.|
 |kMaplyShadowColor|UIColor|If we're drawing a shadow, this is its color.|
 |kMaplyTextOutlineSize|NSNumber|If set, we'll draw an outline around the text (really draw it twice).  The outline will be this large.|
 |kMaplyTextOutlineColor|UIColor|If we're drawing an outline, it's in this color.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyDrawPriority|NSNumber|If set, the labels are sorted by this number.  Larger numbers will be sorted later.|
 |kMaplyFade|NSNumber|The number of seconds to fade a screen label in when it appears and out when it disappears.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 |kMaplyEnableStart|NSNumber|If set, this controls when the resulting objects will be activated.|
 |kMaplyEnableEnd|NSNumber|If set, this controls when the resulting objects will be deactivated.|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addScreenLabels:(NSArray *__nonnull)labels desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more 3D labels to the current scene.
 
 This method will add the given MaplyLabel objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param labels An NSArray of MaplyLabel objects.
 
 @param desc The desciption dictionary which controls how the labels will be constructed.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyTextColor|UIColor|Color we'll use for the text. Black by default.|
 |kMaplyBackgroundColor|UIColor|Color we'll use for the rectangle background.  Use clearColor to make this invisible.|
 |kMaplyFont|UIFont|The font we'll use for the text.|
 |kMaplyLabelHeight|NSNumber|Height of the text in display coordinates.  For the globe these are based on radius = 1.0.|
 |kMaplyLabelWidth|NSNumber|Width of the text in display coordinates.  It's best to set Height and leave this out.  That way the width will be calculated by the toolkit.|
 |kMaplyJustify|NSString|This can be set to @"middle", @"left", or @"right" to justify the text around the location.|
 |kMaplyShadowSize|NSNumber|If set, we'll draw a shadow with the kMaplyShadowColor offset by this amount.  We recommend using an outline instead.|
 |kMaplyShadowColor|UIColor|If we're drawing a shadow, this is its color.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The label will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a label in when it appears and out when it disappears.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyLabelDrawPriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that the geometry will draw on top of anything (respecting the kMaplyDrawPriority).|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addLabels:(NSArray *__nonnull)labels desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more vectors to the current scene.
 
 This method will add the given MaplyVectorObject objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
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
 |kMaplyVecTextureProjection|NSString|This controls how a texture is projected onto an areal feature.  By default we just use the geographic coordinates and stretch them out.  This looks odd for very large features.  If you set this to kMaplyProjectionTangentPlane then we'll take the center of the feature, make a tangent plane and then project the coordinates onto that tangent plane to get texture coordinates.  This looks nice at the poles.  If set to kMaplyProjectionScreen the texture is mapped on after screen space projection around the center of the feature.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a vector in when it appears and out when it disappears.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorDrawPriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that the geometry will draw on top of anything (respecting the kMaplyDrawPriority).|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 |kMaplySelectable|NSNumber boolean|Off by default.  When enabled, the vector feature will be selectable by a user.|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addVectors:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Make a copy of the base object and apply the attributes given for the new version.
 
 This call makes a cheap copy of the vectors in the given MaplyComponentObject and applies the given description to them.  You can use this to make a wider or thinner version of a set of vectors, or change their color, while continuing to draw the originals.  Or not, as the case may be.
 
 This is useful for vector maps where we tend to reuse the same geometry at multiple levels and with different colors and line widths.
 
 Instancing only works with a handful of visual changes.  For instance, you can't make a filled and non-filled version.
 
 @param baseObj The MaplyComponentObject returned by an addVectors: call.  This only works for vectors.
 
 @param desc The description dictionary with controls how vectors will be displayed.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the vector features.|
 |kMaplyVecWidth|NSNumber|If the geometry is not filled, this is the width of the GL lines.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorDrawPriorityDefault.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)instanceVectors:(MaplyComponentObject *__nonnull)baseObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more widened vectors to the current scene.
 
 Build widened vectors
 
 
 @param desc The description dictionary which controls how vectors will be displayed.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the features.|
 |kMaplyVecWidth|NSNumber|If the geometry is not filled, this is the width of the lines.|
 |kMaplyWideVecCoordType|NSNumber|Vectors can be widened in real coordinates (kMaplyWideVecCoordTypeReal) or screen coordinates (kMaplyWideVecCoordTypeScreen).  In the latter case they stay the same size now matter how you zoom.|
 |kMaplyWideVecJoinType|NSNumber|When lines meet in a join there are several options for representing them.  These include kMaplyWideVecMiterJoin, which is a simple miter join and kMaplyWideVecBevelJoin which is a more complicated bevel.  See http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty for how these look.|
 |kMaplyWideVecMiterLimit|NSNumber|When using miter joins you can trigger them at a certain threshold.|
 |kMaplyWideVecTexRepeatLen|NSNumber|This is the repeat size for a texture applied along the widened line.  For kMaplyWideVecCoordTypeScreen this is pixels.|
 |kMaplyVecTexture|UIImage or MaplyTexture|This the texture to be applied to the widened vector.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The vectors will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorDrawPriorityDefault.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addWideVectors:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more model instances.
 
 Each MaplyGeomInstance points to a MaplyGeomModel.  All those passed in here will be grouped and processed together.
 
 
 @param desc The description dictionary which controls how the models are displayed, selected, and so forth.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplySelectable|NSNumber boolean|Off by default.  When enabled, the vector feature will be selectable by a user.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addModelInstances:(NSArray *__nonnull)modelInstances desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or raw geometry models.
 
 Each MaplyGeometryModel holds points and triangles in display space.  These are relatively "raw" geometry and are passed to the geometry manager as is.
 
 
 @param desc The description dictionary which controls how the geometry is displayed, selected, and so forth.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplySelectable|NSNumber boolean|Off by default.  When enabled, the vector feature will be selectable by a user.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addGeometry:(NSArray *__nonnull)geom desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more MaplyShape children to the current scene.
 
 This method will add the given MaplyShape derived objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param shapes An NSArray of MaplyShape derived objects.
 
 @param desc The desciption dictionary which controls how the shapes will look.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the shape features.|
 |kMaplyShapeSampleX|NSNumber|Number of samples to use in one direction when converting to polygons.|
 |kMaplyShapeSampleY|NSNumber|Number of samples to use in the other direction when converting to polygons.|
 |kMaplyShapeInsideOut|NSNumber boolean|If set to YES, we'll make the spheres inside out and such.  Set to NO by default.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The shapes will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The shapes will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a shape in when it appears and out when it disappears.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorShapePriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's on by default, meaning that the geometry can be occluded by things drawn first.|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addShapes:(NSArray *__nonnull)shapes desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more MaplySticker objects to the current scene.
 
 This method will add the given MaplySticker objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param stickers An NSArray of MaplySticker derived objects.
 
 @param desc The desciption dictionary which controls how the stickers will look.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the stickers.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The stickers will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The stickers will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a sticker in when it appears and out when it disappears.|
 |kMaplySampleX|NSNumber|Stickers are broken up along two dimensions to adhere to the globe.  By default this is done adaptively.  If you want to override it, this is the X dimension for the sticker.|
 |kMaplySampleY|NSNumber|If you want to override it, this is the Y dimension for the sticker.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyVectorShapePriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's off by default, meaning that it will draw on top of things before it..|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 |kMaplyShader|NSString|If set, this is the name of the MaplyShader to use when rendering the sticker(s).|
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addStickers:(NSArray *__nonnull)stickers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Modify an existing sticker.  This only supports changing the active textures.
 
 This method will change attributes of a sticker that's currently in use.  At present that's just the images it's displaying.
 
 @param compObj The component object representing one or more existing stickers.
 
 @param desc The description dictionary for changes we're making to the sticker.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyStickerImages|NSARray|The array of images to apply to the sticker.  You can reuse old ones or introduce new ones.|
 */
- (void)changeSticker:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add one or more MaplyBillboard objects to the current scene.
 
 This method will add the given MaplyBillboard objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
 
 @param billboards An NSArray of MaplyBillboard objects.
 
 @param desc The description dictionary that controls how the billboards will look.  It takes the following entries.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the billboards.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The billboards will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The billboards will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyBillboardDrawPriorityDefault.|
 |kMaplyBillboardOrient|NSNumber|Controls the billboard orientation.  It's either directly toward the eye with kMaplyBillboardOrientEye or takes the ground into account with kMaplyBillboardOrientGround.  Ground is the default.
 
 
 @param threadMode MaplyThreadAny is preferred and will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.
 */
- (MaplyComponentObject *__nullable)addBillboards:(NSArray *__nonnull)billboards desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
    Add a particle system to the scene.
    
    This adds a particle system to the scene, but does not kick off any particles.
    
    @param partSys The particle system to start.
    
    @param desc Any additional standard parameters (none at present).
    
    @param threadMode MaplyThreadAny will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.  For particles, it's best to make a separate thread and use MaplyThreadCurrent.
  */
- (MaplyComponentObject *__nullable)addParticleSystem:(MaplyParticleSystem *__nonnull)partSys desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
    Change the render target for a particle system.

    This changes the render target for an existing particle system that's already been created.
    Can pass in nil, which means the particles are rendered to the screen directly.
    This change takes place immediately, so call it on the main thread.
 */
- (void)changeParticleSystem:(MaplyComponentObject *__nonnull)compObj renderTarget:(MaplyRenderTarget *__nullable)target;

/**
    Add a batch of particles to the current scene.
    
    Particles are short term objects, typically very small.  We create them in large groups for efficience.
    
    You'll need to fill out the MaplyParticleSystem initially and then the MaplyParticleBatch to create them.
    
    @param batch The batch of particles to add to an active particle system.
    
    @param threadMode MaplyThreadAny will use another thread, thus not blocking the one you're on.  MaplyThreadCurrent will make the changes immediately, blocking this thread.  For particles, it's best to make a separate thread and use MaplyThreadCurrent.
  */
- (void)addParticleBatch:(MaplyParticleBatch *__nonnull)batch mode:(MaplyThreadMode)threadMode;

/**
 Change the representation of the given vector features.
 
 This will change how any vector features represented by the compObj look.
 
 You can change kMaplyColor, kMaplyMinVis, kMaplyMaxVis, and kMaplyDrawPriority.
 
 This version takes a thread mode.
 */
- (void)changeVector:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Adds the MaplyVectorObject's passed in as lofted polygons.
 
 Lofted polygons are filled polygons draped on top of the globe with height.  By using a transparent color, these can be used to represent selection or relative values on the globe (or map).
 
 
 @param polys An NSArray of MaplyVectorObject.
 
 @param key This is part of an old caching system that's no longer necessary.  Set it to nil.
 
 @param cacheDb This is part of an old caching system that's no longer necessary.  Set it to nil.
 
 @param desc The desciption dictionary which controls how the lofted polys will look.  It takes the following entries.
 
 @param threadMode For MaplyThreadAny we'll do the add on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the add.  MaplyThreadAny is preferred.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the lofted polygons.  A bit of alpha looks good.|
 |kMaplyLoftedPolyHeight|NSNumber|Height of the top of the lofted polygon in display units.  For the globe display units are based on a radius of 1.0.|
 |kMaplyLoftedPolyBase|NSNumber|If present, we'll start the lofted poly at this height.  The height is in globe units, based on a radius of 1.0.|
 |kMaplyLoftedPolyTop|NSNumber boolean|If on we'll create the geometry for the top.  On by default.|
 |kMaplyLoftedPolySide|NSNumber boolean|If on we'll create geometry for the sides.  On by default.|
 |kMaplyLoftedPolyGridSize|NSNumber|The size of the grid (in degrees) we'll use to chop up the vector features to make them follow the sphere (for a globe).|
 |kMaplyLoftedPolyOutline|NSNumber boolean|If set to @(YES) this will draw an outline around the top of the lofted poly in lines.|
 |kMaplyLoftedPolyOutlineBottom|NSNumber boolean|If set to @(YES) this will draw an outline around the bottom of the lofted poly in lines.|
 |kMaplyLoftedPolyOutlineColor|UIColor|If the outline is one this is the outline's color.|
 |kMaplyLoftedPolyOutlineWidth|NSNumber|This is the outline's width if it's turned on.|
 |kMaplyLoftedPolyOutlineDrawPriority|NSNumber|Draw priority of the lines created for the lofted poly outline.|
 |kMaplyLoftedPolyOutlineSide|NSNumber boolean|If set and we're drawing an outline, this will create lines up the sides.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The lofted polys will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The lofted polys will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a lofted poly in when it appears and out when it disappears.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyLoftedPolysShapePriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's on by default, meaning that it can be occluded by geometry coming before it.|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addLoftedPolys:(NSArray *__nonnull)polys desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Add a group of points to the display.
 
 Adds a group of points all at once.  We're assuming you want to draw a lot of points, so you have to group them together into a MaplyPoints.
 
 
 @param points The points to add to the scene.
 
 @param desc The desciption dictionary which controls how the points will look.  It takes the following entries.
 
 @param threadMode For MaplyThreadAny we'll do the add on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the add.  MaplyThreadAny is preferred.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyColor|UIColor|Color we'll use for the lofted polygons.  A bit of alpha looks good.|
 |kMaplyMinVis|NSNumber|This is viewer height above the globe or map.  The lofted polys will only be visible if the user is above this height.  Off by default.|
 |kMaplyMaxVis|NSNumber|This is viewer height above the globe or map.  The lofted polys will only be visible if the user is below this height.  Off by default.|
 |kMaplyMinViewerDist|NSNumber|Minimum distance from the viewer at which to display object(s).|
 |kMaplyMaxViewerDist|NSNumber|Maximum distance from the viewer at which to display object(s).|
 |kMaplyViewableCenterX|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center X coordinate.|
 |kMaplyViewableCenterY|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Y coordinate.|
 |kMaplyViewableCenterZ|MaplyCoordinate3dWrapper|When evaulating min/max viewer distance, we'll use this center Z coordinate.|
 |kMaplyFade|NSNumber|The number of seconds to fade a lofted poly in when it appears and out when it disappears.|
 |kMaplyDrawPriority|NSNumber|Geometry is sorted by this value before being drawn.  This ensures that some objects can come out on top of others.  By default this is kMaplyLoftedPolysShapePriorityDefault.|
 |kMaplyZBufferRead|NSNumber boolean|If set this geometry will respect the z buffer.  It's on by default, meaning that it can be occluded by geometry coming before it.|
 |kMaplyZBufferWrite|NSNumber boolean|If set this geometry will write to the z buffer.  That means following geometry that reads the z buffer will be occluded.  This is off by default.|
 |kMaplyEnable|NSNumber boolean|On by default, but if off then the feature exists, but is not turned on.  It can be enabled with enableObjects:|
 
 
 @return Returns a MaplyComponentObject, which can be used to make modifications or delete the objects created.
 */
- (MaplyComponentObject *__nullable)addPoints:(NSArray * __nonnull)points desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Represent an image as a MaplyTexture.
 
 This version of addTexture: allows more precise control over how the texture is represented.  It replaces the other addTexture: and addTextureToAtlas calls.
 
 @param image The UIImage to add as a texture.
 
 @param desc A description dictionary controlling how the image is converted to a texture and represented in the system.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyTexFormat|NSNumber|The texture format to use for the image.  Consult addTexture:imageFormat:wrapFlags:mode: for a list.  Default is MaplyImageIntRGBA.|
 |kMaplyTexMinFilter|NSNumber|Filter to use for minification.  This can be kMaplyMinFilterNearest or kMaplyMinFilterLinear. Default is kMaplyMinFilterLinear.|
 |kMaplyTexMagFilter|NSNumber|Filter to use for magnification.  This can be kMaplyMinFilterNearest or kMaplyMinFilterLinear. Default is kMaplyMinFilterLinear.|
 |kMaplyTexWrapX|NSNumber boolean|Texture wraps in x direction.  Off by default.|
 |kMaplyTexWrapY|NSNumber boolean|Texture wraps in y direction.  Off by default.|
 |kMaplyTexAtlas|NSNumber boolean|If set, the texture goes into an appropriate atlas.  If not set, it's a standalone texture (default).|
 
 @param threadMode For MaplyThreadAny we'll do the add on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the add.  MaplyThreadAny is preferred.
 */
- (MaplyTexture *__nullable)addTexture:(UIImage *__nonnull)image desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

/**
 Create an empty texture and return it.
 
 Empty textures are used for offscreen rendering and other crazy stuff.  You probably don't want to do this.
 
 @param spec The description dictionary controlling the format and other textures goodies.
 
 |Key|Type|Description|
 |:--|:---|:----------|
 |kMaplyTexFormat|NSNumber|The texture format to use for the image.  Consult addTexture:imageFormat:wrapFlags:mode: for a list.  Default is MaplyImageIntRGBA.|
 |kMaplyTexMinFilter|NSNumber|Filter to use for minification.  This can be kMaplyMinFilterNearest or kMaplyMinFilterLinear. Default is kMaplyMinFilterLinear.|
 |kMaplyTexMagFilter|NSNumber|Filter to use for magnification.  This can be kMaplyMinFilterNearest or kMaplyMinFilterLinear. Default is kMaplyMinFilterLinear.|
 |kMaplyTexWrapX|NSNumber boolean|Texture wraps in x direction.  Off by default.|
 |kMaplyTexWrapY|NSNumber boolean|Texture wraps in y direction.  Off by default.|
 |kMaplyTexAtlas|NSNumber boolean|If set, the texture goes into an appropriate atlas.  If not set, it's a standalone texture (default).|
 |kMaplyTexMipmap|NSNumber boolean|If set, we'll create the given texture with mipmap levels.|
 
 @param sizeX The horizontal size of the textures (in pixels).
 
 @param sizeY Vertical size of the texture (in pixels).
 */
- (MaplyTexture *__nullable)createTexture:(NSDictionary * _Nullable)spec sizeX:(int)sizeX sizeY:(int)sizeY mode:(MaplyThreadMode)threadMode;

/**
 Creates a new texture that references part of an existing texture.
 
 @param x Horizontal offset within the existing texture.
 @param y Vertical offset within the existing texture.
 @param width Width of the chunk to make a new texture.
 @param height Height of the chunk to make a new texture.
 @param threadMode For MaplyThreadAny we'll do the add on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the add.  MaplyThreadAny is preferred if you're on the main thread.
 */
- (MaplyTexture *__nullable)addSubTexture:(MaplyTexture *__nonnull)tex xOffset:(int)x yOffset:(int)y width:(int)width height:(int)height mode:(MaplyThreadMode)threadMode;

/**
 Remove the OpenGL ES textures associated with the given MaplyTextures.
 
 MaplyTextures will remove their associated OpenGL textures when they go out of scope.  This method does it expicitly and clears out the internals of the MaplyTexture.
 
 Only call this if you're managing the texture explicitly and know you're finished with them.
 */
- (void)removeTextures:(NSArray *__nonnull)texture mode:(MaplyThreadMode)threadMode;

/**
 Add a render target to the system
 
 Sets up a render target and will start rendering to it on the next frame.
 
 Keep the render target around so you can remove it later.
 */
- (void)addRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget;

/**
 Set the texture a given render target is writing to.
 
 Render targets start out with one, but you may wish to change it.
 */
- (void)changeRenderTarget:(MaplyRenderTarget * __nonnull)renderTarget tex:(MaplyTexture * __nullable)tex;

/**
 Remove the given render target from the system.
 
 Ask the system to stop drawing to the given render target.  It will do this on the next frame.
 */
- (void)removeRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget;

/**
    Normally the layout layer runs periodically if you change something or when you move around.
    You can ask it to run ASAP right here.  Layout runs on its own thread, so there may still be a delay.
 */
- (void)runLayout;

/**
  Request a one time clear for the render target.

  Rather than clearing every frame, you may want to specifically request a clear.  This will
  be executed at the next frame and then not again.
*/
- (void)clearRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget mode:(MaplyThreadMode)threadMode;

/**
 Remove all information associated with the given MaplyComponentObject's.
 
 Every add call returns a MaplyComponentObject.  This will remove any visible features, textures, selection data, or anything else associated with it.
 
 @param theObjs The MaplyComponentObject's we wish to remove.
 
 @param threadMode For MaplyThreadAny we'll do the removal on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the removal.  MaplyThreadAny is preferred.
 */
- (void)removeObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode;

/**
 Disable a group of MaplyComponentObject's all at once.
 
 By default all of the geometry created for a given object will appear.  If you set kMaplyEnable to @(NO) then it will exist, but not appear.  This has the effect of setting kMaplyEnable to @(NO).
 
 @param theObjs The objects to disable.
 
 @param threadMode For MaplyThreadAny we'll do the disable on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the disable.  MaplyThreadAny is preferred.
 */
- (void)disableObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode;

/**
 Enable a group of MaplyComponentObject's all at once.
 
 By default all of the geometry created for a given object will appear.  If you set kMaplyEnable to @(NO) then it will exist, but not appear.  This has the effect of setting kMaplyEnable to @(YES).
 
 @param theObjs The objects to enable.
 
 @param threadMode For MaplyThreadAny we'll do the enable on another thread.  For MaplyThreadCurrent we'll block the current thread to finish the enable.  MaplyThreadAny is preferred.
 */
- (void)enableObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode;

/**
 Pass a uniform block through to a shader.  Only for Metal.
 
 Custom Metal shaders may have their own uniform blocks associated with a known bufferID.
 This is how you pass those through for objects you've already created.
 Useful for things like custom animation.
 */
- (void)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID forObjects:(NSArray<MaplyComponentObject *> *__nonnull)compObjs mode:(MaplyThreadMode)threadMode;

/**
 Call this to start journaling changes for this thread.
 
 Your can collect up your add/remove/enable changes on the current thread.  Call startChanges to start collecting and endChanges to flush the changes.
 
 This has no real meaning on the main thread and don't collect too many changes.  They take memory.
 */
- (void)startChanges;

/**
 Call this to flush your journal changes out ot the scene.
 
 This is the other end of startChanges.
 */
- (void)endChanges;

/**
 Add a compiled shader.  We'll refer to it by the scene name.
 
 Once you've create a MaplyShader, you'll need to add it to the scene to use it.
 
 @param shader The working shader (be sure valid is true) to add to the scene.
 */
- (void)addShaderProgram:(MaplyShader *__nonnull)shader;

/**
 Look for a shader with the given name.
 
 This is the shader's own name as specified in the init call, not the scene name as might be specified in addShaderProgram:sceneName:
 
 @return Returns the registered shader if it found one.
 */
- (MaplyShader *__nullable)getShaderByName:(NSString *__nonnull)name;

/**
 Remove a shader that was added earlier.
 */
- (void)removeShaderProgram:(MaplyShader *__nonnull)shader;

/**
 Return the coordinate system being used for the display.
 
 This returns the local coordinate system, which is used to unroll the earth (for the globe) or via a scaling factor (for the flat map).
 */
- (MaplyCoordinateSystem *__nullable)coordSystem;

- (void)setClearColor:(UIColor *__nonnull)clearColor;

/// Return the framebuffer size in pixels (no scale)
- (CGSize)getFramebufferSize;

// For internal use only
- (MaplyRenderController * __nullable)getRenderControl;

/// Return the renderer type being used
- (MaplyRenderType)getRenderType;

/**
    Add the given active object to the scene.
    
    Active objects are used for immediate, frame based updates.  They're fairly expensive, so be careful.  After you create one, you add it to the scene here.
  */
- (void)addActiveObject:(MaplyActiveObject *__nonnull)theObj;

/// Remove an active object from the scene.
- (void)removeActiveObject:(MaplyActiveObject *__nonnull)theObj;

/// Remove an array of active objects from the scene
- (void)removeActiveObjects:(NSArray *__nonnull)theObjs;

/**
    Add a MaplyControllerLayer to the globe or map.
    
    At present, layers are for paged geometry such as image tiles or vector tiles.  You can create someting like a MaplyQuadImageTilesLayer, set it up and then hand it to addLayer: to add to the scene.
  */
- (bool)addLayer:(MaplyControllerLayer *__nonnull)layer;

/// Remove a MaplyControllerLayer from the globe or map.
- (void)removeLayer:(MaplyControllerLayer *__nonnull)layer;

/// Remove zero or more MaplyControllerLayer objects from the globe or map.
- (void)removeLayers:(NSArray *__nonnull)layers;

/// Remove all the user created MaplyControllerLayer objects from the globe or map.
- (void)removeAllLayers;

/// Return a tile fetcher we may share between loaders
- (MaplyRemoteTileFetcher * __nonnull)addTileFetcher:(NSString * __nonnull)name;

/**
    If in Metal rendering mode, return the Metal device being used.
  */
- (id<MTLDevice> __nullable)getMetalDevice;

/**
   If in Metal rendering mode, return the shader library set up by the toolkit.
  */
- (id<MTLLibrary> __nullable)getMetalLibrary;

@end

/**
 The Render Controller is a standalone WhirlyGlobe-Maply renderer.
 
 This Render Controller is used for offline rendering.
 */
@interface MaplyRenderController : NSObject<MaplyRenderControllerProtocol>

/// Initialize as an offline renderer of a given target size of the given rendering type
- (instancetype __nullable)initWithSize:(CGSize)size mode:(MaplyRenderType)renderType;

/// Initialize as an offline renderer of a given target size with default renderer (Metal)
- (instancetype __nullable)initWithSize:(CGSize)size;

/// If set up in offline mode, this is how we draw
- (UIImage * __nullable)renderToImage;

/// Return the raw RGBA pixels from the rendered image rather than a UIImage
- (NSData * __nullable)renderToImageData;

@end
