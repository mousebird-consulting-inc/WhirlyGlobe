package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Color;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.InputStream;
import java.util.ArrayList;

import okio.Okio;


public class VectorsTestCase extends MaplyTestCase {

	private final ArrayList<VectorObject> vectors = new ArrayList<>();

	public VectorsTestCase(Activity activity) {
		super(activity, "Vectors Test", TestExecutionImplementation.Both);
		setDelay(4);
	}

	private void overlayCountries(BaseController baseVC) throws Exception {
		VectorInfo vectorInfo = new VectorInfo();
		vectorInfo.setColor(Color.RED);
		vectorInfo.setLineWidth(4.f);

		AssetManager assetMgr = getActivity().getAssets();
		String[] paths = assetMgr.list("country_json_50m");
		for (String path : paths) {
			try (InputStream stream = assetMgr.open("country_json_50m/" + path)) {
				VectorObject vecObject = new VectorObject();
				vecObject.setSelectable(true);
				String json = Okio.buffer(Okio.source(stream)).readUtf8();
				if (vecObject.fromGeoJSON(json)) {
					vectors.add(vecObject);
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
		ComponentObject compObj = baseVC.addVectors(vectors, vectorInfo, RenderController.ThreadMode.ThreadAny);
		// Then change to white
		VectorInfo newVectorInfo = new VectorInfo();
		newVectorInfo.setColor(Color.WHITE);
		newVectorInfo.setLineWidth(4.f);
		baseVC.changeVector(compObj,newVectorInfo,RenderController.ThreadMode.ThreadAny);
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		baseCase.setUpWithMap(mapVC);
		overlayCountries(mapVC);
		return true;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		baseCase.setUpWithGlobe(globeVC);
		overlayCountries(globeVC);
		return true;
	}

	@Override
	public void shutdown() {
		baseCase.shutdown();
		super.shutdown();
	}

	public ArrayList<VectorObject> getVectors() {
		return vectors;
	}

	public final MaplyTestCase baseCase = new GeographyClass(getActivity());
}
