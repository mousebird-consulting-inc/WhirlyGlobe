//
//  SLDOperators.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "SLDOperators.h"


@implementation SLDOperator
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return NO;
}

+ (SLDOperator *)operatorForNode:(DDXMLNode *)node {
    NSString *name = [node localName];
    if ([SLDLogicalOperator matchesElementNamed:name])
        return [[SLDLogicalOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDBinaryComparisonOperator matchesElementNamed:name])
        return [[SLDBinaryComparisonOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDNotOperator matchesElementNamed:name])
        return [[SLDNotOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDIsNullOperator matchesElementNamed:name])
        return [[SLDIsNullOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDIsLikeOperator matchesElementNamed:name])
        return [[SLDIsLikeOperator alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDIsBetweenOperator matchesElementNamed:name])
        return [[SLDIsBetweenOperator alloc] initWithElement:(DDXMLElement *)node];
    return nil;
}
@end

@implementation SLDBinaryComparisonOperator

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [element localName];
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
    static NSSet *set;
    if (!set)
        set = [NSSet setWithArray:@[@"PropertyIsEqualTo", @"PropertyIsNotEqualTo", @"PropertyIsLessThan", @"PropertyIsGreaterThan", @"PropertyIsLessThanOrEqualTo", @"PropertyIsGreaterThanOrEqualTo"]];
    return [set containsObject:elementName];
}

@end


@implementation SLDIsNullOperator

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        
        NSMutableArray<SLDExpression *> *expressions = [NSMutableArray array];
        for (DDXMLNode *child in [element children]) {
            SLDExpression *expression = [SLDExpression expressionForNode:child];
            if (expression)
                [expressions addObject:expression];
        }
        if (expressions.count != 1)
            return nil;
        self.subExpression = expressions[0];
        
        self.predicate = [NSComparisonPredicate predicateWithLeftExpression:self.subExpression.expression rightExpression:[NSExpression expressionForConstantValue:[NSNull null]] modifier:0 type:NSEqualToPredicateOperatorType options:0];

    }
    return self;
}

+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"PropertyIsNull"];
}

@end

@implementation SLDIsLikeOperator


- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        NSMutableArray<SLDExpression *> *expressions = [NSMutableArray array];
        for (DDXMLNode *child in [element children]) {
            SLDExpression *expression = [SLDExpression expressionForNode:child];
            if (expression)
                [expressions addObject:expression];
            if ([expression isKindOfClass:[SLDPropertyNameExpression class]])
                self.propertyExpression = (SLDPropertyNameExpression *)expression;
            else if ([expression isKindOfClass:[SLDLiteralExpression class]])
                self.literalExpression = (SLDLiteralExpression *)expression;
        }
        if (expressions.count != 2)
            return nil;
        if (!self.propertyExpression || !self.literalExpression)
            return nil;
        
        DDXMLNode *matchCaseNode = [element attributeForName:@"matchCase"];
        if (matchCaseNode)
            self.matchCase = ([[matchCaseNode stringValue] isEqualToString:@"true"]);
        else
            self.matchCase = YES;
        
        DDXMLNode *wildCardNode = [element attributeForName:@"wildCard"];
        DDXMLNode *singleCharNode = [element attributeForName:@"singleChar"];
        DDXMLNode *escapeCharNode = [element attributeForName:@"escapeChar"];
        if (!wildCardNode || !singleCharNode || !escapeCharNode)
            return nil;
        
        self.wildCardStr = [wildCardNode stringValue];
        self.singleCharStr = [singleCharNode stringValue];
        self.escapeCharStr = [escapeCharNode stringValue];
        
        if (self.wildCardStr.length != 1 || self.singleCharStr.length != 1 || self.escapeCharStr.length != 1) {
            NSLog(@"SLDIsLikeOperator; attributes invalid.");
            return nil;
        }
        
        unichar oldWildCardChar = [self.wildCardStr characterAtIndex:0];
        unichar oldSingleChar = [self.singleCharStr characterAtIndex:0];
        unichar oldEscapeChar = [self.escapeCharStr characterAtIndex:0];

        unichar newWildCardChar = [@"*" characterAtIndex:0];
        unichar newSingleChar = [@"?" characterAtIndex:0];
        unichar newEscapeChar = [@"\\" characterAtIndex:0];
        
        unichar oldChars[] = {oldWildCardChar, oldSingleChar, oldEscapeChar};
        unichar newChars[] = {newWildCardChar, newSingleChar, newEscapeChar};
        unichar allChars[] = {oldWildCardChar, oldSingleChar, oldEscapeChar, newWildCardChar, newSingleChar, newEscapeChar};

        NSCharacterSet *oldSet = [NSCharacterSet characterSetWithCharactersInString:[[NSString alloc] initWithCharacters:oldChars length:3]];
        NSCharacterSet *newSet = [NSCharacterSet characterSetWithCharactersInString:[[NSString alloc] initWithCharacters:newChars length:3]];
        NSCharacterSet *allSet = [NSCharacterSet characterSetWithCharactersInString:[[NSString alloc] initWithCharacters:allChars length:6]];
        
        NSString *likeStr = self.literalExpression.literal;
        
        NSMutableString *newLikeStr = [NSMutableString string];
        
        NSUInteger oldLen = [likeStr length];
        unichar buffer[oldLen+1];
        [likeStr getCharacters:buffer range:NSMakeRange(0, oldLen)];

        unichar bufc, bufc2;
        
        for (int i=0; i<oldLen; i++) {
            
            bufc = buffer[i];
            
            if (![allSet characterIsMember:bufc]) {
                [newLikeStr appendString:[[NSString alloc] initWithCharacters:&bufc length:1]];
            } else if (bufc == oldSingleChar) {
                [newLikeStr appendString:[[NSString alloc] initWithCharacters:&newSingleChar length:1]];
            } else if (bufc == oldWildCardChar) {
                [newLikeStr appendString:[[NSString alloc] initWithCharacters:&newWildCardChar length:1]];
            } else if (bufc == oldEscapeChar) {
                if (i == oldLen-1) {
                    NSLog(@"SLDIsLikeOperator; literal pattern invalid.");
                    return nil;
                }
                bufc2 = buffer[i+1];
                if (![oldSet characterIsMember:bufc2]) {
                    NSLog(@"SLDIsLikeOperator; literal pattern invalid.");
                    return nil;
                }
                
                if ([newSet characterIsMember:bufc2])
                    [newLikeStr appendString:@"\\\\"];
                if (bufc2 == newEscapeChar)
                    [newLikeStr appendString:@"\\\\"];
                else
                    [newLikeStr appendString:[[NSString alloc] initWithCharacters:&bufc2 length:1]];
                i++;
            } else { // bufc must be in newSet
                [newLikeStr appendString:@"\\\\"];
                if (bufc == newEscapeChar)
                    [newLikeStr appendString:@"\\\\"];
                else
                    [newLikeStr appendString:[[NSString alloc] initWithCharacters:&bufc length:1]];
            }
        }
        
        self.literalExpression.literal = newLikeStr;
        self.literalExpression.expression = [NSExpression expressionForConstantValue:newLikeStr];

        NSComparisonPredicateOptions predOptions = 0;
        if (!self.matchCase)
            predOptions = NSCaseInsensitivePredicateOption;
        
        self.predicate = [NSComparisonPredicate predicateWithLeftExpression:(NSExpression *)self.propertyExpression rightExpression:(NSExpression *)self.literalExpression modifier:0 type:NSLikePredicateOperatorType options:predOptions];
        
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"PropertyIsLike"];
}

