/*
 *  SharedAttributes
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2021 mousebird consulting
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

/// Wrapper for string (Objective-C vs. other)
#ifndef WKString
#define WKString(str) str
#endif

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
#define MaplyRenderHintZBuffer WKString("zbuffer")
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
#define MaplyRenderHintCulling WKString("culling")
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
#define MaplyRendererLightingMode WKString("rendererLightingMode")

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
#define MaplyDrawOffset WKString("drawOffset")
/// This helps decide what order things within the same `drawOrder` are drawn.
/// Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
#define MaplyDrawPriority WKString("drawPriority")
/// This helps decide what order things are drawn in.
/// Takes an NSNumber Int64.
#define MaplyDrawOrder WKString("drawOrder")
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define MaplyMinVis WKString("minVis")
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define MaplyMaxVis WKString("maxVis")
#define MaplyMinVisBand WKString("minVisBand")
#define MaplyMaxVisBand WKString("maxVisBand")
/// Some geometry supports level of detailing based on distance
#define MaplyMinViewerDist WKString("minviewerdist")
#define MaplyMaxViewerDist WKString("maxviewerdist")
/// Zoom related control
#define MaplyZoomSlot WKString("zoomslot")
#define MaplyMinZoomVis WKString("minzoomvis")
#define MaplyMaxZoomVis WKString("maxzoomvis")
/// Viewable center if using geometry offsets
#define MaplyViewableCenterX WKString("viewablecenterx")
#define MaplyViewableCenterY WKString("viewablecentery")
#define MaplyViewableCenterZ WKString("viewablecenterz")
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
#define MaplyFade WKString("fade")
/// How long to fade something in
#define MaplyFadeIn WKString("fadein")
/// How long to fade something back out
#define MaplyFadeOut WKString("fadeout")
/// If set, when an object will begin to fade out
#define MaplyFadeOutTime WKString("fadeouttime")
/// Enable or disable an object.  This can be used to create an object disabled.
#define MaplyEnable WKString("enable")
/// If set, a specific time to enable an object
#define MaplyEnableStart WKString("enablestart")
/// If set, a specific time to disable an object
#define MaplyEnableEnd WKString("enableend")
/// Request a given object take the z buffer into account
#define MaplyZBufferRead WKString("zbufferread")
/// Have a given object write itself to the z buffer
#define MaplyZBufferWrite WKString("zbufferwrite")

/// Assign a shader program to a particular feature.  Use the shader program's name
#define MaplyShaderString WKString("shader")
/// This is the shader we'll normally get by default on triangles
#define MaplyDefaultTriangleShader WKString("Default Triangle;lighting=yes")
#define MaplyTriangleExpShader WKString("Default Triangle;lighting=yes;exp=yes")
/// This shader turns off lighting explicitly (doesn't have the code for it)
#define MaplyNoLightTriangleShader WKString("Default Triangle;lighting=no")
#define MaplyNoLightTriangleExpShader WKString("Default Triangle;lighting=no;exp=yes")
/// This is the line/point shader we'll normaly get by default
#define MaplyDefaultLineShader WKString("Default Line;backface=yes")
/// This point/line shader turns off the backface logic for lines
#define MaplyNoBackfaceLineShader WKString("Default Line;backface=no")
/// Billboard orientation
#define MaplyBillboardOrient WKString("billboardorient")
/// Billboards are oriented toward the eye, but rotate on the ground
#define MaplyBillboardOrientGround WKString("billboardorientground")
/// Billboards are oriented only towards the eye
#define MaplyBillboardOrientEye WKString("billboardorienteye")

/// These are the various shader programs we set up by default
#define MaplyDefaultModelTriShader WKString("Default Triangle;model=yes;lighting=yes")
#define MaplyDefaultTriScreenTexShader WKString("Default Triangle;screentex=yes;lighting=yes")

#define MaplyDefaultTriMultiTexShader WKString("Default Triangle;multitex=yes;lighting=yes")
#define MaplyDefaultTriMultiTexRampShader WKString("Default Triangle;multitex=yes;lighting=yes;ramp=yes")
#define MaplyDefaultMarkerShader WKString("Default marker;multitex=yes;lighting=yes")

#define MaplyDefaultTriNightDayShader WKString("Default Triangle;nightday=yes;multitex=yes;lighting=yes")

#define MaplyBillboardGroundShader WKString("Default Billboard ground")
#define MaplyBillboardEyeShader WKString("Default Billboard eye")

#define MaplyDefaultWideVectorShader WKString("Default Wide Vector")
#define MaplyWideVectorExpShader WKString("Default Wide Vector with expressions")
#define MaplyWideVectorPerformanceShader WKString("Wide Vector performance")
#define MaplyDefaultWideVectorGlobeShader WKString("Default Wide Vector Globe")

#define MaplyScreenSpaceDefaultMotionShader WKString("Default Screenspace Motion")
#define MaplyScreenSpaceDefaultShader WKString("Default Screenspace")
#define MaplyScreenSpaceMaskShader WKString("Screenspace mask")
#define MaplyScreenSpaceExpShader WKString("Screenspace with expressions")

#define MaplyParticleSystemPointDefaultShader WKString("Default Part Sys (Point)")


/// Where we start image layer draw priorities
#define MaplyImageLayerDrawPriorityDefault 100
/// We'll start filling in features right around here
#define MaplyFeatureDrawPriorityBase 20000
#define MaplyStickerDrawPriorityDefault 30000
#define MaplyMarkerDrawPriorityDefault 40000
#define MaplyVectorDrawPriorityDefault 50000
#define MaplyLabelDrawPriorityDefault 60000
#define MaplyLoftedPolysDrawPriorityDefault 70000
#define MaplyShapeDrawPriorityDefault 80000
#define MaplyBillboardDrawPriorityDefault 90000

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
#define MaplyTextColor WKString("textColor")
/// Background color for the text.  Takes a UIColor.
#define MaplyBackgroundColor WKString("backgroundColor")
/// Font to use in rendering text.  Takes a UIFont.
#define MaplyFont WKString("font")
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
#define MaplyLabelHeight WKString("height")
/// Default width of the text.  See height for more info and, in general, use height instead.
#define MaplyLabelWidth WKString("width")
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
#define MaplyLabelJustifyName WKString("justify")
/// Justify label in the middle
#define MaplyLabelJustifyNameMiddle WKString("middle")
/// Justify label to the left
#define MaplyLabelJustifyNameLeft WKString("left")
/// Justify label to the right
#define MaplyLabelJustifyNameRight WKString("right")
/// Text justification within multi-line strings
#define MaplyTextJustify WKString("textjustify")
/// Justify text to the center
#define MaplyTextJustifyCenter WKString("center")
/// Justify text to the left
#define MaplyTextJustifyLeft WKString("left")
/// Justify text to the right
#define MaplyTextJustifyRight WKString("right")
/// Layout text to right/left inside/outside by given number of piels
#define MaplyTextLayoutOffset WKString("layoutoffset")
/// Space between instances of text along line/inside polygon if repeating
#define MaplyTextLayoutSpacing WKString("layoutspacing")
/// How many times to repeat laid out text (-1 infinite, 0 not, or a number)
#define MaplyTextLayoutRepeat WKString("layoutrepeat")
/// Turn on or off layout debugging lines
#define MaplyTextLayoutDebug WKString("layoutdebug")

/// If set, we'll draw a shadow behind each label with this as the stroke size
#define MaplyShadowSize WKString("shadowSize")
/// If shadow size is being used, we can control the shadow color like so
#define MaplyShadowColor WKString("shadowColor")
/// If outline is being used, we can control the color
#define MaplyTextOutlineSize WKString("outlineSize")
/// If outline is being used, we can control the stroke size
#define MaplyTextOutlineColor WKString("outlineColor")
/// If set, we'll use layout logic.  You can just set layoutImportance instead.
#define MaplyLayout WKString("layout")
/// If set, the importance passed to the layout engine
#define MaplyLayoutImportance WKString("layoutImportance")
/// If set, we'll pass a bit field on to the layout placement rules
#define MaplyLayoutPlacement WKString("layoutPlacement")
/// Custom line height for multi-line text
#define MaplyTextLineHeight WKString("lineHeight")

/// These are used for screen and regular markers.
#define MaplyClusterGroupID WKString("clusterGroup")

/// These are used for screen and regular markers.

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
#define MaplyColor WKString("color")

/// Width is used by the vector layer for line widths
#define MaplyVecWidth WKString("width")

/// If filled is set, we draw the areals as filled polygons
#define MaplyFilled WKString("filled")

/// If set, we'll centered each group of vectors and offset with a matrix
/// This fixes jitter.
#define MaplyVecCentered WKString("centered")

/// If set, the texture to apply to the feature
#define MaplyVecTexture WKString("texture")
#define MaplyVecTextureFormat WKString("textureFormat")
#define MaplyVecTexScaleX WKString("texscalex")
#define MaplyVecTexScaleY WKString("texscaley")

/// The projection to use when generating texture coordinates
#define MaplyVecTextureProjection WKString("texprojection")
/// Tangent plane projection for texture coordinates
#define MaplyVecProjectionTangentPlane WKString("texprojectiontanplane")
/// Screen projection for texture coordinates
#define MaplyVecProjectionScreen WKString("texprojectionscreen")
/// No projection for texture coordinates
#define MaplyVecProjectionNone WKString("texprojectionnone")

/// Center of the feature, to use for texture calculations
#define MaplyVecCenterX WKString("veccenterx")
#define MaplyVecCenterY WKString("veccentery")

/// For wide vectors, we can widen them in screen space or display space
#define MaplyWideVecCoordType WKString("wideveccoordtype")

/// Basic or performance mode for wide vectors
#define MaplyWideVecImpl WKString("widevecimplementation")

/// Performance mode for wide vectors
#define MaplyWideVecImplPerf WKString("widevecperformance")

/// Widened vectors are widened in real space.  The width is in meters.
#define MaplyWideVecCoordTypeReal WKString("real")
/// Widened vectors are widened in screen space.  The width is in pixels.
#define MaplyWideVecCoordTypeScreen WKString("screen")

/// For wide vectors we can control the line joins
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
#define MaplyWideVecJoinType WKString("wideveclinejointype")

/// Widened vectors are joined with miters
#define MaplyWideVecMiterJoin WKString("miter")
// Note: Not yet implemented
/// Widened vectors are joined with a curve
//#define kMaplyWideVecRoundJoin @"round"
/// Widened vectors are joined with a bevel
#define MaplyWideVecBevelJoin WKString("bevel")

/// Number of pixels to use in blending the edges of the wide vectors
#define MaplyWideVecEdgeFalloff WKString("edgefalloff")

/// For wide vectors we can control the ends
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
//#define kMaplyWideVecLineCapType @"wideveclinecaptype"

// Note: These are not currently implemented

/// Widened vector ends are flush
//#define kMaplyWideVecButtCap @"butt"
/// Widened vector ends are round (e.g. hot dog roads)
//#define kMaplyWideVecRoundCap @"round"
/// Widened vector ends are extended a bit and then flush
//#define kMaplyWideVecSquareCap @"square"

/// Miter joins will turn to bevel joins past this number of degrees
#define MaplyWideVecMiterLimit WKString("miterLimit")

/// This is the length you'd like the texture to start repeating after.
/// It's real world coordinates for kMaplyWideVecCoordTypeReal and pixel size for kMaplyWideVecCoordTypeScreen
#define MaplyWideVecTexRepeatLen WKString("repeatSize")

/// Offset to left (negative) or right (positive) of the centerline
#define MaplyWideVecOffset WKString("vecOffset")

/// If set we'll break up a vector feature to the given epsilon on a globe surface
#define MaplySubdivEpsilon WKString("subdivisionepsilon")
/// If subdiv epsilon is set we'll look for a subdivision type. Default is simple.
#define MaplySubdivType WKString("subdivisiontype")
/// Subdivide the vector edges along a great circle
#define MaplySubdivGreatCircle WKString("greatcircle")
/// Subdivide the vector edges along a great circle with ellipsoidal math
#define MaplySubdivGreatCirclePrecise WKString("greatcircleprecise")
/// Subdivide the vectors edges along lat/lon
#define MaplySubdivSimple WKString("simple")
/// Clip features along a grid of the given size
#define MaplySubdivGrid WKString("grid")

/// These are used for stickers

/// Sampling size along one dimension
#define MaplySampleX WKString("sampleX")
/// Sampling size along one dimension
#define MaplySampleY WKString("sampleY")
/// Images to use when changing a sticker
#define MaplyStickerImages WKString("images")
/// Image format to use for the new images
#define MaplyStickerImageFormat WKString("imageformat")

/// These are used for lofted polygons

/// Height above the ground
#define MaplyLoftedPolyHeight WKString("height")
/// Height of the base (usually 0)
#define MaplyLoftedPolyBase WKString("base")
/// Boolean that turns on/off top (on by default)
#define MaplyLoftedPolyTop WKString("top")
/// Boolean that turns on/off sides (on by default)
#define MaplyLoftedPolySide WKString("side")
/// Grid size we used to chop the lofted polygons up (10 degress by default)
#define MaplyLoftedPolyGridSize WKString("gridsize")
/// If set, we'll do the outline in lines as well
#define MaplyLoftedPolyOutline WKString("outline")
/// Color of the outline lines
#define MaplyLoftedPolyOutlineColor WKString("outlineColor")
/// Width of the outline lines
#define MaplyLoftedPolyOutlineWidth WKString("outlineWidth")
/// Draw Priority for the outline lines
#define MaplyLoftedPolyOutlineDrawPriority WKString("outlineDrawPriority")
/// Whether to include sides in the outline lines
#define MaplyLoftedPolyOutlineSide WKString("outlineSide")
/// Whether to include the bottom in the outline lines
#define MaplyLoftedPolyOutlineBottom WKString("outlineBottom")

/// These are used by active vector objects
#define MaplyVecHeight WKString("height")
#define MaplyVecMinSample WKString("minSample")

/// These are used by the geometry objects
#define MaplyGeomBoundingBox WKString("boundingbox")
#define MaplyGeomPointSize WKString("pointSize")

/// Single bounding box for the whole model
#define MaplyGeomBoundingBoxSingle WKString("single")
/// Interesection testing at triangle level
#define MaplyGeomBoundingBoxTriangle WKString("triangle")
/// No bounding box for geometry
#define MaplyGeomBoundingBoxNone WKString("none")

/// These are used by Shapes

/// Turn the shape inside out (used for spheres and atmosphere)
#define MaplyShapeInsideOut WKString("shapeinsideout")
#define MaplyShapeCenterX WKString("shapecenterx")
#define MaplyShapeCenterY WKString("shapecentery")
#define MaplyShapeCenterZ WKString("shapecenterz")

/// Used to designate a non-default render target by ID
#define MaplyRenderTargetDesc WKString("rendertarget")

/// Used to refer to component objects by unique ID for, e.g., representation selection
#define MaplyUUIDDesc WKString("uuid")
/// Used to distinguish the particular representation of a unique ID
#define MaplyRepresentationDesc WKString("representation")
