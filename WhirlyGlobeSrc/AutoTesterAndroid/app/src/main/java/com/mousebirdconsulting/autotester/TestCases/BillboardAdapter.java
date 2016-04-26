/*
 *  BillboardAdapter.java
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
import android.graphics.BitmapFactory;
import android.util.Size;

import com.mousebird.maply.Atmosphere;
import com.mousebird.maply.Billboard;
import com.mousebird.maply.BillboardInfo;
import com.mousebird.maply.BillboardManager;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.Light;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Moon;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.ScreenObject;
import com.mousebird.maply.Sun;
import com.mousebirdconsulting.autotester.R;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;


public class BillboardAdapter {

    private static final float EarthRadius = 6371000;

    private GlobeController viewC;
    private Activity activity;
    private MaplyBaseController.ThreadMode threadMode;

    public BillboardAdapter(GlobeController inViewC, Activity activity, MaplyBaseController.ThreadMode threadMode){
        this.viewC = inViewC;
        this.activity = activity;
        this.threadMode = threadMode;
    }

    public void start(){
        addBillboard();
    }
    private void addBillboard(){

        Sun sun = new Sun();
        Light light = sun.makeLight();
        viewC.clearLights();
        viewC.addLight(light);

        //Sun
        Billboard billSun = new Billboard();
        float [] position = sun.asPosition();
        billSun.setCenter(new Point3d(position[0], position[1], 5.4*EarthRadius));
        billSun.setSelectable(false);
        ScreenObject screenObject = new ScreenObject();
        Bitmap bm = BitmapFactory.decodeResource(activity.getResources(), R.drawable.sunImage);

        screenObject.addImage(bm, new float[]{1.0f, 1.0f, 1.0f, 1.0f}, 0.9f, 0.9f);
        billSun.setScreenObject(screenObject);

        BillboardInfo info = new BillboardInfo();
        info.setShaderName(Billboard.MAPLY_BILLBOARD_ORIENTE_EYE);
        info.setDrawPriority(2);
        List<Billboard> billboardsSun = new ArrayList<>();
        billboardsSun.add(billSun);
        ComponentObject componentObject = viewC.addBillboards(billboardsSun, info, this.threadMode);

        //Moon
        Moon moon  = new Moon(Calendar.getInstance());
        Billboard billMoon = new Billboard();
        Point3d moonPosition = moon.asPosition();
        billMoon.setCenter(new Point3d(moonPosition.getX(), moonPosition.getY(), 5.4 * EarthRadius));
        billMoon.setSelectable(false);
        ScreenObject screenObjectMoon = new ScreenObject();
        Bitmap bmMoon = BitmapFactory.decodeResource(activity.getResources(), R.drawable.moon);

        screenObjectMoon.addImage(bmMoon, new float[]{1.0f, 1.0f, 1.0f, 1.0f}, 0.75f, 0.75f);
        billMoon.setScreenObject(screenObjectMoon);
        List<Billboard> moons = new ArrayList<>();
        moons.add(billMoon);
        info.setDrawPriority(3);
        ComponentObject moonObj = viewC.addBillboards(moons, info, threadMode);

        Atmosphere atm = new Atmosphere(viewC, threadMode);
        float [] wavelength = new float[]{0.650f,0.570f,0.475f};
        atm.setWaveLength(wavelength);
        atm.setSunPosition(sun.getDirection());
    }
}

