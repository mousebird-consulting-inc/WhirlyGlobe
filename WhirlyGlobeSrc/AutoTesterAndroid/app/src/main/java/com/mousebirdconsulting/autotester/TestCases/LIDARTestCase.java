package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import com.mousebird.maply.ColorRampGenerator;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LAZQuadReader;
import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.Shader;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Display a small LIDAR data set
 */

public class LIDARTestCase extends MaplyTestCase {
    private static String TAG = "AutoTester";

    private Activity activity;

    public LIDARTestCase(Activity activity) {
        super(activity);
        setTestName("LIDAR Test Case");
        setDelay(1000);
        this.implementation = TestExecutionImplementation.Globe;

        this.activity = activity;
    }

    // Maximum number of points to display at once
    final int MaxDisplayedPoints = 3000000;

    // Bitmap used for ramp shader
    Bitmap colorRamp;

    // Shaders
    Shader pointShaderColor;
    Shader pointShaderRamp;

    // Build a color ramp for use in one of the shaders
    Bitmap buildColorRamp()
    {
        ColorRampGenerator rampGen = new ColorRampGenerator();
        rampGen.addHexColor(0x5e03e1);
        rampGen.addHexColor(0x2900fb);
        rampGen.addHexColor(0x0053f8);
        rampGen.addHexColor(0x02fdef);
        rampGen.addHexColor(0x00fe4f);
        rampGen.addHexColor(0x33ff00);
        rampGen.addHexColor(0xefff01);
        rampGen.addHexColor(0xfdb600);
        rampGen.addHexColor(0xff6301);
        rampGen.addHexColor(0xf01a0a);

        return rampGen.makeImage(256,1);
    }

    String vertexShaderTriPoint =
            "uniform mat4  u_mvpMatrix;\n" +
                    "uniform float     u_pointSize;\n" +
                    "\n" +
                    "attribute vec3 a_position;\n" +
                    "attribute vec4 a_color;\n" +
//            "attribute float a_size;\n" +
                    "\n" +
                    "varying vec4 v_color;\n" +
                    "\n" +
                    "void main()\n" +
                    "{\n" +
                    "   v_color = a_color;\n" +
//                   "   v_color = vec4(1.0,0.0,0.0,1.0);\n" +
                    "   gl_PointSize = u_pointSize;\n" +
                    "   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n" +
                    "}\n"
            ;

    String fragmentShaderTriPoint =
            "precision mediump float;\n" +
                    "\n" +
                    "varying vec4      v_color;\n" +
                    "\n" +
                    "void main()\n" +
                    "{\n" +
                    "  gl_FragColor = v_color;\n" +
                    "}\n"
            ;

    String vertexShaderTriPointRamp =
            "uniform mat4  u_mvpMatrix;\n" +
                    "\n" +
                    "uniform float     u_zmin;\n" +
                    "uniform float     u_zmax;\n" +
                    "uniform float     u_pointSize;\n" +
                    "\n" +
                    "attribute vec3 a_position;\n" +
//            "attribute float a_size;\n" +
                    "attribute float a_elev;\n" +
                    "\n" +
                    "varying float v_colorPos;\n" +
                    "\n" +
                    "void main()\n" +
                    "{\n" +
//                    "   v_colorPos = (a_elev-u_zmin)/(u_zmax-u_zmin);\n" +
                    "   v_colorPos = 0.01;\n" +
                    "\n" +
                    "   gl_PointSize = u_pointSize;\n" +
                    "   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n" +
                    "}\n"
            ;

    String fragmentShaderTriPointRamp =
            "precision mediump float;\n" +
                    "\n" +
                    "uniform sampler2D s_colorRamp;\n" +
                    "\n" +
                    "varying float v_colorPos;\n" +
                    "\n" +
                    "void main()\n" +
                    "{\n" +
//                    "  vec4 color = texture2D(s_colorRamp,vec2(v_colorPos,0.5));\n" +
                    "  vec4 color = texture2D(s_colorRamp,vec2(0.5,0.5));\n" +
                    "  gl_FragColor = color;\n" +
                    "}\n"
            ;

    // Set up the point shader
    Shader buildPointShader(GlobeController globeControl)
    {
        Shader shader = new Shader("Lidar Point Shader",vertexShaderTriPoint,fragmentShaderTriPoint,globeControl);
        if (!shader.valid())
        {
            Log.e("LidarViewer","Failed to set up point shader");
            return null;
        }
        globeControl.addShaderProgram(shader,"Lidar Point Shader");

        return shader;
    }

    // Set up ramp shader
    Shader buildRampShader(GlobeController globeControl)
    {
        colorRamp = buildColorRamp();

        Shader shader = new Shader("Lidar Ramp Shader",vertexShaderTriPointRamp,fragmentShaderTriPointRamp,globeControl);
        if (!shader.valid())
        {
            Log.e("LidarViewer","Failed to set up ramp shader");
            return null;
        }
        globeControl.addShaderProgram(shader,"Lidar Ramp Shader");

        // Note: Debugging
        colorRamp = BitmapFactory.decodeResource(getActivity().getResources(),
                R.drawable.colorramp);

        MaplyTexture colorTex = globeControl.addTexture(colorRamp,new MaplyBaseController.TextureSettings(), MaplyBaseController.ThreadMode.ThreadCurrent);
        shader.addTexture("s_colorRamp",colorTex);


        return shader;
    }

