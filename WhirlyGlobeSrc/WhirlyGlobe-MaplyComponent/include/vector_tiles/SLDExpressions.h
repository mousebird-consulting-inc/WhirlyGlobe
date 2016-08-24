//
//  SLDExpressions.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DDXML.h"


@interface SLDExpression : NSObject
@property (nonatomic, strong) NSExpression * _Nonnull expression;
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName;
+ (SLDExpression * _Nullable)expressionForNode:(DDXMLNode * _Nonnull )node;
@end



@interface SLDPropertyNameExpression : SLDExpression
@property (nonatomic, strong) NSString * _Nonnull propertyName;
- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;
@end



@interface SLDLiteralExpression : SLDExpression
@property (nonatomic, strong) NSString * _Nonnull literal;
- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;
@end

@interface SLDBinaryOperatorExpression : SLDExpression

@property (nonatomic, strong) NSString * _Nonnull elementName;

@property (nonatomic, strong) SLDExpression * _Nonnull leftExpression;
@property (nonatomic, strong) SLDExpression * _Nonnull rightExpression;

- (_Nullable id)initWithElement:(DDXMLElement * _Nonnull)element;
@end

