/*
 *  AtmosphereAdapter.java
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

import com.mousebird.maply.Atmosphere;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MaplyBaseController;


public class AtmosphereAdapter {

    private MaplyBaseController viewC;
    private Atmosphere atmosphere;

    public AtmosphereAdapter(MaplyBaseController viewC) {
        this.viewC = viewC;
        atmosphere = new Atmosphere((GlobeController) this.viewC, MaplyBaseController.ThreadMode.ThreadAny);
        atmosphere.setWaveLength(new float[]{0.650f, 0.570f, 0.475f});
    }

}
