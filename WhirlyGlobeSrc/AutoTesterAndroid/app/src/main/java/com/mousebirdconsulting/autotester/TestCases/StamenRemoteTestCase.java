package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;


public class StamenRemoteTestCase extends MaplyTestCase {

	public StamenRemoteTestCase(Activity activity) {
		super(activity);

		setTestName("Stamen Remote Test");
		setDelay(4);
		this.implementation = TestExecutionImplementation.Both;
	}

	private QuadImageTileLayer setupImageLayer(MaplyBaseController baseController) {

		String cacheDirName = "stamen_watercolor";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();
		RemoteTileSource remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://tile.stamen.com/watercolor/", "png", 0, 18));
		remoteTileSource.setCacheDir(cacheDir);
		SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
		QuadImageTileLayer baseLayer = new QuadImageTileLayer(baseController, coordSystem, remoteTileSource);
		baseLayer.setSimultaneousFetches(1);
		baseLayer.setImageDepth(1);
		baseLayer.setSingleLevelLoading(false);
		baseLayer.setUseTargetZoomLevel(false);
		baseLayer.setCoverPoles(true);
		baseLayer.setHandleEdges(true);
		return baseLayer;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		globeVC.addLayer(setupImageLayer(globeVC));
		globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC)
	{
		mapVC.addLayer(setupImageLayer(mapVC));
		mapVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
		return true;
	}
}
