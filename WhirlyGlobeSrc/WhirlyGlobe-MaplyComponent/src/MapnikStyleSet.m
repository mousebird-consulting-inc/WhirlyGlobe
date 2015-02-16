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

#import "MapnikStyleSet.h"

#import "MaplyRemoteTileSource.h"
#import "MaplyVectorLineStyle.h"
#import "MaplyVectorMarkerStyle.h"
#import "MaplyVectorPolygonStyle.h"
#import "MaplyVectorTextStyle.h"
#import "MapnikStyle.h"
#import "MapnikStyleRule.h"
#import "NSDictionary+StyleRules.h"

@interface MapnikStyleSet() {
  //temporary storage during parsing
  NSString *currentString;
  NSMutableDictionary *currentStyle;
  NSMutableDictionary *currentRule;
  NSMutableDictionary *currentSymbolizer;
  NSMutableDictionary *currentLayer;
  
  NSString *currentName;
  CFAbsoluteTime startTime;
}

@property (nonatomic, strong) NSMutableDictionary *styles;
@property (nonatomic, strong) NSMutableDictionary *layers;
@property (nonatomic, strong) NSMutableDictionary *symbolizers;

@property (nonatomic, assign, readwrite) BOOL parsing;
@property (nonatomic, assign) BOOL success;


@end

@implementation MapnikStyleSet

static NSString *MAP_ELEMENT = @"Map";
static NSString *RULE_ELEMENT = @"Rule";
static NSString *STYLE_ELEMENT = @"Style";
static NSString *LAYER_ELEMENT = @"Layer";
static NSString *STYLENAME_ELEMENT = @"StyleName";
static NSString *FILTER_ELEMENT = @"Filter";
static NSString *LINESYMBOLIZER_ELEMENT = @"LineSymbolizer";
static NSString *POLYGONSYMBOLIZER_ELEMENT = @"PolygonSymbolizer";
static NSString *POLYGONPATTERNSYMBOLIZER_ELEMENT = @"PolygonPatternSymbolizer";
static NSString *TEXTSYMBOLIZER_ELEMENT = @"TextSymbolizer";
static NSString *MARKERSSYMBOLIZER_ELEMENT = @"MarkersSymbolizer";
static NSString *MINSCALEDENOMINATOR_ELEMENT = @"MinScaleDenominator";
static NSString *MAXSCALEDENOMINATOR_ELEMENT = @"MaxScaleDenominator";
static NSString *PARAMETER_ELEMENT = @"Parameter";

static NSString *NAME_ATTRIBUTE = @"name";
static NSString *OPACITY_ATTRIBUTE = @"opacity";
static NSString *OPACITY_ATTRIBUTE_ALT = @"fill-opacity";
static NSString *FILTERMODE_ATTRIBUTE = @"filter-mode";

- (instancetype)init {
  self = [super init];
  if(self) {
    self.tileStyleSettings = [MaplyVectorTileStyleSettings new];
    self.tileStyleSettings.lineScale = [UIScreen mainScreen].scale;
    self.tileStyleSettings.dashPatternScale =  [UIScreen mainScreen].scale;
    self.tileStyleSettings.markerScale = [UIScreen mainScreen].scale;
    self.tileMaxZoom = 14;
    self.alpha = 1.0;
  }
  return self;
}

- (instancetype)initForViewC:(MaplyBaseViewController *)viewC {
  self = [self init];
  if(self) {
    self.viewC = viewC;
  }
  return self;
}

- (void)loadXmlData:(NSData *)docData
{
    startTime = CFAbsoluteTimeGetCurrent();
    self.styleDictionary = [NSMutableDictionary dictionary];
    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:docData];
    docData = nil;
    parser.delegate = self;
    [parser parse];
}

- (void)loadXmlFile:(NSString*)filePath {
  self.parsing = YES;
    
    // Let's find it
    NSString *fullPath = nil;
    if ([[NSFileManager defaultManager] fileExistsAtPath:filePath])
        fullPath = filePath;
    if (!fullPath)
    {
        NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        fullPath = [NSString stringWithFormat:@"%@/%@",docDir,filePath];
        if (![[NSFileManager defaultManager] fileExistsAtPath:fullPath])
            fullPath = nil;
    }
    if (!fullPath)
        fullPath = [[NSBundle mainBundle] pathForResource:filePath ofType:@"xml"];

    return [self loadXmlData:[[NSData alloc] initWithContentsOfFile:fullPath]];
}

- (void)loadJsonData:(NSData *)jsonData
{
    NSError *error = nil;
    self.styleDictionary = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers error:&error];
//    [self generateStyles];
}

- (void)loadJsonFile:(NSString*)filePath {
  if(filePath) {
    NSData *data = [NSData dataWithContentsOfFile:filePath];
    if(data.length) {
      NSError *error;
      NSMutableDictionary *jsonDict = [NSJSONSerialization JSONObjectWithData:data
                                                               options:NSJSONReadingMutableContainers
                                                                 error:&error];
      self.styleDictionary = jsonDict;
        
//        [self generateStyles];
    }
  }
}

