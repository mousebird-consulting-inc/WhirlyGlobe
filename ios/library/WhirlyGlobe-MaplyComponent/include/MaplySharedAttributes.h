/*  MaplySharedAttributes.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <Foundation/Foundation.h>

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
extern NSString * const _Nonnull kMaplyRenderHintZBuffer;
#define kWGRenderHintZBuffer kMaplyRenderHintZBuffer
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
extern NSString * const _Nonnull kMaplyRenderHintCulling;
#define kWGRenderHintCulling kMaplyRenderHintCulling
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
extern NSString * const _Nonnull kMaplyRendererLightingMode;
#define kWGRendererLightingMode kMaplyRendererLightingMode

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
extern NSString * const _Nonnull kMaplyDrawOffset;
#define kWGDrawOffset kMaplyDrawOffset
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
extern NSString * const _Nonnull kMaplyDrawPriority;
#define kWGDrawPriority kMaplyDrawPriority
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
extern NSString * const _Nonnull kMaplyMinVis;
#define kWGMinVis kMaplyMinVis
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
extern NSString * const _Nonnull kMaplyMaxVis;
#define kWGMaxVis kMaplyMaxVis
/// Zoom related control
extern NSString * const _Nonnull kMaplyZoomSlot;
extern NSString * const _Nonnull kMaplyMinZoomVis;
extern NSString * const _Nonnull kMaplyMaxZoomVis;
/// Minimum distance from the viewer at which to display geometry.
extern NSString * const _Nonnull kMaplyViewerMinDist;
/// Maximum distance from the viewer at which to display geometry.
extern NSString * const _Nonnull kMaplyViewerMaxDist;
/// Center to use when evaluating distance to viewable geometry (X)
extern NSString * const _Nonnull kMaplyViewableCenterX;
/// Center to use when evaluating distance to viewable geometry (Y)
extern NSString * const _Nonnull kMaplyViewableCenterY;
/// Center to use when evaluating distance to viewable geometry (Z)
extern NSString * const _Nonnull kMaplyViewableCenterZ;
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
extern NSString * const _Nonnull kMaplyFade;
#define kWGFade kMaplyFade
/// Fade the feature in over time.
extern NSString * const _Nonnull kMaplyFadeIn;
/// Fade the feature out over time
extern NSString * const _Nonnull kMaplyFadeOut;
/// When to start fading out
extern NSString * const _Nonnull kMaplyFadeOutTime;
/// Enable or disable an object.  This can be used to create an object disabled.
extern NSString * const _Nonnull kMaplyEnable;
/// If set, we'll enable the objects only between the start and end time
extern NSString * const _Nonnull kMaplyEnableStart;
/// If set, we'll enable the objects only between the start and end time
extern NSString * const _Nonnull kMaplyEnableEnd;
/// Request a given object take the z buffer into account
extern NSString * const _Nonnull kMaplyZBufferRead;
/// Have a given object write itself to the z buffer
extern NSString * const _Nonnull kMaplyZBufferWrite;
/// Set the render target if the given geometry type supports it
extern NSString * const _Nonnull kMaplyRenderTarget;
/// The the UUID of the object
extern NSString * const _Nonnull kMaplyUUID;
/// The representation of the UUID this object embodies
extern NSString * const _Nonnull kMaplyRepresentation;

/// Assign a shader program to a particular feature.  Use the shader program's name
extern NSString * const _Nonnull kMaplyShader;
/// An NSDictionary containing uniforms to apply to a shader before drawing
extern NSString * const _Nonnull kMaplyShaderUniforms;

/// Keep drawing for this number of frames after we'd normally stop
extern NSString * const _Nonnull kMaplyExtraFrames;

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
extern NSString * const _Nonnull kMaplyTextColor;
#define kWGTextColor kMaplyTextColor
/// Background color for the text.  Takes a UIColor.
extern NSString * const _Nonnull kMaplyBackgroundColor;
#define kWGBackgroundColor kMaplyBackgroundColor
/// Font to use in rendering text.  Takes a UIFont.
extern NSString * const _Nonnull kMaplyFont;
#define kWGFont kMaplyFont
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
extern NSString * const _Nonnull kMaplyLabelHeight;
#define kWGLabelHeight kMaplyLabelHeight
/// Default width of the text.  See height for more info and, in general, use height instead.
extern NSString * const _Nonnull kMaplyLabelWidth;
#define kWGLabelWidth kMaplyLabelWidth
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
extern NSString * const _Nonnull kMaplyJustify;
#define kWGJustify kMaplyJustify
/// If set, we'll draw a shadow behind each label with this as the stroke size
extern NSString * const _Nonnull kMaplyShadowSize;
#define kWGShadowSize kMaplyShadowSize
/// If shadow size is being used, we can control the shadow color like so
extern NSString * const _Nonnull kMaplyShadowColor;
#define kWGShadowColor kMaplyShadowColor
/// If outline is being used, we can control the color
extern NSString * const _Nonnull kMaplyTextOutlineSize;
/// Vertical line spacing.  Defaults to the Font's line spacing
extern NSString * const _Nonnull kMaplyTextLineSpacing;
/// If outline is being used, we can control the stroke size
extern NSString * const _Nonnull kMaplyTextOutlineColor;
/// When creating textures, we may pass in the size
extern NSString * const _Nonnull kMaplyTexSizeX;
/// When creating textures, we may pass in the size
extern NSString * const _Nonnull kMaplyTexSizeY;

/// How to justify multi-line text
extern NSString * const _Nonnull kMaplyTextJustify;
/// Justify text to the right
extern NSString * const _Nonnull kMaplyTextJustifyRight;
/// Justify text to the left
extern NSString * const _Nonnull kMaplyTextJustifyLeft;
/// Justify text to the center
extern NSString * const _Nonnull kMaplyTextJustifyCenter;

/// Controls how text is laid out along a line or polygon.  Set a number (- for left or inside, + for right or outside)
extern NSString * const _Nonnull kMaplyTextLayoutOffset;
/// If laying out along a line (or polygon), the amount of screen space to leave between labels
extern NSString * const _Nonnull kMaplyTextLayoutSpacing;
/// Layout as many labels as possible along a line (or polygon).  Set a number (0 for no repeat, -1 for as many as possible, or a number of instances)
extern NSString * const _Nonnull kMaplyTextLayoutRepeat;
/// Turn on debugging lines for the layout engine
extern NSString * const _Nonnull kMaplyTextLayoutDebug;

/// These are used for screen and regular markers.
extern NSString * const _Nonnull kMaplyClusterGroup;

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
extern NSString * const _Nonnull kMaplyColor;
#define kWGColor kMaplyColor

/// Specify the opacity separately from the alpha channel of "color"
/// Not widely supported
extern NSString * const _Nonnull kMaplyOpacity;

/// Width is used by the vector layer for line widths
extern NSString * const _Nonnull kMaplyVecWidth;
#define kWGVecWidth kMaplyVecWidth

/// If filled is set, we draw the areals as filled polygons
extern NSString * const _Nonnull kMaplyFilled;
#define kWGFilled kMaplyFilled

/// If set, the texture to apply to the feature
extern NSString * const _Nonnull kMaplyVecTexture;
/// The format of the image given by kMaplyVecTexture, default MaplyImage4Layer8Bit
extern NSString * const _Nonnull kMaplyVecTextureFormat;
/// X scale for textures applied to vectors
extern NSString * const _Nonnull kMaplyVecTexScaleX;
/// Y scale for textures applied to vectors
extern NSString * const _Nonnull kMaplyVecTexScaleY;

// scale for markers
extern NSString * const _Nonnull kMaplyMarkerScale;

/// The projection to use when generating texture coordinates
extern NSString * const _Nonnull kMaplyVecTextureProjection;
/// Tangent plane projection for texture coordinates
extern NSString * const _Nonnull kMaplyProjectionTangentPlane;
/// Screen space "projection" for texture coordinates
extern NSString * const _Nonnull kMaplyProjectionScreen;
/// No projection for texture coordinates
extern NSString * const _Nonnull kMaplyProjectionNone;

/// If set to true we'll centered any drawables we create for features
/// This fixes the jittering problem when zoomed in close
extern NSString * const _Nonnull kMaplyVecCentered;

/// Center of the feature, to use for texture calculations
extern NSString * const _Nonnull kMaplyVecCenterX;
extern NSString * const _Nonnull kMaplyVecCenterY;

/// For wide vectors, we can widen them in screen space or display space
extern NSString * const _Nonnull kMaplyWideVecCoordType;

/// Widened vectors are widened in real space.  The width is in meters.
extern NSString * const _Nonnull kMaplyWideVecCoordTypeReal;
/// Widened vectors are widened in screen space.  The width is in pixels.
extern NSString * const _Nonnull kMaplyWideVecCoordTypeScreen;

/// Controls the wide vector implementation.  Basic implementation by default.
extern NSString * const _Nonnull kMaplyWideVecImpl;

/// Default/old implementation of the wide vectors
extern NSString * const _Nonnull kMaplyWideVecImplDefault;

/// Performance implementation of the wide vectors
extern NSString * const _Nonnull kMaplyWideVecImplPerf;

/// For wide vectors we can control the line joins
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
extern NSString * const _Nonnull kMaplyWideVecJoinType;

/// Widened vectors are joined with miters.  Miters exceeding the miter limit are converted into bevels.
extern NSString * const _Nonnull kMaplyWideVecMiterJoin;
/// Widened vectors are joined with miters.  Miters exceeding the miter limit are clipped.
extern NSString * const _Nonnull kMaplyWideVecMiterClipJoin;
/// Widened vectors are joined with miters.  Miters exceeding the miter limit are ignored.
extern NSString * const _Nonnull kMaplyWideVecMiterSimpleJoin;
/// Widened vectors are joined with a circular arc
extern NSString * const _Nonnull kMaplyWideVecRoundJoin;
/// Widened vectors are joined with a bevel
extern NSString * const _Nonnull kMaplyWideVecBevelJoin;
/// No joins.  Also disables endcaps.
extern NSString * const _Nonnull kMaplyWideVecNoneJoin;

/// Determine how wide vectors behave when the ideal geometry is impossible
extern NSString * const _Nonnull kMaplyWideVecFallbackMode;
extern NSString * const _Nonnull kMaplyWideVecFallbackDefault;
/// When the line intersection is out of range, clip it to the range and use it.
extern NSString * const _Nonnull kMaplyWideVecFallbackClip;
/// When the line intersection is out of range, discard it and don't join the like segments.
extern NSString * const _Nonnull kMaplyWideVecFallbackNone;

/// Number of pixels to use in blending the edges of the wide vectors
extern NSString * const _Nonnull kMaplyWideVecEdgeFalloff;

/// For wide vectors we can control the ends
/// See: http://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
extern NSString * const _Nonnull kMaplyWideVecLineCapType;

/// Widened vector ends are flush
extern NSString * const _Nonnull kMaplyWideVecButtCap;
/// Widened vector ends are round (e.g. hot dog roads)
extern NSString * const _Nonnull kMaplyWideVecRoundCap;
/// Widened vector ends are extended a bit and then flush
extern NSString * const _Nonnull kMaplyWideVecSquareCap;

/// Miter joins will turn to bevel joins past this number of degrees
extern NSString * const _Nonnull kMaplyWideVecMiterLimit;

/// This is the length you'd like the texture to start repeating after.
/// It's real world coordinates for kMaplyWideVecCoordTypeReal and pixel size for kMaplyWideVecCoordTypeScreen
extern NSString * const _Nonnull kMaplyWideVecTexRepeatLen;

/// Initial texture coordinates
extern NSString * const _Nonnull kMaplyWideVecTexOffsetX;
extern NSString * const _Nonnull kMaplyWideVecTexOffsetY;

/// Offset to left (negative) or right (positive) of the centerline
extern NSString * const _Nonnull kMaplyWideVecOffset;

/// Close any un-closed areal features when drawing lines for them
extern NSString * const _Nonnull kMaplyVecCloseAreals;

/// If set we'll break up a vector feature to the given epsilon on a globe surface
extern NSString * const _Nonnull kMaplySubdivEpsilon;
/// If subdiv epsilon is set we'll look for a subdivision type. Default is simple.
extern NSString * const _Nonnull kMaplySubdivType;
/// Subdivide the vector edges along a great circle
extern NSString * const _Nonnull kMaplySubdivGreatCircle;
/// Subdivide the vector edges along a great circle with ellipsoidal math
extern NSString * const _Nonnull kMaplySubdivGreatCirclePrecise;
/// Subdivide into a fixed number of segmenets
extern NSString * const _Nonnull kMaplySubdivStatic;
/// Subdivide the vectors edges along lat/lon
extern NSString * const _Nonnull kMaplySubdivSimple;
/// Clip features along a grid of the given size
extern NSString * const _Nonnull kMaplySubdivGrid;
/// Used to turn off selection in vectors
extern NSString * const _Nonnull kMaplySelectable;

/// Attach a name to the generated drawable(s) for debugging purposes
extern NSString * const _Nonnull kMaplyDrawableName;

/// These are used for stickers

/// Sampling size along one dimension
extern NSString * const _Nonnull kMaplySampleX;
#define kWGSampleX kMaplySampleX
/// Sampling size along one dimension
extern NSString * const _Nonnull kMaplySampleY;
#define kWGSampleY kMaplySampleY
/// Images to use when changing a sticker
extern NSString * const _Nonnull kMaplyStickerImages;
/// Image format to use for the new images
extern NSString * const _Nonnull kMaplyStickerImageFormat;

/// These are used for billboards

/// Billboard orientation
extern NSString * const _Nonnull kMaplyBillboardOrient;
/// Billboards are oriented toward the eye, but rotate on the ground
extern NSString * const _Nonnull kMaplyBillboardOrientGround;
/// Billboards are oriented only towards the eye
extern NSString * const _Nonnull kMaplyBillboardOrientEye;

/// These are used for lofted polygons

/// Height above the ground
extern NSString * const _Nonnull kMaplyLoftedPolyHeight;
/// Boolean that turns on/off top (on by default)
extern NSString * const _Nonnull kMaplyLoftedPolyTop;
/// Boolean that turns on/off sides (on by default)
extern NSString * const _Nonnull kMaplyLoftedPolySide;
/// If present, we'll start the lofted poly above 0 height
extern NSString * const _Nonnull kMaplyLoftedPolyBase;
/// Grid size we used to chop the lofted polygons up (10 degress by default)
extern NSString * const _Nonnull kMaplyLoftedPolyGridSize;
/// If set to @(YES) this will draw an outline around the top of the lofted poly in lines
extern NSString * const _Nonnull kMaplyLoftedPolyOutline;
/// If set to @(YES) this will draw an outline around the bottom of the lofted poly in lines
extern NSString * const _Nonnull kMaplyLoftedPolyOutlineBottom;
/// If the outline is one this is the outline's color
extern NSString * const _Nonnull kMaplyLoftedPolyOutlineColor;
/// This is the outline's width if it's turned on
extern NSString * const _Nonnull kMaplyLoftedPolyOutlineWidth;
/// Draw priority of the lines created for the lofted poly outline
extern NSString * const _Nonnull kMaplyLoftedPolyOutlineDrawPriority;
/// If set and we're drawing an outline, this will create lines up the sides
extern NSString * const _Nonnull kMaplyLoftedPolyOutlineSide;

/// These are used for shapes

/// Samples (x) to use when converting shape to polygons
extern NSString * const _Nonnull kMaplyShapeSampleX;
/// Samples (y) to use when converting shape to polygons
extern NSString * const _Nonnull kMaplyShapeSampleY;
/// If set to true, we'll tessellate a shape using the opposite vertex ordering
extern NSString * const _Nonnull kMaplyShapeInsideOut;
/// Center for the shape geometry
extern NSString * const _Nonnull kMaplyShapeCenterX;
extern NSString * const _Nonnull kMaplyShapeCenterY;
extern NSString * const _Nonnull kMaplyShapeCenterZ;

/// These are used by active vector objects
extern NSString * const _Nonnull kMaplyVecHeight;
extern NSString * const _Nonnull kMaplyVecMinSample;

/// These are used by the particle systems
extern NSString * const _Nonnull kMaplyPointSize;
extern const float kMaplyPointSizeDefault;

/// These are used by the texture
extern NSString * const _Nonnull kMaplyTexFormat;
extern NSString * const _Nonnull kMaplyTexMinFilter;
extern NSString * const _Nonnull kMaplyTexMagFilter;
extern NSString * const _Nonnull kMaplyMinFilterNearest;
extern NSString * const _Nonnull kMaplyMinFilterLinear;
extern NSString * const _Nonnull kMaplyTexAtlas;
extern NSString * const _Nonnull kMaplyTexWrapX;
extern NSString * const _Nonnull kMaplyTexWrapY;
extern NSString * const _Nonnull kMaplyTexMipmap;

/// These are the various shader programs we set up by default
extern NSString * const _Nonnull kMaplyShaderDefaultTri;
extern NSString * const _Nonnull kMaplyDefaultTriangleShader;
extern NSString * const _Nonnull kMaplyShaderTriExp;

extern NSString * const _Nonnull kMaplyShaderDefaultModelTri;

extern NSString * const _Nonnull kMaplyShaderDefaultTriNoLighting;
extern NSString * const _Nonnull kMaplyNoLightTriangleShader;
extern NSString * const _Nonnull kMaplyShaderNoLightTriangleExp;
extern NSString * const _Nonnull kMaplyShaderDefaultMarker;

extern NSString * const _Nonnull kMaplyShaderDefaultTriScreenTex;

extern NSString * const _Nonnull kMaplyShaderDefaultTriMultiTex;
extern NSString * const _Nonnull kMaplyShaderDefaultTriMultiTexRamp;
extern NSString * const _Nonnull kMaplyShaderDefaultTriNightDay;

extern NSString * const _Nonnull kMaplyShaderDefaultLine;
extern NSString * const _Nonnull kMaplyDefaultLineShader;

extern NSString * const _Nonnull kMaplyShaderDefaultLineNoBackface;
extern NSString * const _Nonnull kMaplyNoBackfaceLineShader;

extern NSString * const _Nonnull kMaplyShaderBillboardGround;
extern NSString * const _Nonnull kMaplyShaderBillboardEye;

extern NSString * const _Nonnull kMaplyShaderDefaultWideVector;
extern NSString * const _Nonnull kMaplyShaderWideVectorPerformance;
extern NSString * const _Nonnull kMaplyShaderWideVectorExp;

extern NSString * const _Nonnull kMaplyScreenSpaceDefaultMotionProgram;
extern NSString * const _Nonnull kMaplyScreenSpaceDefaultProgram;
extern NSString * const _Nonnull kMaplyScreenSpaceMaskProgram;
extern NSString * const _Nonnull kMaplyScreenSpaceExpProgram;

extern NSString * const _Nonnull kMaplyAtmosphereProgram;
extern NSString * const _Nonnull kMaplyAtmosphereGroundProgram;

extern NSString * const _Nonnull kMaplyShaderParticleSystemPointDefault;
