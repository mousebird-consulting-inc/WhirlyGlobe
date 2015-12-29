package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;

/**
 * Created by jmnavarro on 27/12/15.
 */
public class MapBoxSatelliteTestCase extends MaplyTestCase {

	public MapBoxSatelliteTestCase(Activity activity) {
		super(activity);

		this.setTestName("MapBox Satellite Test");
		this.setSelected(ConfigOptions.getSelectedTest(activity, getTestName()));
	}

	private QuadImageTileLayer setupImageLayer(ConfigOptions.TestType testType, MaplyBaseController baseController) throws Exception {
		String cacheDirName = "mapbox_satellite";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();
		RemoteTileSource remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2/", "png", 0, 22));
		remoteTileSource.setCacheDir(cacheDir);
		SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
		QuadImageTileLayer baseLayer = new QuadImageTileLayer(baseController, coordSystem, remoteTileSource);
		baseLayer.setSimultaneousFetches(1);
		baseLayer.setImageDepth(1);
		baseLayer.setMaxTiles(256);

		baseLayer.setSingleLevelLoading(testType == ConfigOptions.TestType.MapTest);
		baseLayer.setUseTargetZoomLevel(testType == ConfigOptions.TestType.MapTest);
		baseLayer.setCoverPoles(testType == ConfigOptions.TestType.MapTest);
		baseLayer.setHandleEdges(testType == ConfigOptions.TestType.MapTest);

		baseLayer.setDrawPriority(100);
		return baseLayer;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		globeVC.addLayer(this.setupImageLayer(ConfigOptions.TestType.GlobeTest, globeVC));
		globeVC.animatePositionGeo(-3.6704803, 40.5023056, 2.0, 1.0);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		mapVC.addLayer(this.setupImageLayer(ConfigOptions.TestType.MapTest, mapVC));
		mapVC.setPositionGeo(-3.6704803, 40.5023056, 2.0);
		return true;
	}
}
