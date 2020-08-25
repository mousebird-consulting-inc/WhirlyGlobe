package com.mousebird.maply;

import java.lang.ref.WeakReference;

/**
 * Used to wrap a VectorStyle so it can be interfaced from the JNI side.
 * The methods are flattened down to pass as simple stuff around as we can manage.
 */
public class VectorStyleWrapper {
    VectorStyleInterface vectorStyleSet;
    WeakReference<RenderControllerInterface> control;

    protected VectorStyleWrapper() { }

    // Consolidate the vector styles in a way that makes it easier for the JNI side to handle
    VectorStyleWrapper(VectorStyleInterface vectorStyleSet,RenderControllerInterface control) {
        this.vectorStyleSet = vectorStyleSet;
        this.control = new WeakReference<RenderControllerInterface>(control);

        VectorStyle[] styles = vectorStyleSet.allStyles();

        long ids[] = new long[styles.length];
        String categories[] = new String[styles.length];
        boolean geomAdds[] = new boolean[styles.length];
        for (int ii=0;ii<styles.length;ii++) {
            VectorStyle style = styles[ii];
            ids[ii] = style.getUuid();
            categories[ii] = style.getCategory();
            geomAdds[ii] = style.geomIsAdditive();
        }

        initialise(ids,categories,geomAdds);
    }

    // Called from the JNI side to get a list of styles to apply for a given feature
    long[] stylesForFeature(AttrDictionary attrs,int tileX,int tileY,int tileLevel,String layerName) {
        if (control.get() == null)
            return null;
        VectorStyle[] vecStyles = vectorStyleSet.stylesForFeature(attrs,new TileID(tileX,tileY,tileLevel),layerName,control.get());
        if (vecStyles != null) {
            long[] ids = new long[vecStyles.length];
            for (int ii=0;ii<vecStyles.length;ii++)
                ids[ii] = vecStyles[ii].getUuid();
            return ids;
        }

        return null;
    }

    // Called from the JNI side to build objects for a certain style
    public void buildObjects(long styleID,VectorObject vecObjs[], VectorTileData tileData) {
        if (control.get() == null)
            return;

        VectorStyle style = vectorStyleSet.styleForUUID(styleID, control.get());
        if (style != null) {
            style.buildObjects(vecObjs, tileData, control.get());
        }
    }

    // Called from the JNI side to check on a whole layer (short circuits parsing)
    boolean layerShouldDisplay(String layerName,int tileX,int tileY,int tileLevel) {
        return vectorStyleSet.layerShouldDisplay(layerName,new TileID(tileX,tileY,tileLevel));
    }

    // Called by the JNI side to provide a background color for a given zoom level
    // This is typically a Mapbox thing, but we could add it generally.
    int backgroundColor()
    {
        return 0;
    }

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(long uuids[],String categories[],boolean[] geomAdditive);
    native void dispose();

    private long nativeHandle;
}
