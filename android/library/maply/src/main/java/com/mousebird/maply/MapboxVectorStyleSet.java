package com.mousebird.maply;

import android.graphics.Color;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Mapbox Vector Style Set.
 * This parses a Mapbox style sheet and interfaces with the vector parser
 */
public class MapboxVectorStyleSet implements VectorStyleInterface {

    MapboxVectorStyleSet(String styleJSON,VectorStyleSettings inSettings,BaseController inControl)
    {
        this(styleJSON,inSettings,inControl.renderControl);
    }

    // Construct with the JSON data from a string
    MapboxVectorStyleSet(String styleJSON,VectorStyleSettings inSettings,RenderController inControl) {
        control = new WeakReference<RenderController>(inControl);
        if (inSettings == null)
            inSettings = new VectorStyleSettings();
        settings = inSettings;

        AttrDictionary styleDict = new AttrDictionary();
        if (!styleDict.parseFromJSON(styleJSON)) {
            throw new IllegalArgumentException("Bad JSON for style sheet in MapboxVectorStyleSet");
        }

        spriteURL = styleDict.getString("sprite");

        // Sources tell us where to get tiles
        AttrDictionary sourcesDict = styleDict.getDict("sources");
        if (sourcesDict != null) {
            String[] keys = styleDict.getKeys();
            for (String key : keys) {
                Source source = new Source(sourcesDict.getDict(key));
                sources.add(source);
            }
        }

        initialise(styleJSON);
    }

    VectorStyleSettings settings;
    WeakReference<RenderController> control;

    // Calculate an appropriate background color given the zoom level
    int backgroundColorForZoom(double zoom)
    {
        // TODO: Fill in the color calculation
        return Color.WHITE;
    }

    // If there's a sprite sheet, where it's at
    String spriteURL;

    enum SourceType {Vector,Raster};

    // Source for vector tile (or raster) data
    class Source {
        // Name as it appears in the file
        String name;

        // Either vector or raster at present
        SourceType type;

        // TileJSON URL, if present
        String url;

        // If the TileJSON spec is inline, it's here
        AttrDictionary tileSpec;

        Source(String inName,AttrDictionary styleEntry, MapboxVectorStyleSet styleSet,RenderController control) {
            name = inName;

            String typeStr = styleEntry.getString("type");
            if (typeStr == "vector") {
                type = SourceType.Vector;
            } else if (typeStr == "raster") {
                type = SourceType.Raster;
            } else {
                throw new IllegalArgumentException("Unexpected type string in Mapbox Source");
            }

            url = styleEntry.getString("url");
            tileSpec = styleEntry.getDict("tiles");

            if (url == null && tileSpec == null) {
                throw new IllegalArgumentException("Expecting either URL or tileSpec in source " + name);
            }
        }
    }

    ArrayList<Source> sources = new ArrayList<Source>();

    /**
     * These are actually implemented on the C++ side, which communicates
     * with itself.  But we need to here to appear to be using the standard
     * interface.
     */
    public VectorStyle[] stylesForFeature(AttrDictionary attrs,TileID tileID,String layerName,RenderControllerInterface controller)
    {
        return null;
    }
    public VectorStyle[] allStyles()
    {
        return null;
    }
    public boolean layerShouldDisplay(String layerName,TileID tileID)
    {
        return false;
    }
    public VectorStyle styleForUUID(long uuid,RenderControllerInterface controller)
    {
        return null;
    }

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    native void initialise();
    native void dispose();
    private static native void nativeInit();
    protected long nativeHandle;
}
