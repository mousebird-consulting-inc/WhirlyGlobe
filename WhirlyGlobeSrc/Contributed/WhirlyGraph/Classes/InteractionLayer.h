/*
 *  InteractionLayer.h
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 2/3/11.
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

#import <list>
#import <Foundation/Foundation.h>
#import "WhirlyGlobe.h"

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
    
    NSString *name;                  // Feature name
    NSString *iso3;                  // ISO3 code
    FeatureRepType featType;            // What this is
    WhirlyKit::ShapeSet outlines;  // Areal feature outline (may be more than one)
    WhirlyKit::SimpleIdentity outlineRep;  // ID for the outline in the vector layer
    WhirlyKit::SimpleIdentity labelId;  // ID of label in label layer
    float midPoint;  // Distance where we switch from the low res to high res representation
    // Sub-features, such as states
    WhirlyKit::ShapeSet subOutlines;            // IDs in the regionDB
    WhirlyKit::SimpleIdentity subOutlinesRep;  // Represented with a single entity in the vector layer
    WhirlyKit::SimpleIdentity subLabels;       // ID for all the sub outline labels together
    // Lofted polygons representation
    WhirlyKit::SimpleIdentity loftedPolyRep;
};

/* Country Select Message
    The interaction layer fires off one of these
     when the user selects an explicit country (with a name)
 */
#define WhirlyGlobeCountrySelectMsg @"WhirlyGlobeCountrySelect"
@interface CountrySelectMsg : NSObject
{
    NSString *country;
    WhirlyGlobeTapMessage *tap;  // The tap that cause the select
}

@property (nonatomic,retain) NSString *country;
@property (nonatomic,retain) WhirlyGlobeTapMessage *tap;

@end

typedef std::list<FeatureRep *> FeatureRepList;

// Maximum number of features we're willing to represent at once
static const unsigned int MaxFeatureReps = 15;

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
    WhirlyGlobeLoftLayer *loftLayer;
    
    NSDictionary *countryDesc;  // Visual representation for countries and their labels
    NSDictionary *oceanDesc;    // Visual representation for oceans and their labels
    NSDictionary *regionDesc;   // Visual representation for regions (states/provinces) and labels

    // Databases that live on top of the shape files (for fast access)
    WhirlyKit::VectorDatabase *countryDb;
    WhirlyKit::VectorDatabase *oceanDb;
    WhirlyKit::VectorDatabase *regionDb;
    
    FeatureRepList featureReps;   // Countries we're currently representing
    
    float maxEdgeLen;    // Maximum length for a vector edge
    
    NSString *displayField;    // Field to use for displaying loft values.  nil means none
    
    float minLoftVal,maxLoftVal;
}

@property (nonatomic,retain) NSDictionary *countryDesc;
@property (nonatomic,retain) NSDictionary *oceanDesc;
@property (nonatomic,retain) NSDictionary *regionDesc;
@property (nonatomic,assign) float maxEdgeLen;
@property (nonatomic,retain) NSString *displayField;

// Need a pointer to the vector layer to start with
- (id)initWithVectorLayer:(WhirlyKitVectorLayer *)layer labelLayer:(WhirlyKitLabelLayer *)labelLayer loftLayer:(WhirlyGlobeLoftLayer *)loftLayer
                globeView:(WhirlyGlobeView *)globeView countryShape:(NSString *)countryShape oceanShape:(NSString *)oceanShape 
              regionShape:(NSString *)regionShape;

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)scene;

@end
