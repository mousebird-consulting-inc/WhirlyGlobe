//
//  MaplyVectorStyleSimple.m
//  WhirlyGlobe-MaplyComponent
//
//  Created by Steve Gifford on 3/15/16.
//
//

#import "MaplyVectorStyleSimple.h"
#import "MaplyScreenLabel.h"

@implementation MaplyVectorStyleSimpleGenerator
{
    NSMutableDictionary *stylesByUUID;
    NSMutableDictionary *stylesByLayerName;
    MaplyVectorStyleSimplePolygon *polyStyle;
    MaplyVectorStyleSimplePoint *pointStyle;
    MaplyVectorStyleSimpleLinear *linStyle;
}

- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    stylesByUUID = [NSMutableDictionary dictionary];
    stylesByLayerName = [NSMutableDictionary dictionary];
    
    return self;
}

- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC
{
    MaplyVectorStyleSimple *style;
    
    
    // Look for existing layer
    @synchronized (self) {
        style = stylesByLayerName[layer];
        if (style)
            return @[style];
    }
    int layer_order = (int)[attributes[@"layer_order"] integerValue];
    
    int geomType = (int)[attributes[@"geometry_type"] integerValue];
    switch (geomType)
    {
        case GeomTypePoint:
            style = [[MaplyVectorStyleSimplePoint alloc] initWithViewC:viewC];
            style.drawPriority = kMaplyLabelDrawPriorityDefault+layer_order;
            break;
        case GeomTypeLineString:
            style = [[MaplyVectorStyleSimpleLinear alloc] initWithViewC:viewC];
            style.drawPriority = kMaplyVectorDrawPriorityDefault+1000+layer_order;
            break;
        case GeomTypePolygon:
            style = [[MaplyVectorStyleSimplePolygon alloc] initWithViewC:viewC];
            style.drawPriority = kMaplyVectorDrawPriorityDefault+layer_order;
            break;
        default:
            break;
    }
    
    @synchronized (self) {
        stylesByUUID[style.uuid] = style;
        stylesByLayerName[layer] = style;
    }
    
    return @[style];
}

// We'll display all layers
- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID
{
    return true;
}

- (nullable MaplyVectorTileStyle *)styleForUUID:(NSString *__nonnull)uuid viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC
{
    return stylesByUUID[uuid];
}

@end

@implementation MaplyVectorStyleSimple

- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    _viewC = viewC;
    
    // UUID is just unique, not particularly profound
    _uuid = [@(rand()) stringValue];
    
    return self;
}

- (NSString *)getCategory
{
    return nil;
}

- (NSArray * __nullable )buildObjects:(NSArray * _Nonnull)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
{
    return nil;
}

@end

@implementation MaplyVectorStyleSimplePolygon

- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithViewC:viewC];
    float red = drand48()/2.0;
    float green = drand48()/2.0;
    float blue = 0.0;
    _color = [UIColor colorWithRed:red+0.5 green:green+0.5 blue:blue+0.5 alpha:0.5];
    
    return self;
}

- (NSArray * __nullable )buildObjects:(NSArray * _Nonnull)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
{
    NSMutableArray *tessObjs = [NSMutableArray array];
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        MaplyVectorObject *tessObj = [vecObj tesselate];
        if (tessObj)
            [tessObjs addObject:tessObj];
    }
    
    MaplyComponentObject *compObj = [super.viewC addVectors:tessObjs desc:@{kMaplyColor: _color,
                                                                           kMaplyFilled: @(YES),
                                                                           kMaplyDrawPriority: @(self.drawPriority)
                                                                           } mode:MaplyThreadCurrent];
    if (!compObj)
        return nil;
    
    return @[compObj];
}

@end

@implementation MaplyVectorStyleSimplePoint

- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithViewC:viewC];
    _font = [UIFont systemFontOfSize:24.0];
    
    return self;
}

- (NSArray * __nullable )buildObjects:(NSArray * _Nonnull)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
{
    NSMutableArray *labels = [NSMutableArray array];
    
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        NSArray *points = [vecObj splitVectors];
        for (MaplyVectorObject *point in points)
        {
            NSString *name = point.attributes[@"name"];
            MaplyCoordinate center = [point center];
            
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.text = (name ? name : @".");
            label.loc = center;
            
            [labels addObject:label];
        }
    }
    
    if (labels.count == 0)
        return nil;
    
    MaplyComponentObject *compObj = [self.viewC addScreenLabels:labels desc:@{kMaplyTextColor: [UIColor blackColor],
                                                                              kMaplyFont: _font}
                                                           mode:MaplyThreadCurrent];
    
    if (!compObj)
        return nil;
    
    return @[compObj];
}

@end

@implementation MaplyVectorStyleSimpleLinear

- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithViewC:viewC];
    float red = drand48()/2.0;
    float green = drand48()/2.0;
    float blue = drand48()/2.0;
    _color = [UIColor colorWithRed:red+0.5 green:green+0.5 blue:blue+0.5 alpha:1.0];
    
    return self;
}

- (NSArray * __nullable )buildObjects:(NSArray * _Nonnull)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
{
    MaplyComponentObject *compObj = [super.viewC addVectors:vecObjs desc:@{kMaplyColor: _color,
                                                                           kMaplyDrawPriority: @(self.drawPriority),
                                                                           kMaplyFilled: @(NO),
                                                                           kMaplyVecWidth: @(4.0)
                                                                           } mode:MaplyThreadCurrent];

    if (!compObj)
        return nil;

    return @[compObj];
}

@end