- (void)saveAsJSON:(NSString *)filePath
{
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:self.styleDictionary options:0 error:&error];
    if (jsonData)
        [jsonData writeToFile:filePath atomically:NO];
}

- (void)generateStyles {
  self.styles = [NSMutableDictionary dictionary];
  self.layers = [NSMutableDictionary dictionary];
  self.symbolizers = [NSMutableDictionary dictionary];
  
  NSInteger symbolizerId = 0;

  /* Originally we parsed styles in the order they appeared in the file, and set symbolizer
   priority(which set stacking order), based on that, but to match mapnik rendering we need to parse
   them in the order they are listed in layer elements */
  NSMutableArray *stylesToParse = [NSMutableArray array];
  for(NSMutableDictionary *layer in self.styleDictionary.layers) {
    for(NSString *styleName in layer.styles) {
      for(NSMutableDictionary *styleDict in self.styleDictionary.styles) {
        if([styleDict.name isEqualToString:styleName]) {
          [stylesToParse addObject:styleDict];
          break;
        }
      }
    }
  }
  
  for(NSMutableDictionary *styleDict in stylesToParse) {
    MapnikStyle *style = [MapnikStyle new];
    style.name = styleDict.name;
    if([styleDict[@"filter-mode"] isEqualToString:@"first"]) {
      style.filterModeFirst = YES;
    }
    NSArray *rules = styleDict[@"rules"];
    for(NSMutableDictionary *ruleDict in rules) {
      MapnikStyleRule *rule = [[MapnikStyleRule alloc] init];
      if(ruleDict.filter) {
        [rule setFilter:ruleDict.filter];
      } else {
        rule.filterPredicate = [NSPredicate predicateWithValue:YES];
      }
      
      //Rule matching happens based on zoom level, set a zoom from scaleDenominator, or a large/small value that will always match
      if(ruleDict.minScaleDenom) {
        rule.minScaleDenominator = ruleDict.minScaleDenom.integerValue;
      } else {
        rule.maxZoom = 100;
      }
      
      if(ruleDict.maxScaleDenom) {
        rule.maxScaleDenomitator = ruleDict.maxScaleDenom.integerValue;
      } else {
        rule.minZoom = 0;
      }
      
      for(NSDictionary *symbolizerDict in ruleDict[@"symbolizers"]) {
        if(!symbolizerDict[@"type"]) {
          continue;
        }
        symbolizerId++;
        
        NSMutableDictionary *mutableSymbolizerDict = [NSMutableDictionary dictionaryWithDictionary:symbolizerDict];
        //draw priority increments as we go through the rule sets, so objects are stack based on symbolizer order in the file
        mutableSymbolizerDict[@"drawpriority"] = @(symbolizerId + self.drawPriorityOffset);
        if(rule.minZoom >= self.tileMaxZoom) {
          //only set min/max vis when we are at max zoom to make things appear when overzooming
          if(rule.minScaleDenominator != 0) {
            mutableSymbolizerDict[@"minscaledenom"] = @(rule.minScaleDenominator);
          }
          if(rule.maxScaleDenomitator != 0) {
            mutableSymbolizerDict[@"maxscaledenom"] = @(rule.maxScaleDenomitator);
          }
        }
        
        if(symbolizerDict[OPACITY_ATTRIBUTE]) {
          mutableSymbolizerDict[OPACITY_ATTRIBUTE] = @([styleDict[OPACITY_ATTRIBUTE] floatValue] * self.alpha);
        } else {
          mutableSymbolizerDict[OPACITY_ATTRIBUTE] = @(self.alpha);
        }
        
        if([mutableSymbolizerDict[@"type"] isEqualToString:POLYGONSYMBOLIZER_ELEMENT]) {
          if(symbolizerDict[@"fill-opacity"]) {
            mutableSymbolizerDict[@"fill-opacity"] = @([styleDict[@"fill-opacity"] floatValue] * self.alpha);
          } else {
            mutableSymbolizerDict[@"fill-opacity"] = @(self.alpha);
          }
        }
        
        MaplyVectorTileStyle *s = [MaplyVectorTileStyle styleFromStyleEntry:@{@"type": mutableSymbolizerDict[@"type"], @"substyles": @[mutableSymbolizerDict]}
                                                                   settings:self.tileStyleSettings
                                                                      viewC:self.viewC];
        s.uuid = @(symbolizerId);
        if(s) {
          [rule.symbolizers addObject:s];
          self.symbolizers[s.uuid] = s;
        }
      }
      
      if(rule.symbolizers.count) {
        [style.rules addObject:rule];
      }
    }
    
    if(style.rules.count) {
      self.styles[style.name] = style;
    }
  }
  
  for(NSMutableDictionary *layer in self.styleDictionary.layers) {
    NSMutableArray *layerStyles = [NSMutableArray array];
    for(NSString *styleName in layer.styles) {
      MapnikStyle *style = self.styles[styleName];
      if(style) {
        [layerStyles addObject:style];
      }
    }
    
    if(layerStyles.count) {
      self.layers[layer.name] = layerStyles;
    }
  }
  
  NSString *backgroundColorString = self.styleDictionary[@"map"][@"background-color"];
  if(backgroundColorString) {
    self.backgroundColor = [MaplyVectorTiles ParseColor:backgroundColorString];
  }
}


