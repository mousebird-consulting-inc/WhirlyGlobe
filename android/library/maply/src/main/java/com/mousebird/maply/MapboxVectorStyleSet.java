package com.mousebird.maply;

import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Log;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * Mapbox Vector Style Set.
 * This parses a Mapbox style sheet and interfaces with the vector parser
 */
public class MapboxVectorStyleSet implements VectorStyleInterface {

    private MapboxVectorStyleSet() {
    }

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
                try {
                    Source source = new Source(key, sourcesDict.getDict(key), this);
                    sources.add(source);
                }
                catch (Exception e) {
                }
            }
        }

        displayMetrics = inDisplayMetrics;

        initialise(inControl.getScene(),inControl.getCoordSystem(),settings,styleDict);
    }

    /**
     * Set this to override the regular fill shader.
     * Useful if you're going to mix something else into the polygons.
     */
    public void setArealShader(Shader shader) {
        setArealShaderNative(shader.getID());
    }

    native private void setArealShaderNative(long shaderID);

    DisplayMetrics displayMetrics;
    VectorStyleSettings settings;
    WeakReference<RenderControllerInterface> control;

    // Calculate an appropriate background color given the zoom level
    public int backgroundColorForZoom(double zoom)
    {
        return backgroundColorForZoomNative(zoom);
    }

    public native int backgroundColorForZoomNative(double zoom);

    private final ArrayList<LabelInfo> labelInfos = new ArrayList<>();

    // Return a label info
    public LabelInfo labelInfoForFont(String fontName,float fontSize) {
        synchronized (labelInfos) {
            // todo: use a dictionary or something
            for (LabelInfo labelInfo : labelInfos) {
                if (labelInfo.fontSize == fontSize && labelInfo.fontName.equals(fontName))
                    return labelInfo;
            }
        }

        Map<String,File> systemFonts = getSystemFonts();
        File systemFile = systemFonts.get(normalizeFontName(fontName));
        if (systemFile != null) {
            // We have that one!
            return createFromFile(fontName, systemFile, fontSize);
        }

        Typeface family = Typeface.DEFAULT;

        String origName = fontName;
        fontName = regularPattern.matcher(fontName).replaceFirst("");

        String newName = sansPattern.matcher(fontName).replaceFirst("");
        if (!newName.equals(fontName))
        {
            fontName = newName;
            family = Typeface.SANS_SERIF;
        }

        int style = Typeface.NORMAL;
        newName = boldPattern.matcher(fontName).replaceFirst(" ");
        if (!newName.equals(fontName))
        {
            fontName = newName;
            style = style | Typeface.BOLD;

        }

        newName = italicPattern.matcher(fontName).replaceFirst(" ");
        if (!newName.equals(fontName))
        {
            fontName = newName;
            style = style | Typeface.ITALIC;
        }

        // Try again with the reduced name
        systemFile = systemFonts.get(normalizeFontName(fontName));
        if (systemFile != null)
        {
            return createFromFile(origName, systemFile, fontSize);
        }

        // Didn't find it, so make one up
        Typeface typeface = Typeface.create(fontName, style);
        LabelInfo labelInfo = new LabelInfo();
        labelInfo.setTypeface(typeface);
        labelInfo.setFontSize(fontSize);
        labelInfo.fontName = origName;
        synchronized (labelInfos) {
            labelInfos.add(labelInfo);
        }

        return labelInfo;
    }

    private LabelInfo createFromFile(String fontName, File file, float size) {
        Typeface typeface = Typeface.createFromFile(file);
        LabelInfo labelInfo = new LabelInfo();
        labelInfo.setTypeface(typeface);
        labelInfo.setFontSize(size);
        labelInfo.fontName = fontName;
        synchronized (labelInfos) {
            labelInfos.add(labelInfo);
        }
        return labelInfo;
    }

    private static Map<String,File> getSystemFonts() {
        synchronized (syncObj) {
            if (systemFonts == null) {
                try {
                    File[] files = new File(systemFontDir).listFiles();

                    systemFonts = new TreeMap<>(String.CASE_INSENSITIVE_ORDER);
                    for (File f : files) {
                        String fontName = normalizeFontName(f.getName());
                        // If we find multiple fonts differing only by case or extension, just use the first.
                        if (!systemFonts.containsKey(fontName)) {
                            systemFonts.put(fontName, f);
                        }
                    }
                } catch (Exception ex) {
                    Log.w("MapboxVectorStyleSet", "Failed to get system fonts: " + ex.getMessage());
                    systemFonts = new TreeMap<>();
                }
            }
            return systemFonts;
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
                Log.w("Maply", "Expecting either URL or tileSpec in source " + name);
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

    private static String normalizeFontName(String s) {
        return stripAll(s, extensionPattern, separatorPattern);
    }

    private static String replaceAll(String s, String replacement, Pattern p0) {
        return p0.matcher(s).replaceAll(replacement);
    }
    private static String replaceAll(String s, String replacement, Pattern p0, Pattern... pats) {
        s = replaceAll(s, replacement, p0);
        for (Pattern p : pats) {
            s = replaceAll(s, replacement, p);
        }
        return s;
    }
    private static String stripAll(String s, Pattern p0, Pattern... pats) {
        return replaceAll(s, "", p0, pats);
    }

    private static final Pattern whitespacePattern = Pattern.compile("\\s+");
    private static final Pattern separatorPattern = Pattern.compile("[\\s-_]+");
    private static final Pattern extensionPattern = Pattern.compile("\\.\\w+$");
    private static final Pattern regularPattern = Pattern.compile("[\\s-_]regular\\b", Pattern.CASE_INSENSITIVE);
    private static final Pattern sansPattern = Pattern.compile("[\\s-_]sans\\b", Pattern.CASE_INSENSITIVE);
    private static final Pattern boldPattern = Pattern.compile("[\\s-_]bold\\b", Pattern.CASE_INSENSITIVE);
    private static final Pattern italicPattern = Pattern.compile("[\\s-_]italic\\b", Pattern.CASE_INSENSITIVE);

    private static Map<String,File> systemFonts = null;
    private static final Object syncObj = new Object();
    private static final String systemFontDir = "/system/fonts";

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
