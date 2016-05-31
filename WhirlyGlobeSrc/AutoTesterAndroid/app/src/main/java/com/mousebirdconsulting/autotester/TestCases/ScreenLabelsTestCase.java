package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Typeface;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.ArrayList;


public class ScreenLabelsTestCase extends MaplyTestCase {

	private ArrayList<ComponentObject> componentObjects = new ArrayList<>();

	public ScreenLabelsTestCase(Activity activity) {
		super(activity);
		this.setTestName("Screen Labels Test");
		this.setDelay(1000);
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

		LabelInfo labelInfo = new LabelInfo();
		labelInfo.setFontSize(32.f);
		labelInfo.setTextColor(Color.WHITE);
		labelInfo.setTypeface(Typeface.DEFAULT);
//		labelInfo.setLayoutImportance(1.f);
		labelInfo.setMinVis(0.f);
		labelInfo.setMaxVis(2.f);

		ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();

//		for (int ii=0;ii<100;ii++) {
			for (VectorObject object : objects) {
				AttrDictionary attrs = object.getAttributes();
				if (attrs != null) {
					String labelName = attrs.getString("ADMIN");
					if (labelName != null && labelName.length() > 0) {
						ScreenLabel label = new ScreenLabel();
						label.text = labelName;
						label.loc = object.centroid();
						labels.add(label);
					}
				}
			}
//		}

		// Toss in one with an explicit accent
		ScreenLabel label = new ScreenLabel();
		label.text = "Bogotá";
		label.loc = Point2d.FromDegrees(-74.075833, 4.598056);
		// Move this by a ridiculous amount so we can find it
		label.offset = new Point2d(-200,0.0);
		labels.add(label);

		ComponentObject comp = baseVC.addScreenLabels(labels, labelInfo, MaplyBaseController.ThreadMode.ThreadAny);
		if (comp != null) {
			componentObjects.add(comp);
		}
	}

	public ArrayList<ComponentObject> getComponentObjects() {
		return componentObjects;
	}

	public void setComponentObjects(ArrayList<ComponentObject> componentObjects) {
		this.componentObjects = componentObjects;
	}
}
