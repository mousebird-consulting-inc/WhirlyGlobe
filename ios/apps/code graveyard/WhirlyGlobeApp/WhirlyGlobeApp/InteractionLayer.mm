/*
 *  InteractionLayer.mm
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

#import "InteractionLayer.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

FeatureRep::FeatureRep() :
    outlineRep(EmptyIdentity), labelId(EmptyIdentity),
    subOutlinesRep(EmptyIdentity), subLabels(EmptyIdentity),
    midPoint(100.0)
{
}

FeatureRep::~FeatureRep()
{
}

@interface InteractionLayer()
@end

@implementation InteractionLayer

@synthesize countryDesc;
@synthesize oceanDesc;
@synthesize regionDesc;
@synthesize maxEdgeLen;

- (id)initWithVectorLayer:(WhirlyKitVectorLayer *)inVecLayer labelLayer:(WhirlyKitLabelLayer *)inLabelLayer globeView:(WhirlyGlobeView *)inGlobeView
             countryShape:(NSString *)countryShape oceanShape:(NSString *)oceanShape regionShape:(NSString *)regionShape
{
	if ((self = [super init]))
	{
		vectorLayer = inVecLayer;
		labelLayer = inLabelLayer;
		globeView = inGlobeView;
                
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
                             [NSNumber numberWithFloat:0.5],@"fade",
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
                           [NSNumber numberWithFloat:0.5],@"fade",
                           nil],@"label",
                            nil];
        // Visual representation of regions and their labels
        self.regionDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSDictionary dictionaryWithObjectsAndKeys:
                            [NSNumber numberWithBool:YES],@"enable",
                            [NSNumber numberWithInt:1],@"drawOffset",
                            [UIColor grayColor],@"color",
                            [NSNumber numberWithFloat:0.5],@"fade",
                            nil],@"shape",
                           [NSDictionary dictionaryWithObjectsAndKeys:
                            [NSNumber numberWithBool:YES],@"enable",
                            [UIColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0],@"textColor",
                            [UIFont systemFontOfSize:32.0],@"font",
                            [NSNumber numberWithInt:2],@"drawOffset",
                            [NSNumber numberWithFloat:0.5],@"fade",
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
        regionDb = new VectorDatabase(bundleDir,docDir,@"regions",new ShapeReader(regionShape),NULL);

		// Register for the tap and press events
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapSelector:) name:WhirlyGlobeTapMsg object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pressSelector:) name:WhirlyGlobeLongPressMsg object:nil];
	}
	
	return self;
}

- (void)shutdown
{
    // Note: Not handling correctly
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];

    if (countryDb)  delete countryDb;
    if (oceanDb)  delete oceanDb;
    if (regionDb)  delete regionDb;
    for (FeatureRepList::iterator it = featureReps.begin();
         it != featureReps.end(); ++it)
        delete (*it);
    
}

- (void)startWithThread:(WhirlyKitLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)inScene
{
	layerThread = inThread;
	scene = inScene;
    
    [self performSelector:@selector(process:) onThread:layerThread withObject:nil waitUntilDone:NO];
}

// Do any book keeping work that doesn't involve interaction
// We're in the layer thread
- (void)process:(id)sender
{
    countryDb->process();

    [self performSelector:@selector(process:) onThread:layerThread withObject:nil waitUntilDone:NO];
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
        animateRotation = [[AnimateViewRotation alloc] initWithView:globeView rot:newRotQuat howLong:1.0];
        globeView.delegate = animateRotation;
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
    
    CoordSystemDisplayAdapter *adapter = globeView.coordAdapter;

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
        Point3f pt0 = adapter->localToDisplay(Point3f(ringMbr.ll().x(),ringMbr.ll().y(),0.0));
        Point3f pt1 = adapter->localToDisplay(Point3f(ringMbr.lr().x(),ringMbr.lr().y(),0.0));
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
- (FeatureRep *)addCountryRep:(NSDictionary *)arDict
{
    FeatureRep *feat = new FeatureRep();
    feat->featType = FeatRepCountry;
    
    // Look for all the feature that have the same ADMIN field
    // This finds us disjoint features
    NSString *name = [arDict objectForKey:@"ADMIN"];
    UIntSet outlineIds;
    countryDb->getMatchingVectors([NSString stringWithFormat:@"ADMIN like '%@'",name],feat->outlines);

    // Get ready to create the outline
    // We'll need to do it later, though
    NSMutableDictionary *thisCountryDesc = [NSMutableDictionary dictionaryWithDictionary:[countryDesc objectForKey:@"shape"]];

    NSString *region_sel = [arDict objectForKey:@"ADM0_A3"];
    if (name)
    {
        // Look for regions that correspond to the country
        // Note: replace with SQL
        ShapeSet regionShapes;
        regionDb->getMatchingVectors([NSString stringWithFormat:@"ISO like '%@'",region_sel],regionShapes);
        
        // Figure out the placement and size of the label
        // Other things will key off of this size
        GeoCoord loc;
        float labelWidth,labelHeight;
        [self calcLabelPlacement:&feat->outlines loc:loc minWidth:0.3 width:&labelWidth height:&labelHeight];
        
        // We'll tweak the display range of the country based on the label size (a bit goofy)
        feat->midPoint = labelWidth / (globeView.imagePlaneSize * 2.0 * DesiredScreenProj) * globeView.nearPlane;
        if (regionShapes.empty())
            feat->midPoint = 0.0;
        // Don't let the country outline disappear too quickly
        if (feat->midPoint > 0.6)
            feat->midPoint = 0.6;
         
        // Place the country outline
        [thisCountryDesc setObject:[NSNumber numberWithFloat:feat->midPoint] forKey:@"minVis"];
        [thisCountryDesc setObject:[NSNumber numberWithFloat:100.0] forKey:@"maxVis"];
        feat->outlineRep = [vectorLayer addVectors:&feat->outlines desc:thisCountryDesc];
        
        // Make up a label for the country
        // We'll have it appear when we're farther out
        WhirlyKitSingleLabel *countryLabel = [[WhirlyKitSingleLabel alloc] init];
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

        // Add all the region vectors together
        NSMutableDictionary *regionShapeDesc = [NSMutableDictionary dictionaryWithDictionary:[regionDesc objectForKey:@"shape"]];
        [regionShapeDesc setObject:[NSNumber numberWithFloat:0.0] forKey:@"minVis"];
        [regionShapeDesc setObject:[NSNumber numberWithFloat:feat->midPoint] forKey:@"maxVis"];
        feat->subOutlinesRep = [vectorLayer addVectors:&regionShapes desc:regionShapeDesc];
        feat->subOutlines = regionShapes;

        // Add all the region labels together
        NSMutableDictionary *regionLabelDesc = [NSMutableDictionary dictionaryWithDictionary:[regionDesc objectForKey:@"label"]];
        [regionLabelDesc setObject:[NSNumber numberWithFloat:0.0] forKey:@"minVis"];
        [regionLabelDesc setObject:[NSNumber numberWithFloat:feat->midPoint] forKey:@"maxVis"];
        NSMutableArray *labels = [[NSMutableArray alloc] init];
        for (ShapeSet::iterator it=regionShapes.begin();
             it != regionShapes.end(); ++it)
        {
            NSString *regionName = [(*it)->getAttrDict() objectForKey:@"NAME_1"];
            if (regionName)
            {
                GeoCoord regionLoc;
                WhirlyKitSingleLabel *sLabel = [[WhirlyKitSingleLabel alloc] init];
                NSMutableDictionary *thisDesc = [NSMutableDictionary dictionary];
                ShapeSet canShapes;
                canShapes.insert(*it);
                float thisWidth,thisHeight;
                [self calcLabelPlacement:&canShapes loc:regionLoc minWidth:0.004 width:&thisWidth height:&thisHeight];
                sLabel.loc = regionLoc;
                sLabel.text = regionName;
                sLabel.desc = thisDesc;
                // Max out the height of these labels
                float testWidth=thisWidth,testHeight=0.0;
                [sLabel calcWidth:&testWidth height:&testHeight defaultFont:[regionLabelDesc objectForKey:@"font"]];
                // Smaller max height for the regions (in comparison to the countries)
                if (testHeight > 0.05)
                {
                    thisWidth = 0.0;
                    thisHeight = 0.05;
                }
                [thisDesc setObject:[NSNumber numberWithDouble:thisWidth] forKey:@"width"];
                [thisDesc setObject:[NSNumber numberWithDouble:thisHeight] forKey:@"height"];
                [labels addObject:sLabel];
            }
        }
        if ([labels count] > 0)
            feat->subLabels = [labelLayer addLabels:labels desc:regionLabelDesc];
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
                [self addCountryRep:ar->getAttrDict()];
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
//                    NSLog(@"User selected country:\n%@",[selectedShape->getAttrDict() description]);
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

@end
