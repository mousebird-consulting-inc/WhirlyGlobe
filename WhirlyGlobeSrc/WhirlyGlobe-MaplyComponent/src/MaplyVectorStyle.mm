/*
 *  MaplyVectorStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import "MaplyVectorStyle.h"
#import "MaplyVectorLineStyle.h"
#import "MaplyVectorMarkerStyle.h"
#import "MaplyVectorPolygonStyle.h"
#import "MaplyVectorTextStyle.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplyVectorTileStyleSettings

- (id)init
{
    self = [super init];
    _lineScale = 1.0;
    _textScale = 1.0;
    _markerScale = 1.0;
    _markerImportance = 2.0;
    _markerSize = 10.0;
    _mapScaleScale = 1.0;
    _dashPatternScale = 1.0;
    _useWideVectors = true;
    _wideVecCuttoff = 0.0;
    _oldVecWidthScale = 1.0;
    _selectable = false;
  
    return self;
}

- (NSString*)description
{
  return [NSString stringWithFormat:@"%@: lineScale:%f textScale:%f markerScale:%f mapScaleScale:%f",
          [[self class] description], _lineScale, _textScale, _markerScale, _mapScaleScale];
}

@end

@implementation MaplyVectorTileStyle

+ (id)styleFromStyleEntry:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
{
    MaplyVectorTileStyle *tileStyle = nil;
    
    NSString *typeStr = styleEntry[@"type"];
    if ([typeStr isEqualToString:@"LineSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleLine alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else if ([typeStr isEqualToString:@"PolygonSymbolizer"] || [typeStr isEqualToString:@"PolygonPatternSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStylePolygon alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else if ([typeStr isEqualToString:@"TextSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleText alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else if ([typeStr isEqualToString:@"MarkersSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleMarker alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else {
        // Set up one that doesn't do anything
        NSLog(@"Unknown symbolizer type %@",typeStr);
        tileStyle = [[MaplyVectorTileStyle alloc] init];
    }
    
    return tileStyle;
}

- (id)initWithStyleEntry:(NSDictionary *)styleEntry viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    _viewC = viewC;
    _uuid = styleEntry[@"uuid"];
    if ([styleEntry[@"tilegeom"] isEqualToString:@"add"])
        self.geomAdditive = true;
    _selectable = styleEntry[kMaplySelectable];
    
    return self;
}

- (void)resolveVisibility:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings desc:(NSMutableDictionary *)desc
{
    float minVis = DrawVisibleInvalid;
    float maxVis = DrawVisibleInvalid;
    if (styleEntry[@"minscaledenom"])
    {
        float minScale = [styleEntry[@"minscaledenom"] floatValue];
        minVis = [self.viewC heightForMapScale:minScale] * settings.mapScaleScale;
    }
    if (styleEntry[@"maxscaledenom"])
    {
        float maxScale = [styleEntry[@"maxscaledenom"] floatValue];
        maxVis = [self.viewC heightForMapScale:maxScale] * settings.mapScaleScale;
    }
    if (minVis != DrawVisibleInvalid)
    {
        desc[kMaplyMinVis] = @(minVis);
        if (maxVis != DrawVisibleInvalid)
            desc[kMaplyMaxVis] = @(maxVis);
        else
            desc[kMaplyMaxVis] = @(20.0);
    } else if (maxVis != DrawVisibleInvalid)
    {
        desc[kMaplyMinVis] = @(0.0);
        desc[kMaplyMaxVis] = @(maxVis);
    }
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID layer:(MaplyQuadPagingLayer *)layer viewC:(MaplyBaseViewController *)viewC;
{
    return nil;
}


//sometimes we get strings that look like [name]+'\n '+[ele]
- (NSString*)formatText:(NSString*)formatString forObject:(MaplyVectorObject*)vec
{
    if (!formatString)
    {
        return nil;
    }
    
    // Note: This is a terrible hack.  Change the regex string or fix the data.
    {
        NSMutableDictionary *attributes = (NSMutableDictionary *)vec.attributes;
        if (attributes[@"NAME"] && !attributes[@"name"])
            attributes[@"name"] = attributes[@"NAME"];
    }

    @try {
        //Do variable substitution on [ ... ]
        NSMutableString *result;
        {
            NSError *error;
            NSRegularExpression *regex = [[NSRegularExpression alloc] initWithPattern:@"\\[[^\\[\\]]+\\]"
                                                                              options:0
                                                                                error:&error];
            NSArray* matches = [regex matchesInString:formatString
                                              options:0
                                                range:NSMakeRange(0, formatString.length)];
            
            if(matches.count)
            {
                NSDictionary *attributes = vec.attributes;
                result = [NSMutableString stringWithString:formatString];
                for (int i=(int)matches.count-1; i>= 0; i--)
                {
                    NSTextCheckingResult* match = matches[i];
                    NSString *matchedStr = [formatString substringWithRange:NSMakeRange(match.range.location + 1,
                                                                                        match.range.length - 2)];
                    id replacement = attributes[matchedStr]?:@"";
                    if([replacement isKindOfClass:[NSNumber class]])
                    {
                        replacement = [replacement stringValue];
                    }
                    [result replaceCharactersInRange:match.range withString:replacement];
                }
            }
        }
        
        //replace \n with a newline
        if([formatString rangeOfString:@"\\"].location != NSNotFound )
        {
            if(!result)
            {
                result = [NSMutableString stringWithString:formatString];
            }
            [result replaceOccurrencesOfString:@"\\n"
                                    withString:@"\n"
                                       options:0
                                         range:NSMakeRange(0, result.length)];
        }
        
        //replace + and surrounding whitespace
        //This should probably check if the plus is surrounded by '', but for now i havent needed that
        {
            NSError *error;
            NSRegularExpression *regex = [[NSRegularExpression alloc] initWithPattern:@"\\s?\\+\\s?"
                                                                              options:0
                                                                                error:&error];
            NSArray* matches = [regex matchesInString:result?:formatString
                                              options:0
                                                range:NSMakeRange(0, result?result.length:formatString.length)];
            
            if(matches.count)
            {
                if(!result)
                {
                    result = [NSMutableString stringWithString:formatString];
                }
                for (int i=(int)matches.count-1; i>= 0; i--)
                {
                    NSTextCheckingResult* match = matches[i];
                    [result deleteCharactersInRange:match.range];
                }
            }
        }
        
        //replace quotes around quoted strings
        {
            NSError *error;
            NSRegularExpression *regex = [[NSRegularExpression alloc] initWithPattern:@"'[^\\.]+'"
                                                                              options:0
                                                                                error:&error];
            NSArray* matches = [regex matchesInString:result?:formatString
                                              options:0
                                                range:NSMakeRange(0, result?result.length:formatString.length)];
            
            if(matches.count)
            {
                if(!result)
                {
                    result = [NSMutableString stringWithString:formatString];
                }
                for (int i=(int)matches.count-1; i>= 0; i--)
                {
                    NSTextCheckingResult* match = matches[i];
                    NSString *matchedStr = [result substringWithRange:NSMakeRange(match.range.location + 1,
                                                                                        match.range.length - 2)];
                    [result replaceCharactersInRange:match.range withString:matchedStr];
                }
            }
        }
        
        if(result)
        {
            return result;
        } else {
            return formatString;
        }
    }
    @catch (NSException *exception){
        NSLog(@"Error formatting string:\"%@\" exception:%@", formatString, exception);
        return nil;
    }
}


- (NSString*)description
{
    return [NSString stringWithFormat:@"%@ uuid:%@ additive:%d",
          [[self class] description], self.uuid, self.geomAdditive];
}

@end
