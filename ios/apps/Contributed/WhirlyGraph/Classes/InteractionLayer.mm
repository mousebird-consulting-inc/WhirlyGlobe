/*
 *  InteractionLayer.mm
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

#import "InteractionLayer.h"
#import "OptionsViewController.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

FeatureRep::FeatureRep() :
    name(nil), iso3(nil),
    outlineRep(WhirlyKit::EmptyIdentity), labelId(WhirlyKit::EmptyIdentity),
    subOutlinesRep(WhirlyKit::EmptyIdentity), subLabels(WhirlyKit::EmptyIdentity),
    midPoint(100.0), loftedPolyRep(0)
{
}

FeatureRep::~FeatureRep()
{
    if (name)
        [name release];
    name = nil;
    if (iso3)
        [iso3 release];
    iso3 = nil;
}

@implementation CountrySelectMsg

@synthesize country;
@synthesize tap;

- (void) dealloc
{
    self.country = nil;
    self.tap = nil;

    [super dealloc];    
}

@end

@interface InteractionLayer()
@property(nonatomic,retain) WhirlyKitLayerThread *layerThread;
@property(nonatomic,retain) WhirlyKitVectorLayer *vectorLayer;
@property(nonatomic,retain) WhirlyKitLabelLayer *labelLayer;
@property(nonatomic,retain) WhirlyGlobeLoftLayer *loftLayer;
@property(nonatomic,retain) WhirlyGlobeView *globeView;

- (NSNumber *)fetchValueForFeature:(FeatureRep *)feat;
- (void)addLoftedPoly:(FeatureRep *)feat minVal:(float)minVal maxVal:(float)maxVal theNum:(NSNumber *)thisVal;
@end

@implementation InteractionLayer

@synthesize countryDesc;
@synthesize oceanDesc;
@synthesize regionDesc;
@synthesize maxEdgeLen;
@synthesize displayField;

@synthesize layerThread;
@synthesize globeView;
@synthesize vectorLayer;
@synthesize labelLayer;
@synthesize loftLayer;

- (id)initWithVectorLayer:(WhirlyKitVectorLayer *)inVecLayer labelLayer:(WhirlyKitLabelLayer *)inLabelLayer loftLayer:(WhirlyGlobeLoftLayer *)inLoftLayer
                globeView:(WhirlyGlobeView *)inGlobeView
             countryShape:(NSString *)countryShape oceanShape:(NSString *)oceanShape regionShape:(NSString *)regionShape
{
	if ((self = [super init]))
	{
		self.vectorLayer = inVecLayer;
		self.labelLayer = inLabelLayer;
           self.loftLayer = inLoftLayer;
		self.globeView = inGlobeView;
                
        // Visual representation for countries when they first appear
        self.countryDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                            [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES],@"enable",
                             [NSNumber numberWithInt:3],@"drawOffset",
                             [UIColor whiteColor],@"color",
                             [NSNumber numberWithFloat:0.5],@"fade",
                             nil],@"shape",
                            [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES],@"enable",
                             [UIColor clearColor],@"backgroundColor",
                             [UIColor whiteColor],@"textColor",
                             [UIFont boldSystemFontOfSize:32.0],@"font",
                             [NSNumber numberWithInt:4],@"drawOffset",
                             [NSNumber numberWithFloat:0.75],@"fade",
                             nil],@"label",
                            nil];
        // Visual representation for oceans
        self.oceanDesc = [NSDictionary
                            dictionaryWithObjectsAndKeys:
                            [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES],@"enable",
                             [NSNumber numberWithInt:2],@"drawOffset",
                             [UIColor colorWithRed:0.75 green:0.75 blue:1.0 alpha:1.0],@"color",
                             [NSNumber numberWithFloat:0.5],@"fade",
                             nil],@"shape",                          
                          [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSNumber numberWithBool:YES],@"enable",
                           [UIColor colorWithRed:0.75 green:0.75 blue:1.0 alpha:1.0],@"textColor",
                           [NSNumber numberWithInt:4],@"drawOffset",
                           nil],@"label",
                            nil];
        // Visual representation of regions and their labels
        self.regionDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSDictionary dictionaryWithObjectsAndKeys:
                            [NSNumber numberWithBool:YES],@"enable",
                            [NSNumber numberWithInt:1],@"drawOffset",
                            [UIColor grayColor],@"color",
                            nil],@"shape",
                           [NSDictionary dictionaryWithObjectsAndKeys:
                            [NSNumber numberWithBool:YES],@"enable",
                            [UIColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0],@"textColor",
                            [UIFont systemFontOfSize:32.0],@"font",
                            [NSNumber numberWithInt:2],@"drawOffset",
                            nil],@"label",
                           nil];

        // Set up the various DBs
        // If they don't exist, we'll be creating them here and that
        //  will be slooooow
        NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
        NSString *bundleDir = [[NSBundle mainBundle] resourcePath];
        // The country DB we want in memory to speed up taps
        countryDb = new VectorDatabase(bundleDir,docDir,@"countries",new ShapeReader(countryShape),NULL);
        oceanDb = new VectorDatabase(bundleDir,docDir,@"oceans",new ShapeReader(oceanShape),NULL);
//        regionDb = new VectorDatabase(bundleDir,docDir,@"regions",new ShapeReader(regionShape),NULL);
        regionDb = NULL;

		// Register for the tap and press events
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapSelector:) name:WhirlyGlobeTapMsg object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pressSelector:) name:WhirlyGlobeLongPressMsg object:nil];
        
        minLoftVal = 0.0;  maxLoftVal = 1.0;
	}
	
	return self;
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	self.layerThread = nil;
    self.globeView = nil;
    self.vectorLayer = nil;
    self.labelLayer = nil;
    self.loftLayer = nil;
    self.countryDesc = nil;
    self.oceanDesc = nil;
    self.regionDesc = nil;
    if (countryDb)  delete countryDb;
    if (oceanDb)  delete oceanDb;
    if (regionDb)  delete regionDb;
    for (FeatureRepList::iterator it = featureReps.begin();
         it != featureReps.end(); ++it)
        delete (*it);
    
	[super dealloc];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)inScene
{
	self.layerThread = inThread;
	scene = inScene;
    
    [self performSelector:@selector(process:) onThread:layerThread withObject:nil waitUntilDone:NO];
}

// Note: Should really fill this in if this layer is used anywhere else
- (void)shutdown
{
    
}

// Do any book keeping work that doesn't involve interaction
// We're in the layer thread
- (void)process:(id)sender
{
    countryDb->process();

    [self performSelector:@selector(process:) onThread:layerThread withObject:nil waitUntilDone:NO];
}

// Update what we're displaying lofted
// Called from the main thread, probably
- (void)setDisplayField:(NSString *)newDisplayField
{
    [self performSelector:@selector(updateDisplayField:) onThread:layerThread withObject:newDisplayField waitUntilDone:NO];
}

// Somebody tapped the globe
// We're in the main thread here
- (void)tapSelector:(NSNotification *)note
{
	WhirlyGlobeTapMessage *msg = note.object;

    if (RotateToCountry)
    {
        // If we were rotating from one point to another, stop
        [globeView cancelAnimation];

        // Construct a quaternion to rotate from where we are to where
        //  the user tapped
        Eigen::Quaternionf newRotQuat = [globeView makeRotationToGeoCoord:msg.whereGeo keepNorthUp:YES];

        // Rotate to the given position over 1s
        globeView.delegate = [[[AnimateViewRotation alloc] initWithView:globeView rot:newRotQuat howLong:1.0] autorelease];
    }
	
	// Now we need to switch over to the layer thread for the rest of this
	[self performSelector:@selector(pickObject:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

// Someone tapped and held (pressed)
// We're in the main thread here
- (void)pressSelector:(NSNotification *)note
{
	WhirlyGlobeTapMessage *msg = note.object;
    
	// If we were rotating from one point to another, stop
	[globeView cancelAnimation];

    // We need to switch over to the layer thread to search our active outlines
    [self performSelector:@selector(selectObject:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

// Figure out where to put a label
//  and roughly how big.  Loc is already set.  We may tweak it.
- (void)calcLabelPlacement:(ShapeSet *)shapes loc:(GeoCoord &)loc  minWidth:(float)minWidth width:(float *)retWidth height:(float *)retHeight 
{
    double width=0.0,height=0.0;    
    VectorRing *largeLoop = NULL;
    float largeArea = 0.0;

    // Work through all the areals that make up the country
    // We get disconnected loops (think Alaska)
    for (ShapeSet::iterator it = shapes->begin();
         it != shapes->end(); it++)
    {        
        VectorArealRef theAreal = boost::dynamic_pointer_cast<VectorAreal> (*it);
        if (theAreal.get() && !theAreal->loops.empty())
        {
            // We need to find the largest loop.
            // It's there that we want to place the label
            for (unsigned int ii=0;ii<theAreal->loops.size();ii++)
            {
                VectorRing *thisLoop = &(theAreal->loops[ii]);
                float thisArea = GeoMbr(*thisLoop).area();
                if (!largeLoop || (thisArea > largeArea))
                {
                    largeArea = thisArea;
                    largeLoop = thisLoop;
                }
            }
            
        }
    }

    // Now get a width in the direction we care about
    if (largeLoop)
    {
        GeoMbr ringMbr(*largeLoop);
        Point3f pt0(ringMbr.ll().x(),ringMbr.ll().y(),0.0);
        Point3f pt1(ringMbr.lr().x(),ringMbr.lr().y(),0.0);
        width = (pt1-pt0).norm() * 0.5;
        // Don't let the width get too crazy
        width = std::min(width,0.5);
        loc = ringMbr.mid();
        if (width < 1e-5)
            width = minWidth;
    } else
        height = 0.02;
        
    if (retWidth)
        *retWidth = width;
    if (retHeight)
        *retHeight = height;
}

// Find an active feature that the given point falls within
// whichShape points to the overall or region outline we may have found
// We're in the layer thread
- (FeatureRep *)findFeatureRep:(const GeoCoord &)geoCoord height:(float)heightAboveGlobe whichShape:(VectorShapeRef *)whichShape
{
    for (FeatureRepList::iterator it = featureReps.begin();
         it != featureReps.end(); ++it)
    {
        FeatureRep *feat = *it;
        // Test the large outline
        if (heightAboveGlobe > feat->midPoint) {
            for (ShapeSet::iterator it = feat->outlines.begin();
                 it != feat->outlines.end(); ++it)
            {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
                if (ar->geoMbr.inside(geoCoord) && ar->pointInside(geoCoord))
                {
                    if (whichShape)
                        *whichShape = ar;
                    return feat;
                }
            }
        } else {
            // Test the small ones
            for (ShapeSet::iterator sit = feat->subOutlines.begin();
                 sit != feat->subOutlines.end(); ++sit)
            {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*sit);
                if (ar->geoMbr.inside(geoCoord) && ar->pointInside(geoCoord))
                {
                    if (whichShape)
                        *whichShape = ar;
                    return feat;
                }                    
            }
        }
    }
    
    return NULL;
}

// Much of the screen we want a label to take up
static const float DesiredScreenProj = 0.4;

// Add a new country
// We're in the layer thread
- (FeatureRep *)addCountryRep:(NSDictionary *)arDict tap:(WhirlyGlobeTapMessage *)tap
{
    FeatureRep *feat = new FeatureRep();
    feat->featType = FeatRepCountry;
    
    // Look for all the feature that have the same ADMIN field
    // This finds us disjoint features
    NSString *name = [arDict objectForKey:@"ADMIN"];
    feat->name = name;
    [feat->name retain];
    UIntSet outlineIds;
    countryDb->getMatchingVectors([NSString stringWithFormat:@"ADMIN like '%@'",name],feat->outlines);

    // Get ready to create the outline
    // We'll need to do it later, though
    NSMutableDictionary *thisCountryDesc = [NSMutableDictionary dictionaryWithDictionary:[countryDesc objectForKey:@"shape"]];

    NSString *region_sel = [arDict objectForKey:@"ADM0_A3"];
//    NSLog(@"Region = %@",region_sel);
    feat->iso3 = region_sel;
    [feat->iso3 retain];
    
    if (name)
    {                
        // Figure out the placement and size of the label
        // Other things will key off of this size
        GeoCoord loc;
        float labelWidth,labelHeight;
        [self calcLabelPlacement:&feat->outlines loc:loc minWidth:0.3 width:&labelWidth height:&labelHeight];
        
        // We'll tweak the display range of the country based on the label size (a bit goofy)
        feat->midPoint = labelWidth / (globeView.imagePlaneSize * 2.0 * DesiredScreenProj) * globeView.nearPlane;
        feat->midPoint = 0.0;
         
        // Place the country outline
        [thisCountryDesc setObject:[NSNumber numberWithFloat:feat->midPoint] forKey:@"minVis"];
        [thisCountryDesc setObject:[NSNumber numberWithFloat:100.0] forKey:@"maxVis"];
        feat->outlineRep = [vectorLayer addVectors:&feat->outlines desc:thisCountryDesc];
        
        // Make up a label for the country
        // We'll have it appear when we're farther out
        WhirlyKitSingleLabel *countryLabel = [[[WhirlyKitSingleLabel alloc] init] autorelease];
        countryLabel.text = name;
        NSMutableDictionary *labelDesc = [NSMutableDictionary dictionaryWithDictionary:[countryDesc objectForKey:@"label"]];
        [labelDesc setObject:[NSNumber numberWithFloat:feat->midPoint] forKey:@"minVis"];
        [labelDesc setObject:[NSNumber numberWithFloat:100.0] forKey:@"maxVis"];
        
        // Make sure it's not too large
        float testWidth=labelWidth,testHeight=0.0;
        [countryLabel calcWidth:&testWidth height:&testHeight defaultFont:[labelDesc objectForKey:@"font"]];
        if (testHeight > 0.08)
        {
            labelHeight = 0.08;
            labelWidth = 0.0;
        }
        // Look at minimum height as well
        if (testHeight < 0.005)
        {
            labelHeight = 0.005;
            labelWidth = 0.0;
        }
        [labelDesc setObject:[NSNumber numberWithDouble:labelWidth] forKey:@"width"];
        [labelDesc setObject:[NSNumber numberWithDouble:labelHeight] forKey:@"height"];
        // Change the color near the poles
        bool overIce = false;
        if (loc.lat() > 1.25 || loc.lat() < -1.1)
        {
            overIce = true;
            [labelDesc setObject:[UIColor grayColor] forKey:@"textColor"];
        }
        countryLabel.desc = labelDesc;
        countryLabel.loc = loc;
        feat->labelId = [labelLayer addLabel:countryLabel];
        
        // Loft the polygon if we're in that mode
        if (displayField)
        {
            NSNumber *theNum = [self fetchValueForFeature:feat];
            [self addLoftedPoly:feat minVal:minLoftVal maxVal:maxLoftVal theNum:theNum];
        }

    } else {
        // If there's no name (and no subregions) just toss up the outline
        [thisCountryDesc setObject:[NSNumber numberWithFloat:0.0] forKey:@"minVis"];
        [thisCountryDesc setObject:[NSNumber numberWithFloat:100.0] forKey:@"maxVis"];
        feat->outlineRep = [vectorLayer addVectors:&feat->outlines desc:thisCountryDesc];
    }
        
    featureReps.push_back(feat);
    return feat;
}

// Add a new ocean
// We're in the layer thread
- (FeatureRep *)addOceanRep:(VectorArealRef)ar
{
    FeatureRep *feat = new FeatureRep();
    feat->featType = FeatRepOcean;
    // Outline
    ar->subdivide(maxEdgeLen);
    feat->outlines.insert(ar);
    feat->midPoint = 0.0;
    feat->outlineRep = [vectorLayer addVector:ar desc:[oceanDesc objectForKey:@"shape"]];
    
    NSString *name = [ar->getAttrDict() objectForKey:@"Name"];
    if (name)
    {
        // Make up a label for the country
        // We'll have it appear when we're farther out
        NSMutableDictionary *labelDesc = [NSMutableDictionary dictionaryWithDictionary:[oceanDesc objectForKey:@"label"]];
        
        // Figure out where to place it
        GeoCoord loc;
        ShapeSet canShapes;
        canShapes.insert(ar);
        float labelWidth,labelHeight;
        [self calcLabelPlacement:&canShapes loc:loc minWidth:0.3 width:&labelWidth height:&labelHeight];
        [labelDesc setObject:[NSNumber numberWithDouble:labelWidth] forKey:@"width"];
        [labelDesc setObject:[NSNumber numberWithDouble:labelHeight] forKey:@"height"];
        feat->labelId = [labelLayer addLabel:name loc:loc desc:labelDesc];
    }
    
    featureReps.push_back(feat);
    return feat;
}

// Remove the given feature representation
// Including all its vectors and labels at various levels
- (void)removeFeatureRep:(FeatureRep *)feat
{
    FeatureRepList::iterator it = std::find(featureReps.begin(),featureReps.end(),feat);
    if (it != featureReps.end())
    {
        // Remove the vectors
        [vectorLayer removeVector:feat->outlineRep];
        [vectorLayer removeVector:feat->subOutlinesRep];

        // Remove labels
        if (feat->labelId)
            [labelLayer removeLabel:feat->labelId];
        [labelLayer removeLabel:feat->subLabels];
        
        // Remove lofted polys
        [loftLayer removeLoftedPoly:feat->loftedPolyRep];
        
        featureReps.erase(it);
        delete feat;
    }
}

// Try to pick an object
// We're in the layer thread
- (void)pickObject:(WhirlyGlobeTapMessage *)msg
{
    GeoCoord coord = msg.whereGeo;
    
    // Let's look for objects we're already representing
    FeatureRep *theFeat = [self findFeatureRep:coord height:msg.heightAboveSurface whichShape:NULL];
    
    // We found a country or its regions
    if (theFeat)
    {
        // Turn the country/ocean off
        if (msg.heightAboveSurface >= theFeat->midPoint)
            [self removeFeatureRep:theFeat];
        else {
            // Selected a region
            
        }
    } else {
        // Look for a country first
        ShapeSet foundShapes;
        countryDb->findArealsForPoint(coord,foundShapes);
        if (!foundShapes.empty())
        {
            // Toss in anything we found
            for (ShapeSet::iterator it = foundShapes.begin();
                 it != foundShapes.end(); ++it)
            {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
                [self addCountryRep:ar->getAttrDict() tap:msg];
            }
        } else {
            // Look for an ocean
            oceanDb->findArealsForPoint(coord,foundShapes);
            for (ShapeSet::iterator it = foundShapes.begin();
                 it != foundShapes.end(); ++it)
            {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
                [self addOceanRep:ar];
            }
        }
    }
    
    // If we're over the maximum, remove a feature
    while (featureReps.size() > MaxFeatureReps)
        [self removeFeatureRep:*(featureReps.begin())];
}

// Look for an outline to select
// We're in the layer thread
- (void)selectObject:(WhirlyGlobeTapMessage *)msg
{
    GeoCoord coord = msg.whereGeo;
    
    // Look for an object, taking LODs into account
    VectorShapeRef selectedShape;
    FeatureRep *theFeat = [self findFeatureRep:coord height:msg.heightAboveSurface whichShape:&selectedShape];
    
    if (theFeat)
    {
        switch (theFeat->featType)
        {
            case FeatRepCountry:
                if (theFeat->outlines.find(selectedShape) != theFeat->outlines.end())
                {
                    // Let everyone else know the user selected a country
                    CountrySelectMsg *selectMsg = [[[CountrySelectMsg alloc] init] autorelease];
                    selectMsg.country = theFeat->name;
                    selectMsg.tap = msg;
                    [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeCountrySelectMsg object:selectMsg]];
                } else {
//                    NSLog(@"User selected region within country:\n%@",[selectedShape->getAttrDict() description]);
                }
                break;
            case FeatRepOcean:
//                NSLog(@"User selected ocean:\n%@",[selectedShape->getAttrDict() description]);
                break;
        }
    }
    
    // Note: If you want to bring up a view at this point,
    //        don't forget to switch back to the main thread
}


DBWrapper *dbWrapper = nil;

// Query the DB for the field range
- (void)queryFieldRange:(float *)minVal maxVal:(float *)maxVal
{
    *minVal = [[dbWrapper min:displayField] floatValue];
    *maxVal = [[dbWrapper max:displayField] floatValue];
    
//    NSLog(@"Min = %f, max = %f",*minVal,*maxVal);
}

// Temperature basec colors
#if 0
#define kNumTempColors 18
static float TempColors[kNumTempColors][3] =
{
    {0.142,   0.000,   0.850},
    {0.097,   0.112,   0.970},
    {0.160,   0.342,   1.000},
    {0.240,   0.531,   1.000},
    {0.340,   0.692,   1.000},
    {0.460,   0.829,   1.000},
    {0.600,   0.920,   1.000},
    {0.740,   0.978,   1.000},
    {0.920,   1.000,   1.000},
    {1.000,   1.000,   0.920},
    {1.000,   0.948,   0.740},
    {1.000,   0.840,   0.600},
    {1.000,   0.676,   0.460},
    {1.000,   0.472,   0.340},
    {1.000,   0.240,   0.240},
    {0.970,   0.155,   0.210},
    {0.850,   0.085,   0.187},
    {0.650,   0.000,   0.130}
};
#endif
#define kNumPercepColors 8
static float PercepColors[kNumPercepColors][3] =
{
    {51.0/256.0, 204.0/256.0, 255.0/256.0},
    {51.0/256.0, 153.0/256.0, 255.0/256.0},
    {51.0/256.0, 102.0/256.0, 204.0/256.0},
    {51.0/256.0, 51.0/256.0, 204.0/256.0},
    {102.0/256.0, 51.0/256.0, 204.0/256.0},
    {153.0/256.0, 51.0/256.0, 255.0/256.0},
    {255.0/256.0, 0, 127.0/256.0},
    {255.0/256.0, 0, 255.0/256.0}
};

// Calculate a color given a value between 0 and 1
// We'll pick two from our list and then interpolate between
- (void)calcColorVal:(float)unitVal red:(float *)red green:(float *)green blue:(float *)blue
{
    int minColor = std::floor(unitVal * (kNumPercepColors-1));
    int maxColor = std::ceil(unitVal * (kNumPercepColors-1));
    float blend = (unitVal * (kNumPercepColors-1)) - minColor;
    if (minColor < 0)
    {
        blend = 0;
        minColor = 0;
        maxColor = 1;
    }
    if (maxColor >= kNumPercepColors)
    {
        blend = 1.0;
        minColor = kNumPercepColors-1;
        maxColor = kNumPercepColors-1;
    }
    
    float r0,b0,g0,r1,b1,g1;
    r0 = PercepColors[minColor][0];
    g0 = PercepColors[minColor][1];
    b0 = PercepColors[minColor][2];
    r1 = PercepColors[maxColor][0];
    g1 = PercepColors[maxColor][1];
    b1 = PercepColors[maxColor][2];

    *red = (r1-r0)*blend + r0;
    *green = (g1-g0)*blend + g0;
    *blue = (b1-b0)*blend + b0;
    
    *red /= 2;  *green /= 2;  *blue /= 2;
}

// Range above the earth we'll loft things
static const float MaxLoftHeight = 0.1;
static const float MinLoftHeight = 0.01;

static const float LoftAlphaVal = 0.25;

- (NSNumber *)fetchValueForFeature:(FeatureRep *)feat
{
    if (!dbWrapper)
    {
        dbWrapper = [[DBWrapper alloc] init];
        [dbWrapper open];
    }

    NSNumber *thisNum = (feat->iso3 ? [dbWrapper valueForDataSetName:displayField country:feat->iso3] : nil);
    
//    if (thisNum)
//        NSLog(@"%@: %f",feat->iso3,[thisNum floatValue]);
    
    return thisNum;
}

// Add a lofted polygon, querying the DB for the given field
- (void)addLoftedPoly:(FeatureRep *)feat minVal:(float)minVal maxVal:(float)maxVal theNum:(NSNumber *)thisNum
{
//    NSLog(@"minVal = %f, maxVal = %f, thisVal = %f",minVal,maxVal,thisVal);

    if (thisNum)
    {
        // Create a lofted polygon for the country
        NSMutableDictionary *loftCountryDesc = [NSMutableDictionary dictionary];
        float red,green,blue;
        float unitFactor = ([thisNum floatValue] - minVal) / (maxVal - minVal);
        [self calcColorVal:unitFactor red:&red green:&green blue:&blue];
        [loftCountryDesc setObject:[UIColor colorWithRed:red green:green blue:blue alpha:LoftAlphaVal] forKey:@"color"];
        [loftCountryDesc setObject:[NSNumber numberWithFloat:(unitFactor  * (MaxLoftHeight - MinLoftHeight) + MinLoftHeight)] forKey:@"height"];
        [loftCountryDesc setObject:[NSNumber numberWithFloat:1.0] forKey:@"fade"];
        feat->loftedPolyRep = [loftLayer addLoftedPolys:&feat->outlines desc:loftCountryDesc cacheName:feat->iso3];    
    }
}

// Update the display field we're using
// This involves tweaking the lofted polys
// We're in the layer thread
- (void)updateDisplayField:(NSString *)newDisplayField
{
    displayField = newDisplayField;
    
    if (!dbWrapper)
    {
        dbWrapper = [[DBWrapper alloc] init];
        [dbWrapper open];
    }
    
    // Need min and max values for the field
    if (newDisplayField)
    {
        [self queryFieldRange:&minLoftVal maxVal:&maxLoftVal];
    }
    
    // Remove existing lofted polys and replace with new
    for (FeatureRepList::iterator it = featureReps.begin();
         it != featureReps.end(); ++it)
    {
        FeatureRep *featRep = *it;
        
        NSNumber *thisNum = [self fetchValueForFeature:featRep];

        // In some cases we can just change the representation.  This is faster
        if (displayField && featRep->loftedPolyRep != 0 && thisNum)
        {
            // Note: Change addLoftedPoly to handle this logic
            NSMutableDictionary *loftCountryDesc = [NSMutableDictionary dictionary];
            float red,green,blue;
            float unitFactor = ([thisNum floatValue] - minLoftVal) / (maxLoftVal - minLoftVal);
            [self calcColorVal:unitFactor red:&red green:&green blue:&blue];
            [loftCountryDesc setObject:[UIColor colorWithRed:red green:green blue:blue alpha:LoftAlphaVal] forKey:@"color"];
            [loftCountryDesc setObject:[NSNumber numberWithFloat:(unitFactor  * (MaxLoftHeight - MinLoftHeight) + MinLoftHeight)] forKey:@"height"];

            [loftLayer changeLoftedPoly:featRep->loftedPolyRep desc:loftCountryDesc];
        } else {
            // Remove and/or add the new representation
            if (featRep->loftedPolyRep)
            {
                [loftLayer removeLoftedPoly:featRep->loftedPolyRep];
                featRep->loftedPolyRep = 0;
            }

            if (displayField && thisNum && (minLoftVal != maxLoftVal))
                [self addLoftedPoly:featRep minVal:minLoftVal maxVal:maxLoftVal theNum:thisNum];
        }
    }
}

@end
