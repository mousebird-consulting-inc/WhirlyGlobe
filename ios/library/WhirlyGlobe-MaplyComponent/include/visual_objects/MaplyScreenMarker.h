/*
 *  WGScreenMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import <UIKit/UIKit.h>
#import "math/MaplyCoordinate.h"

/** 
    The Screen Marker is a 2D object that displays an image on the screen tracking a given location.
    
    The screen marker will track the given geographic location and display a centered rectangle with the given image.  Essentially it's a free floating icon, similar to the MaplyScreenLabel and it will always be drawn on top of other objects.  If the location would be behind the globe (in globe mode), the marker will disappear.
    
    If you're looking for a 3D marker object, that's the MaplyMarker.
  */
@interface MaplyScreenMarker : NSObject

/** 
    The location we're tracking for this particular screen marker.
    
    Locations are in geographic (lon/lat in radians).
  */
@property (nonatomic,assign) MaplyCoordinate loc;

/** 
    Screen size in points.
    
    The marker will always be this size on the screen.  The size is specified in pixels.
  */
@property (nonatomic,assign) CGSize size;

/** 
    An optional rotation to apply to the screen marker.
    
    This is a rotation we'll apply after the screen position has been calculated.  The angle is in radians counter-clockwise from north.
 */
@property (nonatomic,assign) double rotation;

/** 
    Image or texture to use for the marker.
    
    If set we'll stretch this UIImage (or MaplyTexture) out over the marker rectangle.  If not set, the marker will just be a colored rectangle.  The view controller tracks this object and will reuse its texture and dispose of it as needed.
  */
@property (nonatomic,strong) id  __nullable image;

/** 
    Images to display on the sticker.
    
    If this is set rather than image, we will cycle through these images on the screen marker.  It will take period time to cycle through them all.
  */
@property (nonatomic,strong) NSArray * __nullable images;

/** 
    The time we'll take to cycle through all the images for the marker.
    
    If images are passed in, this is the time it will take to cycle through them all.  By default this is 5s.
  */
@property (nonatomic) double period;

/** 
    Color for this particular marker.
    
    If set, this the color we'll use for the marker or how we'll tint the image.
    
    This overrides the color set in the description dictionary.
  */
@property (nonatomic,strong) UIColor * __nullable color;

/** 
    The layout importance compared to other features, 0 by default.
    
    The toolkit has a simple layout engine that runs several times per second.  It controls the placement of all participating screen based features, such as MaplyScreenLabel and MaplyScreenMaker objects.  This value controls the relative importance of this particular object.  By default that importance is 0 so the object must compete with others.  Setting it to MAXFLOAT will bypass the layout engine entirely and the object will always appear.
 */
@property (nonatomic,assign) float layoutImportance;

/** 
    The size of the marker for layout purposes.
    
    If layoutImportance is not set to MAXFLOAT, the screen marker will get throw into the mix when doing screen layout.  With this, you can set the size of the marker when considering layout.  If you set this to (0,0) the maker will take up no space, but still be considered in the layout.
  */
@property (nonatomic,assign) CGSize layoutSize;

/** 
    Offset in screen coordinates.
    
    Set to zero by default, this is the offset we'll apply to a given screen marker before it's drawn.  The values are screen points.
  */
@property (nonatomic,assign) CGPoint offset;

/** 
    Vertex attributes to apply to this screen marker.
    
    MaplyVertexAttribute objects are passed all the way to the shader.  Read that page for details on what they do.
    
    The array of vertex attributes provided here will be copied onto all the vertices we create for the shader.  This means you can use these to do things for a single billboard in your shader.
 */
@property (nonatomic,strong) NSArray * __nullable vertexAttributes;

/** 
    Screen marker selectability.  On by default
    
    If set, this marker can be selected by the user.  If not set, this screen marker will never appear in selection results.
 */
@property (nonatomic,assign) bool selectable;

/**
    A unique identifier for the marker that's propagated through the system.
  */
@property (nonatomic,retain,nullable) NSString *uniqueID;

/** 
    User data object for selection
 
    When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen marker means to them.
 */
@property (nonatomic,strong) id  __nullable userObject;

@end

/** 
    A version of the maply screen marker that moves.
    
    This version of the screen marker can move in a limited way over time.
  */
@interface MaplyMovingScreenMarker : MaplyScreenMarker

/// The end point for animation
@property (nonatomic,assign) MaplyCoordinate endLoc;

/// How long it will take the screen marker to get to endLoc
@property (nonatomic,assign) NSTimeInterval duration;

@end

typedef MaplyScreenMarker WGScreenMarker;
