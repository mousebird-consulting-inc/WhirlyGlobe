/*
 *  ParticleSystemTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 26/1/16
 *  Copyright 2011-2015 mousebird consulting
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
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import java.io.IOException;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;

public class ParticleSystemTestCase extends MaplyTestCase {

    public ParticleSystemTestCase(Activity activity) {
        super(activity);

        this.setTestName("Particle System Test");
        this.setDelay(20);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {

        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);
        ParticleTileDelegate particleTileDelegate = new ParticleTileDelegate("http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png", 5, 18, globeVC);
        QuadPagingLayer layer = new QuadPagingLayer(globeVC, particleTileDelegate.coordSys, particleTileDelegate);
        globeVC.addLayer(layer);
        return true;
    }

    private class ParticleTileDelegate implements QuadPagingLayer.PagingInterface {

        private String url;
        private ParticleSystem partSys;
        private MaplyBaseController viewC;
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
        private Runnable updateTask;
        private HashMap<String, DataTile> cachedTiles;
        private QuadPagingLayer layer;

        public ParticleTileDelegate(String url, int minZoom, int maxZoom, MaplyBaseController inViewC) throws InterruptedException {
            this.url = url;
            this.minZoom = minZoom;
            this.maxZoom = maxZoom;
            this.coordSys = new SphericalMercatorCoordSystem();
            this.viewC = inViewC;
            this.cachedTiles = new HashMap<>();

            //These govern how particles are structured
            this.updateInterval = 5.0;
            this.particleLifeTime = 10.0;
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
            this.partSys.setParticleSystemType(ParticleSystem.STATE.ParticleSystemPoint.getValue());
            this.partSys.setShaderID(0);
            this.partSys.setPointSize(4);
            this.partSys.addParticleSystemAttribute("a_position", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3.getValue());
            this.partSys.addParticleSystemAttribute("a_dir", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3.getValue());
            this.partSys.addParticleSystemAttribute("a_color", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT4.getValue());
            this.partSys.addParticleSystemAttribute("a_startTime", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_INT.getValue());

            this.locs = null;
            this.dirs = null;
            this.colors = null;
            this.updateTask = new Runnable() {
                @Override
                public void run() {
                    float delay = getDelay();
                    while (delay - updateInterval>0) {
                        try {
                            generateParticles();
                            delay -= updateInterval;
                            Thread.sleep((long) (updateInterval * 1000));
                            System.out.println("Updating...");
                        } catch (InterruptedException e) {

                        }
                    }
                }
            };
            this.updateTask.run();
        }

        public void generateParticles() throws InterruptedException {
            if (this.partSysObj == null) {
                this.partSys.setLifetime(this.particleLifeTime);
                this.partSys.setTotalParticles(this.numParticles);
                this.partSys.setDrawPriority(101000);
                this.partSys.setPointSize(4);
                this.partSys.setBatchSize((int) (this.numParticles / (this.particleLifeTime / this.updateInterval)));
                this.partSysObj = this.viewC.addParticleSystem(this.partSys, MaplyBaseController.ThreadMode.ThreadAny);
            }

            double now = ((double) new Date().getTime() ) - 978303600000.d;

            // Data arrays for particles
            int batchSize = this.partSys.getBatchSize();

            if (locs == null) {
                this.locs = new float[this.partSys.getBatchSize()*3];
                this.dirs = new float[this.partSys.getBatchSize()*3];
                this.colors = new float[this.partSys.getBatchSize()*4];
                this.times = new float[batchSize];
            }

            // Make up some random particles
            for (int ii = 0; ii< this.partSys.getBatchSize(); ii++) {
                //Random location
                locs[ii*3] = (float) (Math.random()*2-1); locs[ii*3+1] = (float) (Math.random()*2-1); locs[ii*3+2] = (float) (Math.random()*2-1);
                float sum = (float) Math.sqrt(locs[ii*3]*locs[ii*3] + locs[ii*3+1] * locs[ii*3+1] + locs[ii*3+2] * locs[ii*3+2]);
                locs[ii*3]/= sum; locs[ii*3+1] /= sum; locs[ii*3+2] /= sum;
                //Random direction
                dirs[ii*3] = (float) Math.random()*2-1; dirs[ii*3+1] = (float) Math.random()*2-1; dirs[ii*3+2] = (float) Math.random()*2-1;
                sum  = (float) Math.sqrt(dirs[ii*3]*dirs[ii*3] + dirs[ii*3+1] * dirs[ii*3+1] + dirs[ii*3+2] * dirs[ii*3+2]);
                dirs[ii*3] /= sum * this.velocityScale; dirs[ii*3+1] /= sum * this.velocityScale; dirs[ii*3+2] /= sum * this.velocityScale;

                this.times[ii] = 0;

                colors[ii*4] = 1.0f; colors[ii*4+1] = 1.0f; colors[ii*4+2] = 1.0f; colors[ii*4+3] = 1.0f;
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