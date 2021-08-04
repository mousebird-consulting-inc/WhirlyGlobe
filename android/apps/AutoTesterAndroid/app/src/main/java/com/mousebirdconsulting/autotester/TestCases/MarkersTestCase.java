package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.Marker;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class MarkersTestCase extends MaplyTestCase
{
    public MarkersTestCase(Activity activity) {
        super(activity);
        setTestName("Markers");
        setDelay(1000);
        this.implementation = MaplyTestCase.TestExecutionImplementation.Both;
    }

    public Collection<ComponentObject> getComponentObjects() {
        return componentObjects;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        baseCase.setOnVectorsLoaded(vectors -> {
            insertMarkers(vectors, mapVC);
            return null;
        });
        baseCase.setUpWithMap(mapVC);
        Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
        mapVC.setPositionGeo(loc.getX(), loc.getX(), 2);
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        baseCase.setOnVectorsLoaded(vectors -> {
            insertMarkers(vectors, globeVC);
            return null;
        });
        baseCase.setUpWithGlobe(globeVC);
        Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
        globeVC.animatePositionGeo(loc.getX(), loc.getX(), 0.9, 1);
        return true;
    }

    @Override
    public void shutdown() {
        controller.removeObjects(componentObjects, RenderController.ThreadMode.ThreadCurrent);
        componentObjects.clear();
        baseCase.shutdown();
        super.shutdown();
    }

    private void insertMarkers(Collection<? extends VectorObject> vectors, BaseController baseVC) {
        final MarkerInfo markerInfo = new MarkerInfo();
        final Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.testtarget);
//		markerInfo.setMinVis(0.f);
//		markerInfo.setMaxVis(2.5f);
        markerInfo.setClusterGroup(0);
        markerInfo.setLayoutImportance(1.f);

        final ArrayList<Marker> markers = new ArrayList<>();
        for (VectorObject vector : vectors) {
            Marker marker = new Marker();
            marker.image = icon;
            final Point2d centroid = vector.centroid();
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

        final ComponentObject object = baseVC.addMarkers(markers, markerInfo, RenderController.ThreadMode.ThreadCurrent);
        if (object != null) {
            componentObjects.add(object);
        }
    }

    private final VectorsTestCase baseCase = new VectorsTestCase(getActivity());
    private final List<ComponentObject> componentObjects = new ArrayList<>();
}
