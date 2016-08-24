//
//  SLDOperators.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DDXML.h"
#import "SLDExpressions.h"


@interface SLDOperator : NSObject
@property (nonatomic, strong) NSPredicate * _Nonnull predicate;
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName;
+ (SLDOperator * _Nullable)operatorForNode:(DDXMLNode * _Nonnull )node;
@end



@interface SLDBinaryComparisonOperator : SLDOperator

@property (nonatomic, assign) BOOL matchCase;
@property (nonatomic, strong) NSString * _Nonnull elementName;

@property (nonatomic, strong) SLDExpression * _Nonnull leftExpression;
@property (nonatomic, strong) SLDExpression * _Nonnull rightExpression;


- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end


@interface SLDNotOperator : SLDOperator

@property (nonatomic, strong) SLDOperator * _Nonnull subOperator;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end


@interface SLDLogicalOperator : SLDOperator

@property (nonatomic, strong) NSString * _Nonnull elementName;
@property (nonatomic, strong) NSArray<SLDOperator *> * _Nonnull subOperators;


- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end

