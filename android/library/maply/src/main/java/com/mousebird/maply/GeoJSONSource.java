package com.mousebird.maply;

import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.sld.sldstyleset.SLDStyleSet;

import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.ByteArrayOutputStream;
import java.net.URL;
import java.util.Arrays;
import java.util.HashMap;
import java.util.ArrayList;

import android.os.AsyncTask;
import android.util.Log;

/**
 * The GeoJSONSource will load vector features from a GeoJSON document, style them with an
 * SLD document, and add them to the globe or map.
 *
 * To use GeoJSONSource first construct one, then set the styleSet, the jsonStream, and the
 * baseController, and finally call startParse() to parse the JSON features, apply the styles,
 * and add them to the globe or map.
 *
 */
public class GeoJSONSource {

    private boolean loaded;
    private boolean enabled;
    private SLDStyleSet styleSet;
    private InputStream jsonStream;
    private MaplyBaseController baseController;
    ArrayList<ComponentObject> componentObjects = new ArrayList<ComponentObject>();

    public boolean isLoaded() {
        return loaded;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        if (this.enabled == enabled || !loaded || baseController == null)
            return;
        this.enabled = enabled;
        if (enabled)
            baseController.enableObjects(componentObjects, MaplyBaseController.ThreadMode.ThreadAny);
        else
            baseController.disableObjects(componentObjects, MaplyBaseController.ThreadMode.ThreadAny);
    }

    /**
     * Sets the SLD styleset used to style vector features based on their attributes.
     * @param styleSet The SLDStyleSet object.
     */
    public void setStyleSet(SLDStyleSet styleSet) {
        this.styleSet = styleSet;
    }

    /**
     * Sets the InputStream from which the JSON document will be read.
     * @param jsonStream The InputStream object which is the source of the JSON document.
     */
    public void setJsonStream(InputStream jsonStream) {
        this.jsonStream = jsonStream;
    }

    /**
     * Sets the globe or map controller to which vector features will be added.
     * @param baseController The MaplyBaseController instance.
     */
    public void setBaseController(MaplyBaseController baseController) {
        this.baseController = baseController;
    }

    public GeoJSONSource() {

        initialise();
        loaded = false;
        enabled = false;

    }

    /**
     *
     * Start parsing the GeoJSON.  This will load the features, apply the styles, and add the
     * styled features to the globe or map.
     *
     */
    public void startParse() {
        startParse(new Runnable() {
            @Override
            public void run() {
            }
        });
    }

    /**
     * Start parsing the GeoJSON.  Call this method if you have a custom Runnable that should
     * be executed after completion.
     *
     * This will load the features, apply the styles, and add the styled features to the
     * globe or map.
     *
     * @param completionBlock Block to execute after completion.
     */
    public void startParse(final Runnable completionBlock) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        int nRead;
        byte[] data = new byte[16384];
        HashMap<String, ArrayList<VectorObject>> featureStyles = new HashMap<String, ArrayList<VectorObject>>();
        ArrayList<ComponentObject> componentObjects = new ArrayList<ComponentObject>();

        try {

            while ((nRead = jsonStream.read(data, 0, data.length)) != -1) {
                buffer.write(data, 0, nRead);
            }
            buffer.flush();

            VectorObject[] vecs = parseData(buffer.toString());

            if (vecs != null) {
                MaplyTileID nullTileID = new MaplyTileID(0,0,0);
                for (VectorObject vecObj : vecs) {
                    VectorStyle[] styles = styleSet.stylesForFeature(vecObj.getAttributes(), nullTileID, "", baseController);
                    if (styles == null || styles.length == 0)
                        continue;;
                    for (VectorStyle style : styles) {
                        ArrayList<VectorObject> featuresForStyle = featureStyles.get(style.getUuid());
                        if (featuresForStyle == null) {
                            featuresForStyle = new ArrayList<VectorObject>();
                            featureStyles.put(style.getUuid(), featuresForStyle);
                        }
                        featuresForStyle.add(vecObj);
                    }
                }
            }

            if (vecs != null) {
                MaplyTileID nullTileID = new MaplyTileID(0, 0, 0);
                for (String uuid : featureStyles.keySet()) {
                    VectorStyle style = styleSet.styleForUUID(uuid, baseController);
                    ArrayList<VectorObject> featuresForStyle = featureStyles.get(uuid);

                    //List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller)
                    ComponentObject[] newCompObjs = style.buildObjects(featuresForStyle, nullTileID, baseController);
                    if (newCompObjs != null && newCompObjs.length > 0)
                       componentObjects.addAll(Arrays.asList(newCompObjs));
                }
            }
            baseController.enableObjects(componentObjects, MaplyBaseController.ThreadMode.ThreadAny);

            this.componentObjects = componentObjects;
            loaded = true;
            enabled = true;
            completionBlock.run();

        } catch (Exception exception) {
            Log.e("ParseTask", "exception", exception);
        }
    }

    native VectorObject[] parseData(String json);

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
