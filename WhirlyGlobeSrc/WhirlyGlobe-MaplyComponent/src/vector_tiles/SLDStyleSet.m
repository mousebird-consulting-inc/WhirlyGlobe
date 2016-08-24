//
//  SLDStyleSet.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "SLDStyleSet.h"
#import "SLDExpressions.h"
#import "SLDOperators.h"
#import "SLDSymbolizers.h"
#import "MaplyVectorTileStyle.h"


@implementation SLDNamedLayer
@end

@implementation SLDUserStyle
@end

@implementation SLDFeatureTypeStyle
@end

@implementation SLDRule
@end

@implementation SLDFilter

@end


@interface SLDStyleSet () {
}

@property (nonatomic, strong) NSMutableDictionary *symbolizers;

@end


@implementation SLDStyleSet {
    NSMutableDictionary *_namedLayers;
    NSInteger symbolizerId;
    
}



- (id)initWithViewC:(MaplyBaseViewController *)viewC useLayerNames:(BOOL)useLayerNames {
    self = [super init];
    if (self) {
        self.viewC = viewC;
        self.useLayerNames = useLayerNames;
        self.tileStyleSettings = [MaplyVectorStyleSettings new];
        self.tileStyleSettings.lineScale = [UIScreen mainScreen].scale;
        self.tileStyleSettings.dashPatternScale =  [UIScreen mainScreen].scale;
        self.tileStyleSettings.markerScale = [UIScreen mainScreen].scale;
        symbolizerId = 0;
        self.symbolizers = [NSMutableDictionary dictionary];
    }
    return self;
}


- (void)loadSldFile:(NSString *__nonnull)filePath {
    // Let's find it
    NSString *fullPath = nil;
    if ([[NSFileManager defaultManager] fileExistsAtPath:filePath])
        fullPath = filePath;
    if (!fullPath)
    {
        NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        fullPath = [NSString stringWithFormat:@"%@/%@",docDir,filePath];
        if (![[NSFileManager defaultManager] fileExistsAtPath:fullPath])
            fullPath = nil;
    }
    if (!fullPath)
        fullPath = [[NSBundle mainBundle] pathForResource:filePath ofType:@"sld"];
    
    return [self loadSldData:[[NSData alloc] initWithContentsOfFile:fullPath]];
}



- (void)loadSldData:(NSData *__nonnull)sldData {
    NSError *error;
    DDXMLDocument *doc = [[DDXMLDocument alloc] initWithData:sldData options:0 error:&error];
    if (error) {
        NSLog(@"Error encountered; parsing of SLD halted.");
        NSLog(@"%@", error);
        return;
    }
    
    DDXMLElement *sldNode = [doc rootElement];
    if (![[sldNode name] isEqualToString:@"StyledLayerDescriptor"]) {
        NSLog(@"Error getting unique StyledLayerDescriptor node.");
        return;
    }
    
    _namedLayers = [NSMutableDictionary dictionary];
    
    NSArray *namedLayerNodes = [sldNode elementsForName:@"NamedLayer"];
    if (namedLayerNodes) {
        for (DDXMLElement *namedLayerNode in namedLayerNodes) {
            SLDNamedLayer *sldNamedLayer = [self loadNamedLayerNode:namedLayerNode];
            if (sldNamedLayer)
                _namedLayers[sldNamedLayer.name] = sldNamedLayer;
        }
    }
}

- (DDXMLNode *)getSingleNodeForNode:(DDXMLNode *)node xpath:(NSString *)xpath error:(NSError **)error {
    NSArray *nodes = [node nodesForXPath:xpath error:error];
    if (nodes && nodes.count == 1)
        return nodes[0];
    return nil;
}


