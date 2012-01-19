//
//  InteractionLayer.m
//  WhirlyMapWorld
//
//  Created by Steve Gifford on 1/10/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import "InteractionLayer.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@interface InteractionLayer()
@property (nonatomic,assign) WhirlyGlobeLayerThread *layerThread;
@property (nonatomic,retain) WhirlyMapView *theView;
@property (nonatomic,retain) NSString *countrySetName;

@end

@implementation InteractionLayer

@synthesize vectorLayer;
@synthesize labelLayer;
@synthesize loftLayer;
@synthesize layerThread;
@synthesize theView;
@synthesize countrySetName;

- (id)initWithMapView:(WhirlyMapView *)mapView
{
    self = [super init];
    if (self)
    {
        self.theView = mapView;
        countryDb = NULL;
    }
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    self.layerThread = nil;
    self.theView = nil;
    
    self.vectorLayer = nil;
    self.labelLayer = nil;
    
    if (countryDb)
        delete countryDb;
    countryDb = NULL;
    
    self.countrySetName = nil;
    
    [super dealloc];
}

// Called in the layer thread
- (void)startWithThread:(WhirlyGlobeLayerThread *)inThread scene:(WhirlyGlobe::GlobeScene *)inScene
{
    self.layerThread = inThread;
    scene = inScene;

    // Set up the country DB
    // We want a cache, so read it or build it
    NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];
    self.countrySetName = @"countries50m";
    NSString *countryShape = [[NSBundle mainBundle] pathForResource:@"ne_50m_admin_0_countries" ofType:@"shp"];
    countryDb = new VectorDatabase(bundleDir,docDir,countrySetName,new ShapeReader(countryShape),NULL,true);
    
    [self performSelector:@selector(addCountries:) withObject:nil afterDelay:0.0];
}

// Load the data
- (void)addCountries:(id)sender
{
    // Visual description of the vectors and labels
    NSDictionary *shapeDesc = 
    [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithBool:YES],@"enable",
     [NSNumber numberWithInt:0],@"drawOffset",
     [UIColor whiteColor],@"color",
     [NSNumber numberWithFloat:1.0],@"fade",
     nil];
    NSDictionary *labelDesc =
    [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithBool:YES],@"enable",
     [UIColor clearColor],@"backgroundColor",
     [UIColor whiteColor],@"textColor",
     [UIFont boldSystemFontOfSize:32.0],@"font",
     [NSNumber numberWithInt:4],@"drawOffset",
     [NSNumber numberWithFloat:0.05],@"width",
     [NSNumber numberWithFloat:1.0],@"fade",
     nil];
    NSDictionary *loftDesc =
    [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithFloat:0.0],@"height",
     [UIColor colorWithRed:1.0 green:1.0 blue:0.0 alpha:1.0],@"color",
     nil];

    // No caches and we have to do it the hard way
    NSMutableArray *labels = [NSMutableArray array];
    
    // Work through the vectors.  This will get big, so don't do this normally.
    ShapeSet shapes;
    for (unsigned int ii=0;ii<countryDb->numVectors();ii++)
    {
        VectorShapeRef shape = countryDb->getVector(ii,true);
        NSString *name = [shape->getAttrDict() objectForKey:@"ADMIN"];
        VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(shape);
        if (ar.get() && name)
        {
            // This frees the attribute memory, which we don't really need
            shape->setAttrDict(nil);
            shapes.insert(shape);
            
            // And build a label.  We'll add these as a group below
            WhirlyGlobeSingleLabel *label = [[[WhirlyGlobeSingleLabel alloc] init] autorelease];
            label.isSelectable = YES;
            label.selectID = Identifiable::genId();
            label.text = name;
            [label setLoc:ar->calcGeoMbr().mid()];
            [labels addObject:label];
            
            // Do the lofted poly right here
            if (loftLayer)
            {
                NSMutableDictionary *thisLoftDesc = [NSMutableDictionary dictionaryWithDictionary:loftDesc];
                float red = drand48(), green = drand48(), blue = drand48();
                [thisLoftDesc setObject:[UIColor colorWithRed:red green:green blue:blue alpha:1.0] forKey:@"color"];
                NSString *polyCache = [NSString stringWithFormat:@"%@_%@",countrySetName,name];
                [self.loftLayer addLoftedPoly:ar desc:thisLoftDesc cacheName:polyCache];
            }
        }
    }
    
    // Toss the vectors on top of the globe
    [self.vectorLayer addVectors:&shapes desc:shapeDesc cacheName:nil];
    
    // And the labels
    [self.labelLayer addLabels:labels desc:labelDesc cacheName:nil];
}


@end
