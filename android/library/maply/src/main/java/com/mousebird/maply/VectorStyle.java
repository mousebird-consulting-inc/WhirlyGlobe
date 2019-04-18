/*
 *  VectorStyle
 *  com.mousebirdconsulting.maply
 *
 *  Created by Steve Gifford.
 *  Copyright 2013-2019 mousebird consulting
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

import java.util.List;

/**
 * A VectorStyle subclass generates the Maply objects for a given pieces of data.
 * This is used by the MapboxVectorTileSource to turn vector tile data into visible objects.
 * Potentially it can be used by other sources as well since there's no dependency on vector tiles.
 */
public interface VectorStyle
{
    /**
     * Return a unique identifier for your style.  This is used to help shortcut data processing.
     */
    public long getUuid();

    /**
     * If there is a category, return it.  We use these to sort created objects.
     */
    public String getCategory();

    /**
     * Set if this geometry is additive (e.g. sticks around) rather than being replaced.
     */
    public boolean geomIsAdditive();

    /**
     * Construct objects related to this style based on the input data.  This is where we build
     * the visible objects for the data corresponding to our style.
     *
     * @param vecObjs The vector objects we'll build a visual representation for.
     * @param tileData Information about the tile and where we put the tile when we're done.
     * @param controller The MaplyBaseController to use when building the visual representation.
     * @return The ComponentObjects created when building a visual representation.
     */
    public void buildObjects(VectorObject vecObjs[], VectorTileData tileData, RenderControllerInterface controller);
}
