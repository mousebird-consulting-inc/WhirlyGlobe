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
    String getUuid();

    /**
     * Set if this geometry is additive (e.g. sticks around) rather than being replaced.
     */
    boolean geomIsAdditive();

    /**
     * Construct objects related to this style based on the input data.  This is where we build
     * the visible objects for the data corresponding to our style.
     *
     * @param objects The vector objects we'll build a visual representation for.
     * @param tileID The tile we're building, if this is used by a tile source.  If not, this is null.
     * @param controller The MaplyBaseController to use when building the visual representation.
     * @return The ComponentObjects created when building a visual representation.
     */
    ComponentObject[] buildObjects(List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller);
}
