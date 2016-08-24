//
//  SLDSymbolizers.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "SLDSymbolizers.h"
#import "SLDWellKnownMarkers.h"

@implementation SLDSymbolizer

+ (NSArray<MaplyVectorTileStyle *> *) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSString *name = [element name];
    if ([SLDLineSymbolizer matchesSymbolizerNamed:name])
        return [SLDLineSymbolizer maplyVectorTileStyleWithElement:element tileStyleSettings:tileStyleSettings viewC:viewC];
    
    
    return nil;
}

+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName {
    return NO;
}

+ (DDXMLNode *)getSingleNodeForNode:(DDXMLNode *)node xpath:(NSString *)xpath error:(NSError **)error {
    NSArray *nodes = [node nodesForXPath:xpath error:error];
    if (nodes && nodes.count == 1)
        return nodes[0];
    return nil;
}


+ (NSString *)stringForLiteralInNode:(DDXMLNode *)node {
    for (DDXMLNode *child in [node children]) {
        if ([[child name] isEqualToString:@"ogc:Literal"])
            return [child stringValue];
    }
    return nil;
}

+ (NSString *)stringForParameterValueTypeNode:(DDXMLNode *)node {
    if ([[node name] isEqualToString:@"ogc:PropertyName"])
        return [NSString stringWithFormat:@"[%@]", [node stringValue]];
    else if ([[node name] isEqualToString:@"ogc:Literal"])
        return [node stringValue];
    return nil;
}


+ (NSDictionary *)dictForSvgParametersInElement:(DDXMLElement *)element {
    NSMutableDictionary *params = [NSMutableDictionary dictionary];
    for (DDXMLElement *paramNode in [element elementsForName:@"se:SvgParameter"]) {
        DDXMLNode *nameNode = [paramNode attributeForName:@"name"];
        if (!nameNode)
            continue;
        NSString *paramName = [nameNode stringValue];
        params[paramName] = [paramNode stringValue];
    }
    return params;
}

@end

@implementation SLDLineSymbolizer

+ (NSArray<MaplyVectorTileStyle *> *) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSError *error;
    DDXMLElement *strokeNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:element xpath:@"se:Stroke" error:&error];
    
    if (!strokeNode)
        return nil;
    
    MaplyVectorTileStyle *s = [SLDLineSymbolizer maplyVectorTileStyleFromStrokeNode:strokeNode tileStyleSettings:tileStyleSettings viewC:viewC];
    
    if (!s)
        return nil;
    
    return @[s];
}

+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName {
    return [symbolizerName isEqualToString:@"se:LineSymbolizer"];
}

+ (MaplyVectorTileStyle *)maplyVectorTileStyleFromStrokeNode:(DDXMLElement *)strokeNode tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSDictionary *strokeParams = [SLDSymbolizer dictForSvgParametersInElement:strokeNode];
    MaplyVectorTileStyle *s = [MaplyVectorTileStyle styleFromStyleEntry:@{@"type": @"LineSymbolizer", @"substyles": @[strokeParams]}
                                                               settings:tileStyleSettings
                                                                  viewC:viewC];
    return s;

}

@end

@implementation SLDPolygonSymbolizer

+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName {
    return [symbolizerName isEqualToString:@"se:PolygonSymbolizer"];
}

+ (NSArray<MaplyVectorTileStyle *> *) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSError *error;
    DDXMLElement *fillNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:element xpath:@"se:Fill" error:&error];
    DDXMLElement *strokeNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:element xpath:@"se:Stroke" error:&error];
    
   
    NSMutableArray <MaplyVectorTileStyle *> *styles = [NSMutableArray array];
    if (strokeNode)
        [styles addObject:[SLDLineSymbolizer maplyVectorTileStyleFromStrokeNode:strokeNode tileStyleSettings:tileStyleSettings viewC:viewC]];
    
    if (fillNode)
        [styles addObject:[SLDPolygonSymbolizer maplyVectorTileStyleFromFillNode:fillNode tileStyleSettings:tileStyleSettings viewC:viewC]];

    if (styles.count > 0)
        return styles;

    return nil;
}

+ (MaplyVectorTileStyle *)maplyVectorTileStyleFromFillNode:(DDXMLElement *)fillNode tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSDictionary *fillParams = [SLDSymbolizer dictForSvgParametersInElement:fillNode];
    MaplyVectorTileStyle *s = [MaplyVectorTileStyle styleFromStyleEntry:@{@"type": @"PolygonSymbolizer", @"substyles": @[fillParams]}
                                                               settings:tileStyleSettings
                                                                  viewC:viewC];
    return s;
}


@end

@implementation SLDPointSymbolizer

+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName {
    return [symbolizerName isEqualToString:@"se:PointSymbolizer"];
}

+ (NSArray<MaplyVectorTileStyle *> *) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    return nil;
}

