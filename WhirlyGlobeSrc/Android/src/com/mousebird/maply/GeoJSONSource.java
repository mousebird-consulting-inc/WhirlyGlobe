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



public class GeoJSONSource {


    private boolean loaded;
    private boolean enabled;
    private SLDStyleSet styleSet;
    private InputStream jsonStream;

    public void setStyleSet(SLDStyleSet styleSet) {
        this.styleSet = styleSet;
    }

    public void setJsonStream(InputStream jsonStream) {
        this.jsonStream = jsonStream;
    }

    public void setBaseController(MaplyBaseController baseController) {
        this.baseController = baseController;
    }

    public void setRelativeDrawPriority(int relativeDrawPriority) {
        this.relativeDrawPriority = relativeDrawPriority;
    }

    private MaplyBaseController baseController;
    private int relativeDrawPriority;
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

    public GeoJSONSource() {

        initialise();
        Log.i("GeoJSONSource", "constructor");
        loaded = false;
        enabled = false;

    }

    public void startParse(final Runnable completionBlock) {

        try {

            HashMap<String, ArrayList<VectorObject>> featureStyles = new HashMap<String, ArrayList<VectorObject>>();
            VectorObject[] vecs;

            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            int nRead;
            byte[] data = new byte[16384];

            while ((nRead = jsonStream.read(data, 0, data.length)) != -1) {
                buffer.write(data, 0, nRead);
            }
            buffer.flush();
            vecs = parseData(buffer.toString());

            if (vecs != null) {
                MaplyTileID nullTileID = new MaplyTileID(0,0,0);
                for (VectorObject vecObj : vecs) {
                    VectorStyle[] styles = styleSet.stylesForFeature(vecObj.getAttributes(), nullTileID, "", baseController);
                    if (styles == null || styles.length == 0)
                        continue;
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
                    componentObjects.addAll(Arrays.asList(style.buildObjects(featuresForStyle, nullTileID, baseController)));
                }
            }
            baseController.enableObjects(componentObjects, MaplyBaseController.ThreadMode.ThreadAny);


        } catch (Exception exception) {
            Log.e("ParseTask", "exception", exception);
        }

        completionBlock.run();
    }

//    public void startParse(final Runnable completionBlock) {
//        Log.i("GeoJSONSource", "startParse(...) flag 1");
//        ParseTask parseTask = new ParseTask(jsonStream, styleSet, baseController);
//
//        class MyRunnable implements Runnable {
//            ParseTask parseTask;
//            public void setParseTask(ParseTask parseTask) {
//                this.parseTask = parseTask;
//            }
//
//            @Override
//            public void run() {
//                Log.i("MyRunnable", "run");
//                loaded = true;
//                enabled = true;
//                componentObjects = this.parseTask.getComponentObjects();
//                this.parseTask = null;
//                completionBlock.run();
//            }
//        }
//        MyRunnable runnable = new MyRunnable();
//        runnable.setParseTask(parseTask);
//        parseTask.setPostRun(runnable);
//
//        Log.i("GeoJSONSource", "startParse(...) flag 2");
//        parseTask.execute(Long.valueOf(0));
//        Log.i("GeoJSONSource", "startParse(...) flag 3");
//    }

    public void startParse() {
        Log.i("GeoJSONSource", "startParse");
        startParse(new Runnable() {
            @Override
            public void run() {
            }
        });
    }

    native VectorObject[] parseData(String json);

    private class ParseTask extends AsyncTask<Long, Void, Long>
    {
        InputStream jsonStream;
        SLDStyleSet styleSet;
        MaplyBaseController baseController;
        Runnable postRun;
        VectorObject[] vecs;
        HashMap<String, ArrayList<VectorObject>> featureStyles = new HashMap<String, ArrayList<VectorObject>>();
        ArrayList<ComponentObject> componentObjects = new ArrayList<ComponentObject>();

        public ArrayList<ComponentObject> getComponentObjects() {
            return componentObjects;
        }

        public void setPostRun(Runnable postRun) {
            this.postRun = postRun;
        }

        ParseTask(InputStream jsonStream, SLDStyleSet styleSet, MaplyBaseController baseController)
        {
            this.jsonStream = jsonStream;
            this.styleSet = styleSet;
            this.baseController = baseController;
            vecs = null;
        }

        @Override
        protected Long doInBackground(Long... args)
        {
            try {
//                long size = jsonStream.getChannel().size();
//                byte[] jsonBytes = new byte[(int) size];
//                jsonStream.read(jsonBytes);

                Log.i("ParseTask", "doInBackground flag 1");
                ByteArrayOutputStream buffer = new ByteArrayOutputStream();

                int nRead;
                byte[] data = new byte[16384];

                while ((nRead = jsonStream.read(data, 0, data.length)) != -1) {
                    buffer.write(data, 0, nRead);
                }

                buffer.flush();
                Log.i("ParseTask", "doInBackground flag 2");

                //return buffer.toByteArray();

                //vecs = parseData(new String(jsonBytes));
                vecs = parseData(buffer.toString());

                Log.i("ParseTask", "doInBackground flag 3");

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
                Log.i("ParseTask", "doInBackground flag 4");


            } catch (Exception exception) {
            }
            return Long.valueOf(0);
        }

        @Override
        protected void onPostExecute(Long result) {
            if (vecs != null) {
                MaplyTileID nullTileID = new MaplyTileID(0, 0, 0);
                for (String uuid : featureStyles.keySet()) {
                    VectorStyle style = styleSet.styleForUUID(uuid, baseController);
                    ArrayList<VectorObject> featuresForStyle = featureStyles.get(uuid);
                    //List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller)
                    componentObjects.addAll(Arrays.asList(style.buildObjects(featuresForStyle, nullTileID, baseController)));
                }
            }
            baseController.enableObjects(componentObjects, MaplyBaseController.ThreadMode.ThreadAny);
            postRun.run();
        }
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
