/*
 *  MaplySharedAttributes.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 2/24/16.
 *  Copyright 2011-2016 mousebird consulting
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

#import "MaplySharedAttributes.h"

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
NSString* const kMaplyRenderHintZBuffer = @"zbuffer";
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
NSString* const kMaplyRenderHintCulling = @"culling";
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
NSString* const kMaplyRendererLightingMode = @"rendererLightingMode";

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
NSString* const kMaplyDrawOffset = @"drawOffset";
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
NSString* const kMaplyDrawPriority = @"drawPriority";
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
NSString* const kMaplyMinVis = @"minVis";
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
NSString* const kMaplyMaxVis = @"maxVis";
/// Minimum distance from the viewer at which to display geometry.
NSString* const kMaplyViewerMinDist = @"minviewerdist";
/// Maximum distance from the viewer at which to display geometry.
NSString* const kMaplyViewerMaxDist = @"maxviewerdist";
/// Center to use when evaluating distance to viewable geometry (X)
NSString* const kMaplyViewableCenterX = @"viewablecenterx";
/// Center to use when evaluating distance to viewable geometry (Y)
NSString* const kMaplyViewableCenterY = @"viewablecentery";
/// Center to use when evaluating distance to viewable geometry (Z)
NSString* const kMaplyViewableCenterZ = @"viewablecenterz";
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
NSString* const kMaplyFade = @"fade";
/// Fade the feature in over time.
NSString* const kMaplyFadeIn = @"fadein";
/// Fade the feature out over time
NSString* const kMaplyFadeOut = @"fadeout";
/// When to start fading out
NSString* const kMaplyFadeOutTime = @"fadeouttime";
/// Enable or disable an object.  This can be used to create an object disabled.
NSString* const kMaplyEnable = @"enable";
/// If set, we'll enable the objects only between the start and end time
NSString* const kMaplyEnableStart = @"enablestart";
/// If set, we'll enable the objects only between the start and end time
NSString* const kMaplyEnableEnd = @"enableend";
/// Request a given object take the z buffer into account
NSString* const kMaplyZBufferRead = @"zbufferread";
/// Have a given object write itself to the z buffer
NSString* const kMaplyZBufferWrite = @"zbufferwrite";

/// Assign a shader program to a particular feature.  Use the shader program's name
NSString* const kMaplyShader = @"shader";
/// An NSDictionary containing uniforms to apply to a shader before drawing
NSString* const kMaplyShaderUniforms = @"shaderuniforms";

/// Stars, moon, stars, atmosphere
const int kMaplyStarsDrawPriorityDefault = 0;
const int kMaplySunDrawPriorityDefault = 2;
const int kMaplyMoonDrawPriorityDefault = 3;
const int kMaplyAtmosphereDrawPriorityDefault = 10;
/// Where we start image layer draw priorities
const int kMaplyImageLayerDrawPriorityDefault = 100;
/// We'll start filling in features right around here
const int kMaplyFeatureDrawPriorityBase = 20000;
const int kMaplyStickerDrawPriorityDefault = 30000;
const int kMaplyMarkerDrawPriorityDefault = 40000;
const int kMaplyVectorDrawPriorityDefault = 50000;
const int kMaplyParticleSystemDrawPriorityDefault = 55000;
const int kMaplyLabelDrawPriorityDefault = 60000;
const int kMaplyLoftedPolysDrawPriorityDefault = 70000;
const int kMaplyShapeDrawPriorityDefault = 80000;
const int kMaplyBillboardDrawPriorityDefault = 90000;
const int kMaplyModelDrawPriorityDefault = 100000;

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
NSString* const kMaplyTextColor = @"textColor";
/// Background color for the text.  Takes a UIColor.
NSString* const kMaplyBackgroundColor = @"backgroundColor";
/// Font to use in rendering text.  Takes a UIFont.
NSString* const kMaplyFont = @"font";
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
NSString* const kMaplyLabelHeight = @"height";
/// Default width of the text.  See height for more info and, in general, use height instead.
NSString* const kMaplyLabelWidth = @"width";
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
NSString* const kMaplyJustify = @"justify";
/// If set, we'll draw a shadow behind each label with this as the stroke size
NSString* const kMaplyShadowSize = @"shadowSize";
/// If shadow size is being used, we can control the shadow color like so
NSString* const kMaplyShadowColor = @"shadowColor";
/// If outline is being used, we can control the color
NSString* const kMaplyTextOutlineSize = @"outlineSize";
/// If outline is being used, we can control the stroke size
NSString* const kMaplyTextOutlineColor = @"outlineColor";
NSString* const kMaplyTexSizeX = @"texsizex";
NSString* const kMaplyTexSizeY = @"texsizey";
NSString* const kMaplyTextJustify = @"textjustify";
NSString* const kMaplyTextJustifyRight = @"right";
NSString* const kMaplyTextJustifyLeft = @"left";
NSString* const kMaplyTextJustifyCenter = @"center";

/// These are used for screen and regular markers.
NSString* const kMaplyClusterGroup = @"clusterGroup";

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
NSString* const kMaplyColor = @"color";

/// Width is used by the vector layer for line widths
NSString* const kMaplyVecWidth = @"width";

/// If filled is set, we draw the areals as filled polygons
NSString* const kMaplyFilled = @"filled";

/// If set, the texture to apply to the feature
NSString* const kMaplyVecTexture = @"texture";
/// X scale for textures applied to vectors
NSString* const kMaplyVecTexScaleX = @"texscalex";
/// Y scale for textures applied to vectors
NSString* const kMaplyVecTexScaleY = @"texscaley";

/// The projection to use when generating texture coordinates
NSString* const kMaplyVecTextureProjection = @"texprojection";
/// Tangent plane projection for texture coordinates
NSString* const kMaplyProjectionTangentPlane = @"texprojectiontanplane";
/// Screen space "projection" for texture coordinates
NSString* const kMaplyProjectionScreen = @"texprojectionscreen";

/// If set to true we'll centered any drawables we create for features
/// This fixes the jittering problem when zoomed in close
NSString* const kMaplyVecCentered = @"centered";

/// Center of the feature, to use for texture calculations
NSString* const kMaplyVecCenterX = @"veccenterx";
NSString* const kMaplyVecCenterY = @"veccentery";

/// For wide vectors, we can widen them in screen space or display space
NSString* const kMaplyWideVecCoordType = @"wideveccoordtype";

/// Widened vectors are widened in real space.  The width is in meters.
NSString* const kMaplyWideVecCoordTypeReal = @"real";
/// Widened vectors are widened in screen space.  The width is in pixels.
NSString* const kMaplyWideVecCoordTypeScreen = @"screen";

/// For wide vectors we can control the line joins
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
NSString* const kMaplyWideVecJoinType = @"wideveclinejointype";

/// Widened vectors are joined with miters
NSString* const kMaplyWideVecMiterJoin = @"miter";
// Note: Not yet implemented
/// Widened vectors are joined with a curve
//NSString* const kMaplyWideVecRoundJoin @"round"
/// Widened vectors are joined with a bevel
NSString* const kMaplyWideVecBevelJoin = @"bevel";

/// Number of pixels to use in blending the edges of the wide vectors
NSString* const kMaplyWideVecEdgeFalloff = @"edgefalloff";

/// For wide vectors we can control the ends
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
//NSString* const kMaplyWideVecLineCapType @"wideveclinecaptype"

// Note: These are not currently implemented

/// Widened vector ends are flush
//NSString* const kMaplyWideVecButtCap @"butt"
/// Widened vector ends are round (e.g. hot dog roads)
//NSString* const kMaplyWideVecRoundCap @"round"
/// Widened vector ends are extended a bit and then flush
//NSString* const kMaplyWideVecSquareCap @"square"

/// Miter joins will turn to bevel joins past this number of degrees
NSString* const kMaplyWideVecMiterLimit = @"miterLimit";

/// This is the length you'd like the texture to start repeating after.
/// It's real world coordinates for kMaplyWideVecCoordTypeReal and pixel size for kMaplyWideVecCoordTypeScreen
NSString* const kMaplyWideVecTexRepeatLen = @"repeatSize";

/// If set we'll break up a vector feature to the given epsilon on a globe surface
NSString* const kMaplySubdivEpsilon = @"subdivisionepsilon";
/// If subdiv epsilon is set we'll look for a subdivision type. Default is simple.
NSString* const kMaplySubdivType = @"subdivisiontype";
/// Subdivide the vector edges along a great circle
NSString* const kMaplySubdivGreatCircle = @"greatcircle";
/// Subdivide into a fixed number of segmenets
NSString* const kMaplySubdivStatic = @"static";
/// Subdivide the vectors edges along lat/lon
NSString* const kMaplySubdivSimple = @"simple";
/// Clip features along a grid of the given size
NSString* const kMaplySubdivGrid = @"grid";
/// Used to turn off selection in vectors
NSString* const kMaplySelectable = @"selectable";

/// These are used for stickers

/// Sampling size along one dimension
NSString* const kMaplySampleX = @"sampleX";
/// Sampling size along one dimension
NSString* const kMaplySampleY = @"sampleY";
/// Images to use when changing a sticker
NSString* const kMaplyStickerImages = @"images";
/// Image format to use for the new images
NSString* const kMaplyStickerImageFormat = @"imageformat";

/// These are used for billboards

/// Billboard orientation
NSString* const kMaplyBillboardOrient = @"billboardorient";
/// Billboards are oriented toward the eye, but rotate on the ground
NSString* const kMaplyBillboardOrientGround = @"billboardorientground";
/// Billboards are oriented only towards the eye
NSString* const kMaplyBillboardOrientEye = @"billboardorienteye";

/// These are used for lofted polygons

/// Height above the ground
NSString* const kMaplyLoftedPolyHeight = @"height";
/// Boolean that turns on/off top (on by default)
NSString* const kMaplyLoftedPolyTop = @"top";
/// Boolean that turns on/off sides (on by default)
NSString* const kMaplyLoftedPolySide = @"side";
/// If present, we'll start the lofted poly above 0 height
NSString* const kMaplyLoftedPolyBase = @"base";
/// Grid size we used to chop the lofted polygons up (10 degress by default)
NSString* const kMaplyLoftedPolyGridSize = @"gridsize";
/// If set to @(YES) this will draw an outline around the top of the lofted poly in lines
NSString* const kMaplyLoftedPolyOutline = @"outline";
/// If set to @(YES) this will draw an outline around the bottom of the lofted poly in lines
NSString* const kMaplyLoftedPolyOutlineBottom = @"outlineBottom";
/// If the outline is one this is the outline's color
NSString* const kMaplyLoftedPolyOutlineColor = @"outlineColor";
/// This is the outline's width if it's turned on
NSString* const kMaplyLoftedPolyOutlineWidth = @"outlineWidth";
/// Draw priority of the lines created for the lofted poly outline
NSString* const kMaplyLoftedPolyOutlineDrawPriority = @"outlineDrawPriority";
/// If set and we're drawing an outline, this will create lines up the sides
NSString* const kMaplyLoftedPolyOutlineSide = @"outlineSide";

/// These are used for shapes

/// Samples (x) to use when converting shape to polygons
NSString* const kMaplyShapeSampleX = @"shapesamplex";
/// Samples (y) to use when converting shape to polygons
NSString* const kMaplyShapeSampleY = @"shapesampley";
/// If set to true, we'll tessellate a shape using the opposite vertex ordering
NSString* const kMaplyShapeInsideOut = @"shapeinsideout";
/// Center for the shape geometry
NSString* const kMaplyShapeCenterX = @"shapecenterx";
NSString* const kMaplyShapeCenterY = @"shapecentery";
NSString* const kMaplyShapeCenterZ = @"shapecenterz";

/// These are used by active vector objects
NSString* const kMaplyVecHeight = @"height";
NSString* const kMaplyVecMinSample = @"minSample";

/// These are used by the particle systems
NSString* const kMaplyPointSize = @"pointSize";
const float kMaplyPointSizeDefault = 4.0;

/// These are used by the texture
NSString* const kMaplyTexFormat = @"texformat";
NSString* const kMaplyTexMinFilter = @"texminfilter";
NSString* const kMaplyTexMagFilter = @"texmagfilter";
NSString* const kMaplyMinFilterNearest = @"texfilternearest";
NSString* const kMaplyMinFilterLinear = @"texfilterlinear";
NSString* const kMaplyTexAtlas = @"texatlas";
NSString* const kMaplyTexWrapX = @"texwrapx";
NSString* const kMaplyTexWrapY = @"texwrapy";

/// These are the various shader programs we set up by default
NSString* const kMaplyShaderDefaultTri = @"Default Triangle;lighting=yes";
NSString* const kMaplyDefaultTriangleShader = @"Default Triangle;lighting=yes";

NSString* const kMaplyShaderDefaultModelTri = @"Default Triangle;model=yes;lighting=yes";

NSString* const kMaplyShaderDefaultTriNoLighting = @"Default Triangle;lighting=no";
NSString* const kMaplyNoLightTriangleShader = @"Default Triangle;lighting=no";

NSString* const kMaplyShaderDefaultTriScreenTex = @"Default Triangle;screentex=yes;lighting=yes";

NSString* const kMaplyShaderDefaultTriMultiTex = @"Default Triangle;multitex=yes;lighting=yes";
NSString* const kMaplyShaderDefaultTriMultiTexRamp = @"Default Triangle;multitex=yes;lighting=yes;ramp=yes";
NSString* const kMaplyShaderDefaultTriNightDay = @"Default Triangle;nightday=yes;multitex=yes;lighting=yes";

NSString* const kMaplyShaderDefaultLine = @"Default Line;backface=yes";
NSString* const kMaplyDefaultLineShader = @"Default Line;backface=yes";

NSString* const kMaplyShaderDefaultLineNoBackface = @"Default Line;backface=no";
NSString* const kMaplyNoBackfaceLineShader = @"Default Line;backface=no";

NSString* const kMaplyShaderBillboardGround = @"Default Billboard ground";
NSString* const kMaplyShaderBillboardEye = @"Default Billboard eye";

NSString* const kMaplyShaderParticleSystemPointDefault = @"Default Part Sys (Point)";
