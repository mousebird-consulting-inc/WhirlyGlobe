/*
 *  MapboxVectorTileParser_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/10/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "WhirlyGlobe.h"
#import "MaplyRenderController.h"
#import "MaplyVectorStyle.h"
#import "MapboxVectorTileParser.h"

namespace WhirlyKit
{

/// iOS version of the vector tile parser
/// Mostly fills in the callbacks
class MapboxVectorTileParser_iOS : public MapboxVectorTileParser
{
public:
    MapboxVectorTileParser_iOS(NSObject<MaplyVectorStyleDelegate> *__nonnull styleDelegate,NSObject<MaplyRenderControllerProtocol> *__nonnull);
    ~MapboxVectorTileParser_iOS();

    // Filter out layers we don't care about
    virtual bool layerShoudParse(const std::string &layerName,VectorTileData * __nonnull tileData);

    // Return a set of styles that will parse the given feature
    virtual SimpleIDSet stylesForFeature(MutableDictionaryRef attributes,const std::string &layerName,VectorTileData * __nonnull tileData);
    
    // If set, we'll tack a debug label in the middle of the tile
    bool debugLabel;
    
    // If set, we'll put an outline around the tile
    bool debugOutline;

    // Parse the data and add specific iOS level stuff on top
    virtual bool parse(RawData * __nonnull rawData,MaplyVectorTileData * __nonnull tileData);

protected:
    /// The styling delegate turns vector data into visible objects in the toolkit
    NSObject<MaplyVectorStyleDelegate> * __nullable styleDelegate;
    
    /// Maply view controller we're adding this data to
    NSObject<MaplyRenderControllerProtocol> * __nullable __weak viewC;
};
    
typedef std::shared_ptr<MapboxVectorTileParser_iOS> MapboxVectorTileParser_iOSRef;
    
}
