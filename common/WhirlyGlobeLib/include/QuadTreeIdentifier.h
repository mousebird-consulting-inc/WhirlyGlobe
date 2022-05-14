/*
 *  QuadTreeIdentifier.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
 *  Copyright 2012-2022 Saildrone Inc
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

#import "WhirlyEigen.h"

namespace WhirlyKit
{

/// Represents a single quad tree node in simplified form
struct QuadTreeIdentifier
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    QuadTreeIdentifier() = default;
    /// Construct with the cell coordinates and level.
    QuadTreeIdentifier(int x,int y,int level) : x(x), y(y), level(level) { }

    static int64_t NodeNumber(int x, int y, int level)
    {
        const int64_t twoToZ = 1 << level;
        
        // total count of all levels below
        const int64_t baseCount = (twoToZ * twoToZ - 1) / 3;   // (4 ^ z - 1) / 3
        
        // number within this level
        const int64_t tileNumber = y * twoToZ + x;
        
        return baseCount + tileNumber;
    }
    int64_t NodeNumber() const { return NodeNumber(x,y,level); }

    /// Comparison based on x,y,level.  Used for sorting
    bool operator < (const QuadTreeIdentifier &that) const;
    
    /// Quality operator
    bool operator == (const QuadTreeIdentifier &that) const;
    
    /// Spatial subdivision along the X axis relative to the space
    int x;
    /// Spatial subdivision along tye Y axis relative to the space
    int y;
    /// Level of detail, starting with 0 at the top (low)
    int level;
};

}
