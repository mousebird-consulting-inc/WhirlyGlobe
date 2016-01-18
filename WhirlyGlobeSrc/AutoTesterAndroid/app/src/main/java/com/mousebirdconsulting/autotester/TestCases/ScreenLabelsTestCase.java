package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.ArrayList;


public class ScreenLabelsTestCase extends MaplyTestCase {

	private ArrayList<ComponentObject> componentObjects = new ArrayList<>();

	public ScreenLabelsTestCase(Activity activity) {
		super(activity);
		this.setTestName("Screen Labels Test");
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithGlobe(globeVC);
		insertLabels(baseView.getVectors(), globeVC);
		globeVC.animatePositionGeo(151.211111, -33.859972, 3, 1);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithMap(mapVC);
		insertLabels(baseView.getVectors(), mapVC);
		mapVC.setPositionGeo(151.211111, -33.859972, 3);
		return true;
	}

	private void insertLabels(ArrayList<VectorObject> objects, MaplyBaseController baseVC) {
		for (VectorObject object : objects) {
			String labelName = object.getAttributes().getString("ADMIN");
			if (labelName != null && labelName.length() > 0) {
				ScreenLabel label = new ScreenLabel();
				label.text = labelName;
				label.loc = object.centroid();

				LabelInfo labelInfo = new LabelInfo();
				ComponentObject comp = baseVC.addScreenLabel(label, labelInfo, MaplyBaseController.ThreadMode.ThreadAny);
				if (comp != null) {
					componentObjects.add(comp);
				}
			}
		}
	}

	public ArrayList<ComponentObject> getComponentObjects() {
		return componentObjects;
	}

	public void setComponentObjects(ArrayList<ComponentObject> componentObjects) {
		this.componentObjects = componentObjects;
	}
}
