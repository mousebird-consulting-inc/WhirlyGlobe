/*
 *  LibTestCase.java
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

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemManager;
import com.mousebird.maply.Scene;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class LibTestCase extends MaplyTestCase {

    public LibTestCase(Activity activity) {
        super(activity);

        this.setTestName("Screen Labels Test");
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        ParticleSystem system = new ParticleSystem();
        system.setLifetime(3);
        system.setName("test");
        system.setParticleSystemType(ParticleSystem.STATE.ParticleSystemPoint.getValue());
        system.setDrawPriority(100);
        ParticleBatch batch = new ParticleBatch();
        batch.setBatchSize(100);
        int ba = batch.getBatchSize();
        System.out.println("data->" + ba);

        return true;
    }
}
