/*
 *  GeoJSONSource.java
 *  WhirlyGlobeLib
 *
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

import com.mousebird.maply.sld.sldstyleset.SLDStyleSet;

import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Vector;

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
    private WeakReference<RenderControllerInterface> baseController;
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
            baseController.get().enableObjects(componentObjects, RenderController.ThreadMode.ThreadAny);
        else
            baseController.get().disableObjects(componentObjects, RenderController.ThreadMode.ThreadAny);
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
    public void setBaseController(RenderControllerInterface baseController) {
        this.baseController = new WeakReference<RenderControllerInterface>(baseController);
    }

    public GeoJSONSource() {

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
        HashMap<Long, ArrayList<VectorObject>> featureStyles = new HashMap<Long, ArrayList<VectorObject>>();
        ArrayList<ComponentObject> componentObjects = new ArrayList<ComponentObject>();

        try {

            while ((nRead = jsonStream.read(data, 0, data.length)) != -1) {
                buffer.write(data, 0, nRead);
            }
            buffer.flush();

            VectorObject vecs = new VectorObject();
            vecs.fromGeoJSON(buffer.toString());

            if (vecs != null) {
                TileID nullTileID = new TileID(0,0,0);
                for (VectorObject vecObj : vecs) {
                    VectorStyle[] styles = styleSet.stylesForFeature(vecObj.getAttributes(), nullTileID, "", baseController.get());
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
                TileID nullTileID = new TileID(0, 0, 0);
                Mbr bounds = new Mbr(new Point2d(-Math.PI,-Math.PI/2.0),new Point2d(Math.PI,Math.PI/2.0));
                VectorTileData tileData = new VectorTileData(nullTileID,bounds,bounds);
                for (Long uuid : featureStyles.keySet()) {
                    VectorStyle style = styleSet.styleForUUID(uuid, baseController.get());
                    ArrayList<VectorObject> featuresForStyle = featureStyles.get(uuid);

                    //List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller)
                    style.buildObjects(featuresForStyle.toArray(new VectorObject[0]), tileData, baseController.get());
                    ComponentObject[] newCompObjs = tileData.getComponentObjects();
                    if (newCompObjs != null && newCompObjs.length > 0)
                       componentObjects.addAll(Arrays.asList(newCompObjs));
                }
            }
            baseController.get().enableObjects(componentObjects, RenderController.ThreadMode.ThreadAny);

            this.componentObjects = componentObjects;
            loaded = true;
            enabled = true;
            completionBlock.run();

        } catch (Exception exception) {
            Log.e("ParseTask", "exception", exception);
        }
    }
}
