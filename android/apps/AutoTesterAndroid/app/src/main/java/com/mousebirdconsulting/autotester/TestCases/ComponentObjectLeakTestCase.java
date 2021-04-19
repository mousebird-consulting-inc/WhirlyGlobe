/*  ComponentObjectLeakTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2021 mousebird consulting
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
 */
package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.BaseController;
import com.mousebird.maply.MarkerInfo;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.ScreenMarker;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;


public class ComponentObjectLeakTestCase extends MaplyTestCase {

    private static final int COUNT = 200;
    private static final int ITERATIONS = 5000;

    public ComponentObjectLeakTestCase(Activity activity) {
        super(activity, "ComponentObject Leak Test");
        setDelay(2);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);
        globeVC.setPositionGeo(0.5, 0.5, 1);
        addAndRemove(COUNT, ITERATIONS);
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        baseCase = new StamenRemoteTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);
        mapVC.setPositionGeo(0.5, 0.5, 1);
        addAndRemove(COUNT, ITERATIONS);
        return true;
    }

    @Override
    public void shutdown() {
        stop = true;
        if (baseCase != null) {
            baseCase.shutdown();
            baseCase = null;
        }
        super.shutdown();
    }

    private void addAndRemove(int count, int iterations) {
        stop = false;
        Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.sticker);
        addAndRemove(count, iterations, icon);
    }

    private void addAndRemove(int count, int iterations, Bitmap image) {
        BaseController viewC = controller;
        if (iterations > 0 && viewC != null && !stop) {
            viewC.addPostSurfaceRunnable(() -> {
                addAndRemove(count, image);
                addAndRemove(count, iterations - 1, image);
            }, 1);
        }
    }

    private void addAndRemove(int count, Bitmap image) {
        BaseController viewC = controller;
        if (viewC == null) {
            return;
        }

        MarkerInfo markerInfo = new MarkerInfo();
//        markerInfo.setMinVis(0.f);
//        markerInfo.setMaxVis(1.f);
//        markerInfo.setColor(1);
        markerInfo.setDrawPriority(100100);

        ArrayList<ComponentObject> markers = new ArrayList<>(count);
        for (int i = 0; i < count && !stop; i++) {
            ScreenMarker marker = new ScreenMarker();
            marker.image = image;
            marker.loc = new Point2d(Math.random(), Math.random());
            marker.size = new Point2d(64,64);
            ComponentObject componentObject = viewC.addScreenMarker(marker,markerInfo, RenderController.ThreadMode.ThreadAny);
            markers.add(componentObject);
        }

        for (int i = 0; i < count && !stop; i++) {
            viewC.removeObject(markers.get(i), RenderController.ThreadMode.ThreadAny);
        }
    }

    boolean stop = false;
    MaplyTestCase baseCase = null;
}
