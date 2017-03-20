/*
 *  MaplyGDALRetileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/2/13.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

@interface MaplyGDALRetileSource : NSObject<MaplyTileSource>

- (nonnull instancetype)initWithURL:(NSString *__nonnull)baseURL baseName:(NSString *__nonnull)baseName ext:(NSString *__nonnull)ext coordSys:(MaplyCoordinateSystem *__nonnull)coordSys levels:(int)numLevels;

@property (nonatomic,strong, nullable) NSString *cacheDir;

@end

