/*
 *  QuadParticleSystemTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2014 mousebird consulting
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
import android.graphics.BitmapFactory;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.ScreenMarker;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;


public class ComponentObjectLeakTestCase extends MaplyTestCase {

    private static final int COUNT = 100;

    public ComponentObjectLeakTestCase(Activity activity) {
        super(activity);
        setTestName("ComponentObject Leak Test");
        setDelay(2000);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        for (int i = 0; i < COUNT; i++) {
            addAndRemove(globeVC, COUNT);
        }

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.sticker);
        for (int i = 0; i < COUNT; i++) {
            addAndRemove(mapVC, COUNT);
        }

        return true;
    }

    private void addAndRemove(MaplyBaseController viewC, int count) {
        MarkerInfo markerInfo = new MarkerInfo();
        markerInfo.setMinVis(0.f);
        markerInfo.setMaxVis(1.f);
        markerInfo.setColor(1);
        markerInfo.setDrawPriority(100100);

        ArrayList<ComponentObject> markers = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            ScreenMarker marker = new ScreenMarker();
            marker.image = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.sticker);
            marker.loc = new Point2d(Math.random(), Math.random());
            ComponentObject componentObject = viewC.addScreenMarker(marker,markerInfo, MaplyBaseController.ThreadMode.ThreadAny);
            markers.add(componentObject);
        }

        for (int i = 0; i< count; i++) {
            viewC.removeObject(markers.get(i), MaplyBaseController.ThreadMode.ThreadCurrent);
        }

        markers.clear();
    }
}
