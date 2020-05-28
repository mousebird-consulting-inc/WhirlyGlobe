package com.mousebird.maply;

import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.DisplayMetrics;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Mapbox Vector Style Set.
 * This parses a Mapbox style sheet and interfaces with the vector parser
 */
public class MapboxVectorStyleSet implements VectorStyleInterface {

    public MapboxVectorStyleSet(String styleJSON,VectorStyleSettings inSettings,DisplayMetrics inDisplayMetrics,RenderControllerInterface inControl) {
        AttrDictionary styleDict = new AttrDictionary();
        if (!styleDict.parseFromJSON(styleJSON)) {
            throw new IllegalArgumentException("Bad JSON for style sheet in MapboxVectorStyleSet");
        }

        combinedInit(styleDict,inSettings,inDisplayMetrics,inControl);
    }

    // Construct with the JSON data from a string
    public MapboxVectorStyleSet(AttrDictionary styleDict,VectorStyleSettings inSettings,DisplayMetrics inDisplayMetrics,RenderControllerInterface inControl) {
        combinedInit(styleDict,inSettings,inDisplayMetrics,inControl);
    }

    // Used by both constructors
    private void combinedInit(AttrDictionary styleDict,VectorStyleSettings inSettings,DisplayMetrics inDisplayMetrics,RenderControllerInterface inControl)
    {
        // Fault in the ComponentObject native implementation.
        // Because the first time it can be called in this case is C++ side
        ComponentObject testObj = new ComponentObject();

        control = new WeakReference<RenderControllerInterface>(inControl);
        if (inSettings == null)
            inSettings = new VectorStyleSettings();
        settings = inSettings;

        spriteURL = styleDict.getString("sprite");

        // Sources tell us where to get tiles
        AttrDictionary sourcesDict = styleDict.getDict("sources");
        if (sourcesDict != null) {
            String[] keys = sourcesDict.getKeys();
            for (String key : keys) {
                Source source = new Source(key,sourcesDict.getDict(key),this);
                sources.add(source);
            }
        }

        displayMetrics = inDisplayMetrics;

        initialise(inControl.getScene(),inControl.getCoordSystem(),settings,styleDict);
    }

    DisplayMetrics displayMetrics;
    VectorStyleSettings settings;
    WeakReference<RenderControllerInterface> control;

    // Calculate an appropriate background color given the zoom level
    public int backgroundColorForZoom(double zoom)
    {
        return backgroundColorForZoomNative(zoom);
    }

    public native int backgroundColorForZoomNative(double zoom);

    ArrayList<LabelInfo> labelInfos = new ArrayList<LabelInfo>();

    // Return a label info
    public LabelInfo labelInfoForFont(String fontName,float fontSize) {
        synchronized (this) {
            for (LabelInfo labelInfo: labelInfos) {
                if (labelInfo.fontSize == fontSize && labelInfo.fontName.equals(fontName))
                    return labelInfo;
            }

            // Didn't find it, so make one up
            // TODO: What about bold, italic, etc??
            Typeface typeface = Typeface.create(fontName,Typeface.NORMAL);
            LabelInfo labelInfo = new LabelInfo();
            labelInfo.setTypeface(typeface);
            labelInfo.setFontSize(fontSize);
            labelInfo.fontName = fontName;
            labelInfos.add(labelInfo);

            return labelInfo;
        }
    }

    // Calculate text width based on the typeface
    public double calculateTextWidth(String text,LabelInfo labelInfo)
    {
        Paint paint = new Paint();
        paint.setTextSize(labelInfo.fontSize);
        paint.setTypeface(labelInfo.getTypeface());
        Rect bounds = new Rect();
        paint.getTextBounds(text,0,text.length(), bounds);

        return bounds.right - bounds.left;
    }

    // If there's a sprite sheet, where it's at
    public String spriteURL;

    public enum SourceType {Vector,Raster};

    // Source for vector tile (or raster) data
    public class Source {
        // Name as it appears in the file
        String name;

        // Either vector or raster at present
        SourceType type;

        // TileJSON URL, if present
        String url;

        // If the TileJSON spec is inline, it's here
        AttrDictionary tileSpec;

        Source(String inName,AttrDictionary styleEntry, MapboxVectorStyleSet styleSet) {
            name = inName;

            String typeStr = styleEntry.getString("type");
            if (typeStr.equals("vector")) {
                type = SourceType.Vector;
            } else if (typeStr.equals("raster")) {
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

    public ArrayList<Source> sources = new ArrayList<Source>();

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
    native void initialise(Scene scene,CoordSystem coordSystem,VectorStyleSettings settings,AttrDictionary styleDict);
    native void dispose();
    private static native void nativeInit();
    protected long nativeHandle;
}
