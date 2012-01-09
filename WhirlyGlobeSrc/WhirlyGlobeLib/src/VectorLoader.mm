/*
 *  VectorLoader.mm
 *  WhirlyGlobeLib
 *
 *  Created by Stephen Gifford on 3/7/11.
 *  Copyright 2011 mousebird consulting. All rights reserved.
 *
 */

#import "VectorLoader.h"
#import "ShapeReader.h"
#import "NSDictionary+Stuff.h"

using namespace WhirlyGlobe;

@implementation VectorLoaderInfo
@synthesize desc;

- (id)initWithShape:(WhirlyGlobe::VectorShapeRef)inShape desc:(NSDictionary *)inDesc
{
    if ((self = [super init]))
    {
        shape = inShape;
        // Note: Should do a copy on in the property
        self.desc = [inDesc copy];
    }
    
    return self;
}

- (void)dealloc
{
    self.desc = nil;
    [super dealloc];
}

@end

// Information associated with a particular reader
// Target/selector is called per object from the reader
@interface ReaderInfo : NSObject
{
    VectorReader    *reader;
    NSObject        *target;
    SEL             selector;
    NSDictionary    *desc;
}

@property (nonatomic,assign) VectorReader *reader;
@property (nonatomic,assign) NSObject *target;
@property (nonatomic,assign) SEL selector;
@property (nonatomic,retain) NSDictionary *desc;

@end

@implementation ReaderInfo

@synthesize reader;
@synthesize target;
@synthesize selector;
@synthesize desc;

- (void)dealloc
{
    delete self.reader;
    self.desc = nil;
    [super dealloc];
}

@end


// ----------- Vector Loader -----------

@interface VectorLoader()
@property (nonatomic,retain) WhirlyGlobeLayerThread *layerThread;
@property (nonatomic,retain) VectorLayer *vecLayer;
@property (nonatomic,retain) LabelLayer *labelLayer;
@property (nonatomic,retain) NSMutableArray *readers;
@end

@implementation VectorLoader

@synthesize layerThread;
@synthesize vecLayer;
@synthesize labelLayer;
@synthesize readers;

- (id)initWithVectorLayer:(VectorLayer *)inVecLayer labelLayer:(LabelLayer *)inLabelLayer
{
    if ((self = [super init]))
    {
        self.vecLayer = inVecLayer;
        self.labelLayer = inLabelLayer;
        curReader = 0;
        self.readers = [[[NSMutableArray alloc] init] autorelease];
    }
    
    return self;
}

- (void)dealloc
{
    self.vecLayer = nil;
    self.labelLayer = nil;
    self.readers = nil;
    [super dealloc];
}

// Just schedule us for later
- (void)startWithThread:(WhirlyGlobeLayerThread *)inLayerThread scene:(WhirlyGlobe::GlobeScene *)inScene
{
    self.layerThread = inLayerThread;
    [self performSelector:@selector(process:) withObject:nil];
}

- (void)process:(id)sender
{
    if (curReader < [self.readers count])
    {
        ReaderInfo *readerInfo = [self.readers objectAtIndex:curReader];
        
        // Grab the next shape and go
        VectorShapeRef shape = readerInfo.reader->getNextObject(NULL);
        if (shape.get())
        {
            VectorLoaderInfo *loaderInfo = [[[VectorLoaderInfo alloc] initWithShape:shape desc:readerInfo.desc] autorelease];
            
            // If there's a label, figure out where it goes
            loaderInfo->loc = shape->calcGeoMbr().mid();
            
            // Let the caller know what we're doing
            if (readerInfo.target)
                [readerInfo.target performSelector:readerInfo.selector withObject:loaderInfo];
            
            // See what we're supposed to create
            NSDictionary *shapeInfo = [loaderInfo.desc objectForKey:@"shape" checkType:[NSDictionary class] default:nil];
            NSDictionary *labelInfo = [loaderInfo.desc objectForKey:@"label" checkType:[NSDictionary class] default:nil];
            
            if (shapeInfo)
            {
                // Note: Should be setting the newId
                [self.vecLayer addVector:loaderInfo->shape desc:shapeInfo];
            }
            
            if (labelInfo)
            {
                NSString *labelAttr = [loaderInfo.desc objectForKey:@"attrForLabel" checkType:[NSString class] default:@"name"];
                NSString *labelStr = [loaderInfo->shape->getAttrDict() objectForKey:labelAttr checkType:[NSString class] default:nil];
                if (labelStr)
                    [self.labelLayer addLabel:labelStr loc:loaderInfo->loc desc:loaderInfo.desc];
            }
            
        } else
            curReader++;
    }
    
    // Schedule the next one
    // Note: Could be smarter about this
    [self performSelector:@selector(process:) onThread:layerThread withObject:nil waitUntilDone:NO];
}

- (void)runAddReader:(ReaderInfo *)readerInfo
{
    [self.readers addObject:readerInfo];
}

// Add a reader into our queue of stuff to look at
- (void)addReader:(WhirlyGlobe::VectorReader *)reader target:(NSObject *)target selector:(SEL)selector desc:(NSDictionary *)defaultDict
{
    ReaderInfo *readerInfo = [[[ReaderInfo alloc] init] autorelease];
    readerInfo.reader = reader;
    readerInfo.target = target;
    readerInfo.selector = selector;
    readerInfo.desc = defaultDict;
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddReader:readerInfo];
    else
        [self performSelector:@selector(runAddReader:) onThread:layerThread withObject:readerInfo waitUntilDone:NO];
}

- (BOOL)addShapeFile:(NSString *)fileName target:(NSObject *)target selector:(SEL)selector desc:(NSDictionary *)defaultDict
{
    if (!fileName)
        return NO;
    ShapeReader *shapeReader = new ShapeReader(fileName);
    if (!shapeReader->isValid())
    {
        delete shapeReader;
        return NO;
    }
    
    [self addReader:shapeReader target:target selector:selector desc:defaultDict];
    return YES;
}

@end
