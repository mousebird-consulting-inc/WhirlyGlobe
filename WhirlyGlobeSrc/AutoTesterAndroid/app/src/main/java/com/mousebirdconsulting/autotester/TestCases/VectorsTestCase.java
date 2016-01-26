package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.res.AssetManager;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import org.apache.commons.io.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;


public class VectorsTestCase extends MaplyTestCase {

	private ArrayList<VectorObject> vectors = new ArrayList<VectorObject>();

	public VectorsTestCase(Activity activity) {
		super(activity);

		setTestName("Vectors Test");
	}

	private void overlayCountries(MaplyBaseController baseVC) throws Exception {
		VectorInfo vectorInfo = new VectorInfo();
		vectorInfo.setColor(1,0,0,1);
		vectorInfo.setLineWidth(4.f);
		vectorInfo.setDrawPriority(100000);

		AssetManager assetMgr = getActivity().getAssets();
		String[] paths = assetMgr.list("country_json_50m");
		for (String path : paths) {
			InputStream stream = assetMgr.open("country_json_50m/" + path);
			try {
				VectorObject object = new VectorObject();
				String json = IOUtils.toString(stream, Charset.defaultCharset());
				if (object.fromGeoJSON(json)) {
					vectors.add(object);
				}
			} finally {
				try {
					stream.close();
				} catch (IOException e) {
				}
			}
		}

		// Build a really big vector for testing
//		VectorObject bigVecObj = new VectorObject();
//		Point2d pts[] = new Point2d[20000];
//		for (int ii=0;ii<20000;ii++)
//			pts[ii] = new Point2d(Math.random(), Math.random());
//		bigVecObj.addAreal(pts);

		// Add as red
		ComponentObject compObj = baseVC.addVectors(vectors, vectorInfo, MaplyBaseController.ThreadMode.ThreadAny);
		// Then change to green
		VectorInfo newVectorInfo = new VectorInfo();
		newVectorInfo.setColor(0,1,0,1);
		newVectorInfo.setLineWidth(4.f);
		newVectorInfo.setDrawPriority(100000);
		baseVC.changeVectors(compObj,newVectorInfo,MaplyBaseController.ThreadMode.ThreadAny);
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		MapBoxSatelliteTestCase mapBoxSatelliteTestCase = new MapBoxSatelliteTestCase(getActivity());
		mapBoxSatelliteTestCase.setUpWithMap(mapVC);
		overlayCountries(mapVC);
		return true;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		MapBoxSatelliteTestCase mapBoxSatelliteTestCase = new MapBoxSatelliteTestCase(getActivity());
		mapBoxSatelliteTestCase.setUpWithGlobe(globeVC);
		overlayCountries(globeVC);
		return true;
	}

	public ArrayList<VectorObject> getVectors() {
		return vectors;
	}

}
