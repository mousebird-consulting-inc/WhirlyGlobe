/*
 *  MaplySharedAttributes.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2012 mousebird consulting
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
#define kWGRenderHintZBuffer @"zbuffer"
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
#define kWGRenderHintCulling @"culling"
/// This is an NSNumber specifying the WhirlyKitSceneRenderer to use (1 or 2).  Default is 2
#define kWGRendererOpenGLVersion @"sceneRendererVersion"
/// These are options for lighting modes, basically different default shader programs.  Only works with OpenGL ES 2.0 mode.
/// Accepted values are: none,regular
#define kWGRendererLightingMode @"rendererLightingMode"

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
#define kWGDrawOffset @"drawOffset"
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
#define kWGDrawPriority @"drawPriority"
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define kWGMinVis @"minVis"
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define kWGMaxVis @"maxVis"
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
#define kWGFade @"fade"

/// Default draw offset for 3D markers.  Set to avoid label/marker intererence
#define kWGMarkerDrawOffsetDefault 1
#define kWGMarkerDrawPriorityDefault 1

/// Default draw offset for vectors.
#define kWGVectorDrawOffsetDefault 3
#define kWGVectorDrawPriorityDefault 3

/// Default draw offset for stickers
#define kWGStickerDrawOffsetDefault 2
#define kWGStickerDrawPriorityDefault 2

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
#define kWGTextColor @"textColor"
/// Background color for the text.  Takes a UIColor.
#define kWGBackgroundColor @"backgroundColor"
/// Font to use in rendering text.  Takes a UIFont.
#define kWGFont @"font"
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
#define kWGLabelHeight @"height"
/// Default width of the text.  See height for more info and, in general, use height instead.
#define kWGLabelWidth @"width"
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
#define kWGJustify @"justify"
/// If set, we'll draw a shadow behind each label with this as the stroke size
#define kWGShadowSize @"shadowSize"
/// If shadow size is being used, we can control the shadow color like so
#define kWGShadowColor @"shadowColor"

/// Default draw offset for 3D labels.  This is set to avoid label/marker interference
#define kWGLabelDrawOffsetDefault 2
#define kWGLabelDrawPriorityDefault 2

/// These are used for screen and regular markers.

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
#define kWGColor @"color"

/// Width is used by the vector layer for line widths
#define kWGVecWidth @"width"

/// If filled is set, we draw the areals as filled polygons
#define kWGFilled @"filled"

/// If sample is set we'll break the line up before laying it down on the globe
#define kWGSample @"sample"

/// These are used for stickers

/// Sampling size along one dimension
#define kWGSampleX @"sampleX"
/// Sampling size along one dimension
#define kWGSampleY @"sampleY"

