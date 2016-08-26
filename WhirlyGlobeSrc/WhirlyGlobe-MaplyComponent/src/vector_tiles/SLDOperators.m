//
//  SLDOperators.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "SLDOperators.h"


@implementation SLDOperator
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return NO;
}

+ (SLDOperator *)operatorForNode:(DDXMLNode *)node {
    NSString *name = [node name];
    if ([SLDLogicalOperator matchesElementNamed:name])
        return [[SLDLogicalOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDBinaryComparisonOperator matchesElementNamed:name])
        return [[SLDBinaryComparisonOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDNotOperator matchesElementNamed:name])
        return [[SLDNotOperator alloc] initWithElement:(DDXMLElement *)node];
    return nil;
}
@end

@implementation SLDBinaryComparisonOperator

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [[[element name] componentsSeparatedByString:@":"] lastObject];
        DDXMLNode *matchCaseNode = [element attributeForName:@"matchCase"];
        if (matchCaseNode)
            self.matchCase = ([[matchCaseNode stringValue] isEqualToString:@"true"]);
        else
            self.matchCase = YES;
        
        NSMutableArray<SLDExpression *> *expressions = [NSMutableArray array];
        for (DDXMLNode *child in [element children]) {
            SLDExpression *expression = [SLDExpression expressionForNode:child];
            if (expression)
                [expressions addObject:expression];
        }
        if (expressions.count != 2)
            return nil;
        self.leftExpression = expressions[0];
        self.rightExpression = expressions[1];
        
        NSPredicateOperatorType opType;
        if ([self.elementName isEqualToString:@"PropertyIsEqualTo"])
            opType = NSEqualToPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"PropertyIsNotEqualTo"])
            opType = NSNotEqualToPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"PropertyIsLessThan"])
            opType = NSLessThanPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"PropertyIsGreaterThan"])
            opType = NSGreaterThanPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"PropertyIsLessThanOrEqualTo"])
            opType = NSLessThanOrEqualToPredicateOperatorType;
        else if ([self.elementName isEqualToString:@"PropertyIsGreaterThanOrEqualTo"])
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
    NSString *stripped = [[elementName componentsSeparatedByString:@":"] lastObject];
    static NSSet *set;
    if (!set)
        set = [NSSet setWithArray:@[@"PropertyIsEqualTo", @"PropertyIsNotEqualTo", @"PropertyIsLessThan", @"PropertyIsGreaterThan", @"PropertyIsLessThanOrEqualTo", @"PropertyIsGreaterThanOrEqualTo"]];
    return [set containsObject:stripped];
}

@end


@implementation SLDNotOperator

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        NSMutableArray<SLDOperator *> *subOperators = [NSMutableArray array];
        
        for (DDXMLNode *child in [element children]) {
            SLDOperator *operator = [SLDOperator operatorForNode:child];
            if (operator)
                [subOperators addObject:operator];
        }
        
        if (subOperators.count != 1)
            return nil;
        
        self.subOperator = subOperators[0];
        
        self.predicate = [NSCompoundPredicate notPredicateWithSubpredicate:self.subOperator.predicate];
        
    }
    return self;
}

+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    NSString *stripped = [[elementName componentsSeparatedByString:@":"] lastObject];
    return [stripped isEqualToString:@"Not"];
}

@end


@implementation SLDLogicalOperator


- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [[[element name] componentsSeparatedByString:@":"] lastObject];
        NSMutableArray<SLDOperator *> *subOperators = [NSMutableArray array];
        
        for (DDXMLNode *child in [element children]) {
            SLDOperator *operator = [SLDOperator operatorForNode:child];
            if (operator)
                [subOperators addObject:operator];
        }
        self.subOperators = subOperators;
        
        NSMutableArray <NSPredicate *> *subPredicates = [NSMutableArray array];
        for (SLDOperator *subOperator in self.subOperators) {
            [subPredicates addObject:subOperator.predicate];
        }
        
        if ([self.elementName isEqualToString:@"And"])
            self.predicate = [NSCompoundPredicate andPredicateWithSubpredicates:subPredicates];
        else if ([self.elementName isEqualToString:@"Or"])
            self.predicate = [NSCompoundPredicate orPredicateWithSubpredicates:subPredicates];
        else
            return nil;
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    NSString *stripped = [[elementName componentsSeparatedByString:@":"] lastObject];
    return ([stripped isEqualToString:@"And"] || [stripped isEqualToString:@"Or"]);
}

@end

