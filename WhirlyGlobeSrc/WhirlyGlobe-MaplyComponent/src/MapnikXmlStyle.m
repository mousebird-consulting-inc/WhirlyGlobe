/*
 *  MapnikXmlStyle.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2014 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "MapnikXmlStyle.h"

#import "MaplyRemoteTileSource.h"
#import "MaplyVectorLineStyle.h"
#import "MaplyVectorMarkerStyle.h"
#import "MaplyVectorPolygonStyle.h"
#import "MaplyVectorTextStyle.h"
#import "MapnikStyle.h"
#import "MapnikStyleRule.h"

@interface MapnikXmlStyle() {
  //temporary storage during parsing
  NSString *currentString;
  MapnikStyle *currentStyle;
  MapnikStyleRule *currentRule;
  MaplyVectorTileStyle *currentSymbolizer;
  NSInteger symbolizerId;
  NSString *currentName;
  CFAbsoluteTime startTime;
}

@property (nonatomic, strong) MaplyRemoteTileInfo *tileSource;

@property (nonatomic, strong) NSMutableDictionary *parameters;
@property (nonatomic, strong) NSMutableDictionary *styles;
@property (nonatomic, strong) NSMutableDictionary *layers;
@property (nonatomic, strong) NSMutableDictionary *symbolizers;

@property (nonatomic, assign) BOOL parsing;
@property (nonatomic, assign) BOOL success;

@property (nonatomic, weak) MaplyBaseViewController *viewC;
@property (nonatomic, strong) MaplyVectorTileStyleSettings *tileStyleSettings;

@end

@implementation MapnikXmlStyle

static NSString *RULE_ELEMENT = @"Rule";
static NSString *STYLE_ELEMENT = @"Style";
static NSString *LAYER_ELEMENT = @"Layer";
static NSString *STYLENAME_ELEMENT = @"StyleName";
static NSString *FILTER_ELEMENT = @"Filter";
static NSString *LINESYMBOLIZER_ELEMENT = @"LineSymbolizer";
static NSString *POLYGONSYMBOLIZER_ELEMENT = @"PolygonSymbolizer";
static NSString *TEXTSYMBOLIZER_ELEMENT = @"TextSymbolizer";
static NSString *MARKERSSYMBOLIZER_ELEMENT = @"MarkersSymbolizer";
static NSString *MINSCALEDENOMINATOR_ELEMENT = @"MinScaleDenominator";
static NSString *MAXSCALEDENOMINATOR_ELEMENT = @"MaxScaleDenominator";
static NSString *PARAMETER_ELEMENT = @"Parameter";

static NSString *NAME_ATTRIBUTE = @"name";
static NSString *OPACITY_ATTRIBUTE = @"opacity";
static NSString *FILTERMODE_ATTRIBUTE = @"filter-mode";


- (instancetype)initForTileSource:(MaplyRemoteTileInfo *)tileSource
                            viewC:(MaplyBaseViewController *)viewC {
  self = [super init];
  if(self) {
    self.viewC = viewC;
    self.tileStyleSettings = [MaplyVectorTileStyleSettings new];
    self.tileStyleSettings.lineScale = [UIScreen mainScreen].scale;
    
    self.styles = [NSMutableDictionary new];
    self.layers = [NSMutableDictionary new];
    self.parameters = [NSMutableDictionary new];
    self.symbolizers = [NSMutableDictionary new];
    symbolizerId = 0;
  }
  return self;
}


- (void)loadXmlFile:(NSString*)filePath {
  startTime = CFAbsoluteTimeGetCurrent();
  NSData *docData = [[NSData alloc] initWithContentsOfFile:filePath];
  NSXMLParser *parser = [[NSXMLParser alloc] initWithData:docData];
  docData = nil;
  parser.delegate = self;
  self.parsing = YES;
  [parser parse];
}


#pragma mark - VectorStyleDelegate
- (NSArray*)stylesForFeature:(MaplyVectorObject*)feature
                  attributes:(NSDictionary*)attributes
                      onTile:(MaplyTileID)tileID
                     inLayer:(NSString*)layer {
  NSMutableArray *symbolizers = [NSMutableArray new];
  NSArray *styles = self.layers[layer];

  for(MapnikStyle *style in styles) {
    for(MapnikStyleRule *rule in style.rules) {
      if(tileID.level <= rule.maxZoom && (tileID.level >= rule.minZoom ||
                                          (tileID.level == self.tileSource.maxZoom && rule.minZoom >= self.tileSource.maxZoom))) {
        //some rules dont take effect until after max zoom, so we need to apply them at maxZoom
        if([rule.filterPredicate evaluateWithObject:attributes]) {
          [symbolizers addObjectsFromArray:rule.symbolizers];
          if(style.filterModeFirst) {
            //filter mode first means we stop applying rules after the first match
            //https://github.com/mapnik/mapnik/issues/706
            break;
          }
        }
      }
    }
  }
  
  return symbolizers;
}


- (BOOL)layerShouldDisplay:(NSString*)layer {
  return self.layers[layer] != nil;
}


- (MaplyVectorTileStyle*)styleForUUID:(NSString *)uuid {
  return self.symbolizers[uuid];
}


#pragma mark - NSXMLDelegate
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName
    attributes:(NSDictionary *)attributeDict {

  if([elementName isEqualToString:LINESYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:MARKERSSYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:POLYGONSYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:TEXTSYMBOLIZER_ELEMENT]) {
    NSMutableDictionary *mutableAttributes = [NSMutableDictionary dictionaryWithDictionary:attributeDict];
    if(currentRule.minScaleDenominator != 0) {
      mutableAttributes[@"minscaledenom"] = @(currentRule.minScaleDenominator);
    }
    if(currentRule.maxScaleDenomitator != 0) {
      mutableAttributes[@"maxscaledenom"] = @(currentRule.maxScaleDenomitator);
    }
    attributeDict = mutableAttributes;
  }
  
  if([elementName isEqualToString:STYLE_ELEMENT]) {
    if(attributeDict[NAME_ATTRIBUTE]) {
      currentStyle = [MapnikStyle new];
      currentStyle.name = attributeDict[NAME_ATTRIBUTE];
      if([attributeDict[FILTERMODE_ATTRIBUTE] isEqualToString:@"first"]) {
        currentStyle.filterModeFirst = YES;
      }
      if(attributeDict[OPACITY_ATTRIBUTE]) {
        currentStyle.opacity = [attributeDict[OPACITY_ATTRIBUTE] floatValue];
      }
      self.styles[attributeDict[NAME_ATTRIBUTE]] = currentStyle;
    }
  } else if([elementName isEqualToString:RULE_ELEMENT]) {
    currentRule = [MapnikStyleRule new];
  } else if([elementName isEqualToString:LAYER_ELEMENT]) {
    currentName = attributeDict[NAME_ATTRIBUTE];
  } else if([elementName isEqualToString:LINESYMBOLIZER_ELEMENT]) {
    MaplyVectorTileStyle *style = [[MaplyVectorTileStyleLine alloc] initWithStyleEntry:@{@"substyles":@[attributeDict]}
                                                                              settings:self.tileStyleSettings
                                                                                 viewC:self.viewC];
    style.uuid = [self getSymbolizerUUID];
    self.symbolizers[style.uuid] = style;
    [currentRule.symbolizers addObject:style];
  } else if([elementName isEqualToString:POLYGONSYMBOLIZER_ELEMENT]) {
    MaplyVectorTileStyle *style = [[MaplyVectorTileStylePolygon alloc] initWithStyleEntry:@{@"substyles":@[attributeDict]}
                                                                                 settings:self.tileStyleSettings
                                                                                    viewC:self.viewC];
    style.uuid = [self getSymbolizerUUID];
    self.symbolizers[style.uuid] = style;
    [currentRule.symbolizers addObject:style];
  } else if([elementName isEqualToString:TEXTSYMBOLIZER_ELEMENT]) {
    MaplyVectorTileStyle *style = [[MaplyVectorTileStyleText alloc] initWithStyleEntry:@{@"substyles":@[attributeDict]}
                                                                              settings:self.tileStyleSettings
                                                                                 viewC:self.viewC];
    style.uuid = [self getSymbolizerUUID];
    currentSymbolizer = style;
    self.symbolizers[style.uuid] = style;
    [currentRule.symbolizers addObject:style];
  } else if([elementName isEqualToString:MARKERSSYMBOLIZER_ELEMENT]) {
    MaplyVectorTileStyle *style = [[MaplyVectorTileStyleMarker alloc] initWithStyleEntry:@{@"substyles":@[attributeDict]}
                                                                                settings:self.tileStyleSettings
                                                                                   viewC:self.viewC];
    style.uuid = [self getSymbolizerUUID];
    self.symbolizers[style.uuid] = style;
    [currentRule.symbolizers addObject:style];
  } else if([elementName isEqualToString:PARAMETER_ELEMENT]) {
    currentName = attributeDict[NAME_ATTRIBUTE];
  }
  currentString = nil;
}


- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string{
  if(currentString) {
    currentString = [currentString stringByAppendingString:string];
  } else {
    currentString = string;
  }
}


- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName {
  if([elementName isEqualToString:STYLE_ELEMENT]) {
    //Discard styles with no rules
    if(!currentStyle.rules.count) {
      [self.styles removeObjectForKey:currentStyle.name];
    }
    currentStyle = nil;
  } else if([elementName isEqualToString:RULE_ELEMENT]) {
    if(!currentRule.filterPredicate) {
      //rules with no filter always match
      currentRule.filterPredicate = [NSPredicate predicateWithValue:YES];
    }
    if(currentRule.maxZoom == 0) {
      currentRule.maxZoom = 30;
    }
    if(currentRule.symbolizers.count) { //only commit rules with symbolizers
      [currentStyle addRule:currentRule];
    }
    
    currentRule = nil;
  } else if([elementName isEqualToString:LAYER_ELEMENT]) {
    currentName = nil;
  } else if([elementName isEqualToString:STYLENAME_ELEMENT]) {
    if(currentString) {
      if(self.styles[currentString]) {
        NSMutableArray *layerStyles = self.layers[currentName];
        if(!layerStyles) {
          layerStyles = [NSMutableArray array];
          self.layers[currentName] = layerStyles;
        }
        [layerStyles addObject:self.styles[currentString]];
      }
    }
  } else if([elementName isEqualToString:FILTER_ELEMENT]) {
    [currentRule setFilter:currentString];
  } else if([elementName isEqualToString:TEXTSYMBOLIZER_ELEMENT]) {
    ((MaplyVectorTileStyleText*)currentSymbolizer).textField = currentString;
  } else if([elementName isEqualToString:MAXSCALEDENOMINATOR_ELEMENT]) {
    currentRule.maxScaleDenomitator = [currentString intValue];
  } else if([elementName isEqualToString:MINSCALEDENOMINATOR_ELEMENT]) {
    currentRule.minScaleDenominator = [currentString intValue];
  } else if([elementName isEqualToString:PARAMETER_ELEMENT]) {
    if(currentString && currentName) {
      self.parameters[currentName] = currentString;
    }
    currentName = nil;
  }
  
  currentString = nil;
  currentSymbolizer = nil;
}


- (void)parserDidEndDocument:(NSXMLParser *)parser {
  //NSLog(@"Parse time:%f, %d styles", CFAbsoluteTimeGetCurrent() - startTime, self.styles.count);
  self.success = YES;
  [self cleanup];
}


- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError{
  NSLog(@"Parse Error Occured %@:%@", parseError, parseError.description);
  self.success = NO;
  [self cleanup];
}


- (void)parser:(NSXMLParser *)parser validationErrorOccurred:(NSError *)validError{
  NSLog(@"Validation Error Occured %@:%@", validError, validError.description);
  self.parsing = NO;
  [self cleanup];
}

#pragma mark - parsing helpers
- (void)cleanup {
  self.parsing = NO;
  currentName = nil;
  currentRule = nil;
  currentStyle = nil;
  currentString = nil;
}


- (NSString*)getSymbolizerUUID {
  return [NSString stringWithFormat:@"%d", symbolizerId++];
}

@end
