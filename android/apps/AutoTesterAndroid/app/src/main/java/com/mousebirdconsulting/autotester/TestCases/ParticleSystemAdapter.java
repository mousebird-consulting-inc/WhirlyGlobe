package com.mousebirdconsulting.autotester.TestCases;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Handler;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import java.io.IOException;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;


public class ParticleSystemAdapter implements QuadPagingLayer.PagingInterface {

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
    private LayerThread particleThread;


    public ParticleSystemAdapter(GlobeController globeVC, LayerThread particleThread, String url, int minZoom, int maxZoom) {
        this.url = url;
        this.minZoom = minZoom;
        this.maxZoom = maxZoom;
        this.particleThread = particleThread;
        this.coordSys = new SphericalMercatorCoordSystem();
        this.viewC = globeVC;
        this.cachedTiles = new HashMap<>();

        //These govern how particles are structured
        this.updateInterval = 1.0;
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
        this.partSys.setParticleSystemType(ParticleSystem.STATE.ParticleSystemPoint);
        this.partSys.setPointSize(4);
        this.partSys.addParticleSystemAttribute("a_position", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
        this.partSys.addParticleSystemAttribute("a_dir", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
        this.partSys.addParticleSystemAttribute("a_color", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT4);
        this.partSys.addParticleSystemAttribute("a_startTime", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_INT);

        this.locs = null;
        this.dirs = null;
        this.colors = null;

        QuadPagingLayer layer = new QuadPagingLayer(globeVC, coordSys, this);
        globeVC.addLayer(layer);

        scheduleUpdateTask();
    }

    double startTime = 0.0;

    void scheduleUpdateTask()
    {
        if (startTime == 0.0)
            startTime = (double) new Date().getTime()/1000.0;

        Runnable run = new Runnable() {
            @Override
            public void run() {
                try {
                    generateParticles();
                    Thread.sleep((long) (updateInterval * 1000));
                    System.out.println("Updating...");
                } catch (InterruptedException e) {

                }
                scheduleUpdateTask();
            }
        };
        Handler handler = new Handler(particleThread.getLooper());
        handler.postDelayed(run, (long)(updateInterval*1000));
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
            float x = (float)Math.random()*2-1;
            float y = (float)Math.random()*2-1;
            float z = (float)Math.random()*2-1;
            float sum = (float)Math.sqrt(x*x+y*y+z*z);
            x /= sum;  y /= sum;  z /= sum;
            locs[ii*3] = x; locs[ii*3+1] = y; locs[ii*3+2] = z;
            //Random direction
            dirs[ii*3] = (float) Math.random()*2-1; dirs[ii*3+1] = (float) Math.random()*2-1; dirs[ii*3+2] = (float) Math.random()*2-1;
            //dirs[ii*3] = 0.f;  dirs[ii*3+1] = 0.f;  dirs[ii*3+2] = 0.f;

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
        private ParticleSystemAdapter adapter = null;
        private Bitmap bm = null;
        private com.squareup.okhttp.Call call = null;
        private MaplyTileID tileID = null;
        private QuadPagingLayer layer = null;
        private OkHttpClient client = new OkHttpClient();
        private int inWhitch = -1;

        ConnectionTask(String inUrl, ParticleSystemAdapter adapter, QuadPagingLayer inLayer, MaplyTileID inTileID, int inWhitch) {
            this.adapter = adapter;
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
            DataTile tile = this.adapter.getDataTile(tileID);

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
            this.adapter.clearTile(tileID);
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
                this.adapter.clearTile(tileID);
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

    @Override
    public void tileDidUnload(MaplyTileID maplyTileID) {

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

    @Override
    public void clear()
    {
    }
}
