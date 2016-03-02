/*
 *  MaplyRemoteTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
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

#import <vector>
#import <set>
#import "AFHTTPRequestOperation.h"

namespace Maply
{
// Used to store active tile fetching operations
class TileFetchOp
{
public:
    TileFetchOp(MaplyTileID tileID) : tileID(tileID), op(nil) { }
    
    bool operator < (const TileFetchOp &that) const
    {
        if (tileID.level == that.tileID.level)
        {
            if (tileID.x == that.tileID.x)
                return tileID.y < that.tileID.y;
            else
                return tileID.x < that.tileID.y;
        } else
            return tileID.level < that.tileID.level;
    }
    
    MaplyTileID tileID;
    AFHTTPRequestOperation *op;
};
typedef std::set<TileFetchOp> TileFetchOpSet;
}
