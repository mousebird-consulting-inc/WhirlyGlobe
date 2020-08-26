/*
 *  QuadSamplingParams.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/14/19.
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

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

/**
 Sampling parameters.
 
 These are used to describe how we want to break down the globe or
 flat projection onto the globe.
 */
class SamplingParams
{
public:
    SamplingParams();
    virtual ~SamplingParams();
    
    bool operator == (const SamplingParams &) const;
    
    /// The coordinate system we'll be sampling from.
    CoordSystemRef coordSys;
    /// Bounding box for the coordinate system
    MbrD coordBounds;

    /// Min zoom level for sampling
    int minZoom;
    /// Max zoom level for sampling
    int maxZoom;
    
    /// We'll report out to this zoom level, even if we don't load them
    int reportedMaxZoom;
    
    /// Maximum number of tiles to load
    int maxTiles;
    
    /// Cutoff for loading tiles.  This is size in screen space (pixels^2)
    double minImportance;
    
    /// Normally we always load the lowest level
    /// If this is set we only load those lowest level tiles that pass this test
    double minImportanceTop;
    
    /// Generate geometry to cover the north and south poles
    /// Only works for world-wide projections
    bool coverPoles;
    
    /// If set, generate skirt geometry to hide the edges between levels
    bool edgeMatching;
    
    /// Tesselation values per level for breaking down the coordinate system (e.g. globe)
    int tessX,tessY;
    
    /// If set, we'll always load the lowest level first
    bool forceMinLevel;
    
    /// If non-zero we'll only force min level loading above this height
    double forceMinLevelHeight;
    
    /// If set, we'll try to load a single level
    bool singleLevel;
    
    /// If set, the tiles are clipped to this boundary
    MbrD clipBounds;
    
    /// Do we need globe geometry for this sampling set or nah?
    bool generateGeom;
    
    /**
     Detail the levels you want loaded in target level mode.
     
     The layer calculates the optimal target level.  The entries in this array are relative to that level or absolute.  For example [0,-4,-2] means the layer will always try to load levels 0, targetLevel-4 and targetLevel-2, but only the latter two if they make sense.
     */
    std::vector<int> levelLoads;
    
    /**
     Set the min importance for just one level.
     
     This is useful if you want your lower levels loaded more aggressively.
     */
    void setImportanceLevel(double minImportance,int level);
    
    std::vector<double> importancePerLevel;
};

    
}