+ (MaplyVectorTileStyle *)maplyVectorTileStyleFromPointSymbolizerNode:(DDXMLElement *)pointSymbolizerNode tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSError *error;
    NSMutableDictionary *pointParams = [NSMutableDictionary dictionary];

    DDXMLElement *graphicNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:pointSymbolizerNode xpath:@"se:Graphic" error:&error];
    
    if (!graphicNode)
        return nil;
    
    for (DDXMLNode *child in [graphicNode children]) {
        if ([[child name] isEqualToString:@"se:ExternalGraphic"]) {
            
            NSLog(@"Skipping MarkerSymbolizer with ExternalGraphic.");
            return nil;
            
        } else if ([[child name] isEqualToString:@"se:Mark"]) {
            
            // Marker shape type
            DDXMLElement *wellKnownNameNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:child xpath:@"se:WellKnownName" error:&error];
            
            if (!wellKnownNameNode) {
                NSLog(@"Skipping MarkerSymbolizer with Mark but without WellKnownName.");
                return nil;
            }
            
            NSString *wellKnownName = [wellKnownNameNode stringValue];
            
            UIImage *image;
            if ([wellKnownName isEqualToString:@"square"])
                image = [SLDWellKnownMarkers squareImage];
            else if ([wellKnownName isEqualToString:@"circle"])
                image = [SLDWellKnownMarkers circleImage];
            else if ([wellKnownName isEqualToString:@"triangle"])
                image = [SLDWellKnownMarkers triangleImage];
            else if ([wellKnownName isEqualToString:@"star"])
                image = [SLDWellKnownMarkers starImage];
            else if ([wellKnownName isEqualToString:@"cross"])
                image = [SLDWellKnownMarkers crossImage];
            else if ([wellKnownName isEqualToString:@"x"])
                image = [SLDWellKnownMarkers xImage];
            else {
                NSLog(@"Skipping MarkerSymbolizer with unrecognized WellKnownName for Mark.");
                return nil;
            }
            pointParams[@"image"] = image;
            
            // Fill color
            DDXMLElement *fillNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:child xpath:@"se:Fill" error:&error];
            NSDictionary *fillParams = [SLDSymbolizer dictForSvgParametersInElement:fillNode];
            
            if (fillParams[@"fill"])
                pointParams[@"fill"] = fillParams[@"fill"];
            
            // Stroke color
            DDXMLElement *strokeNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:child xpath:@"se:Stroke" error:&error];
            NSDictionary *strokeParams = [SLDSymbolizer dictForSvgParametersInElement:strokeNode];
            
            if (strokeParams[@"stroke"])
                pointParams[@"stroke"] = strokeParams[@"stroke"];

            
        } else if ([[child name] isEqualToString:@"se:Size"]) {
            // Marker size
            NSString *size = [SLDSymbolizer stringForLiteralInNode:child];
            if (size) {
                pointParams[@"width"] = size;
                pointParams[@"height"] = size;
            }
            
        }
    }
    
    MaplyVectorTileStyle *s = [MaplyVectorTileStyle styleFromStyleEntry:@{@"type": @"MarkersSymbolizer", @"substyles": @[pointParams]}
                                                               settings:tileStyleSettings
                                                                  viewC:viewC];
    return s;
}

@end

@implementation SLDTextSymbolizer

+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName {
    return [symbolizerName isEqualToString:@"se:TextSymbolizer"];
}

+ (NSArray<MaplyVectorTileStyle *> *) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    return nil;
}

+ (MaplyVectorTileStyle *)maplyVectorTileStyleFromTextSymbolizerNode:(DDXMLElement *)textSymbolizerNode tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC {
    
    NSError *error;
    NSMutableDictionary *labelParams = [NSMutableDictionary dictionary];
    
    // Label text
    DDXMLElement *labelNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:textSymbolizerNode xpath:@"se:Label" error:&error];
    if (labelNode) {
        for (DDXMLNode *child in [labelNode children]) {
            NSString *value = [SLDSymbolizer stringForParameterValueTypeNode:child];
            if (value) {
                labelParams[@"value"] = value;
                break;
            }
        }
    }
    
    
    // Ignore specified font for now.
    DDXMLElement *fontNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:textSymbolizerNode xpath:@"se:Font" error:&error];
    

    // Label placement
    DDXMLElement *labelPlacementNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:textSymbolizerNode xpath:@"se:LabelPlacement" error:&error];
    if (labelPlacementNode) {
        DDXMLElement *pointPlacementNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:labelPlacementNode xpath:@"se:PointPlacement" error:&error];
        DDXMLElement *linePlacementNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:labelPlacementNode xpath:@"se:LinePlacement" error:&error];
        
        if (pointPlacementNode)
            labelParams[@"placement"] = @"point";
        else if (linePlacementNode)
            labelParams[@"placement"] = @"line";
    }
    
    
    // Label halo
    DDXMLElement *haloNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:textSymbolizerNode xpath:@"se:Halo" error:&error];
    if (haloNode) {
        DDXMLElement *haloRadiusNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:haloNode xpath:@"se:Radius" error:&error];
        DDXMLElement *haloFillNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:haloNode xpath:@"se:Fill" error:&error];
        
        if (haloRadiusNode) {
            NSString *haloRadius = [SLDSymbolizer stringForLiteralInNode:haloRadiusNode];
            if (haloRadius)
                labelParams[@"halo-radius"] = haloRadius;
        }
        if (haloFillNode) {
            
            NSDictionary *haloFillParams = [SLDSymbolizer dictForSvgParametersInElement:haloFillNode];
            if (haloFillParams[@"fill"])
                labelParams[@"halo-fill"] = haloFillParams[@"fill"];
        }
    }
    
    // Label fill
    DDXMLElement *fillNode = (DDXMLElement *)[SLDSymbolizer getSingleNodeForNode:textSymbolizerNode xpath:@"se:Fill" error:&error];
    if (fillNode) {
        
        NSDictionary *labelFillParams = [SLDSymbolizer dictForSvgParametersInElement:fillNode];
        if (labelFillParams[@"fill"])
            labelParams[@"fill"] = labelFillParams[@"fill"];
        if (labelFillParams[@"fill-opacity"])
            labelParams[@"opacity"] = labelFillParams[@"fill-opacity"];
    }

    
    MaplyVectorTileStyle *s = [MaplyVectorTileStyle styleFromStyleEntry:@{@"type": @"TextSymbolizer", @"substyles": @[labelParams]}
                                                               settings:tileStyleSettings
                                                                  viewC:viewC];
    return s;
}

@end








