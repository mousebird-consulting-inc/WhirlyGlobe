//
//  SLDExpressions.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "SLDExpressions.h"

@implementation SLDExpression
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return NO;
}

+ (SLDExpression *)expressionForNode:(DDXMLNode *)node {
    
    NSString *name = [node localName];
    if ([SLDPropertyNameExpression matchesElementNamed:name])
        return [[SLDPropertyNameExpression alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDLiteralExpression matchesElementNamed:name])
        return [[SLDLiteralExpression alloc] initWithElement:(DDXMLElement *)node];
    else if ([SLDBinaryOperatorExpression matchesElementNamed:name])
        return [[SLDBinaryOperatorExpression alloc] initWithElement:(DDXMLElement *)node];
    return nil;
}
@end




@implementation SLDLiteralExpression

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.literal = [element stringValue];
        self.expression = [NSExpression expressionForConstantValue:self.literal];
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"Literal"];
}

@end

@implementation SLDPropertyNameExpression

- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.propertyName = [element stringValue];
        self.expression = [NSExpression expressionForKeyPath:self.propertyName];
    }
    return self;
}


+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    return [elementName isEqualToString:@"PropertyName"];
}

@end



@implementation SLDBinaryOperatorExpression
- (_Nullable id)initWithElement:(DDXMLElement *)element {
    self = [super init];
    if (self) {
        self.elementName = [element localName];
        
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
        
        NSString *funcExpression;
        
        
        if ([self.elementName isEqualToString:@"Add"])
            funcExpression = @"add:to:";
        else if ([self.elementName isEqualToString:@"Sub"])
            funcExpression = @"from:subtract:";
        else if ([self.elementName isEqualToString:@"Mul"])
            funcExpression = @"multiply:by:";
        else if ([self.elementName isEqualToString:@"Div"])
            funcExpression = @"divide:by:";
        else
            return nil;
        
        NSArray *arguments = @[
                               [NSExpression expressionForFunction:@"castObject:toType:" arguments:@[self.leftExpression, [NSExpression expressionForConstantValue:@"NSNumber"]]],
                               [NSExpression expressionForFunction:@"castObject:toType:" arguments:@[self.rightExpression, [NSExpression expressionForConstantValue:@"NSNumber"]]] ];
        
        self.expression = [NSExpression expressionForFunction:funcExpression arguments:arguments];
        
    }
    return self;
}

+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName {
    static NSSet *set;
    if (!set)
        set = [NSSet setWithArray:@[@"Add", @"Sub", @"Mul", @"Div"]];
    return [set containsObject:elementName];
}

@end