    // Add a single LAZ file for paging
    void addLAZFile(GlobeController globeControl,String fileName)
    {
        try {
            // Open up the file
            File dbFile = getSQLiteFile(fileName + ".sqlite", fileName + ".sqlite");
//            File dbFile = new File("/storage/self/primary/" + fileName + ".sqlite");
            LAZQuadReader.Settings settings = new LAZQuadReader.Settings();
//            settings.colorScale = 1<<16-1;
            settings.colorScale = 255;
            settings.pointSize = 12.f;
            settings.zOffset = 2.f;
            LAZQuadReader quadReader = new LAZQuadReader(dbFile, settings, globeControl);
            if (quadReader.hasColor())
            {
                quadReader.setShader(pointShaderColor);
            } else {
                quadReader.setShader(pointShaderRamp);
            }
            Point3d ll = quadReader.getLL();
            Point3d ur = quadReader.getUR();

            Point2d locCenter = quadReader.getCenter();
            Point3d center = quadReader.coordSys.localToGeographic(new Point3d(locCenter.getX(),locCenter.getY(),0.0));

            // Set up a paging layer
            QuadPagingLayer pagingLayer = new QuadPagingLayer(globeControl,quadReader.coordSys,quadReader);
            pagingLayer.setSimultaneousFetches(4);
            pagingLayer.setMaxTiles(quadReader.getNumTilesFromMaxPoints(MaxDisplayedPoints));
            pagingLayer.setImportance(128*128);
            pagingLayer.setTileHeightRange(ll.getZ(),ur.getZ());
            pagingLayer.setUseParentTileBounds(false);
            pagingLayer.setSingleLevelLoading(false);
            globeControl.addLayer(pagingLayer);

            // Drop a label in the middle so we can find it
            ScreenLabel label = new ScreenLabel();
            label.text = fileName;
            label.loc = new Point2d(center.getX(),center.getY());
            LabelInfo labelInfo = new LabelInfo();
            labelInfo.setLayoutImportance(Float.MAX_VALUE);
            labelInfo.setMinVis(0.1f);
            labelInfo.setMaxVis(10.0f);

            globeControl.addScreenLabel(label,labelInfo, MaplyBaseController.ThreadMode.ThreadAny);
        }
        catch (Exception e)
        {
            Log.e("LidarViewer","Failed to open Sqlite db: " + e.toString());
        }
    }

    // Copy a file out of the bundle
    private File getSQLiteFile(String assetMbTile, String mbTileFilename) throws IOException {

        ContextWrapper wrapper = new ContextWrapper(getActivity());
        File mbTilesDirectory =  wrapper.getDir("lazdbs", Context.MODE_PRIVATE);

        InputStream is = getActivity().getAssets().open(assetMbTile);
        File of = new File(mbTilesDirectory, mbTileFilename);

        if (of.exists()) {
            return of;
        }

        OutputStream os = new FileOutputStream(of);
        byte[] mBuffer = new byte[1024];
        int length;
        while ((length = is.read(mBuffer))>0) {
            os.write(mBuffer, 0, length);
        }
        os.flush();
        os.close();
        is.close();

        return of;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {

        // setup base layer tiles
        String cacheDirName = "carto_db";
        File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
        cacheDir.mkdir();
        RemoteTileSource remoteTileSource = new RemoteTileSource(globeVC, new RemoteTileInfo("http://light_all.basemaps.cartocdn.com/light_all/", "png", 0, 22));
        remoteTileSource.setCacheDir(cacheDir);
        SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();

        // globeControl is the controller when using MapDisplayType.Globe
        // mapControl is the controller when using MapDisplayType.Map
        QuadImageTileLayer baseLayer = new QuadImageTileLayer(globeVC, coordSystem, remoteTileSource);
        baseLayer.setImageDepth(1);
        baseLayer.setSingleLevelLoading(false);
        baseLayer.setUseTargetZoomLevel(false);
        baseLayer.setCoverPoles(true);
        baseLayer.setHandleEdges(true);

        // add layer and position
        globeVC.addLayer(baseLayer);
        globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);

        // Set up the shaders
        pointShaderColor = buildPointShader(globeVC);
        pointShaderRamp = buildRampShader(globeVC);

        // Now let's set up a LAZ pager
//        addLAZFile("stadium-utm-quad-data");
//        addLAZFile("SanSimeon_big");
//        addLAZFile("st-helens_new_quad_data");
        addLAZFile(globeVC,"stadium-utm-quad-data");
//        addLAZFile("44123A1305_ALL-quad-data");

        globeVC.setKeepNorthUp(false);
        globeVC.setAllowTilt(true);
        globeVC.setPositionGeo(-2.1478445529937744, 0.7689281105995178, 0.001);

        return true;
    }

}