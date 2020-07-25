/*
 *  MaplySharedAttributes.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
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

#import <Foundation/Foundation.h>

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
extern NSString* const kMaplyRenderHintZBuffer;
#define kWGRenderHintZBuffer kMaplyRenderHintZBuffer
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
extern NSString* const kMaplyRenderHintCulling;
#define kWGRenderHintCulling kMaplyRenderHintCulling
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
extern NSString* const kMaplyRendererLightingMode;
#define kWGRendererLightingMode kMaplyRendererLightingMode

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
extern NSString* const kMaplyDrawOffset;
#define kWGDrawOffset kMaplyDrawOffset
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
extern NSString* const kMaplyDrawPriority;
#define kWGDrawPriority kMaplyDrawPriority
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
extern NSString* const kMaplyMinVis;
#define kWGMinVis kMaplyMinVis
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
extern NSString* const kMaplyMaxVis;
#define kWGMaxVis kMaplyMaxVis
/// Minimum distance from the viewer at which to display geometry.
extern NSString* const kMaplyViewerMinDist;
/// Maximum distance from the viewer at which to display geometry.
extern NSString* const kMaplyViewerMaxDist;
/// Center to use when evaluating distance to viewable geometry (X)
extern NSString* const kMaplyViewableCenterX;
/// Center to use when evaluating distance to viewable geometry (Y)
extern NSString* const kMaplyViewableCenterY;
/// Center to use when evaluating distance to viewable geometry (Z)
extern NSString* const kMaplyViewableCenterZ;
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
extern NSString* const kMaplyFade;
#define kWGFade kMaplyFade
/// Fade the feature in over time.
extern NSString* const kMaplyFadeIn;
/// Fade the feature out over time
extern NSString* const kMaplyFadeOut;
/// When to start fading out
extern NSString* const kMaplyFadeOutTime;
/// Enable or disable an object.  This can be used to create an object disabled.
extern NSString* const kMaplyEnable;
/// If set, we'll enable the objects only between the start and end time
extern NSString* const kMaplyEnableStart;
/// If set, we'll enable the objects only between the start and end time
extern NSString* const kMaplyEnableEnd;
/// Request a given object take the z buffer into account
extern NSString* const kMaplyZBufferRead;
/// Have a given object write itself to the z buffer
extern NSString* const kMaplyZBufferWrite;
/// Set the render target if the given geometry type supports it
extern NSString* const kMaplyRenderTarget;

/// Assign a shader program to a particular feature.  Use the shader program's name
extern NSString* const kMaplyShader;
/// An NSDictionary containing uniforms to apply to a shader before drawing
extern NSString* const kMaplyShaderUniforms;

/// Keep drawing for this number of frames after we'd normally stop
extern NSString* const kMaplyExtraFrames;

/// Stars, moon, stars, atmosphere
extern const int kMaplyStarsDrawPriorityDefault;
extern const int kMaplySunDrawPriorityDefault;
extern const int kMaplyMoonDrawPriorityDefault;
extern const int kMaplyAtmosphereDrawPriorityDefault;
/// Where we start image layer draw priorities
extern const int kMaplyImageLayerDrawPriorityDefault;
/// We'll start filling in features right around here
extern const int kMaplyFeatureDrawPriorityBase;
extern const int kMaplyStickerDrawPriorityDefault;
extern const int kMaplyMarkerDrawPriorityDefault;
extern const int kMaplyVectorDrawPriorityDefault;
extern const int kMaplyParticleSystemDrawPriorityDefault ;
extern const int kMaplyLabelDrawPriorityDefault;
extern const int kMaplyLoftedPolysDrawPriorityDefault;
extern const int kMaplyShapeDrawPriorityDefault;
extern const int kMaplyBillboardDrawPriorityDefault;
extern const int kMaplyModelDrawPriorityDefault;
extern const int kMaplyMaxDrawPriorityDefault;

#define kWGMarkerDrawPriorityDefault kMaplyMarkerDrawPriorityDefault
#define kWGVectorDrawPriorityDefault kMaplyVectorDrawPriorityDefault
#define kWGStickerDrawPriorityDefault kMaplyStickerDrawPriorityDefault

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
extern NSString* const kMaplyTextColor;
#define kWGTextColor kMaplyTextColor
/// Background color for the text.  Takes a UIColor.
extern NSString* const kMaplyBackgroundColor;
#define kWGBackgroundColor kMaplyBackgroundColor
/// Font to use in rendering text.  Takes a UIFont.
extern NSString* const kMaplyFont;
#define kWGFont kMaplyFont
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
extern NSString* const kMaplyLabelHeight;
#define kWGLabelHeight kMaplyLabelHeight
/// Default width of the text.  See height for more info and, in general, use height instead.
extern NSString* const kMaplyLabelWidth;
#define kWGLabelWidth kMaplyLabelWidth
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
extern NSString* const kMaplyJustify;
#define kWGJustify kMaplyJustify
/// If set, we'll draw a shadow behind each label with this as the stroke size
extern NSString* const kMaplyShadowSize;
#define kWGShadowSize kMaplyShadowSize
/// If shadow size is being used, we can control the shadow color like so
extern NSString* const kMaplyShadowColor;
#define kWGShadowColor kMaplyShadowColor
/// If outline is being used, we can control the color
extern NSString* const kMaplyTextOutlineSize;
/// Vertical line spacing.  Defaults to the Font's line spacing
extern NSString* const kMaplyTextLineSpacing;
/// If outline is being used, we can control the stroke size
extern NSString* const kMaplyTextOutlineColor;
/// When creating textures, we may pass in the size
extern NSString* const kMaplyTexSizeX;
/// When creating textures, we may pass in the size
extern NSString* const kMaplyTexSizeY;

/// How to justify multi-line text
extern NSString* const kMaplyTextJustify;
/// Justify text to the right
extern NSString* const kMaplyTextJustifyRight;
/// Justify text to the left
extern NSString* const kMaplyTextJustifyLeft;
/// Justify text to the center
extern NSString* const kMaplyTextJustifyCenter;

/// These are used for screen and regular markers.
extern NSString* const kMaplyClusterGroup;

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
extern NSString* const kMaplyColor;
#define kWGColor kMaplyColor

/// Width is used by the vector layer for line widths
extern NSString* const kMaplyVecWidth;
#define kWGVecWidth kMaplyVecWidth

/// If filled is set, we draw the areals as filled polygons
extern NSString* const kMaplyFilled;
#define kWGFilled kMaplyFilled

/// If set, the texture to apply to the feature
extern NSString* const kMaplyVecTexture;
/// X scale for textures applied to vectors
extern NSString* const kMaplyVecTexScaleX;
/// Y scale for textures applied to vectors
extern NSString* const kMaplyVecTexScaleY;

/// The projection to use when generating texture coordinates
extern NSString* const kMaplyVecTextureProjection;
/// Tangent plane projection for texture coordinates
extern NSString* const kMaplyProjectionTangentPlane;
/// Screen space "projection" for texture coordinates
extern NSString* const kMaplyProjectionScreen;

/// If set to true we'll centered any drawables we create for features
/// This fixes the jittering problem when zoomed in close
extern NSString* const kMaplyVecCentered;

/// Center of the feature, to use for texture calculations
extern NSString* const kMaplyVecCenterX;
extern NSString* const kMaplyVecCenterY;

/// For wide vectors, we can widen them in screen space or display space
extern NSString* const kMaplyWideVecCoordType;

/// Widened vectors are widened in real space.  The width is in meters.
extern NSString* const kMaplyWideVecCoordTypeReal;
/// Widened vectors are widened in screen space.  The width is in pixels.
extern NSString* const kMaplyWideVecCoordTypeScreen;

/// For wide vectors we can control the line joins
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
extern NSString* const kMaplyWideVecJoinType;

/// Widened vectors are joined with miters
extern NSString* const kMaplyWideVecMiterJoin;
// Note: Not yet implemented
/// Widened vectors are joined with a curve
//extern NSString* const kMaplyWideVecRoundJoin @"round"
/// Widened vectors are joined with a bevel
extern NSString* const kMaplyWideVecBevelJoin;

/// Number of pixels to use in blending the edges of the wide vectors
extern NSString* const kMaplyWideVecEdgeFalloff;

/// For wide vectors we can control the ends
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
//extern NSString* const kMaplyWideVecLineCapType @"wideveclinecaptype"

// Note: These are not currently implemented

/// Widened vector ends are flush
//extern NSString* const kMaplyWideVecButtCap;
/// Widened vector ends are round (e.g. hot dog roads)
//extern NSString* const kMaplyWideVecRoundCap;
/// Widened vector ends are extended a bit and then flush
//extern NSString* const kMaplyWideVecSquareCap;

/// Miter joins will turn to bevel joins past this number of degrees
extern NSString* const kMaplyWideVecMiterLimit;

/// This is the length you'd like the texture to start repeating after.
/// It's real world coordinates for kMaplyWideVecCoordTypeReal and pixel size for kMaplyWideVecCoordTypeScreen
extern NSString* const kMaplyWideVecTexRepeatLen;

/// If set we'll break up a vector feature to the given epsilon on a globe surface
extern NSString* const kMaplySubdivEpsilon;
/// If subdiv epsilon is set we'll look for a subdivision type. Default is simple.
extern NSString* const kMaplySubdivType;
/// Subdivide the vector edges along a great circle
extern NSString* const kMaplySubdivGreatCircle;
/// Subdivide into a fixed number of segmenets
extern NSString* const kMaplySubdivStatic;
/// Subdivide the vectors edges along lat/lon
extern NSString* const kMaplySubdivSimple;
/// Clip features along a grid of the given size
extern NSString* const kMaplySubdivGrid;
/// Used to turn off selection in vectors
extern NSString* const kMaplySelectable;

/// These are used for stickers

/// Sampling size along one dimension
extern NSString* const kMaplySampleX;
#define kWGSampleX kMaplySampleX
/// Sampling size along one dimension
extern NSString* const kMaplySampleY;
#define kWGSampleY kMaplySampleY
/// Images to use when changing a sticker
extern NSString* const kMaplyStickerImages;
/// Image format to use for the new images
extern NSString* const kMaplyStickerImageFormat;

/// These are used for billboards

/// Billboard orientation
extern NSString* const kMaplyBillboardOrient;
/// Billboards are oriented toward the eye, but rotate on the ground
extern NSString* const kMaplyBillboardOrientGround;
/// Billboards are oriented only towards the eye
extern NSString* const kMaplyBillboardOrientEye;

/// These are used for lofted polygons

/// Height above the ground
extern NSString* const kMaplyLoftedPolyHeight;
/// Boolean that turns on/off top (on by default)
extern NSString* const kMaplyLoftedPolyTop;
/// Boolean that turns on/off sides (on by default)
extern NSString* const kMaplyLoftedPolySide;
/// If present, we'll start the lofted poly above 0 height
extern NSString* const kMaplyLoftedPolyBase;
/// Grid size we used to chop the lofted polygons up (10 degress by default)
extern NSString* const kMaplyLoftedPolyGridSize;
/// If set to @(YES) this will draw an outline around the top of the lofted poly in lines
extern NSString* const kMaplyLoftedPolyOutline;
/// If set to @(YES) this will draw an outline around the bottom of the lofted poly in lines
extern NSString* const kMaplyLoftedPolyOutlineBottom;
/// If the outline is one this is the outline's color
extern NSString* const kMaplyLoftedPolyOutlineColor;
/// This is the outline's width if it's turned on
extern NSString* const kMaplyLoftedPolyOutlineWidth;
/// Draw priority of the lines created for the lofted poly outline
extern NSString* const kMaplyLoftedPolyOutlineDrawPriority;
/// If set and we're drawing an outline, this will create lines up the sides
extern NSString* const kMaplyLoftedPolyOutlineSide;

/// These are used for shapes

/// Samples (x) to use when converting shape to polygons
extern NSString* const kMaplyShapeSampleX;
/// Samples (y) to use when converting shape to polygons
extern NSString* const kMaplyShapeSampleY;
/// If set to true, we'll tessellate a shape using the opposite vertex ordering
extern NSString* const kMaplyShapeInsideOut;
/// Center for the shape geometry
extern NSString* const kMaplyShapeCenterX;
extern NSString* const kMaplyShapeCenterY;
extern NSString* const kMaplyShapeCenterZ;

/// These are used by active vector objects
extern NSString* const kMaplyVecHeight;
extern NSString* const kMaplyVecMinSample;

/// These are used by the particle systems
extern NSString* const kMaplyPointSize;
extern const float kMaplyPointSizeDefault;

/// These are used by the texture
extern NSString* const kMaplyTexFormat;
extern NSString* const kMaplyTexMinFilter;
extern NSString* const kMaplyTexMagFilter;
extern NSString* const kMaplyMinFilterNearest;
extern NSString* const kMaplyMinFilterLinear;
extern NSString* const kMaplyTexAtlas;
extern NSString* const kMaplyTexWrapX;
extern NSString* const kMaplyTexWrapY;
extern NSString* const kMaplyTexMipmap;

/// These are the various shader programs we set up by default
extern NSString* const kMaplyShaderDefaultTri;
extern NSString* const kMaplyDefaultTriangleShader;

extern NSString* const kMaplyShaderDefaultModelTri;

extern NSString* const kMaplyShaderDefaultTriNoLighting;
extern NSString* const kMaplyNoLightTriangleShader;
extern NSString* const kMaplyShaderDefaultMarker;

extern NSString* const kMaplyShaderDefaultTriScreenTex;

extern NSString* const kMaplyShaderDefaultTriMultiTex;
extern NSString* const kMaplyShaderDefaultTriMultiTexRamp;
extern NSString* const kMaplyShaderDefaultTriNightDay;

extern NSString* const kMaplyShaderDefaultLine;
extern NSString* const kMaplyDefaultLineShader;

extern NSString* const kMaplyShaderDefaultLineNoBackface;
extern NSString* const kMaplyNoBackfaceLineShader;

extern NSString* const kMaplyShaderBillboardGround;
extern NSString* const kMaplyShaderBillboardEye;

extern NSString* const kMaplyShaderDefaultWideVector;

extern NSString* const kMaplyScreenSpaceDefaultMotionProgram;
extern NSString* const kMaplyScreenSpaceDefaultProgram;

extern NSString* const kMaplyShaderParticleSystemPointDefault;
