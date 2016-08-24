//
//  SLDStyleSet.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MapboxVectorTiles.h"
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

@property (nonatomic, strong) NSMutableArray * _Nullable symbolizers;

@end



@class SLDOperator;
@class SLDExpression;

@interface SLDFilter : NSObject

@property (nonatomic, strong) SLDOperator * _Nonnull operator;

@end



@interface SLDStyleSet : NSObject <MaplyVectorStyleDelegate>

@property (nonatomic, assign) BOOL useLayerNames;
@property (nonatomic, weak, nullable) MaplyBaseViewController *viewC;
@property (nonatomic, strong, nullable) MaplyVectorStyleSettings *tileStyleSettings;

- (id _Nullable)initWithViewC:(MaplyBaseViewController * _Nonnull)viewC useLayerNames:(BOOL)useLayerNames;

- (void)loadSldFile:(NSString *__nonnull)filePath;
- (void)loadSldData:(NSData *__nonnull)sldData;

@end












