/*
 *  InteractionLayer.h
 *  WhirlyGlobeTester
 *
 *  Created by Steve Gifford on 10/26/11.
 *  Copyright 2011 mousebird consulting
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
#import "WhirlyGlobe.h"

// Notification for control dictionary changes
#define kWGControlChange @"WGControlChange"

// Key names in the control parameters dictionary
#define kWGCountryControl @"WGCountryControl"
#define kWGMarkerControl @"WGMarkerControl"
#define kWGParticleControl @"WGParticleControl"
#define kWGLoftedControl @"WGLoftedControl"
#define kWGGridControl @"WGGridControl"
#define kWGStatsControl @"WGStatsControl"

// Notification of selection.  String is the object
#define kWGSelectionNotification @"WGSelectionNotification"

// Values for the various types
typedef enum {IsOff=0,OnNonCached,OnCached} WGSegmentEnum;

#define DoAutoSpin false

// We'll start spinning after this much time
#define kAutoSpinInterval 20.0
// How far to spin within a second
#define kAutoSpinDegrees 2.50

/** Interaction Layer
    Controls data display and interaction for the globe.
 */
@interface InteractionLayer : NSObject <WhirlyKitLayer>
{
	WhirlyKitLayerThread * __weak layerThread;
	WhirlyGlobe::GlobeScene *scene;
	WhirlyGlobeView * __weak globeView;

	WhirlyKitVectorLayer * __weak vectorLayer;
	WhirlyKitLabelLayer * __weak labelLayer;
    WhirlyKitParticleSystemLayer * __weak particleSystemLayer;
    WhirlyKitMarkerLayer * __weak markerLayer;
    WhirlyGlobeLoftLayer * __weak loftLayer;
    WhirlyKitSelectionLayer * __weak selectionLayer;
    
    WhirlyKit::VectorDatabase *countryDb;  // Country outlines
    WhirlyKit::VectorDatabase *cityDb;  // City points

    WhirlyKit::SimpleIDSet partSysIDs;  // Particle systems added to globe
    WhirlyKit::SimpleIDSet vectorIDs;   // Vectors added to globe
    WhirlyKit::SimpleIDSet labelIDs;   // Labels added to the globe
    WhirlyKit::SimpleIDSet markerTexIDs;  // Textures added to the globe for markers
    WhirlyKit::SimpleIDSet markerIDs;  // Markers added to the globe    
    WhirlyKit::SimpleIDSet loftedPolyIDs;  // Lofted polygons added to the globe
    
    WhirlyKit::SimpleIDSet labelSelectIDs;  // Selection IDs used for labels
    WhirlyKit::SimpleIDSet markerSelectIDs;  // Selection IDs used for markers
    
    NSDictionary *options;  // Options for what to display and how
    bool loftedPolys;       // If set, we'll loft any country that gets tapped
    NSObject *autoSpinner;  // If we're autospinning, the object doing the work
    CFTimeInterval lastTouched;  // When the user last interacted with the globe
}

// Initialize with a globe view.  All the rest is optional.
- (id)initWithGlobeView:(WhirlyGlobeView *)globeView;

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)scene;

@property (nonatomic,weak) WhirlyKitVectorLayer *vectorLayer;
@property (nonatomic,weak) WhirlyKitLabelLayer *labelLayer;
@property (nonatomic,weak) WhirlyKitParticleSystemLayer *particleSystemLayer;
@property (nonatomic,weak) WhirlyKitMarkerLayer *markerLayer;
@property (nonatomic,weak) WhirlyGlobeLoftLayer *loftLayer;
@property (nonatomic,weak) WhirlyKitSelectionLayer *selectionLayer;

@end
