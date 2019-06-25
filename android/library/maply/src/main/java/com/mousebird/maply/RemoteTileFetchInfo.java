/*
 *  RemoteTileFetchInfo.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/21/19.
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

import java.io.File;

import okhttp3.Request;

/**
 * Fetch Info for remote tile fetches.
 * <br>
 * The URL (required) and cacheFile (optional) for the given fetch.
 * This is the object the RemoteTileFetcher expects for the fetchInfo member of the TileFetchRequest.
 */
public class RemoteTileFetchInfo
{
    // Used for sorting
    public long uniqueID = Identifiable.genID();

    /**
     * URL to fetch from.  Also headers and things like that.
     */
    public Request urlReq;

    /**
     * Optional cache file location.
     * We'll try to read from here or cache to here after a successful fetch.
     */
    public File cacheFile;
}
