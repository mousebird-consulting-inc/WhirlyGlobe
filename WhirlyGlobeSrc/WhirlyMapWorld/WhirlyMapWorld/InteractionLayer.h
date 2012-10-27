//
//  InteractionLayer.h
//  WhirlyMapWorld
//
//  Created by Steve Gifford on 1/10/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WhirlyGlobe.h"

@interface InteractionLayer : NSObject <WhirlyKitLayer>
{
	WhirlyKitLayerThread *layerThread;
	WhirlyGlobe::GlobeScene *scene;
	WhirlyGlobeView *theView;
    
	WhirlyKitVectorLayer *vectorLayer;
	WhirlyKitLabelLayer *labelLayer;
    WhirlyGlobeLoftLayer *loftLayer;
    
    WhirlyKit::VectorDatabase *countryDb;  // Country outlines
    
    NSString *countrySetName;
    AnimateViewRotation *animateRotation;
}

// Initialize with a map view.  All the rest is optional.
- (id)initWithGlobeView:(WhirlyGlobeView *)mapView;

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)scene;

@property (nonatomic,assign) WhirlyKitVectorLayer *vectorLayer;
@property (nonatomic,assign) WhirlyKitLabelLayer *labelLayer;
@property (nonatomic,assign) WhirlyGlobeLoftLayer *loftLayer;

@end
