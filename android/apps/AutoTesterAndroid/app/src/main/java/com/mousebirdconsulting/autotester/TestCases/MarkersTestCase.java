package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Marker;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;

public class MarkersTestCase extends MaplyTestCase
{
    private ArrayList<ComponentObject> componentObjects = new ArrayList<>();

    public MarkersTestCase(Activity activity) {
        super(activity);
        setTestName("Markers Test");
        setDelay(1000);
        this.implementation = MaplyTestCase.TestExecutionImplementation.Both;
    }

    public ArrayList<ComponentObject> getComponentObjects() {
        return componentObjects;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        VectorsTestCase baseView = new VectorsTestCase(getActivity());
        baseView.setUpWithMap(mapVC);
        insertMarkers(baseView.getVectors(), mapVC);
        Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
        mapVC.setPositionGeo(loc.getX(), loc.getX(), 2);
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        VectorsTestCase baseView = new VectorsTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);
        insertMarkers(baseView.getVectors(), globeVC);
        Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
        globeVC.animatePositionGeo(loc.getX(), loc.getX(), 0.9, 1);
        return true;
    }

    private void insertMarkers(ArrayList<VectorObject> vectors, MaplyBaseController baseVC) {
        MarkerInfo markerInfo = new MarkerInfo();
        Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.testtarget);
//		markerInfo.setMinVis(0.f);
//		markerInfo.setMaxVis(2.5f);
        markerInfo.setClusterGroup(0);
        markerInfo.setLayoutImportance(1.f);

        ArrayList<Marker> markers = new ArrayList<Marker>();
        for (VectorObject vector : vectors) {
            Marker marker = new Marker();
            marker.image = icon;
            Point2d centroid = vector.centroid();
            if (centroid != null) {
                marker.loc = centroid;
                marker.size = new Point2d(0.05,0.05);
                marker.selectable = true;
//				marker.offset = new Point2d(-64,-64);
                AttrDictionary attrs = vector.getAttributes();
                if (attrs != null) {
                    marker.userObject = attrs.getString("ADMIN");
                    markers.add(marker);
                }
            }
        }

        ComponentObject object = baseVC.addMarkers(markers, markerInfo, MaplyBaseController.ThreadMode.ThreadAny);
        if (object != null) {
            componentObjects.add(object);
        }
    }
}
