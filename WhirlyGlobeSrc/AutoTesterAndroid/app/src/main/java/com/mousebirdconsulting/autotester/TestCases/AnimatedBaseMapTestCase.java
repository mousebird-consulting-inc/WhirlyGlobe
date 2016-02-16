package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MultiplexTileSource;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.WeatherShader;

import java.io.File;


public class AnimatedBaseMapTestCase extends MaplyTestCase {

	public AnimatedBaseMapTestCase(Activity activity) {
		super(activity);
		setTestName("Animated BaseMap Test");
		setDelay(20);
	}

	private QuadImageTileLayer setupImageLayer(MaplyBaseController baseController, ConfigOptions.TestType testType) throws Exception {
		WeatherShader weatherShader = new WeatherShader(baseController);
		baseController.addShaderProgram(weatherShader, weatherShader.getName());
		String cacheDirName = "forecastio";
		File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
		cacheDir.mkdir();
		RemoteTileInfo[] sources = new RemoteTileInfo[5];
		for (int i = 0; i < 5; i++) {
			sources[i] = new RemoteTileInfo("http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer" + i + "/", "png", 0, 6);
		}
		SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
		MultiplexTileSource multiplexTileSource = new MultiplexTileSource(baseController, sources, coordSystem);
		QuadImageTileLayer forecastIOLayer = new QuadImageTileLayer(baseController, coordSystem, multiplexTileSource);
		forecastIOLayer.setSimultaneousFetches(1);
		forecastIOLayer.setDrawPriority(MaplyBaseController.ImageLayerDrawPriorityDefault + 100);
		forecastIOLayer.setBorderTexel(0);
		forecastIOLayer.setImageDepth(sources.length);
		forecastIOLayer.setCurrentImage(1.5f,false);
		forecastIOLayer.setAnimationPeriod(6.0f);
		forecastIOLayer.setShaderName(weatherShader.getName());

		forecastIOLayer.setSingleLevelLoading(testType == ConfigOptions.TestType.MapTest);
		forecastIOLayer.setUseTargetZoomLevel(testType == ConfigOptions.TestType.MapTest);
		forecastIOLayer.setCoverPoles(testType == ConfigOptions.TestType.MapTest);
		forecastIOLayer.setHandleEdges(testType == ConfigOptions.TestType.MapTest);

		multiplexTileSource.setCacheDir(cacheDir);
		return forecastIOLayer;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		CartoDBDarkTestCase mapBoxSatelliteTestCase = new CartoDBDarkTestCase(this.getActivity());
		mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
		globeVC.addLayer(setupImageLayer(globeVC, ConfigOptions.TestType.GlobeTest));
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		CartoDBDarkTestCase mapBoxSatelliteTestCase = new CartoDBDarkTestCase(this.getActivity());
		mapBoxSatelliteTestCase.setUpWithMap(mapVC);
		mapVC.addLayer(setupImageLayer(mapVC, ConfigOptions.TestType.MapTest));
		return true;
	}
}
