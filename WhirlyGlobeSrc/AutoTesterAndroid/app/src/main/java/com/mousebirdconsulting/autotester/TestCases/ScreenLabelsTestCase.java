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
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.ScreenMarker;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.ArrayList;


public class ScreenLabelsTestCase extends MaplyTestCase {

	private ArrayList<ComponentObject> componentObjects = new ArrayList<>();

	public ScreenLabelsTestCase(Activity activity) {
		super(activity);
		this.setTestName("Screen Labels Test");
		this.setDelay(1000);
		this.implementation = TestExecutionImplementation.Both;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithGlobe(globeVC);
		insertLabels(baseView.getVectors(), globeVC);
		Point2d loc = Point2d.FromDegrees(-74.075833, 4.598056);
		globeVC.animatePositionGeo(loc.getX(), loc.getY(), 3, 1);
		return true;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithMap(mapVC);
		insertLabels(baseView.getVectors(), mapVC);
		Point2d loc = Point2d.FromDegrees(-74.075833, 4.598056);
		mapVC.setPositionGeo(loc.getX(), loc.getY(), 3);
		return true;
	}

	// Make a screen label at a given location (in degrees)
	ScreenLabel makeLabel(double lon,double lat,String text,float importance)
	{
		ScreenLabel label = new ScreenLabel();
		label.loc = Point2d.FromDegrees(lon,lat);
		label.text = text;
		label.layoutImportance = importance;

		return label;
	}

	// If set, we'll put markers around the points for debugging
	static boolean addMarkers = false;

	private void insertLabels(ArrayList<VectorObject> objects, MaplyBaseController baseVC) {

		LabelInfo labelInfo = new LabelInfo();
		labelInfo.setFontSize(38.5f);
		labelInfo.setTextColor(Color.WHITE);
		labelInfo.setBackgroundColor(Color.RED);
		labelInfo.setTypeface(Typeface.DEFAULT);
//		labelInfo.setLayoutImportance(1.f);
		labelInfo.setLayoutPlacement(LabelInfo.LayoutLeft|LabelInfo.LayoutRight|LabelInfo.LayoutCenter);
		labelInfo.setMinVis(0.f);
		labelInfo.setMaxVis(2.5f);

		MarkerInfo markerInfo = new MarkerInfo();
		markerInfo.setDrawPriority(labelInfo.getDrawPriority() - 1);

		ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
		ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();

		for (VectorObject object : objects) {
			AttrDictionary attrs = object.getAttributes();
			if (attrs != null) {
				String labelName = attrs.getString("ADMIN");
				if (labelName != null && labelName.length() > 0) {
					ScreenLabel label = new ScreenLabel();
					label.text = labelName;
					label.loc = object.centroid();
					labels.add(label);

					if (addMarkers) {
						ScreenMarker marker = new ScreenMarker();
						marker.loc = label.loc;
						marker.size = new Point2d(32.f,32.f);
						markers.add(marker);
					}
				}
			}
		}

		// Toss in a few with explicit diacritics
		labels.add(makeLabel(-74.075833, 4.4, "Bogotá",1.f));
		labels.add(makeLabel(-74.075833, 4.598056, "Bogotá2",1.f));
		labels.add(makeLabel(6.0219, 47.2431, "Besançon",1.f));
		labels.add(makeLabel(4.361, 43.838, "Nîmes",1.f));
		labels.add(makeLabel(4.9053, 43.9425, "Morières-lès-Avignon",1.f));
		labels.add(makeLabel(11.616667, 44.833333, "Ferrara",1.f));
		labels.add(makeLabel(7, 49.233333, "Saarbrücken",1.f));

		ComponentObject comp = baseVC.addScreenLabels(labels, labelInfo, MaplyBaseController.ThreadMode.ThreadAny);
		if (comp != null)
			componentObjects.add(comp);
		if (addMarkers) {
			comp = baseVC.addScreenMarkers(markers, markerInfo, MaplyBaseController.ThreadMode.ThreadAny);
			if (comp != null)
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
