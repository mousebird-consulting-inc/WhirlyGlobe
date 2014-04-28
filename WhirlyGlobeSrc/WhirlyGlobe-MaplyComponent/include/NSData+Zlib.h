/*
 *  NSData+Zlib.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/7/13.
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

#import <Foundation/Foundation.h>

/** The NSData zlib category adds compress and uncompress methods to
    NSData.
  */
@interface NSData(zlib)

/// Return a compressed version of the data.
- (NSData *) compressData;

/// Return an uncompressed verison of the given data
- (NSData *) uncompressGZip;

/// test if the data is zlib compressed
- (BOOL)isCompressed;

@end

// A function we can call to force the linker to bring in categories
#ifdef __cplusplus
extern "C"
#endif
void NSDataDummyFunc();