@end


@implementation SLDIsBetweenOperator

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        
        for (DDXMLNode *child in [element children]) {
            NSString *childName = [child localName];
            SLDExpression *expression = [SLDExpression expressionForNode:child];
            if (!self.subExpression && expression)
                self.subExpression = expression;
            else if (!self.lowerBoundaryExpression && [childName isEqualToString:@"LowerBoundary"]) {
                for (DDXMLNode *child2 in [child children]) {
                    expression = [SLDExpression expressionForNode:child2];
                    if (!self.lowerBoundaryExpression && expression)
                        self.lowerBoundaryExpression = expression;
                    else {
                        NSLog(@"SLDIsBetweenOperator: Unexpected structure.");
                        return nil;
                    }
                }
            } else if (!self.upperBoundaryExpression && [childName isEqualToString:@"UpperBoundary"]) {
                for (DDXMLNode *child2 in [child children]) {
                    expression = [SLDExpression expressionForNode:child2];
                    if (!self.upperBoundaryExpression && expression)
                        self.upperBoundaryExpression = expression;
                    else {
                        NSLog(@"SLDIsBetweenOperator: Unexpected structure.");
                        return nil;
                    }
                }
            } else {
                NSLog(@"SLDIsBetweenOperator: Unexpected structure.");
                return nil;
            }
        }
        if (!self.subExpression || !self.lowerBoundaryExpression || !self.upperBoundaryExpression) {
            NSLog(@"SLDIsBetweenOperator: Missing required element(s).");
            return nil;
        }
        
        NSExpression *boundsExpression = [NSExpression expressionForAggregate:@[self.lowerBoundaryExpression, self.upperBoundaryExpression]];
        
        self.predicate = [NSComparisonPredicate predicateWithLeftExpression:(NSExpression *)self.subExpression rightExpression:boundsExpression modifier:0 type:NSBetweenPredicateOperatorType options:0];

        
    }
    return self;
}



+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"PropertyIsBetween"];
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
    return [elementName isEqualToString:@"Not"];
}

@end


@implementation SLDLogicalOperator


- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [element localName];
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
    return ([elementName isEqualToString:@"And"] || [elementName isEqualToString:@"Or"]);
}

@end

