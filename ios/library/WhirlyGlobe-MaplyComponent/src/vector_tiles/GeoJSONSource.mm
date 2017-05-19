//
//  GeoJSONSource.m
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-18.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "GeoJSONSource.h"
#import "SLDStyleSet.h"
#import "MaplyVectorObject_private.h"
#import "VectorData.h"

using namespace WhirlyKit;

@implementation GeoJSONSource {
 
    __weak MaplyBaseViewController *_baseVC;
    NSURL *_geoJSONURL, *_sldURL;
    NSArray *_compObjs;
    bool _loaded, _enabled;
    int _relativeDrawPriority;
    SLDStyleSet *_styleSet;
}

- (id)initWithViewC:(MaplyBaseViewController *)baseVC GeoJSONURL:(NSURL *)geoJSONURL sldURL:(NSURL *)sldURL relativeDrawPriority:(int)relativeDrawPriority {
    self = [super init];
    if (self) {
        
        if (![geoJSONURL isFileURL] || ![sldURL isFileURL]) {
            NSLog(@"Required: File URLs.");
            return nil;
        }
        
        _baseVC = baseVC;
        _geoJSONURL = geoJSONURL;
        _sldURL = sldURL;
        _loaded = false;
        _enabled = false;
        _relativeDrawPriority = relativeDrawPriority;
    }
    return self;
}

- (bool) loaded {
    return _loaded;
}

- (bool) enabled {
    return _enabled;
}

- (void) setEnabled:(bool)enabled {
    __strong MaplyBaseViewController *baseVC = _baseVC;
    if (_enabled == enabled || !_loaded || !baseVC)
        return;
    _enabled = enabled;
    if (_enabled)
        [baseVC enableObjects:_compObjs mode:MaplyThreadAny];
    else
        [baseVC disableObjects:_compObjs mode:MaplyThreadAny];
}

- (void)startParse {
    [self startParseWithCompletion:^{}];
}

- (void)startParseWithCompletion:(nonnull void (^)()) completionBlock {
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        __strong MaplyBaseViewController *baseVC = _baseVC;
        if (!baseVC) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completionBlock();
                return;
            });
        }
        
        _styleSet = [[SLDStyleSet alloc] initWithViewC:baseVC useLayerNames:NO relativeDrawPriority:_relativeDrawPriority];
        [_styleSet loadSldURL:_sldURL];

        ShapeSet shapes;
        NSData *geoJSONData = [NSData dataWithContentsOfURL:_geoJSONURL];
        NSString *crs;
        bool parsed = VectorParseGeoJSON(shapes, geoJSONData, &crs);
        
        if (!parsed  || shapes.empty()) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completionBlock();
                return;
            });
        }
        
        NSMutableDictionary *featureStyles = [NSMutableDictionary new];
        MaplyTileID nullTileID = {0, 0, 0};
        NSMutableArray *compObjs = [NSMutableArray array];
        
        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it) {
            
            NSMutableDictionary *attributes = (*it)->getAttrDict();
            
            NSMutableArray *vectorObjs = [NSMutableArray array];
            
            VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
            
            if (points) {
                attributes[@"geometry_type"] = @"POINT";
                [self processPoints:points andVectorObjs:vectorObjs];
            } else if (lin) {
                attributes[@"geometry_type"] = @"LINESTRING";
                [self processLinear:lin andVectorObjs:vectorObjs];
            } else if (ar) {
                attributes[@"geometry_type"] = @"POLYGON";
                [self processAreal:ar andVectorObjs:vectorObjs];
            }
            
            NSArray *styles = [_styleSet stylesForFeatureWithAttributes:attributes onTile:nullTileID inLayer:@"" viewC:baseVC];
            
            if (!styles || styles.count == 0)
                continue;
            
            for(NSObject<MaplyVectorStyle> *style in styles) {
                NSMutableArray *featuresForStyle = featureStyles[style.uuid];
                if(!featuresForStyle) {
                    featuresForStyle = [NSMutableArray new];
                    featureStyles[style.uuid] = featuresForStyle;
                }
                [featuresForStyle addObjectsFromArray:vectorObjs];
            }
            for (MaplyVectorObject *vecObj in vectorObjs) {
                vecObj.attributes = attributes;
            }
            
        }
        
        NSArray *symbolizerKeys = [featureStyles.allKeys sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"self" ascending:YES]]];
        dispatch_async(dispatch_get_main_queue(), ^{
            
            for(id key in symbolizerKeys) {
                NSObject<MaplyVectorStyle> *symbolizer = [_styleSet styleForUUID:key viewC:baseVC];
                NSArray *features = featureStyles[key];
                [compObjs addObjectsFromArray:[symbolizer buildObjects:features forTile:nullTileID viewC:baseVC]];
            }
            
            _compObjs = compObjs;
            [baseVC enableObjects:compObjs mode:MaplyThreadAny];
            _loaded = true;
            _enabled = true;
            completionBlock();
        });


    });
    
}



- (void)processPoints:(VectorPointsRef &)points andVectorObjs:(NSMutableArray *)vectorObjs {
    
    for (unsigned int ii=0;ii<points->pts.size();ii++) {
        const Point2f &pt = points->pts[ii];
        MaplyCoordinate coord = MaplyCoordinateMake(pt.x(), pt.y());
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithPoint:coord attributes:nil];
        [vectorObjs addObject:vecObj];
    }

}

- (void)processLinear:(VectorLinearRef &)linear andVectorObjs:(NSMutableArray *)vectorObjs {
    MaplyCoordinate staticCoords[GEOJSON_MAX_POINTS];
    if (linear->pts.size() > 0 && linear->pts.size() < GEOJSON_MAX_POINTS) {
        for (unsigned int ii=0;ii<linear->pts.size();ii++) {
            const Point2f &pt = linear->pts[ii];
            staticCoords[ii] = MaplyCoordinateMake(pt.x(), pt.y());
        }
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:staticCoords numCoords:linear->pts.size() attributes:nil];
        [vectorObjs addObject:vecObj];
    } else {
        NSLog(@"Skipping LineString with too many points");
    }
}

- (void)processAreal:(VectorArealRef &)areal andVectorObjs:(NSMutableArray *)vectorObjs {
    MaplyCoordinate staticCoords[GEOJSON_MAX_POINTS];
    
    MaplyVectorObject *polyVecObj;
    NSMutableArray *lineVecObjs = [NSMutableArray array];
    
    for (unsigned int li=0;li<areal->loops.size();li++) {
        const VectorRing &ring = areal->loops[li];
        if (ring.size() < GEOJSON_MAX_POINTS) {
            for (unsigned int ii=0;ii<ring.size();ii++) {
                const Point2f &pt = ring[ii];
                staticCoords[ii] = MaplyCoordinateMake(pt.x(), pt.y());
            }
            if (li==0)
                polyVecObj = [[MaplyVectorObject alloc] initWithAreal:staticCoords numCoords:ring.size()-1 attributes:nil];
            else if (polyVecObj)
                [polyVecObj addHole:staticCoords numCoords:ring.size()-1];
            else
                break;
            [lineVecObjs addObject:[[MaplyVectorObject alloc] initWithLineString:staticCoords numCoords:ring.size() attributes:nil]];
        } else {
            NSLog(@"GeoJSONSource; skipping Polygon with too many points");
            polyVecObj = nil;
            break;
        }
    }
    if (polyVecObj) {
        [vectorObjs addObject:polyVecObj];
        [vectorObjs addObjectsFromArray:lineVecObjs];
    }
    
}

@end
