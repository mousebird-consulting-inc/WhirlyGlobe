package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Typeface;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.ScreenLabel;
import com.mousebird.maply.ScreenMarker;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.ArrayList;


public class ScreenLabelsTestCase extends MaplyTestCase {

	private ArrayList<ComponentObject> componentObjects = new ArrayList<>();

	public ScreenLabelsTestCase(Activity activity) {
		super(activity);
		this.setTestName("Screen Labels");
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

	ScreenLabel makeLabel(double lon,double lat,String text,float importance)
	{
		return makeLabel(lon,lat,text,importance,0.0f);
	}

	// Make a screen label at a given location (in degrees)
	ScreenLabel makeLabel(double lon,double lat,String text,float importance, float rot)
	{
		ScreenLabel label = new ScreenLabel();
		label.loc = Point2d.FromDegrees(lon,lat);
		label.text = text;
		label.layoutImportance = importance;
		label.rotation = rot;

		return label;
	}

	// If set, we'll put markers around the points for debugging
	static boolean addMarkers = true;

	private void insertLabels(ArrayList<VectorObject> objects, BaseController baseVC) {

		LabelInfo labelInfo = new LabelInfo();
		labelInfo.setFontSize(32.f);
		labelInfo.setTextColor(Color.BLACK);
		labelInfo.setBackgroundColor(Color.BLUE);
		labelInfo.setTypeface(Typeface.DEFAULT);
		labelInfo.setLayoutImportance(1.f);
//		labelInfo.setLayoutImportance(Float.MAX_VALUE);
		labelInfo.setLayoutPlacement(LabelInfo.LayoutRight | LabelInfo.LayoutCenter);
		labelInfo.setTextJustify(LabelInfo.TextJustify.TextLeft);
//		labelInfo.setMinVis(0.f);
//		labelInfo.setMaxVis(2.5f);
		labelInfo.setOutlineColor(Color.WHITE);
		labelInfo.setOutlineSize(2.f);
//		labelInfo.layoutImportance = Float.MAX_VALUE;

		MarkerInfo markerInfo = new MarkerInfo();
		markerInfo.setDrawPriority(labelInfo.getDrawPriority() + 1);

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
					label.selectable = true;
//					label.layoutImportance = 1.f;
//					label.layoutImportance = Float.MAX_VALUE;
//					label.layoutSize = new Point2d(1000.0f, 256.f );
//					label.offset = new Point2d(0.0,-300);
//					label.rotation = Math.PI/4.0;
					labels.add(label);

					if (addMarkers) {
						ScreenMarker marker = new ScreenMarker();
						marker.loc = label.loc;
						marker.size = new Point2d(8.f,8.f);
						markers.add(marker);
					}
				}
			}
		}

		// Toss in a few with explicit diacritics
		labels.add(makeLabel(-74.075833, 4.4, "Bogotá", 2.f, (float)Math.PI/4.0f));
		labels.add(makeLabel(-74.075833, 4.598056, "Bogotá2", 2.f));
		labels.add(makeLabel(6.0219, 47.2431, "Besançon", 2.f));
		labels.add(makeLabel(4.361, 43.838, "Nîmes", 2.f));
		labels.add(makeLabel(4.9053, 43.9425, "Morières-lès-Avignon", 2.f));
		labels.add(makeLabel(11.616667, 44.833333, "Ferrara", 2.f));
		labels.add(makeLabel(7, 49.233333, "Saarbrücken", 2.f));

		labels.add(makeLabel(0.0, 0.0, "abcdef\nghijklmn\nopqrstu\nvwxyzA\nBCDEF\nGHIJKLMN\nOPQRTST\nUVWXYZ", 10.f));
		if (addMarkers)
		{
			ScreenMarker marker = new ScreenMarker();
			marker.loc = Point2d.FromDegrees(0.0,0.0);
			marker.size = new Point2d(8.f, 8.f);
			markers.add(marker);
		}
		labels.add(makeLabel(1.0, -5.0, "abcdef\nghijklmn\nopqrstu\nvwxyz", 10.f));
		if (addMarkers)
		{
			ScreenMarker marker = new ScreenMarker();
			marker.loc = Point2d.FromDegrees(1.0,-5.0);
			marker.size = new Point2d(8.f, 8.f);
			markers.add(marker);
		}

		ScreenLabel nimesLabel = makeLabel(4.361, 43.838, "Nîmes", Float.MAX_VALUE);
		// Tall and skinny for testing
		nimesLabel.layoutSize = new Point2d(1.0,200.0);
		labels.add(nimesLabel);

		// Test the layout engine very explicitly
//		int i = 0;
//		for (double o = 0.0; o < 0.5; o += 0.05) {
//
//			labels.add(makeLabel(4.361 + o, 43.838 + o, "Nîmes " + ++i, Float.MAX_VALUE));
//		}

		ComponentObject comp = baseVC.addScreenLabels(labels, labelInfo, RenderController.ThreadMode.ThreadAny);
		if (comp != null)
			componentObjects.add(comp);
		if (addMarkers) {
			comp = baseVC.addScreenMarkers(markers, markerInfo, RenderController.ThreadMode.ThreadAny);
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
