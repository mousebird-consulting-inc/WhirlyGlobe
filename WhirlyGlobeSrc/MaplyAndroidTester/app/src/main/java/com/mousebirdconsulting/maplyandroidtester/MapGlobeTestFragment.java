package com.mousebirdconsulting.maplyandroidtester;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MultiplexTileSource;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.TestImageSource;
import com.mousebird.maply.TestQuadPager;

import java.io.File;

/**
 * This fragment manages the map (globe), adding and removing content as the user requests.
 * 
 */
public class MapGlobeTestFragment extends Fragment implements ConfigOptions.ConfigOptionsListener
{
	MapController mapControl = null;
	GlobeController globeControl = null;
	MaplyBaseController baseControl = null;

	/**
	 * Set the fragment test mode before onCreateView is called.
	 *
	 */
    ConfigOptions.MapType mode = ConfigOptions.MapType.GlobeMap;

    View topView = null;
			
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState)
	{
		// This means we've already been created once
		if (baseControl != null)
		{
			return baseControl.getContentView();
		}

    	// Create the Maply Controller
		View theView = null;
		switch (mode)
		{
            case FlatMap:
                mapControl = new MapController(getActivity());
                baseControl = mapControl;
			break;
            case GlobeMap:
                globeControl = new GlobeController(getActivity());
                globeControl.setPositionGeo(0, 0, 2.0);
                baseControl = globeControl;
			break;
		}
		theView = baseControl.getContentView();

		// This should keep the Fragment around for configuration changes
//		setRetainInstance(true);

		return baseControl.getContentView();
	}
	
	@Override
	public void onDestroy()
	{
		super.onDestroy();
	}
	
	QuadImageTileLayer baseLayer = null;
	QuadImageTileLayer forecastIOLayer = null;
	
	// Set up a simple image layer
	QuadImageTileLayer setupImageLayer(QuadImageTileLayer.TileSource tileSource,RemoteTileSource remoteTileSource,String cacheDirName,int imageDepth)
	{
		// Set up the layer
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		baseLayer = new QuadImageTileLayer(baseControl,coordSys,tileSource);
		baseLayer.setSimultaneousFetches(1);
        baseLayer.setImageDepth(imageDepth);

        // Note: Debugging
        if (imageDepth > 2) {
            baseLayer.setCurrentImage(1.5f);
            baseLayer.setImageFormat(QuadImageTileLayer.ImageFormat.MaplyImageUShort565);
            baseLayer.setShaderName(multiTexShader.getName());
        }
		
		if (mapControl != null)
		{
			baseLayer.setSingleLevelLoading(true);
			baseLayer.setUseTargetZoomLevel(true);
			baseLayer.setCoverPoles(false);
			baseLayer.setHandleEdges(false);
		} else {
			baseLayer.setSingleLevelLoading(false);
			baseLayer.setUseTargetZoomLevel(false);
			baseLayer.setCoverPoles(true);
			baseLayer.setHandleEdges(true);
		}

		// Cache directory for tiles
		if (remoteTileSource != null && cacheDirName != null)
		{
			File cacheDir = new File(getActivity().getCacheDir(),cacheDirName);
			cacheDir.mkdir();
			remoteTileSource.setCacheDir(cacheDir);
		}
		
		return baseLayer;
	}

    // Test shader
    TestShader multiTexShader = null;

	// Called when the user changes what is selected
	public void userChangedSelections(ConfigOptions config)
	{
		String cacheDirName = null;
		QuadImageTileLayer.TileSource tileSource = null;
		RemoteTileSource remoteTileSource = null;
        int imageDepth = 1;

		// Get rid of the existing base layer
		if (baseLayer != null)
			baseControl.removeLayer(baseLayer);
		
		baseControl.setPerfInterval(0);

		// Base layers
        switch (config.baseSection)
        {
            case BlankLayer:
                break;
            case GeographyClass:
                // Note: Put MBTiles in resources
                break;
            case MapboxRegular:
//					cacheDirName = "mapbox_regular";
//					tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zswgei2n/","png",0,22);
                break;
            case MapboxSatellite:
                cacheDirName = "mapbox_satellite";
                remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2/","png",0,22));
                break;
            case OSMMapquest:
                cacheDirName = "osm_mapquest";
                remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://otile1.mqcdn.com/tiles/1.0.0/osm/","png",0,18));
                break;
            case StamenWatercolor:
                cacheDirName = "stamen_watercolor";
                remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://tile.stamen.com/watercolor/","png",0,18));
                break;
            case QuadTest:
                tileSource = new TestImageSource(getActivity().getMainLooper(),0,22);
                break;
            case QuadTestAnimate:
                tileSource = new TestImageSource(getActivity().getMainLooper(),0,22);
                imageDepth = 4;
                if (multiTexShader == null) {
                    multiTexShader = new TestShader(baseControl);
                    baseControl.addShaderProgram(multiTexShader, multiTexShader.getName());
                }
                break;
            case QuadVectorTest:
            {
                TestQuadPager testPager = new TestQuadPager(0,22);
                SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
                QuadPagingLayer pagingLayer = new QuadPagingLayer(mapControl,coordSys,testPager);
                pagingLayer.setSimultaneousFetches(6);
                pagingLayer.setImportance(256*256);
                pagingLayer.setSingleLevelLoading(true);
                pagingLayer.setUseTargetZoomLevel(true);
                mapControl.addLayer(pagingLayer);
            }
                break;
        }

		// New base layer
		if (remoteTileSource != null)
			tileSource = remoteTileSource;

		if (tileSource != null)
		{
			baseLayer = setupImageLayer(tileSource,remoteTileSource,cacheDirName,imageDepth);
			baseControl.addLayer(baseLayer);		
		}

        // Overlay layers
        if (config.overlays[0])
        {
            if (forecastIOLayer == null)
            {
                RemoteTileInfo[] sources = new RemoteTileInfo[5];
                for (int ii=0;ii<5;ii++)
                    sources[ii] = new RemoteTileInfo("http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer" + ii + "/","png",0,6);
                cacheDirName = "forecastio";
                SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
                MultiplexTileSource multiTileSource = new MultiplexTileSource(sources,coordSys);

                forecastIOLayer = new QuadImageTileLayer(baseControl,coordSys,multiTileSource);
                forecastIOLayer.setSimultaneousFetches(1);
                forecastIOLayer.setDrawPriority(MaplyBaseController.ImageLayerDrawPriorityDefault+100);
                forecastIOLayer.setBorderTexel(0);
                forecastIOLayer.setCurrentImage(2.5f);
//						forecastIOLayer.setImageFormat(QuadImageTileLayer.ImageFormat.MaplyImageUByteRed);

                if (mapControl != null)
                {
                    forecastIOLayer.setSingleLevelLoading(true);
                    forecastIOLayer.setUseTargetZoomLevel(true);
                    forecastIOLayer.setCoverPoles(false);
                    forecastIOLayer.setHandleEdges(false);
                } else {
                    forecastIOLayer.setSingleLevelLoading(false);
                    forecastIOLayer.setUseTargetZoomLevel(false);
                    forecastIOLayer.setCoverPoles(true);
                    forecastIOLayer.setHandleEdges(true);
                }

                // Cache directory for tiles
                if (cacheDirName != null)
                {
                    File cacheDir = new File(getActivity().getCacheDir(),cacheDirName);
                    cacheDir.mkdir();
                    multiTileSource.setCacheDir(cacheDir);
                }
                baseControl.addLayer(forecastIOLayer);
            }
        } else {
            if (forecastIOLayer != null)
                baseControl.removeLayer(forecastIOLayer);
        }
    }
}
