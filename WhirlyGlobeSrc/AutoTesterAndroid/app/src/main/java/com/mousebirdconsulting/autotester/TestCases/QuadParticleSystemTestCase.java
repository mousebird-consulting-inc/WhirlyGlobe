/*
 *  QuadParticleSystemTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2014 mousebird consulting
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
package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.LayerDrawable;
import android.os.Handler;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.QuadTracker;
import com.mousebird.maply.QuadTrackerPointReturn;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import java.io.IOException;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;


public class QuadParticleSystemTestCase extends MaplyTestCase {
    public QuadParticleSystemTestCase(Activity activity) {
        super(activity);
        setDelay(20);
        setTestName("Quad Particle System Test");
    }

    LayerThread particleThread = null;

    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {

        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        globeVC.onSurfaceCreatedTask(new Runnable() {
            @Override
            public void run() {
                particleThread = globeVC.makeLayerThread();
                ParticleTileDelegate delegate = null;
                try {
                    delegate = new ParticleTileDelegate("http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png", 5, 18, globeVC, particleThread);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                QuadPagingLayer layer = new QuadPagingLayer(globeVC, delegate.coordSys, delegate);
                globeVC.addLayer(layer);
            }
        });
        return true;
    }

    private class ParticleTileDelegate implements QuadPagingLayer.PagingInterface {

        private String url;
        private ParticleSystem partSys;
        private GlobeController viewC;
        private ComponentObject partSysObj;
        private float locs[];
        private float dirs[];
        private float colors[];
        private float[] times;
        private float velocityScale;
        private int numVelocityColors;
        private float velocityColors[][];
        private int minZoom, maxZoom;
        private double updateInterval;
        private double particleLifeTime;
        private int numParticles;
        private SphericalMercatorCoordSystem coordSys;
        private HashMap<String, DataTile> cachedTiles;
        private QuadPagingLayer layer;
        private Runnable updateTask;
        private LayerThread particleThread = null;
        private QuadTracker tileTrack;

        public ParticleTileDelegate(String url, int minZoom, int maxZoom, GlobeController inViewC, LayerThread thread) throws InterruptedException {
            this.url = url;
            this.minZoom = minZoom;
            this.maxZoom = maxZoom;
            this.coordSys = new SphericalMercatorCoordSystem();
            this.viewC = inViewC;
            particleThread = thread;
            this.cachedTiles = new HashMap<>();

            //These govern how particles are structured
            this.updateInterval = 0.1;
            this.particleLifeTime = 4.0;
            this.numParticles = 100000;
            this.velocityScale = 0.1f;

            // Colors we'll use
            this.velocityColors = new float[3][4];
            //Color 1
            this.velocityColors[0][0] = 0.6f; //red
            this.velocityColors[0][1] = 1.0f; //green
            this.velocityColors[0][2] = 0.6f; //blue
            this.velocityColors[0][3] = 1.f; //alpha
            //Color 2
            this.velocityColors[1][0] = 0.6f; //red
            this.velocityColors[1][1] = 0.6f; //green
            this.velocityColors[1][2] = 1.f; //blue
            this.velocityColors[1][3] = 1.f; //alpha
            //Color 3
            this.velocityColors[2][0] = 1.f; //red
            this.velocityColors[2][1] = 0.6f; //green
            this.velocityColors[2][2] = 0.6f; //blue
            this.velocityColors[2][3] = 1.f; //alpha

            // Set up the particle system we'll feed with particles

            this.partSys = new ParticleSystem("Particle Wind Test");
            this.partSys.setLifetime(this.particleLifeTime);
            this.partSys.setTotalParticles(this.numParticles);
            this.partSys.setDrawPriority(101000);
            this.partSys.setPointSize(8.f);
            this.partSys.setBatchSize((int) (this.numParticles / (this.particleLifeTime / this.updateInterval)));
            this.partSys.addParticleSystemAttribute("a_position", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
            this.partSys.addParticleSystemAttribute("a_dir", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
            this.partSys.addParticleSystemAttribute("a_color", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT4);
            this.partSys.addParticleSystemAttribute("a_startTime", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT);

            this.partSysObj = this.viewC.addParticleSystem(this.partSys, MaplyBaseController.ThreadMode.ThreadCurrent);


            tileTrack = new QuadTracker(viewC);

            tileTrack.setMinLevel(minZoom);
            Point2d ll = new Point2d();
            Point2d ur = new Point2d();
            ll.setValue(coordSys.ll.getX(), coordSys.ll.getY());
            ur.setValue(coordSys.ur.getX(), coordSys.ur.getY());
            tileTrack.setCoordSystem(coordSys, ll, ur);

            this.locs = null;
            this.dirs = null;
            this.colors = null;

            scheduleUpdateTask();
        }

        void scheduleUpdateTask()
        {
            Runnable run = new Runnable() {
                @Override
                public void run() {
                    try {
                        generateParticles();
                    }
                    catch (Exception e)
                    {
                    }

                    scheduleUpdateTask();
                }
            };
            Handler handler = new Handler(particleThread.getLooper());
            handler.postDelayed(run, (long)(updateInterval*1000));
        }

        public void generateParticles() throws InterruptedException {

            double now = ((double) new Date().getTime()/1000.0 ) - partSys.getBasetime();

            //double now = ((double) new Date().getTime() ) - 978303600000.d;

            // Data arrays for particles
            if (locs == null) {
                this.locs = new float[this.partSys.getBatchSize()*3];
                this.dirs = new float[this.partSys.getBatchSize()*3];
                this.colors = new float[this.partSys.getBatchSize()*4];
                this.times = new float[this.partSys.getBatchSize()];
                for (int ii = 0; ii < this.partSys.getBatchSize(); ii++){
                    this.times[ii] = 0;
                }
            }

            //Generate some screen coordinates for sampling

            QuadTrackerPointReturn points [] = new QuadTrackerPointReturn[this.partSys.getBatchSize()];

            for (int ii = 0; ii < this.partSys.getBatchSize(); ii++)
            {
                points[ii] = new QuadTrackerPointReturn();
                points[ii].setScreenU(Math.random());
                points[ii].setScreenV(Math.random());
            }

            //Figure out which the samples show up in a tile on the earth
            //We do it this way so we're not wasting particles on parts of the globe that aren't visible

            int datos[] = points[0].getMaplyTileID();
            points = tileTrack.tiles(points, this.partSys.getBatchSize());

            //Generate particles from those samples

            int whichPart = 0;

            for (int ii = 0; ii < this.partSys.getBatchSize(); ii++){

                int data[] = points[ii].getMaplyTileID();
                Log.e("DEBUG", "Valores "+data[0]+" "+data[1]+" "+data[2]);
                MaplyTileID tileID = new MaplyTileID(data[0], data[1], data[2]);

                //Look the associated tile
                DataTile dataTile = getDataTile(tileID);

                if (dataTile != null){
                    float time = this.times[ii];
                    Point3d coordA = new Point3d();
                    coordA.setValue(points[ii].getLocX(), points[ii].getLocY(), 0.0);

                    double velU =  0.0;
                    double velV =  0.0;
                    Point2d pt = new Point2d();
                    pt.setValue(points[ii].getTileU(), points[ii].getTileV());
                    double result [] = dataTile.getValue(pt);

                    if (result != null){
                        //There are a lot of empty values in the data so we'll skip those
                        if (result[0] != 0 || result[1] != 0){
                            velU = (result[0] - 127.0)/128.0;
                            velV = (result[1] - 127.0)/128.0;
                            double vel = Math.sqrt(velU * velU + velV * velV)/1.41421356237 * 3;

                            Point3d coordB = new Point3d();
                            coordB.setValue(coordA.getX() + velU * velocityScale, coordA.getY() + velV *velocityScale, 0.0);

                            //Convert to display coordinates


                            Point3d dispA = viewC.displayCoord(coordA, coordSys);
                            Point3d dispB = viewC.displayCoord(coordB, coordSys);
                            Point3d calcDir = new Point3d(dispB.getX() - dispA.getX(), dispB.getY() -dispA.getY(), dispB.getZ() - dispA.getZ());

                            this.locs[whichPart*ii] = (float) dispA.getX();
                            this.locs[whichPart*ii +1] = (float) dispA.getY();
                            this.locs[whichPart*ii +2] = (float) dispA.getZ();

                            this.dirs[whichPart*ii] = (float) calcDir.getX();
                            this.dirs[whichPart*ii +1] = (float) calcDir.getY();
                            this.dirs[whichPart*ii +2] = (float) calcDir.getZ();

                            //Calculate a color based on the velocity
                            float color[] = colorForVel((float)vel);

                            this.colors[whichPart*ii] = color[0];
                            this.colors[whichPart*ii+1] = color[1];
                            this.colors[whichPart*ii+2] = color[2];
                            this.colors[whichPart*ii+3] = color[3];

                            whichPart++;
                        }
                    }
                }
            }

            ParticleBatch batch = new ParticleBatch(this.partSys);
            batch.addAttribute("a_position", this.locs);
            batch.addAttribute("a_dir", this.dirs);
            batch.addAttribute("a_color", this.colors);
            batch.addAttribute("a_startTime", this.times);
            viewC.addParticleBatch(batch, MaplyBaseController.ThreadMode.ThreadAny);
        }

        //Interpolate a color based on the velocity of the particle
        public float[] colorForVel(float vel) {
            vel = Math.max(0.f, vel);
            vel = Math.min(1.f, vel);

            float [] color1, color2;
            if (vel <0.5) {
                color1 = this.velocityColors[0];
                color2 = this.velocityColors[1];
            }
            else {
                color1 = this.velocityColors[1];
                color2 = this.velocityColors[2];
            }
            float [] color = new float[4];
            color[0] = (color1[0] - color2[0]) *vel + color1[0]; //r
            color[1] = (color1[1] - color2[1]) *vel + color1[1]; //g
            color[2] = (color1[2] - color2[2]) *vel + color1[2]; //b
            color[3] = (color1[3] - color2[3]) *vel + color1[3]; //a

            return color;
        }

        @Override
        public int minZoom() {
            return this.minZoom;
        }

        @Override
        public int maxZoom() {
            return this.maxZoom;
        }

        @Override
        public void startFetchForTile(QuadPagingLayer quadPagingLayer, MaplyTileID maplyTileID) {
            DataTile tile = this.getDataTile(maplyTileID);
            if (tile == null) {
                tile = new DataTile();
                tile.tileID = maplyTileID;
                this.addDataTile(tile);
            }

            for (int ii = 0; ii < 2; ii++) {
                String uOrv = (ii==0 ? "u" : "v");
                String xStr = String.valueOf(maplyTileID.x);
                String yStr = String.valueOf(maplyTileID.y);
                String zStr = String.valueOf(maplyTileID.level);
                String newUrl = this.url;
                newUrl = newUrl.replace("{dir}", uOrv).replace("{z}", zStr).replace("{y}", yStr).replace("{x}", xStr);
                ConnectionTask task = new ConnectionTask(newUrl, this, quadPagingLayer, maplyTileID, ii);
                task.fetchTile();
            }
        }

        private class ConnectionTask implements com.squareup.okhttp.Callback{

            private URL url = null;
            private ParticleTileDelegate tileDelegate = null;
            private Bitmap bm = null;
            private com.squareup.okhttp.Call call = null;
            private MaplyTileID tileID = null;
            private QuadPagingLayer layer = null;
            private OkHttpClient client = new OkHttpClient();
            private int inWhitch = -1;

            ConnectionTask(String inUrl, ParticleTileDelegate tileDelegate, QuadPagingLayer inLayer, MaplyTileID inTileID, int inWhitch) {
                this.tileDelegate = tileDelegate;
                this.layer = inLayer;
                this.tileID = inTileID;
                this.inWhitch = inWhitch;

                try {
                    this.url = new URL(inUrl);
                }
                catch (IOException e) {
                }
            }

            protected void fetchTile() {
                // Load the data from that URL
                Request request = new Request.Builder().url(this.url).build();

                this.call = client.newCall(request);
                call.enqueue(this);
            }

            private void reportTile(int witch) {
                DataTile tile = this.tileDelegate.getDataTile(tileID);

                //Happens if the tile is removed before the request comes back
                if (tile == null)
                    return;

                tile.setImage(bm, witch);
                if (tile.isComplete()) {
                    layer.tileDidLoad(tileID);
                }

            }

            @Override
            public void onFailure(Request request, IOException e) {
                this.tileDelegate.clearTile(tileID);
                layer.tileFailedToLoad(tileID);
                Log.e("Maply", "Failed to fetch remote tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
            }

            @Override
            public void onResponse(Response response) throws IOException {
                if (response.isSuccessful()) {
                    byte[] rawImage = null;

                    try {
                        rawImage = response.body().bytes();
                        bm = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length);

                    }
                    catch (Exception e) {
                        Log.e("Maply", "Failed to fetch remote tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
                    }
                    reportTile(this.inWhitch);
                }
                else {
                    this.tileDelegate.clearTile(tileID);
                    layer.tileFailedToLoad(tileID);
                }

            }
        }
        public DataTile getDataTile(MaplyTileID tileID) {
            return this.cachedTiles.get(tileID.toString());
        }

        public void clearTile(MaplyTileID tileID) {
            this.cachedTiles.remove(tileID.toString());
        }

        public void addDataTile(DataTile tile) {
            this.cachedTiles.put(tile.tileID.toString(), tile);
        }
    }

    private class DataTile {

        private Bitmap tiles [];
        private int pixSizeX, pixSizeY;
        private MaplyTileID tileID;

        public DataTile() {
            this.tiles = new Bitmap[2];
            this.tiles[0] = null;
            this.tiles[1] = null;
        }

        public boolean isComplete() {
            for (int ii=0 ; ii < 2; ii++) {
                if (this.tiles[ii] == null) {
                    return false;
                }
            }
            return true;
        }

        public void setImage(Bitmap image, int which) {
            this.pixSizeX  = image.getWidth();
            this.pixSizeY = image.getHeight();
            this.tiles[which] = image;
        }

        public double[] getValue(Point2d pt) {
            if (!this.isComplete()) {
                return null;
            }
            double x = pt.getX();
            double y = pt.getY();

            y = 1.0 -y;
            int whereX = (int)x *this.pixSizeX;
            int whereY = (int)y *this.pixSizeY;
            whereX = Math.max(0, whereX);
            whereY = Math.max(0, whereY);
            whereX = Math.min(255, whereX);
            whereY = Math.min(255, whereY);
            double[] result = {whereX, whereY};
            return result;
        }

        @Override
        public String toString() {
            return tileID.x+"_"+tileID.y+"_"+tileID.level;
        }

        @Override
        public int hashCode() {
            return this.toString().hashCode();
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == null) {   return false;  }
            if (getClass() != obj.getClass()) {  return false;   }
            final DataTile other = (DataTile) obj;
            if (this.tileID.x != other.tileID.x) {  return false;    }
            if (this.tileID.y != other.tileID.y) {    return false;     }
            if (this.tileID.level != other.tileID.level) {     return false;   }
            return true;
        }
    }
}
