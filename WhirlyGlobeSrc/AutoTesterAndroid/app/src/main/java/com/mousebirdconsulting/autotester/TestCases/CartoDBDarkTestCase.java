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
public class CartoDBDarkTestCase extends MaplyTestCase {

	public CartoDBDarkTestCase(Activity activity) {
		super(activity);

		setTestName("CartoDB Dark Matter Test");
		setDelay(20);
	}

	private QuadImageTileLayer setupImageLayer(ConfigOptions.TestType testType, MaplyBaseController baseController) throws Exception {
		String cacheDirName = "cartodb_dark";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();
		RemoteTileSource remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://dark_all.basemaps.cartocdn.com/dark_all/", "png", 0, 22));
		remoteTileSource.setCacheDir(cacheDir);
		SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
		QuadImageTileLayer baseLayer = new QuadImageTileLayer(baseController, coordSystem, remoteTileSource);

		baseLayer.setSingleLevelLoading(testType == ConfigOptions.TestType.MapTest);
		baseLayer.setUseTargetZoomLevel(testType == ConfigOptions.TestType.MapTest);
		baseLayer.setCoverPoles(testType != ConfigOptions.TestType.MapTest);
		baseLayer.setHandleEdges(testType != ConfigOptions.TestType.MapTest);

		baseLayer.setDrawPriority(MaplyBaseController.ImageLayerDrawPriorityDefault);
		return baseLayer;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
//		globeVC.setKeepNorthUp(false);
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
