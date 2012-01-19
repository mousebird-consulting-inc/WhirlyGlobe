//
//  InteractionLayer.h
//  WhirlyMapWorld
//
//  Created by Steve Gifford on 1/10/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WhirlyGlobe/WhirlyGlobe.h>

@interface InteractionLayer : NSObject <WhirlyGlobeLayer>
{
	WhirlyGlobeLayerThread *layerThread;
	WhirlyGlobe::GlobeScene *scene;
	WhirlyMapView *theView;
    
	WhirlyGlobeVectorLayer *vectorLayer;
	WhirlyGlobeLabelLayer *labelLayer;
    WhirlyGlobeLoftLayer *loftLayer;
    
    WhirlyGlobe::VectorDatabase *countryDb;  // Country outlines
    
    NSString *countrySetName;
}

// Initialize with a map view.  All the rest is optional.
- (id)initWithMapView:(WhirlyMapView *)mapView;

// Called in the layer thread
- (void)startWithThread:(WhirlyGlobeLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)scene;

@property (nonatomic,assign) WhirlyGlobeVectorLayer *vectorLayer;
@property (nonatomic,assign) WhirlyGlobeLabelLayer *labelLayer;
@property (nonatomic,assign) WhirlyGlobeLoftLayer *loftLayer;

@end
