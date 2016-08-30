package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
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
public class CartoDBMapTestCase extends MaplyTestCase {

	public CartoDBMapTestCase(Activity activity) {
		super(activity);

		setTestName("CartoDB Light Test");
		setDelay(20);
		this.implementation = TestExecutionImplementation.Both;
	}

	private QuadImageTileLayer setupImageLayer(ConfigOptions.TestType testType, MaplyBaseController baseController) throws Exception {
		String cacheDirName = "cartodb_light";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();
		RemoteTileSource remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://light_all.basemaps.cartocdn.com/light_all/", "png", 0, 22));
		// Note: Turn this on to get more information from the tile source
//		remoteTileSource.debugOutput = true;
		remoteTileSource.setCacheDir(cacheDir);
		SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
		QuadImageTileLayer baseLayer = new QuadImageTileLayer(baseController, coordSystem, remoteTileSource);

		if (testType == ConfigOptions.TestType.MapTest)
		{
//			baseLayer.setSingleLevelLoading(true);
//			baseLayer.setUseTargetZoomLevel(true);
//			baseLayer.setMultiLevelLoads(new int[]{-3});
			baseLayer.setCoverPoles(false);
			baseLayer.setHandleEdges(false);
		} else {
			baseLayer.setCoverPoles(true);
			baseLayer.setHandleEdges(true);
		}

		baseLayer.setDrawPriority(MaplyBaseController.ImageLayerDrawPriorityDefault);
		return baseLayer;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
//		globeVC.setKeepNorthUp(false);
		globeVC.addLayer(this.setupImageLayer(ConfigOptions.TestType.GlobeTest, globeVC));
		Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
		globeVC.animatePositionGeo(loc.getX(), loc.getY(), 2.0, 1.0);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		mapVC.addLayer(this.setupImageLayer(ConfigOptions.TestType.MapTest, mapVC));
		Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
		mapVC.setPositionGeo(loc.getX(), loc.getY(), 2.0);
		return true;
	}
}
