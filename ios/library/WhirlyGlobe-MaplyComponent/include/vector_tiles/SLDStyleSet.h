//
//  SLDStyleSet.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MapboxVectorTiles.h"

/** @brief Class corresponding to the sld:NamedLayer element
 @see http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDNamedLayer : NSObject

@property (nonatomic, strong) NSString * _Nullable name;
@property (nonatomic, strong) NSArray * _Nullable userStyles;

@end

/** @brief Class corresponding to the sld:UserStyle element
 @see http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDUserStyle : NSObject

@property (nonatomic, strong) NSString * _Nullable name;
@property (nonatomic, strong) NSArray * _Nullable featureTypeStyles;

@end

/** @brief Class corresponding to the se:FeatureTypeStyle element
 @see http://schemas.opengis.net/se/1.1.0/FeatureStyle.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDFeatureTypeStyle : NSObject

@property (nonatomic, strong) NSArray * _Nullable rules;

@end

/** @brief Class corresponding to the se:Rule element
 @see http://schemas.opengis.net/se/1.1.0/FeatureStyle.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDRule : NSObject

@property (nonatomic, strong) NSArray * _Nullable filters;
@property (nonatomic, strong) NSArray * _Nullable elseFilters;

@property (nonatomic, strong) NSNumber * _Nullable minScaleDenominator;
@property (nonatomic, strong) NSNumber * _Nullable maxScaleDenominator;
@property (nonatomic, strong) NSNumber * _Nullable relativeDrawPriority;

@property (nonatomic, strong) NSMutableArray * _Nullable symbolizers;

@end



@class SLDOperator;
@class SLDExpression;

/** @brief Class corresponding to the ogc:Filter element
 @see http://schemas.opengis.net/filter/1.1.0/filter.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/filter/1.0.0/filter.xsd for SLD v1.0.0
 */
@interface SLDFilter : NSObject

@property (nonatomic, strong) SLDOperator * _Nonnull sldOperator;

@end

/** @brief Class corresponding to the sld:StyledLayerDescriptor element
 
    The sld:StyledLayerDescriptor element is the root element of the Styled Layer Descriptor document.
 
    Implements the MaplyVectorStyleDelegate protocol for matching and applying styles to vector objects.
 @see http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 @see MaplyVectorStyleDelegate
 */
@interface SLDStyleSet : NSObject <MaplyVectorStyleDelegate>

@property (nonatomic, assign) BOOL useLayerNames;
@property (nonatomic, weak, nullable) MaplyBaseViewController *viewC;
@property (nonatomic, strong, nullable) MaplyVectorStyleSettings *tileStyleSettings;

/** 
    Constructs a SLDStyleSet object.
 
    After constructing the SLDStyleSet object, call loadSldURL: or loadSldData:baseURL: to parse the desired SLD document tree and create the corresponding symbolizers.
 
    @param viewC The map or globe view controller.
 
    @param useLayerNames Whether to use names of NamedLayer elements as a criteria in matching styles.
 
    @param relativeDrawPriority The z-order relative to other vector features. This will be incremented internally for each style rule, so if you have multiple SLDStyleSets, leave some space between the relativeDrawPriority of each.
 */
- (id _Nullable)initWithViewC:(MaplyBaseViewController * _Nonnull)viewC useLayerNames:(BOOL)useLayerNames relativeDrawPriority:(int)relativeDrawPriority;

- (void)loadSldURL:(NSURL *__nullable)url;
- (void)loadSldData:(NSData *__nonnull)sldData baseURL:(NSURL *__nonnull)baseURL;

@end












