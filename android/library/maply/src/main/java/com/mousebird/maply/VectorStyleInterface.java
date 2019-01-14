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
    VectorStyle[] stylesForFeature(AttrDictionary attrs,MaplyTileID tileID,String layerName,MaplyBaseController controller);

    /**
     * Returns true if the given layer (by name) should be displayed at all.  If you return
     * false, then those vector objects will not be displayed and potentially not even parsed.
     * @param layerName Name of the layer to be displayed (or not)
     * @param tileID If we're using a tile source, this is the tile ID.  If not, it will be null.
     */
    boolean layerShouldDisplay(String layerName,MaplyTileID tileID);

    /**
     * Returns a vector style corresponding to the given unique ID.  Each vector style has to
     * have a unique ID which we'll use for sorting things.
     * @param uuid Unique ID
     * @param controller Base controller used for the geometry.
     * @return Vector Style to be used in displaying geometry.
     */
    VectorStyle styleForUUID(String uuid,MaplyBaseController controller);
}
