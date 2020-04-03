/*
 *  MaplySharedAttributes.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 2/24/16.
 *  Copyright 2011-2019 mousebird consulting
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

// We pull in as many of the shared attributes as possible
#define WKString(str) @str
#import "SharedAttributes.h"
#import "MaplySharedAttributes.h"

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
NSString* const kMaplyRenderHintZBuffer = MaplyRenderHintZBuffer;
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
NSString* const kMaplyRenderHintCulling = MaplyRenderHintCulling;
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
NSString* const kMaplyRendererLightingMode = MaplyRendererLightingMode;

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
NSString* const kMaplyDrawOffset = MaplyDrawOffset;
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
NSString* const kMaplyDrawPriority = MaplyDrawPriority;
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
NSString* const kMaplyMinVis = MaplyMinVis;
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
NSString* const kMaplyMaxVis = MaplyMaxVis;
/// Minimum distance from the viewer at which to display geometry.
NSString* const kMaplyViewerMinDist = MaplyMinVisBand;
/// Maximum distance from the viewer at which to display geometry.
NSString* const kMaplyViewerMaxDist = MaplyMaxVisBand;
/// Center to use when evaluating distance to viewable geometry (X)
NSString* const kMaplyViewableCenterX = MaplyViewableCenterX;
/// Center to use when evaluating distance to viewable geometry (Y)
NSString* const kMaplyViewableCenterY = MaplyViewableCenterY;
/// Center to use when evaluating distance to viewable geometry (Z)
NSString* const kMaplyViewableCenterZ = MaplyViewableCenterZ;
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
NSString* const kMaplyFade = MaplyFade;
/// Fade the feature in over time.
NSString* const kMaplyFadeIn = MaplyFadeIn;
/// Fade the feature out over time
NSString* const kMaplyFadeOut = MaplyFadeOut;
/// When to start fading out
NSString* const kMaplyFadeOutTime = MaplyFadeOutTime;
/// Enable or disable an object.  This can be used to create an object disabled.
NSString* const kMaplyEnable = MaplyEnable;
/// If set, we'll enable the objects only between the start and end time
NSString* const kMaplyEnableStart = MaplyEnableStart;
/// If set, we'll enable the objects only between the start and end time
NSString* const kMaplyEnableEnd = MaplyEnableEnd;
/// Request a given object take the z buffer into account
NSString* const kMaplyZBufferRead = MaplyZBufferRead;
/// Have a given object write itself to the z buffer
NSString* const kMaplyZBufferWrite = MaplyZBufferWrite;
/// Set the render target if the given geometry type supports it
NSString* const kMaplyRenderTarget = MaplyRenderTargetDesc;

/// Assign a shader program to a particular feature.  Use the shader program's name
NSString* const kMaplyShader = MaplyShaderString;
/// An NSDictionary containing uniforms to apply to a shader before drawing
NSString* const kMaplyShaderUniforms = @"shaderuniforms";

NSString* const kMaplyExtraFrames = @"extraFrames";


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
// Unlikely to have any draw priorities here or beyond.
const int kMaplyMaxDrawPriorityDefault = 100100;

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
NSString* const kMaplyTextColor = MaplyTextColor;
/// Background color for the text.  Takes a UIColor.
NSString* const kMaplyBackgroundColor = MaplyBackgroundColor;
/// Font to use in rendering text.  Takes a UIFont.
NSString* const kMaplyFont = MaplyFont;
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
NSString* const kMaplyLabelHeight = MaplyLabelHeight;
/// Default width of the text.  See height for more info and, in general, use height instead.
NSString* const kMaplyLabelWidth = MaplyLabelWidth;
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
NSString* const kMaplyJustify = MaplyLabelJustifyName;
/// If set, we'll draw a shadow behind each label with this as the stroke size
NSString* const kMaplyShadowSize = MaplyShadowSize;
/// If shadow size is being used, we can control the shadow color like so
NSString* const kMaplyShadowColor = MaplyShadowColor;
/// If outline is being used, we can control the color
NSString* const kMaplyTextOutlineSize = MaplyTextOutlineSize;
/// Vertical line spacing.  Defaults to the Font's line spacing
NSString* const kMaplyTextLineSpacing = MaplyTextLineHeight;
/// If outline is being used, we can control the stroke size
NSString* const kMaplyTextOutlineColor = MaplyTextOutlineColor;
NSString* const kMaplyTexSizeX = @"texsizex";
NSString* const kMaplyTexSizeY = @"texsizey";
NSString* const kMaplyTextJustify = MaplyTextJustify;
NSString* const kMaplyTextJustifyRight = MaplyTextJustifyRight;
NSString* const kMaplyTextJustifyLeft = MaplyTextJustifyLeft;
NSString* const kMaplyTextJustifyCenter = MaplyTextJustifyCenter;

/// These are used for screen and regular markers.
NSString* const kMaplyClusterGroup = MaplyClusterGroupID;

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
NSString* const kMaplyColor = MaplyColor;

/// Width is used by the vector layer for line widths
NSString* const kMaplyVecWidth = MaplyVecWidth;

/// If filled is set, we draw the areals as filled polygons
NSString* const kMaplyFilled = MaplyFilled;

/// If set, the texture to apply to the feature
NSString* const kMaplyVecTexture = MaplyVecTexture;
/// X scale for textures applied to vectors
NSString* const kMaplyVecTexScaleX = MaplyVecTexScaleX;
/// Y scale for textures applied to vectors
NSString* const kMaplyVecTexScaleY = MaplyVecTexScaleY;

/// The projection to use when generating texture coordinates
NSString* const kMaplyVecTextureProjection = MaplyVecTextureProjection;
/// Tangent plane projection for texture coordinates
NSString* const kMaplyProjectionTangentPlane = MaplyVecProjectionTangentPlane;
/// Screen space "projection" for texture coordinates
NSString* const kMaplyProjectionScreen = MaplyVecProjectionScreen;

/// If set to true we'll centered any drawables we create for features
/// This fixes the jittering problem when zoomed in close
NSString* const kMaplyVecCentered = MaplyVecCentered;

/// Center of the feature, to use for texture calculations
NSString* const kMaplyVecCenterX = MaplyVecCenterX;
NSString* const kMaplyVecCenterY = MaplyVecCenterY;

/// For wide vectors, we can widen them in screen space or display space
NSString* const kMaplyWideVecCoordType = MaplyWideVecCoordType;

/// Widened vectors are widened in real space.  The width is in meters.
NSString* const kMaplyWideVecCoordTypeReal = MaplyWideVecCoordTypeReal;
/// Widened vectors are widened in screen space.  The width is in pixels.
NSString* const kMaplyWideVecCoordTypeScreen = MaplyWideVecCoordTypeScreen;

/// For wide vectors we can control the line joins
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
NSString* const kMaplyWideVecJoinType = MaplyWideVecJoinType;

/// Widened vectors are joined with miters
NSString* const kMaplyWideVecMiterJoin = MaplyWideVecMiterJoin;
// Note: Not yet implemented
/// Widened vectors are joined with a curve
//NSString* const kMaplyWideVecRoundJoin @"round"
/// Widened vectors are joined with a bevel
NSString* const kMaplyWideVecBevelJoin = MaplyWideVecBevelJoin;

/// Number of pixels to use in blending the edges of the wide vectors
NSString* const kMaplyWideVecEdgeFalloff = MaplyWideVecEdgeFalloff;

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
NSString* const kMaplyWideVecMiterLimit = MaplyWideVecMiterLimit;

/// This is the length you'd like the texture to start repeating after.
/// It's real world coordinates for kMaplyWideVecCoordTypeReal and pixel size for kMaplyWideVecCoordTypeScreen
NSString* const kMaplyWideVecTexRepeatLen = MaplyWideVecTexRepeatLen;

/// If set we'll break up a vector feature to the given epsilon on a globe surface
NSString* const kMaplySubdivEpsilon = MaplySubdivEpsilon;
/// If subdiv epsilon is set we'll look for a subdivision type. Default is simple.
NSString* const kMaplySubdivType = MaplySubdivType;
/// Subdivide the vector edges along a great circle
NSString* const kMaplySubdivGreatCircle = MaplySubdivGreatCircle;
/// Subdivide into a fixed number of segmenets
NSString* const kMaplySubdivStatic = @"static";
/// Subdivide the vectors edges along lat/lon
NSString* const kMaplySubdivSimple = MaplySubdivSimple;
/// Clip features along a grid of the given size
NSString* const kMaplySubdivGrid = MaplySubdivGrid;
/// Used to turn off selection in vectors
NSString* const kMaplySelectable = @"selectable";

/// These are used for stickers

/// Sampling size along one dimension
NSString* const kMaplySampleX = MaplySampleX;
/// Sampling size along one dimension
NSString* const kMaplySampleY = MaplySampleY;
/// Images to use when changing a sticker
NSString* const kMaplyStickerImages = MaplyStickerImages;
/// Image format to use for the new images
NSString* const kMaplyStickerImageFormat = MaplyStickerImageFormat;

/// These are used for billboards

/// Billboard orientation
NSString* const kMaplyBillboardOrient = MaplyBillboardOrient;
/// Billboards are oriented toward the eye, but rotate on the ground
NSString* const kMaplyBillboardOrientGround = MaplyBillboardOrientGround;
/// Billboards are oriented only towards the eye
NSString* const kMaplyBillboardOrientEye = MaplyBillboardOrientEye;

/// These are used for lofted polygons

/// Height above the ground
NSString* const kMaplyLoftedPolyHeight = MaplyLoftedPolyHeight;
/// Boolean that turns on/off top (on by default)
NSString* const kMaplyLoftedPolyTop = MaplyLoftedPolyTop;
/// Boolean that turns on/off sides (on by default)
NSString* const kMaplyLoftedPolySide = MaplyLoftedPolySide;
/// If present, we'll start the lofted poly above 0 height
NSString* const kMaplyLoftedPolyBase = MaplyLoftedPolyBase;
/// Grid size we used to chop the lofted polygons up (10 degress by default)
NSString* const kMaplyLoftedPolyGridSize = MaplyLoftedPolyGridSize;
/// If set to @(YES) this will draw an outline around the top of the lofted poly in lines
NSString* const kMaplyLoftedPolyOutline = MaplyLoftedPolyOutline;
/// If set to @(YES) this will draw an outline around the bottom of the lofted poly in lines
NSString* const kMaplyLoftedPolyOutlineBottom = MaplyLoftedPolyOutlineBottom;
/// If the outline is one this is the outline's color
NSString* const kMaplyLoftedPolyOutlineColor = MaplyLoftedPolyOutlineColor;
/// This is the outline's width if it's turned on
NSString* const kMaplyLoftedPolyOutlineWidth = MaplyLoftedPolyOutlineWidth;
/// Draw priority of the lines created for the lofted poly outline
NSString* const kMaplyLoftedPolyOutlineDrawPriority = MaplyLoftedPolyOutlineDrawPriority;
/// If set and we're drawing an outline, this will create lines up the sides
NSString* const kMaplyLoftedPolyOutlineSide = MaplyLoftedPolyOutlineSide;

/// These are used for shapes

/// Samples (x) to use when converting shape to polygons
NSString* const kMaplyShapeSampleX = @"shapesamplex";
/// Samples (y) to use when converting shape to polygons
NSString* const kMaplyShapeSampleY = @"shapesampley";
/// If set to true, we'll tessellate a shape using the opposite vertex ordering
NSString* const kMaplyShapeInsideOut = MaplyShapeInsideOut;
/// Center for the shape geometry
NSString* const kMaplyShapeCenterX = MaplyShapeCenterX;
NSString* const kMaplyShapeCenterY = MaplyShapeCenterY;
NSString* const kMaplyShapeCenterZ = MaplyShapeCenterZ;

/// These are used by active vector objects
NSString* const kMaplyVecHeight = MaplyVecHeight;
NSString* const kMaplyVecMinSample = MaplyVecMinSample;

/// These are used by the particle systems
NSString* const kMaplyPointSize = MaplyGeomPointSize;
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
NSString* const kMaplyTexMipmap = @"texmipmap";

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

NSString* const kMaplyShaderDefaultMarker = @"Default marker;multitex=yes;lighting=yes";

NSString* const kMaplyShaderDefaultLine = @"Default Line;backface=yes";
NSString* const kMaplyDefaultLineShader = @"Default Line;backface=yes";

NSString* const kMaplyShaderDefaultLineNoBackface = @"Default Line;backface=no";
NSString* const kMaplyNoBackfaceLineShader = @"Default Line;backface=no";

NSString* const kMaplyShaderBillboardGround = @"Default Billboard ground";
NSString* const kMaplyShaderBillboardEye = @"Default Billboard eye";

NSString* const kMaplyShaderDefaultWideVector = @"Default Wide Vector";

NSString* const kMaplyScreenSpaceDefaultMotionProgram = @"Default Screenspace Motion";
NSString* const kMaplyScreenSpaceDefaultProgram = @"Default Screenspace";

NSString* const kMaplyShaderParticleSystemPointDefault = @"Default Part Sys (Point)";
