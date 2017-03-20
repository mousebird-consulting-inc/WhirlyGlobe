/*
 *  InteractionLayer.h
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 2/3/11.
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

#import <list>
#import <Foundation/Foundation.h>
#import "WhirlyGlobe.h"

typedef std::set<WhirlyKit::SimpleIdentity> SimpleIDSet;

typedef enum {FeatRepCountry,FeatRepOcean} FeatureRepType;

// Set this to have the model rotate to the currently selected country
#define RotateToCountry true

// Representation of a large feature that has parts, such as a country or ocean
// This tracks all the various labels, region outlines and so forth
class FeatureRep
{
public:
    FeatureRep();
    ~FeatureRep();
    
    FeatureRepType featType;            // What this is
    WhirlyKit::ShapeSet outlines;  // Areal feature outline (may be more than one)
    WhirlyKit::SimpleIdentity outlineRep;  // ID for the outline in the vector layer
    WhirlyKit::SimpleIdentity labelId;  // ID of label in label layer
    float midPoint;  // Distance where we switch from the low res to high res representation
    // Sub-features, such as states
    WhirlyKit::ShapeSet subOutlines;            // IDs in the regionDB
    WhirlyKit::SimpleIdentity subOutlinesRep;  // Represented with a single entity in the vector layer
    WhirlyKit::SimpleIdentity subLabels;       // ID for all the sub outline labels together
};

typedef std::list<FeatureRep *> FeatureRepList;

// Maximum number of features we're willing to represent at once
static const unsigned int MaxFeatureReps = 8;

/* (Whirly Globe) Interaction Layer
    This handles user interaction (taps) and manipulates data in the
    vector and label layers accordingly.
 */
@interface InteractionLayer : NSObject <WhirlyKitLayer>
{
	WhirlyKitLayerThread *layerThread;
	WhirlyGlobe::GlobeScene *scene;
	WhirlyGlobeView *globeView;
	WhirlyKitVectorLayer *vectorLayer;
	WhirlyKitLabelLayer *labelLayer;
    
    NSDictionary *countryDesc;  // Visual representation for countries and their labels
    NSDictionary *oceanDesc;    // Visual representation for oceans and their labels
    NSDictionary *regionDesc;   // Visual representation for regions (states/provinces) and labels

    // Databases that live on top of the shape files (for fast access)
    WhirlyKit::VectorDatabase *countryDb;
    WhirlyKit::VectorDatabase *oceanDb;
    WhirlyKit::VectorDatabase *regionDb;
    
    FeatureRepList featureReps;   // Countries we're currently representing
    
    float maxEdgeLen;    // Maximum length for a vector edge
    
    AnimateViewRotation *animateRotation;
}

@property (nonatomic,strong) NSDictionary *countryDesc;
@property (nonatomic,strong) NSDictionary *oceanDesc;
@property (nonatomic,strong) NSDictionary *regionDesc;
@property (nonatomic,assign) float maxEdgeLen;

// Need a pointer to the vector layer to start with
- (id)initWithVectorLayer:(WhirlyKitVectorLayer *)layer labelLayer:(WhirlyKitLabelLayer *)labelLayer globeView:(WhirlyGlobeView *)globeView
             countryShape:(NSString *)countryShape oceanShape:(NSString *)oceanShape regionShape:(NSString *)regionShape;

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)scene;

@end
