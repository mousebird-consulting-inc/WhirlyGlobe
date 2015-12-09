/*
 *  MaplyTexture.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/17/15.
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

package com.mousebird.maply;

/**
 * Encapsulates a texture in WhirlyGlobe-Maply toolkit.  You can create these with the
 * base controller by adding them.  These are basically just handles.
 */
public class MaplyTexture
{
    public MaplyTexture()
    {
        controller = null;
        texID = 0;
    }
    MaplyBaseController controller;
    long texID;
}
