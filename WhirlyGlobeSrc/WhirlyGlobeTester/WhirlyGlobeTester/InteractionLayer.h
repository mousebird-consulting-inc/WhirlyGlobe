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
#import <WhirlyGlobe/WhirlyGlobe.h>

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

// We'll start spinning after this much time
#define kAutoSpinInterval 20.0
// How far to spin within a second
#define kAutoSpinDegrees 2.50

/** Interaction Layer
    Controls data display and interaction for the globe.
 */
@interface InteractionLayer : NSObject <WhirlyGlobeLayer>
{
	WhirlyGlobeLayerThread *layerThread;
	WhirlyGlobe::GlobeScene *scene;
	WhirlyGlobeView *globeView;

	WhirlyGlobeVectorLayer *vectorLayer;
	WhirlyGlobeLabelLayer *labelLayer;
    WhirlyGlobeParticleSystemLayer *particleSystemLayer;
    WhirlyGlobeMarkerLayer *markerLayer;
    WhirlyGlobeLoftLayer *loftLayer;
    WhirlyGlobeSelectionLayer *selectionLayer;
    
    WhirlyGlobe::VectorDatabase *countryDb;  // Country outlines
    WhirlyGlobe::VectorDatabase *cityDb;  // City points

    WhirlyGlobe::SimpleIDSet partSysIDs;  // Particle systems added to globe
    WhirlyGlobe::SimpleIDSet vectorIDs;   // Vectors added to globe
    WhirlyGlobe::SimpleIDSet labelIDs;   // Labels added to the globe
    WhirlyGlobe::SimpleIDSet markerTexIDs;  // Textures added to the globe for markers
    WhirlyGlobe::SimpleIDSet markerIDs;  // Markers added to the globe    
    WhirlyGlobe::SimpleIDSet loftedPolyIDs;  // Lofted polygons added to the globe
    
    WhirlyGlobe::SimpleIDSet labelSelectIDs;  // Selection IDs used for labels
    WhirlyGlobe::SimpleIDSet markerSelectIDs;  // Selection IDs used for markers
    
    NSDictionary *options;  // Options for what to display and how
    bool loftedPolys;       // If set, we'll loft any country that gets tapped
    NSObject *autoSpinner;  // If we're autospinning, the object doing the work
    NSDate *lastTouched;    // When the user last interacted with the globe
}

// Initialize with a globe view.  All the rest is optional.
- (id)initWithGlobeView:(WhirlyGlobeView *)globeView;

// Called in the layer thread
- (void)startWithThread:(WhirlyGlobeLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)scene;

@property (nonatomic,assign) WhirlyGlobeVectorLayer *vectorLayer;
@property (nonatomic,assign) WhirlyGlobeLabelLayer *labelLayer;
@property (nonatomic,assign) WhirlyGlobeParticleSystemLayer *particleSystemLayer;
@property (nonatomic,assign) WhirlyGlobeMarkerLayer *markerLayer;
@property (nonatomic,assign) WhirlyGlobeLoftLayer *loftLayer;
@property (nonatomic,assign) WhirlyGlobeSelectionLayer *selectionLayer;

@end
