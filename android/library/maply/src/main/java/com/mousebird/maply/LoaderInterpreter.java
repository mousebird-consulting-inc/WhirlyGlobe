/*
 *  LoaderInterpreter
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/19.
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
 *  Loader Interpreter converts raw data into images and objects.
 *  <br>
 *  Converts data returned from a remote source (or cache) into images and/or
 *  MaplyComponentObjects that have already been added to the view (disabled).
 */
public interface LoaderInterpreter
{
    /**
     * Set when the loader first starts up.
     *
     * If you need to tweak loader settings, do it here.
     */
    public void setLoader(QuadLoaderBase loader);

    /**
     *  Parse the data coming back from a remote request and turn it into something we can use.
     *
     *  Convert the data passed in to image and component objects (e.g. add stuff to the visuals).
     *  Everything added should be disabled to start.
     */
    public void dataForTile(LoaderReturn loadReturn,QuadLoaderBase loader);
}
