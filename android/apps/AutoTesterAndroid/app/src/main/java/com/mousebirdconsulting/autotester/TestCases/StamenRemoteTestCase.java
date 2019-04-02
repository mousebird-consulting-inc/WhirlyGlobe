package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.os.Handler;

import com.mousebird.maply.BaseController;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.QuadImageLoader;
import com.mousebird.maply.RemoteTileInfoNew;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.SamplingParams;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;


public class StamenRemoteTestCase extends MaplyTestCase {

	public StamenRemoteTestCase(Activity activity) {
		super(activity);

		setTestName("Stamen Watercolor Remote");
		setDelay(4);
		this.implementation = TestExecutionImplementation.Both;
	}

	private QuadImageLoader setupImageLoader(ConfigOptions.TestType testType, BaseController baseController) {
		String cacheDirName = "stamen_watercolor4";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();

		RemoteTileInfoNew tileInfo = new RemoteTileInfoNew("http://tile.stamen.com/watercolor/{z}/{x}/{y}.png",0, 18);
		tileInfo.cacheDir = cacheDir;

		SamplingParams params = new SamplingParams();
		params.setCoordSystem(new SphericalMercatorCoordSystem());
		params.setCoverPoles(true);
		params.setEdgeMatching(true);
		params.setMinZoom(tileInfo.minZoom);
		params.setMaxZoom(tileInfo.maxZoom);
		params.setSingleLevel(true);

		QuadImageLoader loader = new QuadImageLoader(params,tileInfo,baseController);
		loader.setImageFormat(RenderController.ImageFormat.MaplyImageUShort565);
		loader.setDebugMode(true);

		//		final Handler handler = new Handler();
//		handler.postDelayed(new Runnable() {
//			@Override
//			public void run() {
//				baseLayer.reload();
//			}
//		}, 4000);

		return loader;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		setupImageLoader(ConfigOptions.TestType.GlobeTest,globeVC);
		globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
//		globeVC.setZoomLimits(0.0,1.0);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC)
	{
		setupImageLoader(ConfigOptions.TestType.MapTest,mapVC);
		mapVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
		mapVC.setAllowRotateGesture(true);
//		mapVC.setZoomLimits(0.0,1.0);
		return true;
	}
}
