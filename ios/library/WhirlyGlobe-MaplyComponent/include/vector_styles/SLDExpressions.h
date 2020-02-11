//
//  SLDExpressions.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2019 mousebird consulting.
//

#import <Foundation/Foundation.h>
@class DDXMLNode;
@class DDXMLElement;

/** @brief Base class for elements derived from the ogc:expression abstract element.
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDExpression : NSObject
@property (nonatomic, strong) NSExpression * _Nonnull expression;
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName;
+ (SLDExpression * _Nullable)expressionForNode:(DDXMLNode * _Nonnull )node;
@end


/** @brief Class corresponding to the ogc:PropertyName element
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDPropertyNameExpression : SLDExpression
@property (nonatomic, strong) NSString * _Nonnull propertyName;
- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;
@end

/** @brief Class corresponding to the ogc:Literal element
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDLiteralExpression : SLDExpression
@property (nonatomic, strong) id _Nonnull literal;
- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;
@end

/** @brief Class corresponding to the ogc:BinaryOperatorType elements
 @see http://schemas.opengis.net/filter/1.1.0/expr.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/expr.xsd for SLD v1.0.0
 */
@interface SLDBinaryOperatorExpression : SLDExpression

@property (nonatomic, strong) NSString * _Nonnull elementName;

@property (nonatomic, strong) SLDExpression * _Nonnull leftExpression;
@property (nonatomic, strong) SLDExpression * _Nonnull rightExpression;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;
@end

