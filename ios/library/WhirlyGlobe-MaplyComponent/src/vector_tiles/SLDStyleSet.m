//
//  SLDStyleSet.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "SLDStyleSet.h"
#import "SLDExpressions.h"
#import "SLDOperators.h"
#import "SLDSymbolizers.h"
#import "MaplyVectorTileStyle.h"
#import "DDXML.h"


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
    NSURL *_baseURL;
    int _relativeDrawPriority;
    
}


/** 
    Constructs a SLDStyleSet object.
    
    After constructing the SLDStyleSet object, call loadSldURL: or loadSldData:baseURL: to parse the desired SLD document tree and create the corresponding symbolizers.
    
    @param viewC The map or globe view controller.
    
    @param useLayerNames Whether to use names of NamedLayer elements as a criteria in matching styles.
 
    @param relativeDrawPriority The z-order relative to other vector features. This will be incremented internally for each style rule, so if you have multiple SLDStyleSets, leave some space between the relativeDrawPriority of each.
 */
- (id)initWithViewC:(MaplyBaseViewController *)viewC useLayerNames:(BOOL)useLayerNames relativeDrawPriority:(int)relativeDrawPriority {
    self = [super init];
    if (self) {
        self.viewC = viewC;
        self.useLayerNames = useLayerNames;
        self.tileStyleSettings = [MaplyVectorStyleSettings new];
        self.tileStyleSettings.lineScale = [UIScreen mainScreen].scale;
        self.tileStyleSettings.dashPatternScale =  [UIScreen mainScreen].scale;
        self.tileStyleSettings.markerScale = [UIScreen mainScreen].scale;
        self.tileStyleSettings.useWideVectors = true;
        symbolizerId = 0;
        _relativeDrawPriority = relativeDrawPriority;
        self.symbolizers = [NSMutableDictionary dictionary];
    }
    return self;
}

/** 
    Load SLD document from the specified URL
    
    Currently only file URLs are supported.
    
    @param url The URL from which to load the SLD document.
 */
- (void)loadSldURL:(NSURL *__nullable)url {
    if (!url)
        url = [[NSBundle mainBundle] URLForResource:@"default" withExtension:@"sld"];
    NSURL *baseURL = [url URLByDeletingLastPathComponent];
    if ([url isFileURL]) {
        if (![[NSFileManager defaultManager] fileExistsAtPath:[url path]])
            url = [[NSBundle mainBundle] URLForResource:@"default" withExtension:@"sld"];
        [self loadSldData:[NSData dataWithContentsOfURL:url] baseURL:baseURL];
    } else {
        // network URL
        NSLog(@"Network SLD URLs not yet implemented.");
    }
}

/** 
    Load SLD document from data
    
    Load an SLD document from data.
    
    @param sldData The SLD document
    
    @param baseURL The base URL from which any external references (e.g. images) will be located.
 */
