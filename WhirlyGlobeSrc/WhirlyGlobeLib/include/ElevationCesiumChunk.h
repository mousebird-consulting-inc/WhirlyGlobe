/*
 *  ElevationChunk.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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

#import "ElevationChunk.h"
#import "VectorData.h"
#import <vector>

using namespace std;

@interface WhirlyKitElevationCesiumChunk : NSObject<WhirlyKitElevationChunkProtocol>

- (id)initWithCesiumData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY;

@property (nonatomic, readonly) VectorTrianglesRef mesh;

@property (nonatomic, readonly) vector<unsigned int> westVertices;
@property (nonatomic, readonly) vector<unsigned int> southVertices;
@property (nonatomic, readonly) vector<unsigned int> eastVertices;
@property (nonatomic, readonly) vector<unsigned int> northVertices;

@end