#pragma mark - VectorStyleDelegate
- (NSArray*)stylesForFeatureWithAttributes:(NSDictionary*)attributes
                                    onTile:(MaplyTileID)tileID
                                   inLayer:(NSString*)layer {
  NSMutableArray *symbolizers = [NSMutableArray new];
  NSArray *styles = self.layers[layer];

  for(MapnikStyle *style in styles) {
    for(MapnikStyleRule *rule in style.rules) {
      if(tileID.level <= rule.maxZoom && (tileID.level >= rule.minZoom ||
                                          (tileID.level == _tileMaxZoom && rule.minZoom >= _tileMaxZoom))) {
        //some rules dont take effect until after max zoom, so we need to apply them at maxZoom
        @try {
          if([rule.filterPredicate evaluateWithObject:attributes]) {
            [symbolizers addObjectsFromArray:rule.symbolizers];
            if(style.filterModeFirst) {
              //filter mode first means we stop applying rules after the first match
              //https://github.com/mapnik/mapnik/issues/706
              break;
            }
          }
        }
        @catch (NSException *exception) {
          NSLog(@"Error evaluating rule:%@", rule.filterPredicate);
        }
      }
    }
  }
  
  return symbolizers;
}


- (BOOL)layerShouldDisplay:(NSString*)layer {
  return self.layers[layer] != nil;
}


- (MaplyVectorTileStyle*)styleForUUID:(NSString *)uuid viewC:(MaplyBaseViewController *)viewC {
  return self.symbolizers[uuid];
}


#pragma mark - NSXMLDelegate
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName
    attributes:(NSDictionary *)attributeDict {

  if([elementName isEqualToString:LINESYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:MARKERSSYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:POLYGONSYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:POLYGONPATTERNSYMBOLIZER_ELEMENT] ||
     [elementName isEqualToString:TEXTSYMBOLIZER_ELEMENT]) {
    NSMutableDictionary *mutableAttributes = [NSMutableDictionary dictionaryWithDictionary:attributeDict];
    if([elementName isEqualToString:POLYGONPATTERNSYMBOLIZER_ELEMENT]) {
      mutableAttributes[@"type"] = POLYGONSYMBOLIZER_ELEMENT;
    } else {
      mutableAttributes[@"type"] = elementName;
    }
    [currentRule.symbolizers addObject:mutableAttributes];
    currentSymbolizer = mutableAttributes;
  } else if([elementName isEqualToString:STYLE_ELEMENT]) {
    if(attributeDict[NAME_ATTRIBUTE]) {
      currentStyle = [attributeDict mutableCopy];
    }
  } else if([elementName isEqualToString:RULE_ELEMENT]) {
    currentRule = [NSMutableDictionary dictionary];
  } else if([elementName isEqualToString:LAYER_ELEMENT]) {
    currentLayer = [attributeDict mutableCopy];
    [self.styleDictionary.layers addObject:currentLayer];
  } else if([elementName isEqualToString:PARAMETER_ELEMENT]) {
    currentName = attributeDict[NAME_ATTRIBUTE];
  } else if([elementName isEqualToString:MAP_ELEMENT]) {
    self.styleDictionary[@"map"] = attributeDict;
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
    if(currentStyle.rules.count) {
      [self.styleDictionary.styles addObject:currentStyle];
    }
    currentStyle = nil;
  } else if([elementName isEqualToString:RULE_ELEMENT]) {
    if(currentRule.symbolizers.count) { //only commit rules with symbolizers
      [currentStyle.rules addObject:currentRule];
    }
    
    currentRule = nil;
  } else if([elementName isEqualToString:LAYER_ELEMENT]) {
    currentName = nil;
    currentLayer = nil;
  } else if([elementName isEqualToString:STYLENAME_ELEMENT]) {
    if(currentString) {
      [currentLayer.styles addObject:currentString];
    }
  } else if([elementName isEqualToString:FILTER_ELEMENT]) {
    [currentRule setFilter:currentString];
  } else if([elementName isEqualToString:TEXTSYMBOLIZER_ELEMENT]) {
    currentSymbolizer[@"value"] = currentString;
  } else if([elementName isEqualToString:MAXSCALEDENOMINATOR_ELEMENT]) {
    currentRule.maxScaleDenom = @([currentString intValue]);
  } else if([elementName isEqualToString:MINSCALEDENOMINATOR_ELEMENT]) {
    currentRule.minScaleDenom = @([currentString intValue]);
  } else if([elementName isEqualToString:PARAMETER_ELEMENT]) {
    if(currentString && currentName) {
      self.styleDictionary.parameters[currentName] = currentString;
    }
    currentName = nil;
  }
  
  currentString = nil;
  currentSymbolizer = nil;
}


- (void)parserDidEndDocument:(NSXMLParser *)parser {
  //NSLog(@"Parse time:%f, %d styles", CFAbsoluteTimeGetCurrent() - startTime, self.styles.count);
//  [self generateStyles];
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
  currentLayer = nil;
}


@end
