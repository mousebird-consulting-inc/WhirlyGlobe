/*
 *  ClusteredMarkersTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.BitmapFactory;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.ScreenMarker;
import com.mousebird.maply.VectorObject;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.BasicClusterGenerator;
import com.mousebird.maply.RenderController;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;
import java.util.List;

public class ClusteredMarkersTestCase extends MaplyTestCase {

    public ClusteredMarkersTestCase(Activity activity) {
        super(activity);
        setTestName("Clustered Markers");
        setDelay(1000);
        this.implementation = TestExecutionImplementation.Both;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        VectorsTestCase baseView = new VectorsTestCase(getActivity());
        baseView.setUpWithMap(mapVC);
        insertClusteredMarkers(baseView.getVectors(), mapVC);
        mapVC.setPositionGeo(pos.getX(), pos.getY(), 2);
        return true;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        VectorsTestCase baseView = new VectorsTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);
        insertClusteredMarkers(baseView.getVectors(), globeVC);
        globeVC.animatePositionGeo(pos.getX(), pos.getY(), 0.9, 1);

        globeVC.setPerfInterval(20);

        return true;
    }

    Point2d pos = Point2d.FromDegrees(-3.6704803, 40.5023056);

    private void insertClusteredMarkers(List<VectorObject> vectors, BaseController inController) {
        Point2d size = new Point2d(32, 32);
        List<ScreenMarker> markers = new ArrayList<>();
        Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.sticker);
        MaplyTexture tex = inController.addTexture(icon,new RenderController.TextureSettings(), RenderController.ThreadMode.ThreadCurrent);

//        inController.addClusterGenerator(new BasicClusterGenerator(new int[]{Color.argb(165, 255, 255, 0)}, 1, new Point2d(64, 64), inController, inController.getActivity()));

        int which = 0;
        for (VectorObject v : vectors) {
            // Note: Increase this to test capacity
            for (int ii=0;ii<1;ii++) {
                ScreenMarker marker = new ScreenMarker();
                marker.tex = tex;
                marker.loc = v.centroid();
                marker.size = size;
                marker.selectable = true;

                if (marker.loc != null)
                    markers.add(marker);
            }
            which++;
        }

        MarkerInfo info = new MarkerInfo();
        info.setLayoutImportance(1.f);
//        info.setClusterGroup(1);
        info.setClusterGroup(0);

        inController.addScreenMarkers(markers, info, RenderController.ThreadMode.ThreadAny);
    }
}