- (SLDNamedLayer *)loadNamedLayerNode:(DDXMLElement *)namedLayerNode {
    
    SLDNamedLayer *sldNamedLayer = [[SLDNamedLayer alloc] init];
    
    NSError *error;
    DDXMLNode *nameNode = [self getSingleNodeForNode:namedLayerNode xpath:@"se:Name" error:&error];
    if (!nameNode) {
        NSLog(@"Error: NamedLayer is missing Name element");
        if (error)
            NSLog(@"%@", error);
        return nil;
    }
    
    sldNamedLayer.name = [nameNode stringValue];
    NSLog(@"layer %@ %@", [nameNode name], [nameNode stringValue]);
    
    
    NSMutableArray *userStyles = [NSMutableArray array];
    NSArray *userStyleNodes = [namedLayerNode elementsForName:@"UserStyle"];
    if (userStyleNodes) {
        for (DDXMLElement *userStyleNode in userStyleNodes) {
            SLDUserStyle *sldUserStyle = [self loadUserStyleNode:userStyleNode];
            if (sldUserStyle)
                [userStyles addObject:sldUserStyle];
        }
    }
    sldNamedLayer.userStyles = userStyles;
    return sldNamedLayer;
}

- (SLDUserStyle *)loadUserStyleNode:(DDXMLElement *)userStyleNode {
    NSError *error;
    NSLog(@"loadUserStyleNode");
    SLDUserStyle *sldUserStyle = [[SLDUserStyle alloc] init];
    DDXMLNode *nameNode = [self getSingleNodeForNode:userStyleNode xpath:@"se:Name" error:&error];
    if (nameNode)
        sldUserStyle.name = [nameNode stringValue];
    
    NSMutableArray *featureTypeStyles = [NSMutableArray array];
    NSArray *featureTypeStyleNodes = [userStyleNode elementsForName:@"FeatureTypeStyle"];
    if (featureTypeStyleNodes) {
        for (DDXMLElement *featureTypeStyleNode in featureTypeStyleNodes) {
            SLDFeatureTypeStyle *featureTypeStyle = [self loadFeatureTypeStyleNode:featureTypeStyleNode];
            if (featureTypeStyle)
                [featureTypeStyles addObject:featureTypeStyle];
        }
    }
    sldUserStyle.featureTypeStyles = featureTypeStyles;
    
    return sldUserStyle;
}

- (SLDFeatureTypeStyle *)loadFeatureTypeStyleNode:(DDXMLElement *)featureTypeStyleNode {
    NSError *error;
    NSLog(@"loadFeatureTypeStyleNode");
    SLDFeatureTypeStyle *featureTypeStyle = [[SLDFeatureTypeStyle alloc] init];

    NSMutableArray *rules = [NSMutableArray array];
    NSArray *ruleNodes = [featureTypeStyleNode elementsForName:@"Rule"];
    if (ruleNodes) {
        for (DDXMLElement *ruleNode in ruleNodes) {
            SLDRule *rule = [self loadRuleNode:ruleNode];
            if (rule)
                [rules addObject:rule];
        }
    }
    featureTypeStyle.rules = rules;
    
    return featureTypeStyle;
}

- (SLDRule *)loadRuleNode:(DDXMLElement *)ruleNode {
    NSError *error;
    NSLog(@"loadRuleNode");
    SLDRule *rule = [[SLDRule alloc] init];
    
    
    NSMutableArray *filters = [NSMutableArray array];
    NSArray *filterNodes = [ruleNode elementsForName:@"Filter"];
    if (filterNodes) {
        for (DDXMLElement *filterNode in filterNodes) {
            SLDFilter *filter = [self loadFilterNode:filterNode];
            if (filter)
                [filters addObject:filter];
        }
    }
    rule.filters = filters;
    
    NSMutableArray *elseFilters = [NSMutableArray array];
    NSArray *elseFilterNodes = [ruleNode elementsForName:@"ElseFilter"];
    if (elseFilterNodes) {
        for (DDXMLElement *filterNode in elseFilterNodes) {
            SLDFilter *filter = [self loadFilterNode:filterNode];
            if (filter)
                [elseFilters addObject:filter];
        }
    }
    rule.elseFilters = elseFilters;
    
    NSNumberFormatter *nf = [[NSNumberFormatter alloc] init];
    
    DDXMLNode *minScaleNode = [self getSingleNodeForNode:ruleNode xpath:@"MinScaleDenominator" error:&error];
    if (minScaleNode)
        rule.minScaleDenominator = [nf numberFromString:[minScaleNode stringValue]];
    DDXMLNode *maxScaleNode = [self getSingleNodeForNode:ruleNode xpath:@"MaxScaleDenominator" error:&error];
    if (maxScaleNode)
        rule.maxScaleDenominator = [nf numberFromString:[maxScaleNode stringValue]];

    [self loadSymbolizersForRule:rule andRuleNode:ruleNode];
    
    return rule;
}

