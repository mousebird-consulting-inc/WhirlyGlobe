/*
 *  ParticleSystemTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 26/1/16
 *  Copyright 2011-2015 mousebird consulting
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

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.util.Date;

public class SimpleParticleSystemTestCase extends MaplyTestCase {

    public SimpleParticleSystemTestCase(Activity activity) {
        super(activity);

        this.setTestName("Simple Particle System Test");
        this.setDelay(2000);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {

        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        this.viewC = globeVC;
        globeVC.onSurfaceCreatedTask(new Runnable() {
            @Override
            public void run() {
                setupParticles();
            }
        });

        return true;
    }

    private ParticleSystem partSys;
    private MaplyBaseController viewC;
    private ComponentObject partSysObj;
    private float locs[];
    private float dirs[];
    private float colors[];
    private double particleLifeTime;
    private int numParticles;
    private double updateInterval;
    private float velocityScale;
    private Runnable updateTask;

    void setupParticles()
    {
        this.updateInterval = 1.0;
        this.particleLifeTime = 10.0;
        this.numParticles = 100000;
        this.velocityScale = 0.1f;

        this.updateTask = new Runnable() {
            @Override
            public void run() {
                float delay = getDelay();
                while (delay - updateInterval>0) {
                    try {
                        generateParticles();
                        delay -= updateInterval;
                        Thread.sleep((long) (updateInterval * 1000));
                        System.out.println("Updating...");
                    } catch (InterruptedException e) {

                    }
                }
            }
        };
        this.updateTask.run();
    }

    public void generateParticles() throws InterruptedException {
        if (this.partSysObj == null) {
            this.partSys = new ParticleSystem("Test Particle System");
            this.partSys.setLifetime(this.particleLifeTime);
            this.partSys.setTotalParticles(this.numParticles);
            this.partSys.setDrawPriority(101000);
            this.partSys.setPointSize(4);
            this.partSys.setBatchSize((int) (this.numParticles / (this.particleLifeTime / this.updateInterval)));
            this.partSysObj = this.viewC.addParticleSystem(this.partSys, MaplyBaseController.ThreadMode.ThreadAny);
        }

        double now = ((double) new Date().getTime() ) - 978303600000.d;

        // Data arrays for particles
        int batchSize = this.partSys.getBatchSize();

        if (locs == null) {
            this.locs = new float[this.partSys.getBatchSize()*3];
            this.dirs = new float[this.partSys.getBatchSize()*3];
            this.colors = new float[this.partSys.getBatchSize()*4];
//            this.times = new float[batchSize];
        }

        // Make up some random particles
        for (int ii = 0; ii< this.partSys.getBatchSize(); ii++) {
            //Random location
            float x = (float)Math.random()*2-1;
            float y = (float)Math.random()*2-1;
            float z = (float)Math.random()*2-1;
            float sum = (float)Math.sqrt(x*x+y*y+z*z);
            x /= sum;  y /= sum;  z /= sum;
            locs[ii*3] = x; locs[ii*3+1] = y; locs[ii*3+2] = z;
            //Random direction
//                dirs[ii*3] = (float) Math.random()*2-1; dirs[ii*3+1] = (float) Math.random()*2-1; dirs[ii*3+2] = (float) Math.random()*2-1;
            dirs[ii*3] = 0.f;  dirs[ii*3+1] = 0.f;  dirs[ii*3+2] = 0.f;

//            this.times[ii] = 0;

            colors[ii*4] = 1.0f; colors[ii*4+1] = 1.0f; colors[ii*4+2] = 1.0f; colors[ii*4+3] = 1.0f;
        }

        ParticleBatch batch = new ParticleBatch(this.partSys);
        batch.addAttribute("a_position", this.locs);
        batch.addAttribute("a_dir", this.dirs);
        batch.addAttribute("a_color", this.colors);
//        batch.addAttribute("a_startTime", this.times);
        viewC.addParticleBatch(batch, MaplyBaseController.ThreadMode.ThreadAny);
    }
}
