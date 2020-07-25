/*
 *  VectorStyleInterface
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

/**
 * The Vector Style Interface returns Vector Style objects for a given list of attributes.
 * It also can be used to control which layers are displayed, possibly circumventing a large
 * amount of processing.
 * <br>
 * This class is a generator for the styles, which do the actual work.  We use UUIDs for the styles
 * so that we can collect all the vectors together for a particular style and then call that
 * style with a single list of data.  This makes data display far more efficient.
 */
public interface VectorStyleInterface
{
    /**
     * Return the styles that match to the given list of attributes.  Each of these styles
     * will be called to generate visible geometry.  Order is not important.
     * @param attrs The attribute dictionary from the vector feature.
     * @param tileID The tileID if we're building from a tile source. null if not.
     * @param layerName The layer name if the data is coming from vector tiles.
     * @param controller Maply base controller where the
     * @return A list of vector styles that apply to the vector data with the given attributes.
     */
    public VectorStyle[] stylesForFeature(AttrDictionary attrs,TileID tileID,String layerName,RenderControllerInterface controller);

    /**
     * Return the full list of styles.
     */
    public VectorStyle[] allStyles();

    /**
     * Returns true if the given layer (by name) should be displayed at all.  If you return
     * false, then those vector objects will not be displayed and potentially not even parsed.
     * @param layerName Name of the layer to be displayed (or not)
     * @param tileID If we're using a tile source, this is the tile ID.  If not, it will be null.
     */
    public boolean layerShouldDisplay(String layerName,TileID tileID);

    /**
     * Returns a vector style corresponding to the given unique ID.  Each vector style has to
     * have a unique ID which we'll use for sorting things.
     * @param uuid Unique ID
     * @param controller Base controller used for the geometry.
     * @return Vector Style to be used in displaying geometry.
     */
    public VectorStyle styleForUUID(long uuid,RenderControllerInterface controller);

    /**
     * Return a color for the background (clear color).
     */
    public int backgroundColorForZoom(double zoom);
}
