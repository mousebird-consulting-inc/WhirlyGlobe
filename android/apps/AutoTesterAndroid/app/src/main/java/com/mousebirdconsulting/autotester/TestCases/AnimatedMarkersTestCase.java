package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.Marker;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.RenderControllerInterface;
import com.mousebird.maply.VectorObject;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class AnimatedMarkersTestCase extends MaplyTestCase
{
    public AnimatedMarkersTestCase(Activity activity) {
        super(activity, "Animated Markers Test", MaplyTestCase.TestExecutionImplementation.Both);
        setDelay(1000);
    }

    public ArrayList<ComponentObject> getComponentObjects() {
        return componentObjects;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        baseCase.setOnVectorsLoaded(vectors -> {
            insertMarkers(vectors, mapVC);
            return null;
        });
        baseCase.setUpWithMap(mapVC);
        final Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
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
        final Point2d loc = Point2d.FromDegrees(-3.6704803, 40.5023056);
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
        final Bitmap icon0 = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.teardrop);
        final Bitmap icon1 = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.teardrop_stroked);
        final MaplyTexture[] textures = new MaplyTexture[] {
            baseVC.addTexture(icon0, null, RenderController.ThreadMode.ThreadCurrent),
            baseVC.addTexture(icon1, null, RenderController.ThreadMode.ThreadCurrent)
        };
//		markerInfo.setMinVis(0.f);
//		markerInfo.setMaxVis(2.5f);

        final ArrayList<Marker> markers = new ArrayList<>();
        for (VectorObject vector : vectors) {
            final Marker marker = new Marker();
            marker.images = textures;
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

        ComponentObject object = baseVC.addMarkers(markers, markerInfo, RenderController.ThreadMode.ThreadCurrent);
        if (object != null) {
            componentObjects.add(object);
        }
    }

    private final VectorsTestCase baseCase = new VectorsTestCase(getActivity());
    private final ArrayList<ComponentObject> componentObjects = new ArrayList<>();
}
