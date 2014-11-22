/*
 *  MaplySharedAttributes.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2013 mousebird consulting
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

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
#define kMaplyRenderHintZBuffer @"zbuffer"
#define kWGRenderHintZBuffer kMaplyRenderHintZBuffer
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
#define kMaplyRenderHintCulling @"culling"
#define kWGRenderHintCulling kMaplyRenderHintCulling
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
#define kMaplyRendererLightingMode @"rendererLightingMode"
#define kWGRendererLightingMode kMaplyRendererLightingMode

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
#define kMaplyDrawOffset @"drawOffset"
#define kWGDrawOffset kMaplyDrawOffset
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
#define kMaplyDrawPriority @"drawPriority"
#define kWGDrawPriority kMaplyDrawPriority
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define kMaplyMinVis @"minVis"
#define kWGMinVis kMaplyMinVis
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define kMaplyMaxVis @"maxVis"
#define kWGMaxVis kMaplyMaxVis
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
#define kMaplyFade @"fade"
#define kWGFade kMaplyFade
/// Enable or disable an object.  This can be used to create an object disabled.
#define kMaplyEnable @"enable"
/// Request a given object take the z buffer into account
#define kMaplyZBufferRead @"zbufferread"
/// Have a given object write itself to the z buffer
#define kMaplyZBufferWrite @"zbufferwrite"

/// Assign a shader program to a particular feature.  Use the shader program's name
#define kMaplyShader @"shader"
/// This is the shader we'll normally get by default on triangles
#define kMaplyDefaultTriangleShader @"Default Triangle;lighting=yes"
/// This shader turns off lighting explicitly (doesn't have the code for it)
#define kMaplyNoLightTriangleShader @"Default Triangle;lighting=no"
/// This is the line/point shader we'll normaly get by default
#define kMaplyDefaultLineShader @"Default Line;backface=yes"
/// This point/line shader turns off the backface logic for lines
#define kMaplyNoBackfaceLineShader @"Default Line;backface=no"
/// The default billboard shader
#define kMaplyBillboardShader @"Default Billboard"

/// Where we start image layer draw priorities
#define kMaplyImageLayerDrawPriorityDefault 100
/// We'll start filling in features right around here
#define kMaplyFeatureDrawPriorityBase 20000
#define kMaplyStickerDrawPriorityDefault 30000
#define kMaplyMarkerDrawPriorityDefault 40000
#define kMaplyVectorDrawPriorityDefault 50000
#define kMaplyLabelDrawPriorityDefault 60000
#define kMaplyLoftedPolysDrawPriorityDefault 70000
#define kMaplyShapeDrawPriorityDefault 80000
#define kMaplyBillboardDrawPriorityDefault 90000

#define kWGMarkerDrawPriorityDefault kMaplyMarkerDrawPriorityDefault
#define kWGVectorDrawPriorityDefault kMaplyVectorDrawPriorityDefault
#define kWGStickerDrawPriorityDefault kMaplyStickerDrawPriorityDefault

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
#define kMaplyTextColor @"textColor"
#define kWGTextColor kMaplyTextColor
/// Background color for the text.  Takes a UIColor.
#define kMaplyBackgroundColor @"backgroundColor"
#define kWGBackgroundColor kMaplyBackgroundColor
/// Font to use in rendering text.  Takes a UIFont.
#define kMaplyFont @"font"
#define kWGFont kMaplyFont
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
#define kMaplyLabelHeight @"height"
#define kWGLabelHeight kMaplyLabelHeight
/// Default width of the text.  See height for more info and, in general, use height instead.
#define kMaplyLabelWidth @"width"
#define kWGLabelWidth kMaplyLabelWidth
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
#define kMaplyJustify @"justify"
#define kWGJustify kMaplyJustify
/// If set, we'll draw a shadow behind each label with this as the stroke size
#define kMaplyShadowSize @"shadowSize"
#define kWGShadowSize kMaplyShadowSize
/// If shadow size is being used, we can control the shadow color like so
#define kMaplyShadowColor @"shadowColor"
#define kWGShadowColor kMaplyShadowColor
/// If outline is being used, we can control the color
#define kMaplyTextOutlineSize @"outlineSize"
/// If outline is being used, we can control the stroke size
#define kMaplyTextOutlineColor @"outlineColor"

/// These are used for screen and regular markers.

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
#define kMaplyColor @"color"
#define kWGColor kMaplyColor

/// Width is used by the vector layer for line widths
#define kMaplyVecWidth @"width"
#define kWGVecWidth kMaplyVecWidth

/// If filled is set, we draw the areals as filled polygons
#define kMaplyFilled @"filled"
#define kWGFilled kMaplyFilled

/// If set, the texture to apply to the feature
#define kMaplyVecTexture @"texture"
#define kMaplyVecTexScaleX @"texscalex"
#define kMaplyVecTexScaleY @"texscaley"

/// The projection to use when generating texture coordinates
#define kMaplyVecTextureProjection @"texprojection"
/// Tangent plane projection for texture coordinates
#define kMaplyProjectionTangentPlane @"texprojectiontanplane"

/// If set to true we'll centered any drawables we create for features
/// This fixes the jittering problem when zoomed in close
#define kMaplyVecCentered @"centered"

/// Center of the feature, to use for texture calculations
#define kMaplyVecCenterX @"veccenterx"
#define kMaplyVecCenterY @"veccentery"

/// For wide vectors, we can widen them in screen space or display space
#define kMaplyWideVecCoordType @"wideveccoordtype"

/// Widened vectors are widened in real space.  The width is in meters.
#define kMaplyWideVecCoordTypeReal @"real"
/// Widened vectors are widened in screen space.  The width is in pixels.
#define kMaplyWideVecCoordTypeScreen @"screen"

/// For wide vectors we can control the line joins
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
#define kMaplyWideVecJoinType @"wideveclinejointype"

/// Widened vectors are joined with miters
#define kMaplyWideVecMiterJoin @"miter"
/// Widened vectors are joined with a curve
#define kMaplyWideVecRoundJoin @"round"
/// Widened vectors are joined with a bevel
#define kMaplyWideVecBevelJoin @"bevel"

/// For wide vectors we can control the ends
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
#define kMaplyWideVecLineCapType @"wideveclinecaptype"

/// Widened vector ends are flush
#define kMaplyWideVecButtCap @"butt"
/// Widened vector ends are round (e.g. hot dog roads)
#define kMaplyWideVecRoundCap @"round"
/// Widened vector ends are extended a bit and then flush
#define kMaplyWideVecSquareCap @"square"

/// This number defines a limit past which the wide vector will switch from miters to bevels
#define kMaplyWideVecMiterLimit @"miterLimit"

/// This is the length you'd like the texture to start repeating after.
/// It's real world coordinates for kMaplyWideVecCoordTypeReal and pixel size for kMaplyWideVecCoordTypeScreen
#define kMaplyWideVecTexRepeatLen @"repeatSize"

/// If set we'll break up a vector feature to the given epsilon on a globe surface
#define kMaplySubdivEpsilon @"subdivisionepsilon"
/// If subdiv epsilon is set we'll look for a subdivision type. Default is simple.
#define kMaplySubdivType @"subdivisiontype"
/// Subdivide the vector edges along a great circle
#define kMaplySubdivGreatCircle @"greatcircle"
/// Subdivide into a fixed number of segmenets
#define kMaplySubdivStatic @"static"
/// Subdivide the vectors edges along lat/lon
#define kMaplySubdivSimple @"simple"
/// Clip features along a grid of the given size
#define kMaplySubdivGrid @"grid"
/// Used to turn off selection in vectors
#define kMaplySelectable @"selectable"

/// These are used for stickers

/// Sampling size along one dimension
#define kMaplySampleX @"sampleX"
#define kWGSampleX kMaplySampleX
/// Sampling size along one dimension
#define kMaplySampleY @"sampleY"
#define kWGSampleY kMaplySampleY
/// Images to use when changing a sticker
#define kMaplyStickerImages @"images"
/// Image format to use for the new images
#define kMaplyStickerImageFormat @"imageformat"

/// These are used for lofted polygons

/// Height above the ground
#define kMaplyLoftedPolyHeight @"height"
/// Boolean that turns on/off top (on by default)
#define kMaplyLoftedPolyTop @"top"
/// Boolean that turns on/off sides (on by default)
#define kMaplyLoftedPolySide @"side"
/// If present, we'll start the lofted poly above 0 height
#define kMaplyLoftedPolyBase @"base"
/// Grid size we used to chop the lofted polygons up (10 degress by default)
#define kMaplyLoftedPolyGridSize @"gridsize"
/// If set to @(YES) this will draw an outline around the top of the lofted poly in lines
#define kMaplyLoftedPolyOutline @"outline"
/// If set to @(YES) this will draw an outline around the bottom of the lofted poly in lines
#define kMaplyLoftedPolyOutlineBottom @"outlineBottom"
/// If the outline is one this is the outline's color
#define kMaplyLoftedPolyOutlineColor @"outlineColor"
/// This is the outline's width if it's turned on
#define kMaplyLoftedPolyOutlineWidth @"outlineWidth"
/// Draw priority of the lines created for the lofted poly outline
#define kMaplyLoftedPolyOutlineDrawPriority @"outlineDrawPriority"
/// If set and we're drawing an outline, this will create lines up the sides
#define kMaplyLoftedPolyOutlineSide @"outlineSide"

/// These are used by active vector objects
#define kMaplyVecHeight @"height"
#define kMaplyVecMinSample @"minSample"

/// These are the various shader programs we set up by default
#define kMaplyShaderDefaultTri @"Default Triangle;lighting=yes"
#define kMaplyShaderDefaultTriNoLighting @"Default Triangle;lighting=no"
#define kMaplyShaderDefaultTriMultiTex @"Default Triangle;multitex=yes;lighting=yes"
#define kMaplyShaderDefaultLine @"Default Line;backface=yes"
#define kMaplyShaderDefaultLineNoBackface @"Default Line;backface=no"

