//
//  SLDStyleSet.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 Ranen Ghosh. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MapboxVectorTiles.h"
#import "DDXMLElementAdditions.h"
#import "DDXML.h"


@interface SLDNamedLayer : NSObject

@property (nonatomic, strong) NSString * _Nullable name;
@property (nonatomic, strong) NSArray * _Nullable userStyles;

@end



@interface SLDUserStyle : NSObject

@property (nonatomic, strong) NSString * _Nullable name;
@property (nonatomic, strong) NSArray * _Nullable featureTypeStyles;

@end



@interface SLDFeatureTypeStyle : NSObject

@property (nonatomic, strong) NSArray * _Nullable rules;

@end



@interface SLDRule : NSObject

@property (nonatomic, strong) NSArray * _Nullable filters;
@property (nonatomic, strong) NSArray * _Nullable elseFilters;

@property (nonatomic, strong) NSNumber * _Nullable minScaleDenominator;
@property (nonatomic, strong) NSNumber * _Nullable maxScaleDenominator;

@property (nonatomic, strong) NSArray * _Nullable symbolizers;

@end



@class SLDOperator;

@interface SLDFilter : NSObject

@property (nonatomic, strong) SLDOperator *operator;

@end



@interface SLDStyleSet : NSObject <MaplyVectorStyleDelegate>

- (void)loadSldFile:(NSString *__nonnull)filePath;
- (void)loadSldData:(NSData *__nonnull)sldData;
- (void)generateStyles;

@end







@interface SLDExpression : NSObject
@property (nonatomic, strong) NSExpression * _Nonnull expression;
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName;

@end



@interface SLDPropertyNameExpression : SLDExpression
@property (nonatomic, strong) NSString *propertyName;
- (_Nullable id)initWithElement:(DDXMLElement *)element;
@end



@interface SLDLiteralExpression : SLDExpression
@property (nonatomic, strong) NSString *literal;
- (_Nullable id)initWithElement:(DDXMLElement *)element;
@end




@interface SLDOperator : NSObject
@property (nonatomic, strong) NSPredicate * _Nonnull predicate;
+ (BOOL)matchesElementNamed:(NSString * _Nonnull)elementName;
@end



@interface SLDBinaryComparisonOperator : SLDOperator

@property (nonatomic, assign) BOOL matchCase;
@property (nonatomic, strong) NSString *elementName;

@property (nonatomic, strong) SLDExpression *leftExpression;
@property (nonatomic, strong) SLDExpression *rightExpression;


- (_Nullable id)initWithElement:(DDXMLElement *)element;

@end



@interface SLDLogicalOperator : SLDOperator

@property (nonatomic, strong) NSString *elementName;
@property (nonatomic, strong) NSArray<SLDOperator *> *subOperators;


- (_Nullable id)initWithElement:(DDXMLElement *)element;

@end













