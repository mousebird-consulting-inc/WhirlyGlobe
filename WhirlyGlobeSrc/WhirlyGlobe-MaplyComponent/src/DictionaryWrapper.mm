/*
 *  DictionaryWrapper.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/16/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "DictionaryWrapper_private.h"
#import "UIColor+Stuff.h"

// We'll subclass the dictionary so we can mess with its innards
class iosDictionary : public WhirlyKit::Dictionary
{
public:
    NSMutableDictionary *makeNSDictionary()
    {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        for (FieldMap::iterator it = fields.begin(); it != fields.end(); ++it)
        {
            NSString *str = [NSString stringWithFormat:@"%s",it->first.c_str()];
            id valObj = nil;
            switch (it->second->type())
            {
                case WhirlyKit::DictTypeString:
                {
                    std::string valStr;
                    it->second->asString(valStr);
                    valObj = [NSString stringWithFormat:@"%s",valStr.c_str()];
                }
                break;
                case WhirlyKit::DictTypeInt:
                    valObj = [NSNumber numberWithInt:it->second->asInt()];
                break;
                case WhirlyKit::DictTypeDouble:
                    valObj = [NSNumber numberWithDouble:it->second->asDouble()];
                break;
                default:
                    break;
            }
            if (valObj)
                dict[str] = valObj;
        }
        
        return dict;
    }
};

@implementation NSMutableDictionary(Dictionary)

+ (NSMutableDictionary *)DictionaryWithMaplyDictionary:(WhirlyKit::Dictionary *)dict
{
    iosDictionary *iosDict = (iosDictionary *)dict;
    return iosDict->makeNSDictionary();
}

@end

@implementation NSDictionary(Dictionary)

- (void)copyToMaplyDictionary:(WhirlyKit::Dictionary *)dict
{
    for (NSString *name in self.allKeys)
    {
        if ([name isKindOfClass:[NSString class]])
        {
            const char *cName = [name cStringUsingEncoding:NSASCIIStringEncoding];
            id valObj = self[name];
            if ([valObj isKindOfClass:[NSString class]])
            {
                NSString *str = valObj;
                // Note: Porting.  This won't handle unicode very well.
                const char *cStr = [str cStringUsingEncoding:NSASCIIStringEncoding];
                dict->setString(cName, cStr);
            } else if ([valObj isKindOfClass:[NSNumber class]])
            {
                NSNumber *num = valObj;
                int iVal = [num integerValue];
                double fVal = [num doubleValue];
                if ((double)iVal == fVal)
                    dict->setInt(cName, iVal);
                else
                    dict->setDouble(cName, fVal);
            } else if ([valObj isKindOfClass:[UIColor class]])
            {
                UIColor *color = valObj;
                int iVal = [color asInt];
                dict->setInt(cName, iVal);
            }
        }
    }
}

@end

void NSDictionaryDummyFunc2()
{
}

