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

import android.os.Handler;
import android.renderscript.Sampler;

import java.util.ArrayList;

/**
 * The Quad Image Loader is for paging image pyramids local or remote.
 * <br>
 * This layer pages image pyramids.  They can be local or remote, in any coordinate system Maply
 * supports and you provide a TileInfoNew conformant object to do the actual image tile fetching.
 *
 * You probably don't have to implement your own tile source.
 * Go look at the RemoteTileFetch and MBTileFetcher objects.  Those will do remote and local fetching.
 */
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
    public QuadImageLoader(final SamplingParams params,TileInfoNew tileInfo,BaseController control)
    {
        super(control, params, 1);

        tileInfos = new TileInfoNew[1];
        tileInfos[0] = tileInfo;
        valid = true;

        Handler handler = new Handler(control.getActivity().getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                if (!valid)
                    return;

                delayedInit(params);
            }
        });
    }

    /**
     * Change the tile source and force a reload.
     */
    public void changeTileInfo(TileInfoNew tileInfo)
    {
        tileInfos = new TileInfoNew[1];
        tileInfos[0] = tileInfo;

        super.changeTileInfo(tileInfos);
    }
}
