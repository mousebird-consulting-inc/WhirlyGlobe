/*
 *  VectorLoader.h
 *  WhirlyGlobeLib
 *
 *  Created by Stephen Gifford on 3/7/11.
 *  Copyright 2011 mousebird consulting. All rights reserved.
 *
 */

#import "DataLayer.h"
#import "Drawable.h"
#import "VectorLayer.h"
#import "LabelLayer.h"

/* Vector Info
    This is passed to the target/selector right before we create a drawable.
 */
@interface VectorLoaderInfo : NSObject
{
@public
    // The feature we're going to represent
    WhirlyGlobe::VectorShapeRef shape;
    
    // Where we'll draw the label, if there is one
    WhirlyGlobe::GeoCoord loc;
    
    // Attributes for visual and label representation
    // Setting this to nil will kill the feature entirely
    // Setting shape or label to nil will kill that component
    NSMutableDictionary *desc;
}

@property (nonatomic,retain) NSMutableDictionary *desc;

@end

/* Vector Loader description dictionary
    shape   <NSDictionary>  [See VectorLayer for details]
    label   <NSDictionary>  [See LabelLayer for details]
 */

/* Vector Loader
	Loads the vector data as it comes from one or more readers.
    This will insert itself into the layer thread run loop and add
     a feature every time it gets called until it runs out.
    Each reader you hand it may have an associated target/selector.
     This gets called for every vector feature you add.
    You can pass in defaults which control how the feature is represented.
 */
@interface VectorLoader : NSObject<WhirlyGlobeLayer>
{
    WhirlyGlobeLayerThread *layerThread;
    VectorLayer *vecLayer;   // We'll add our vector data here
    LabelLayer *labelLayer;  // Labels go here

    unsigned int curReader;   // Which reader we're looking at
    NSMutableArray *readers;  // Readers (and callbacks) we'll use
}

// Need a vector, and possibly label, layer(s) to feed data into
- (id)initWithVectorLayer:(VectorLayer *)layer labelLayer:(LabelLayer *)labelLayer;

// Add this reader to the list to be read from
// VectorLoader is now responsible for deletion
- (void)addReader:(WhirlyGlobe::VectorReader *)reader target:(NSObject *)target selector:(SEL)selector desc:(NSDictionary *)defaultDict;

// Convenience routine that creates a shapefile reader and adds that to the list
// The target/selector are called with VectorInfo.  See that for details
- (BOOL)addShapeFile:(NSString *)fileName target:(NSObject *)target selector:(SEL)selector desc:(NSDictionary *)defaultDict;

@end