- (void)loadSldData:(NSData *__nonnull)sldData baseURL:(NSURL *__nonnull)baseURL {
    
    _baseURL = baseURL;
 
    NSError *error;
    DDXMLDocument *doc = [[DDXMLDocument alloc] initWithData:sldData options:0 error:&error];
    if (error) {
        NSLog(@"Error encountered; parsing of SLD halted.");
        NSLog(@"%@", error);
        return;
    }
    
    DDXMLElement *sldNode = [doc rootElement];
    if (![[sldNode localName] isEqualToString:@"StyledLayerDescriptor"]) {
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

/** 
    Gets a single node for the provided element name.
 */
- (DDXMLNode *)getSingleChildNodeForNode:(DDXMLNode *)node childName:(NSString *)childName {
    
    if (node.kind != DDXMLElementKind)
        return nil;
    DDXMLElement *element = (DDXMLElement *)node;
    for (DDXMLNode *child in [element children]) {
        if ([[child localName] isEqualToString:childName])
            return child;
    }
    return nil;
}

- (NSString *)stringForLiteralInNode:(DDXMLNode *)node {
    for (DDXMLNode *child in [node children]) {
        if ([[child localName] isEqualToString:@"Literal"])
            return [child stringValue];
    }
    for (DDXMLNode *child in [node children]) {
        if (child.kind == DDXMLTextKind)
            return [child stringValue];
    }
    return nil;
}

/** 
    Load the NamedLayer xml element into a SLDNamedLayer object.
    
    @param  namedLayerNode The DDXMLElement corresponding to the NamedLayer element in the document tree.
 */
- (SLDNamedLayer *)loadNamedLayerNode:(DDXMLElement *)namedLayerNode {
    
    SLDNamedLayer *sldNamedLayer = [[SLDNamedLayer alloc] init];
    
    NSError *error;
    
    DDXMLNode *nameNode = [self getSingleChildNodeForNode:namedLayerNode childName:@"Name"];
    
    if (!nameNode) {
        NSLog(@"Error: NamedLayer is missing Name element");
        if (error)
            NSLog(@"%@", error);
        return nil;
    }
    
    sldNamedLayer.name = [nameNode stringValue];
    
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

/** 
    Load the UserStyle xml element into a SLDUserStyle object.
 
    @param  userStyleNode The DDXMLElement corresponding to the UserStyle element in the document tree.
 */
- (SLDUserStyle *)loadUserStyleNode:(DDXMLElement *)userStyleNode {
    SLDUserStyle *sldUserStyle = [[SLDUserStyle alloc] init];

    DDXMLNode *nameNode = [self getSingleChildNodeForNode:userStyleNode childName:@"Name"];
    
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

/** 
    Load the FeatureTypeStyle xml element into a SLDFeatureTypeStyle object.
 
    @param  featureTypeStyleNode The DDXMLElement corresponding to the FeatureTypeStyle element in the document tree.
 */
- (SLDFeatureTypeStyle *)loadFeatureTypeStyleNode:(DDXMLElement *)featureTypeStyleNode {
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

/** 
    Load the Rule xml element into a SLDRule object.
 
    @param  ruleNode The DDXMLElement corresponding to the Rule element in the document tree.
 */
- (SLDRule *)loadRuleNode:(DDXMLElement *)ruleNode {
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
    
    DDXMLNode *minScaleNode = [self getSingleChildNodeForNode:ruleNode childName:@"MinScaleDenominator"];
    if (minScaleNode)
        rule.minScaleDenominator = [nf numberFromString:[minScaleNode stringValue]];

    DDXMLNode *maxScaleNode = [self getSingleChildNodeForNode:ruleNode childName:@"MaxScaleDenominator"];
    if (maxScaleNode)
        rule.maxScaleDenominator = [nf numberFromString:[maxScaleNode stringValue]];
    
    NSArray *vendorOptionNodes = [ruleNode elementsForName:@"VendorOption"];
    if (vendorOptionNodes) {
        for (DDXMLElement *vendorOptionNode in vendorOptionNodes) {
            DDXMLNode *optionNameNode = [vendorOptionNode attributeForName:@"name"];
            NSString *optionName;
            if (optionNameNode)
                optionName = [optionNameNode stringValue];
            if (optionName && [optionName isEqualToString:@"relativeDrawPriority"]) {
                NSString *sVal = [self stringForLiteralInNode:vendorOptionNode];
                if (sVal) {
                    rule.relativeDrawPriority = @([sVal intValue]);
                    break;
                }
            }
            
        }
    }

    [self loadSymbolizersForRule:rule andRuleNode:ruleNode];
    
    return rule;
}

/** 
    Loops through the Rule's symbolizers in the document tree and generates the corresponding objects.
 
    @param rule The SLDRule object to own the generated symbolizer objects.
 
    @param ruleNode The DDXMLElement corresponding to the Rule element in the document tree.
 */
- (void)loadSymbolizersForRule:(SLDRule *)rule andRuleNode:(DDXMLElement *)ruleNode {
    rule.symbolizers = [NSMutableArray array];
    
    int relativeDrawPriority = _relativeDrawPriority;
    if (rule.relativeDrawPriority)
        relativeDrawPriority += rule.relativeDrawPriority.intValue;

    // Allows certain params from symbolizers to be accessed by subsequent symbolizers within a rule.
    NSMutableDictionary *crossSymbolizerParams = [NSMutableDictionary dictionary];
    
    for (DDXMLNode *child in [ruleNode children]) {
        NSArray <MaplyVectorTileStyle *> *symbolizers = [SLDSymbolizer maplyVectorTileStyleWithElement:(DDXMLElement *)child tileStyleSettings:self.tileStyleSettings viewC:self.viewC minScaleDenom:rule.minScaleDenominator maxScaleDenom:rule.maxScaleDenominator relativeDrawPriority:_relativeDrawPriority crossSymbolizerParams:crossSymbolizerParams baseURL:_baseURL];
        
        if (symbolizers) {
            _relativeDrawPriority += 1;
            relativeDrawPriority += 1;
            for (MaplyVectorTileStyle * symbolizer in symbolizers) {
                symbolizer.uuid = @(symbolizerId);
                self.symbolizers[@(symbolizerId)] = symbolizer;
                symbolizerId += 1;
                [rule.symbolizers addObject:symbolizer];
            }
        }
        
        

    }
}

/** 
    Load the Filter xml element into a SLDFilter object.
 
    @param  filterNode The DDXMLElement corresponding to the Filter element in the document tree.
 */
- (SLDFilter *)loadFilterNode:(DDXMLElement *)filterNode {
    
    SLDFilter *filter = [[SLDFilter alloc] init];
    
    for (DDXMLNode *child in [filterNode children]) {
        SLDOperator *operator = [SLDOperator operatorForNode:child];
        if (operator) {
            filter.sldOperator = operator;
            break;
        }
        else
            NSLog(@"SLDFilter; Unmatched operator: %@", [child localName]);
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
        // If we're not using layer names for matching, check all layers for
        // matching styles.
        for (SLDNamedLayer *namedLayer in [_namedLayers allValues]) {
            NSArray *styles = [self stylesForFeatureWithAttributes:attributes onTile:tileID inNamedLayer:namedLayer viewC:viewC];
            if (styles && styles.count > 0)
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
                if (rule.filters.count == 0 && rule.elseFilters.count == 0)
                    matched = true;
                for (SLDFilter *filter in rule.filters) {
                    if ([filter.sldOperator.predicate evaluateWithObject:attributes]) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    for (SLDFilter *filter in rule.elseFilters) {
                        if ([filter.sldOperator.predicate evaluateWithObject:attributes]) {
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
