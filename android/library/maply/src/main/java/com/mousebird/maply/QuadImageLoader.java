/*
 *  QuadImageLoader.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
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

package com.mousebird.maply;

import android.renderscript.Sampler;

public class QuadImageLoader extends QuadImageLoaderBase
{
    boolean valid = false;

    /**
     *  Initialize with a single tile info object and the sampling parameters.
     *
     *  @param params The sampling parameters describing how to break down the data for projection onto a globe or map.
     *  @param tileInfo A single tile info object describing where the data is and how to get it.
     *  @param control the controller to add objects to.
     */
    QuadImageLoader(SamplingParams params,TileInfoNew tileInfo,MaplyBaseController control)
    {
        super(control);

        tileInfos = new TileInfoNew[1];
        tileInfos[0] = tileInfo;
        valid = true;

        // Let's delay the
        control.getActivity().getMainLooper()
    }

    TileInfoNew[] tileInfos = null;
}
