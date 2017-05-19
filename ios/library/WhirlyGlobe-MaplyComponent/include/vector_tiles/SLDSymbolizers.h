//
//  SLDSymbolizers.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DDXML.h"
#import "MaplyVectorTileStyle.h"

/** @brief Base class for Symbolizer elements
 @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDSymbolizer : NSObject

/** 
    Returns whether this class can parse the symbolizer corresponding to the provided element name.
 
    Each subclass matches different symbolizer elements.
 */
+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName;

/** 
    Produces MaplyVectorTileStyle objects for an SLD Symbolizer element
 
    Parses the XML subtree and returns an array of corresponding MaplyVectorTileStyle objects.
 
    @param element The XML element corresponding to a symbolizer
 
    @param tileStyleSettings The base MaplyVectorStyleSettings settings to apply.
 
    @param viewC The map or globe view controller.
 
    @param minScaleDenom If non-null, the minimum map scale at which to apply any constructed symbolizer.
 
    @param maxScaleDenom If non-null, the maximum map scale at which to apply any constructed symbolizer.
 
    @param relativeDrawPriority The z-order relative to other vector features.
 
    @param baseURL The base URL from which external resources (e.g. images) will be located.
 
    @return An array of MaplyVectorTileStyle objects corresponding to the particular XML element.
 @see MaplyVectorTileStyle
 @see MaplyVectorStyleSettings
 */
+ (NSArray<MaplyVectorTileStyle *> * _Nullable) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings * _Nonnull)tileStyleSettings viewC:(MaplyBaseViewController * _Nonnull)viewC minScaleDenom:(NSNumber * _Nonnull)minScaleDenom maxScaleDenom:(NSNumber * _Nonnull)maxScaleDenom relativeDrawPriority:(int)relativeDrawPriority crossSymbolizerParams:(NSMutableDictionary * _Nonnull)crossSymbolizerParams baseURL:(NSURL * _Nonnull)baseURL;
@end

/** @brief Class corresponding to the LineSymbolizer element
 @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDLineSymbolizer : SLDSymbolizer
@end

/** @brief Class corresponding to the PolygonSymbolizer element
 @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDPolygonSymbolizer : SLDSymbolizer
@end

/** @brief Class corresponding to the PointSymbolizer element
 @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDPointSymbolizer : SLDSymbolizer
@end

/** @brief Class corresponding to the TextSymbolizer element
 @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
@interface SLDTextSymbolizer : SLDSymbolizer
@end



