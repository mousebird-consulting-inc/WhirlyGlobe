//
//  SLDStyleSet.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 Ranen Ghosh. All rights reserved.
//

#import "SLDStyleSet.h"



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






@implementation SLDExpression
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return NO;
}
@end




@implementation SLDLiteralExpression

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.literal = [element stringValue];
        self.expression = [NSExpression expressionForConstantValue:self.literal];
        NSLog(@"SLDLiteralExpression %@", self.literal);
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"ogc:Literal"];
}

@end

@implementation SLDPropertyNameExpression

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.propertyName = [element stringValue];
        self.expression = [NSExpression expressionForKeyPath:[NSString stringWithFormat:@"attributes.%@", self.propertyName]];
        NSLog(@"SLDPropertyNameExpression %@", self.propertyName);
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"ogc:PropertyName"];
}

@end



@implementation SLDOperator
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return NO;
}
@end

@implementation SLDBinaryComparisonOperator

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [element name];
        DDXMLNode *matchCaseNode = [element attributeForName:@"matchCase"];
        if (matchCaseNode)
            self.matchCase = ([[matchCaseNode stringValue] isEqualToString:@"true"]);
        else
            self.matchCase = YES;

        NSMutableArray<SLDExpression *> *expressions = [NSMutableArray array];
        for (DDXMLNode *child in [element children]) {
            NSString *childName = [child name];
            if ([SLDPropertyNameExpression matchesElementNamed:childName])
                [expressions addObject:[[SLDPropertyNameExpression alloc] initWithElement:(DDXMLElement *)child]];
            else if ([SLDLiteralExpression matchesElementNamed:childName])
                [expressions addObject:[[SLDLiteralExpression alloc] initWithElement:(DDXMLElement *)child]];
        }
        if (expressions.count != 2)
            return nil;
        self.leftExpression = expressions[0];
        self.rightExpression = expressions[1];
        
        NSPredicateOperatorType opType;
        if ([self.elementName isEqualToString:@"ogc:PropertyIsEqualTo"])
            opType = NSEqualToPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"ogc:PropertyIsNotEqualTo"])
            opType = NSNotEqualToPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"ogc:PropertyIsLessThan"])
            opType = NSLessThanPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"ogc:PropertyIsGreaterThan"])
            opType = NSGreaterThanPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"ogc:PropertyIsLessThanOrEqualTo"])
            opType = NSLessThanOrEqualToPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"ogc:PropertyIsGreaterThanOrEqualTo"])
            opType = NSGreaterThanOrEqualToPredicateOperatorType;
        else
            return nil;
        
        NSComparisonPredicateOptions predOptions = 0;
        if (!self.matchCase)
            predOptions = NSCaseInsensitivePredicateOption;
        
        self.predicate = [NSComparisonPredicate predicateWithLeftExpression:self.leftExpression.expression rightExpression:self.rightExpression.expression modifier:0 type:opType options:predOptions];
        
        
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    static NSSet *set;
    if (!set)
        set = [NSSet setWithArray:@[@"ogc:PropertyIsEqualTo", @"ogc:PropertyIsNotEqualTo", @"ogc:PropertyIsLessThan", @"ogc:PropertyIsGreaterThan", @"ogc:PropertyIsLessThanOrEqualTo", @"ogc:PropertyIsGreaterThanOrEqualTo"]];
    return [set containsObject:elementName];
}

@end


@implementation SLDLogicalOperator


- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [element name];
        NSMutableArray<SLDOperator *> *subOperators = [NSMutableArray array];
        
        for (DDXMLNode *child in [element children]) {
            NSString *childName = [child name];
            if ([SLDLogicalOperator matchesElementNamed:childName])
                [subOperators addObject:[[SLDLogicalOperator alloc] initWithElement:(DDXMLElement *)child]];
            else if ([SLDBinaryComparisonOperator matchesElementNamed:childName])
                [subOperators addObject:[[SLDBinaryComparisonOperator alloc] initWithElement:(DDXMLElement *)child]];
        }
        self.subOperators = subOperators;
        
        NSMutableArray <NSPredicate *> *subPredicates = [NSMutableArray array];
        for (SLDOperator *subOperator in self.subOperators) {
            [subPredicates addObject:subOperator.predicate];
        }
        
        if ([self.elementName isEqualToString:@"ogc:And"])
            self.predicate = [NSCompoundPredicate andPredicateWithSubpredicates:subPredicates];
        else if ([self.elementName isEqualToString:@"ogc:Or"])
            self.predicate = [NSCompoundPredicate orPredicateWithSubpredicates:subPredicates];
        else
            return nil;
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return ([elementName isEqualToString:@"ogc:And"] || [elementName isEqualToString:@"ogc:Or"]);
}

@end










@implementation SLDStyleSet {
    NSMutableArray *_namedLayers;
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
    
    
    _namedLayers = [NSMutableArray array];
    
    NSArray *namedLayerNodes = [sldNode elementsForName:@"NamedLayer"];
    if (namedLayerNodes) {
        for (DDXMLElement *namedLayerNode in namedLayerNodes) {
            SLDNamedLayer *sldNamedLayer = [self loadNamedLayerNode:namedLayerNode];
            if (sldNamedLayer)
                [_namedLayers addObject:sldNamedLayer];
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

    NSMutableArray *rules;
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
    
    for (DDXMLNode *child in [ruleNode children]) {
        NSString *name = [child name];
        if ([name isEqualToString:@"LineSymbolizer"]) {
        } else if ([name isEqualToString:@"PolygonSymbolizer"]) {
        } else if ([name isEqualToString:@"PointSymbolizer"]) {
        } else if ([name isEqualToString:@"TextSymbolizer"]) {
        } else if ([name isEqualToString:@"RasterSymbolizer"]) {
        }
    }
    
    return rule;
}




- (SLDFilter *)loadFilterNode:(DDXMLElement *)filterNode {
    
    NSError *error;
    NSLog(@"loadFilterNode");
    SLDFilter *filter = [[SLDFilter alloc] init];
    
    for (DDXMLNode *child in [filterNode children]) {
        NSString *childName = [child name];
        if ([SLDLogicalOperator matchesElementNamed:childName]) {
            filter.operator = [[SLDLogicalOperator alloc] initWithElement:(DDXMLElement *)child];
            break;
        }
        else if ([SLDBinaryComparisonOperator matchesElementNamed:childName]) {
            filter.operator = [[SLDBinaryComparisonOperator alloc] initWithElement:(DDXMLElement *)child];
            break;
        }
        else
            NSLog(@"unmatched %@", childName);
    }

    return filter;
}













- (void)generateStyles {
    
}

#pragma mark - MaplyVectorStyleDelegate

- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(MaplyBaseViewController *__nonnull)viewC {
    return nil;
}

- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID {
    return NO;
}

- (nullable NSObject<MaplyVectorStyle> *)styleForUUID:(NSString *__nonnull)uiid viewC:(MaplyBaseViewController *__nonnull)viewC {
    return nil;
}

@end
