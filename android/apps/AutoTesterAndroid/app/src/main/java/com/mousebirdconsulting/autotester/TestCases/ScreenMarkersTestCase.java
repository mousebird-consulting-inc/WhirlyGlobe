package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.RenderControllerInterface;
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
	public ScreenMarkersTestCase(Activity activity) {
		super(activity, "Screen Markers", TestExecutionImplementation.Both);
		setDelay(1000);
	}

	@Override
	public boolean setUpWithMap(MapController mapVC) throws Exception {
		baseCase = new VectorsTestCase(getActivity());
		baseCase.setUpWithMap(mapVC);
		insertMarkers(baseCase.getVectors(), mapVC);
		Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
		mapVC.setPositionGeo(loc.getX(), loc.getX(), 2);
		mapVC.setAllowRotateGesture(true);
		return true;
	}

	@Override
	public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
		baseCase = new VectorsTestCase(getActivity());
		baseCase.setUpWithGlobe(globeVC);
		insertMarkers(baseCase.getVectors(), globeVC);
		Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
		globeVC.animatePositionGeo(loc.getX(), loc.getX(), 0.9, 1);
		globeVC.setKeepNorthUp(false);
		return true;
	}

	@Override
	public void shutdown() {
		controller.removeObjects(componentObjects, RenderControllerInterface.ThreadMode.ThreadCurrent);
		componentObjects.clear();
		if (baseCase != null) {
			baseCase.shutdown();
			baseCase = null;
		}
		super.shutdown();
	}

	private void insertMarkers(ArrayList<VectorObject> vectors, BaseController baseVC) {
		MarkerInfo markerInfo = new MarkerInfo();
		Bitmap[] icons = new Bitmap[]{
				BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.testtarget),
				BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.teardrop),
		};
		MaplyTexture[] textures = new MaplyTexture[] {
				baseVC.addTexture(icons[0],null, RenderControllerInterface.ThreadMode.ThreadCurrent),
				baseVC.addTexture(icons[1],null, RenderControllerInterface.ThreadMode.ThreadCurrent),
		};
//		markerInfo.setMinVis(0.f);
//		markerInfo.setMaxVis(2.5f);

		ArrayList<ScreenMarker> markers = new ArrayList<>();
		for (VectorObject vector : vectors) {
			Point2d centroid = vector.centroid();
			if (centroid != null) {
				ScreenMarker marker = new ScreenMarker();
				marker.tex = textures[markers.size() % textures.length];
				marker.loc = centroid;
				marker.size = new Point2d(128, 128);
				marker.rotation = Math.random() * 2.f * Math.PI;
				marker.selectable = true;

				// Use the layout engine for some of the markers and not others
				marker.layoutImportance = (centroid.getX() < 0) ? 1 : Float.MAX_VALUE;

//				marker.offset = new Point2d(0.0,-200);
//				marker.layoutSize = new Point2d(0.0, 64.0);
				AttrDictionary attrs = vector.getAttributes();
				if (attrs != null) {
					marker.userObject = attrs.getString("ADMIN");
					markers.add(marker);
				}
			}
		}

		ComponentObject object = baseVC.addScreenMarkers(markers, markerInfo, RenderController.ThreadMode.ThreadAny);
		if (object != null) {
			componentObjects.add(object);
		}
	}

	private VectorsTestCase baseCase;
	private final ArrayList<ComponentObject> componentObjects = new ArrayList<>();
}
