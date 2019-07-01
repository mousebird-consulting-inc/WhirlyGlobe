/*
 *  ObjectLoaderReturn.java
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2019 mousebird consulting
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

package com.mousebird.maply;


/**
 *  This version of the loader return is used by the ImageLoaderInterpreter.
 *
 *  When image tiles load, the interpeter fills in these contents, which can
 *  include any sort of ComponentObject and, of course, images.
 */
public class ObjectLoaderReturn extends LoaderReturn
{
    ObjectLoaderReturn(QuadLoaderBase loader) {
        super(loader);
    }

    /**
     * If any component objects are associated with the tile, these are them.
     * They need to start disabled.  The system will enable and delete them when it is time.
     */
    public native void addComponentObjects(ComponentObject[] compObjs);

    /**
     * Add a single component object to the tile.
     * The system will manage it after that.
     * @param compObj
     */
    public void addComponentObject(ComponentObject compObj) {
        ComponentObject[] compObjs = new ComponentObject[1];
        compObjs[0] = compObj;
        addComponentObjects(compObjs);
    }
}