- (void)loadSymbolizersForRule:(SLDRule *)rule andRuleNode:(DDXMLElement *)ruleNode {
    NSError *error;
    rule.symbolizers = [NSMutableArray array];
    
    for (DDXMLNode *child in [ruleNode children]) {
        NSString *name = [child name];
        NSArray <MaplyVectorTileStyle *> *symbolizers = [SLDSymbolizer maplyVectorTileStyleWithElement:child tileStyleSettings:self.tileStyleSettings viewC:self.viewC];
        
        if (symbolizers) {
                for (MaplyVectorTileStyle * symbolizer in symbolizers) {
                symbolizer.uuid = @(symbolizerId);
                self.symbolizers[@(symbolizerId)] = symbolizer;
                symbolizerId += 1;
                [rule.symbolizers addObject:symbolizer];
            }
        }
        
        

    }
}


- (SLDFilter *)loadFilterNode:(DDXMLElement *)filterNode {
    
    NSLog(@"loadFilterNode");
    SLDFilter *filter = [[SLDFilter alloc] init];
    
    for (DDXMLNode *child in [filterNode children]) {
        SLDOperator *operator = [SLDOperator operatorForNode:child];
        if (operator) {
            filter.operator = operator;
            break;
        }
        else
            NSLog(@"unmatched %@", [child name]);
    }

    return filter;
}




#pragma mark - MaplyVectorStyleDelegate

- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(MaplyBaseViewController *__nonnull)viewC {
    
    if (self.useLayerNames) {
        SLDNamedLayer *namedLayer = _namedLayers[layer];
        if (!namedLayer)
            return nil;
        return [self stylesForFeatureWithAttributes:attributes onTile:tileID inNamedLayer:namedLayer viewC:viewC];
        
    } else {
        for (SLDNamedLayer *namedLayer in [_namedLayers allValues]) {
            NSArray *styles = [self stylesForFeatureWithAttributes:attributes onTile:tileID inNamedLayer:namedLayer viewC:viewC];
            if (styles)
                return styles;
        }
    }
    
    return nil;
}

- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                        inNamedLayer:(SLDNamedLayer *__nonnull)namedLayer
                                               viewC:(MaplyBaseViewController *__nonnull)viewC {
    
    NSMutableArray *styles = [NSMutableArray array];
    bool matched;
    for (SLDUserStyle *userStyle in namedLayer.userStyles) {
        for (SLDFeatureTypeStyle *featureTypeStyle in userStyle.featureTypeStyles) {
            for (SLDRule *rule in featureTypeStyle.rules) {
                matched = false;
                for (SLDFilter *filter in rule.filters) {
                    if ([filter.operator.predicate evaluateWithObject:attributes]) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    for (SLDFilter *filter in rule.elseFilters) {
                        if ([filter.operator.predicate evaluateWithObject:attributes]) {
                            matched = true;
                            break;
                        }
                    }
                }
                if (matched) {
                    [styles addObjectsFromArray:rule.symbolizers];
                }
            }
        }
    }
    
    return styles;
}



- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID {
    return YES;
}

- (nullable NSObject<MaplyVectorStyle> *)styleForUUID:(NSString *__nonnull)uuid viewC:(MaplyBaseViewController *__nonnull)viewC {
    return self.symbolizers[uuid];
}

@end
