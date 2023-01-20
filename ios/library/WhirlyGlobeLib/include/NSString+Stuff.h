/*
 *  NSString+Stuff.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/11/11.
 *  Copyright 2011-2023 mousebird consulting
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

#import <UIKit/UIKit.h>
#import <string>

@interface NSString(Stuff)

/// Build an NSString from a std::wstring
+ (nullable NSString*) stringWithwstring:(const std::wstring&)ws;

/// Version of `cStringUsingEncoding` safe to assign to `std::string`, as long as `withDefault:` is non-null
- (nullable const char *)cStringUsingEncoding:(NSStringEncoding)encoding
                                  withDefault:(nullable const char*)def NS_RETURNS_INNER_POINTER;

/// Convert and return a std::string with UTF-8 encoding, or blank on failure
- (std::string) asStdString;

@end

// A function we can call to force the linker to bring in categories
#ifdef __cplusplus
extern "C"
#endif
void NSStringDummyFunc();
