/*
 *  RemoteTileInfoNew.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/20/19.
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
import java.net.URL;
import java.util.ArrayList;

import okhttp3.Headers;
import okhttp3.Request;

/**
 *  Remote Tile Info Object (New)
 *  <br>
 *  Not to be confused with the old one, which works with the older loading subsystem,
 *  the new remote tile info object contains min/max zoom, coordinate system and URL
 *  information for fetching individual data tiles.
 */
public class RemoteTileInfoNew extends TileInfoNew
{
    protected ArrayList<String> baseURLs = new ArrayList<String>();

    /**
     *  The timeout assigned to the NSMutableURLRequest we're using to fetch tiles.
     *
     *  This is not set by default.  If set, we'll use this value as the timeout on the
     *  Request we use for fetching tiles.
     *  This lets you extent it where appropriate or shorten it if you like.
     */
    float timeOut = 0.0f;


    /**
     * The cache directory for data tiles.
     * <br>
     * In general, we want to cache.  The globe, in particular,
     * is going to fetch the same tiles over and over, quite a lot.
     * The cacheing behavior is a little dumb.  It will just write
     * files to the given directory forever.  If you're interacting
     * with a giant image pyramid, that could be problematic.
     */
    public File cacheDir = null;

    /**
     *  Optional headers to add to the Request.
     *  <br>
     *  These are name/data pairs which will be stuck in the NSURLRequest header.
     */
    public Headers headers = null;

    /**
     *  Optional coordinate system describing the tile set.
     *  <br>
     *  This coordinate system is required if the tile info will need
     *  to evaluate valid tiles as defined by the addValidBounds:coordSystem: call.
     */
    public CoordSystem coordSys = null;

    /**
     * If set, we'll test the tiles against this bounding box before trying to fetch them.
     */
    public Mbr validBounds = null;

    /**
     * If set, the bounding box is in this coordinate system.
     */
    public CoordSystem validCoordSys = null;

    /**
     * Empty constructor.  Fill in the baseURL, minZoom and maxZoom at least.
     */
    RemoteTileInfoNew()
    {
    }

    /**
     *  Initialize with enough information to fetch remote tiles.
     *
     *  This version of the init method takes all the explicit
     *  information needed to fetch remote tiles.  This includes the
     *  base URL and min and max zoom levels.
     *
     *  @param inBaseURL The base URL for fetching TMS tiles.  This is a replacement URL with {x}, {y}, and {z} in the string.
     *
     *  @param inMinZoom The minimum zoom level to fetch.  This really should be 0.
     *
     *  @param inMaxZoom The maximum zoom level to fetch.
     */
    public RemoteTileInfoNew(String inBaseURL,int inMinZoom,int inMaxZoom)
    {
        baseURLs.add(inBaseURL);
        minZoom = inMinZoom;
        maxZoom = inMaxZoom;
    }

    /**
     * Some sources offer the same content from multiple URLs for speed.
     * You can add more than one baseURL in that case and it'll pick among them.
     */
    public void addBaseURL(String baseURL)
    {
        baseURLs.add(baseURL);
    }

    /**
     *  Add a bounding box that defined validity for any tile before it's fetched.
     *  <br>
     *  Not all data sources cover all possible tiles.  If you know your data source does not,
     *  you can specify what area is valid ahead of times.  Tiles that do not overlap that area
     *  will not be loaded.
     */
    public void addValidBounds(Mbr inBbox,CoordSystem inCoordSys)
    {
        validBounds = inBbox;
        validCoordSys = inCoordSys;
    }

    /**
     * Used internally to check the validity of a tile if the validBounds are set.
     */
    boolean tileIsValid(TileID tileID)
    {
        if (validBounds == null || validCoordSys == null)
            return true;

        Mbr wholeMbr = coordSys.getBounds();

        // Build a bounding box for this particular tile
        int numLevel = 1<<tileID.level;
        double spanX = wholeMbr.ur.getX() - wholeMbr.ll.getX();
        double spanY = wholeMbr.ur.getY() - wholeMbr.ll.getY();
        double dx = spanX/numLevel;  double dy = spanY/numLevel;
        Point3d pts[] = new Point3d[4];
        pts[0] = new Point3d(wholeMbr.ll.getX()+dx*tileID.x,wholeMbr.ll.getY()+dy*tileID.y,0.0);
        pts[1] = new Point3d(wholeMbr.ll.getX()+dx*(tileID.x+1),wholeMbr.ll.getY()+dy*tileID.y,0.0);
        pts[2] = new Point3d(wholeMbr.ll.getX()+dx*(tileID.x+1),wholeMbr.ll.getY()+dy*(tileID.y+1),0.0);
        pts[3] = new Point3d(wholeMbr.ll.getX()+dx*tileID.x,wholeMbr.ll.getY()+dy*(tileID.y+1),0.0);

        // Project the corners into our coord sys
        Mbr tileMbr = new Mbr();
        for (Point3d pt : pts) {
            Point3d validPt = CoordSystem.CoordSystemConvert3d(coordSys,validCoordSys,pt);
            tileMbr.addPoint(new Point2d(validPt.getX(),validPt.getY()));
        }

        return tileMbr.overlaps(validBounds);
    }

    /**
     * Construct a URL for a given tile.
     * If you'd like to override this you can construct your own URL.
     * If you need to mess with Request parameters look for buildRequest().
     */
    public URL buildURL(int x,int y,int level,boolean flipY)
    {
        if (flipY)
            y = ((1<<level)-y)-1;

        String url = null;
        url = baseURLs.get( x % baseURLs.size()).replace("{x}","" + x).replace("{y}","" + y).replace("{z}","" + level);

        URL retURL = null;
        try {
            retURL = new URL(url);
        }
        catch (Exception e)
        {
        }

        return retURL;
    }

    // OkHTTP wants to track requests by source
    Object NET_TAG = new Object();

    protected ArrayList<String> headerNames = new ArrayList<String>();
    protected ArrayList<String> headerVals = new ArrayList<String>();

    /**
     * Add a header key/value to the HTTP request before it goes out.
     */
    public void addHeader(String name,String val)
    {
        headerNames.add(name);
        headerVals.add(val);
    }

    /**
     * Build an okHTTP request
     */
    public Request buildRequest(URL url, Object OkHTTP_TAG)
    {
        Request.Builder builder = new Request.Builder();
        builder.url(url);
        if (OkHTTP_TAG != null)
            builder.tag(OkHTTP_TAG);
        for (int ii=0;ii<headerNames.size();ii++)
            builder.addHeader(headerNames.get(ii),headerVals.get(ii));

        return builder.build();
    }

    /**
     * Build a cache name for the cache file
     */
    public String buildCacheName(int x,int y,int level,boolean flipY)
    {
        String name = "";
        name += level + "_" + x + "_" + y;

        return name;
    }

    /**
     * Build the URL and other info for a single tile fetch.
     * Returns is a RemoteTileFetcheInfo object.
     */
    @Override public Object fetchInfoForTile(TileID tileID,boolean flipY)
    {
        RemoteTileFetchInfo fetchInfo = new RemoteTileFetchInfo();
        fetchInfo.urlReq = buildRequest(buildURL(tileID.x,tileID.y,tileID.level,flipY),NET_TAG);
        if (cacheDir != null) {
            fetchInfo.cacheFile = new File(cacheDir,buildCacheName(tileID.x,tileID.y,tileID.level,flipY));
        }

        return fetchInfo;
    }
}
