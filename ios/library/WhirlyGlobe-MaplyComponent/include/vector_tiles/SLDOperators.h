//
//  SLDOperators.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DDXML.h"
#import "SLDExpressions.h"

/** @brief Base class for elements of ogc:comparisonOps or ogc:logicOps.
 
    Elements of ogc:spatialOps are not supported.
 @see http://schemas.opengis.net/filter/1.1.0/filter.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/filter.xsd for SLD v1.0.0
 */
@interface SLDOperator : NSObject
@property (nonatomic, strong) NSPredicate * _Nonnull predicate;
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName;
+ (SLDOperator * _Nullable)operatorForNode:(DDXMLNode * _Nonnull )node;
@end


/** @brief Class corresponding to the ogc:BinaryComparisonOpType elements
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDBinaryComparisonOperator : SLDOperator

@property (nonatomic, assign) BOOL matchCase;
@property (nonatomic, strong) NSString * _Nonnull elementName;

@property (nonatomic, strong) SLDExpression * _Nonnull leftExpression;
@property (nonatomic, strong) SLDExpression * _Nonnull rightExpression;


- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end


@interface SLDIsNullOperator : SLDOperator

@property (nonatomic, strong) SLDExpression * _Nonnull subExpression;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end


@interface SLDIsLikeOperator : SLDOperator

@property (nonatomic, strong, nullable) NSString *wildCardStr;
@property (nonatomic, strong, nullable) NSString *singleCharStr;
@property (nonatomic, strong, nullable) NSString *escapeCharStr;
@property (nonatomic, assign) BOOL matchCase;
@property (nonatomic, strong) SLDPropertyNameExpression * _Nonnull propertyExpression;
@property (nonatomic, strong) SLDLiteralExpression * _Nonnull literalExpression;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end

@interface SLDIsBetweenOperator : SLDOperator

@property (nonatomic, strong) SLDExpression * _Nonnull subExpression;
@property (nonatomic, strong) SLDExpression * _Nonnull lowerBoundaryExpression;
@property (nonatomic, strong) SLDExpression * _Nonnull upperBoundaryExpression;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end



/** @brief Class corresponding to the ogc:Not element
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDNotOperator : SLDOperator

@property (nonatomic, strong) SLDOperator * _Nonnull subOperator;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end

/** @brief Class corresponding to the ogc:BinaryLogicOpType elements
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDLogicalOperator : SLDOperator

@property (nonatomic, strong) NSString * _Nonnull elementName;
@property (nonatomic, strong) NSArray<SLDOperator *> * _Nonnull subOperators;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;

@end

