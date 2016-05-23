package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenMarker;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;

/**
 * Created by jmnavarro on 30/12/15.
 */
public class ScreenMarkersTestCase extends MaplyTestCase
{
	private ArrayList<ComponentObject> componentObjects = new ArrayList<>();

	public ScreenMarkersTestCase(Activity activity) {
		super(activity);
		setTestName("Screen Markers Test");
		setDelay(1000);
	}

	public ArrayList<ComponentObject> getComponentObjects() {
		return componentObjects;
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithMap(mapVC);
		insertMarkers(baseView.getVectors(), mapVC);
		mapVC.setPositionGeo(-3.6704803, 40.5023056, 2);
		return true;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		VectorsTestCase baseView = new VectorsTestCase(getActivity());
		baseView.setUpWithGlobe(globeVC);
		insertMarkers(baseView.getVectors(), globeVC);
		globeVC.animatePositionGeo(-3.6704803, 40.5023056, 0.9, 1);
		return true;
	}

	private void insertMarkers(ArrayList<VectorObject> vectors, MaplyBaseController baseVC) {
		MarkerInfo markerInfo = new MarkerInfo();
		Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.testtarget);
		markerInfo.setMinVis(0.f);
		markerInfo.setMaxVis(1.5f);

		ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();
		for (VectorObject vector : vectors) {
			ScreenMarker marker = new ScreenMarker();
			marker.image = icon;
			Point2d centroid = vector.centroid();
			if (centroid != null) {
				marker.loc = centroid;
				marker.size = new Point2d(128, 128);
				marker.rotation = Math.random() * 2.f * Math.PI;
//				marker.offset = new Point2d(-64,-64);
				AttrDictionary attrs = vector.getAttributes();
				if (attrs != null) {
					marker.userObject = attrs.getString("ADMIN");
					markers.add(marker);
				}
			}
		}

		ComponentObject object = baseVC.addScreenMarkers(markers, markerInfo, MaplyBaseController.ThreadMode.ThreadAny);
		if (object != null) {
			componentObjects.add(object);
		}
	}
}
