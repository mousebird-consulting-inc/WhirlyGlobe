/*  MapboxKindaMap.kt
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/2020.
 *  Copyright 2011-2022 mousebird consulting
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
package com.mousebird.maply

import android.content.res.Resources
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.Looper
import android.util.DisplayMetrics
import android.util.Log
import androidx.core.net.toFile
import okhttp3.*
import java.io.*
import java.lang.ref.WeakReference
import kotlin.collections.ArrayList

/**
 * Convenience class for loading a Mapbox-style vector tiles-probably kinda map.
 * You give it a style sheet and it figures out the rest.
 * Set the various settings before it gets going to modify how it works.
 * Callbacks control various pieces that might need to be intercepted.
 */
@Suppress("unused", "MemberVisibilityCanBePrivate")
open class MapboxKindaMap(
        var styleSheetJSON: String?,
        var styleURL: Uri?,
        var localMBTiles: Sequence<File>? = null,
        inControl: BaseController,
        var styleSettings: VectorStyleSettings = defaultStyle()) {

    companion object {
        fun defaultStyle() = VectorStyleSettings().apply {
            baseDrawPriority = QuadImageLoaderBase.BaseDrawPriorityDefault + 1000
            drawPriorityPerLevel = 100
        }
    }
    
    constructor(styleURL: Uri, control: BaseController, styleSettings: VectorStyleSettings = defaultStyle()) :
            this(null, styleURL, null, control, styleSettings)
    constructor(styleJSON: String, control: BaseController, styleSettings: VectorStyleSettings = defaultStyle()) :
            this(styleJSON, null, null, control, styleSettings)
    constructor(styleURL: Uri, localMBTilesFile: File, control: BaseController, styleSettings: VectorStyleSettings = defaultStyle()) :
            this(null, styleURL, sequenceOf(localMBTilesFile), control, styleSettings)
    constructor(styleJSON: String, localMBTilesFile: File, control: BaseController, styleSettings: VectorStyleSettings = defaultStyle()) :
            this(styleJSON, null, sequenceOf(localMBTilesFile), control, styleSettings)

    var styleSheet: MapboxVectorStyleSet? = null; private set
    var styleSheetImage: MapboxVectorStyleSet? = null; private set
    var styleSheetVector: MapboxVectorStyleSet? = null; private set
    var spriteJSON: String? = null; private set
    var spritePNG: Bitmap? = null; private set
    var mapboxInterp: MapboxVectorInterpreter? = null; private set
    var loader: QuadLoaderBase? = null; private set
    var offlineRender: RenderController? = null; private set
    var lineScale = 0.0
    var textScale = 0.0
    var markerScale = 0.0
    var maxConcurrentLoad: Int? = null
    var debugMode = false
    var running: Boolean = false; private set       // true after resource fetch and stylesheet setup

    /* If set, we build an image/vector hybrid where the polygons go into
     *  the image layer and the linears and points are represented as vectors
     * Otherwise, it's all put in a PagingLayer as vectors.  This is better for an overlay.
     */
    var imageVectorHybrid = true

    /* If set, we'll sort all polygons into the background.
     * Works well zoomed out, less enticing zoomed in.
     */
    var backgroundAllPolys = false

    /**
     * If set, we'll fetch and use the sources from the style sheet.
     * If not set, the sources have to be provided externally.
     */
    var fetchSources = true

    /**
     * If set, we'll fetch and use the sprites from the style sheet.
     */
    var fetchSprites = true

    /**
     * If set, a top level directory where we'll cache everything
     */
    var cacheDir: File? = null

    /**
     * You can override the file loaded for a particular purpose.
     * This includes: the TileJSON files, sprite sheets, and the style sheet itself
     * For example, if you want to load from the bundle, but not have to change
     *  anything in the style sheet, just do this
     */
    var mapboxURLFor: (Uri) -> Uri = {
        file: Uri ->
        file
    }

    /**
     * Control the sprite sheet resolution
     */
    var spriteResFor: (String) -> Int = { uri ->
        val isMapbox = uri.startsWith("mapbox://") ||
                       uri.contains("mapbox.com/")
        if (isMapbox) 4 else 2
    }

    /**
     * You can override the font to use for a given font name in the style.
     * Font names in the style often don't map directly to local font names.
     */
    var mapboxFontFor: (String) -> String = {
        name: String ->
        name
    }

    /**
     * Override HTTP request building to provide authorization, etc.
     */
    var requestFor: (Uri) -> Request.Builder = {
        Request.Builder().url(it.toString())
    }

    /**
     * If set, this will be called right after everything is set up
     * This is after all the configuration files are fetched so
     * you can make any final tweaks to loading objects here
     */
    var postSetup: (MapboxKindaMap) -> Unit = { }

    /**
     * This is the importance value used in the sampler for loading
     * It's roughly the maximum number of pixels you want a tile to be on the screen
     * before you load its children.
     * 1024^2 is good for vector tiles, 256^2 is good for image tiles
     */
    var minImportance = 1024.0 * 1024.0

    /**
     * Set higher than maxZoom to allow scaling of stylesheet
     * features beyond the zoom level allowed by the data
     */
    var reportedMaxZoom: Int? = null

    var sampleParams: SamplingParams? = null; private set

    // These are run after a successful load of all the style
    // sheet pieces, or immediately if loading has already finished.
    fun addPostLoadRunnable(r: Runnable) {
        synchronized(postLoadRunnables) {
            if (running) {
                r.run()
            } else {
                postLoadRunnables.add(r)
            }
        }
    }

    fun addErrorRunnable(r: (String,Boolean,Exception?) -> Unit) {
        synchronized(loadErrorRunnables) {
            loadErrorRunnables.add(r)
        }
    }

    val displayMetrics: DisplayMetrics get() =
        control.get()?.activity?.resources?.displayMetrics ?: Resources.getSystem().displayMetrics

    // Information about the sources as we fetch them
    private fun addTask(task: Call) {
        control.get()?.activity?.runOnUiThread {
            outstandingFetches.add(task)
        }
    }

    private fun clearTask(task: Call) {
        control.get()?.activity?.runOnUiThread {
            outstandingFetches.remove(task)
        }
    }

    // Check if we've finished loading stuff
    protected fun checkFinished() {
        // Start the map if no outstanding fetches are running
        control.get()?.activity?.runOnUiThread {
            if (!stopping && !finished && outstandingFetches.all { it == null }) {
                finished = true
                startLoader()
            }
        }     }

    // If we're using a cache dir, look for the file there
    protected fun cacheResolve(url: Uri) : File? {
        val fileRef = cacheName(url)
        if (fileRef != null && fileRef.exists()) {
            return fileRef
        }
        return null
    }

    // Generate a workable cache file path
    protected fun cacheName(url: Uri) : File? {
        // It's already local
        if (url.scheme == "file" && url.toFile().exists()) {
            return url.toFile()
        }

        // If the cache dir doesn't exist, we need to create it
        val theCacheDir = cacheDir ?: return null
        if (!theCacheDir.isDirectory && !theCacheDir.mkdirs())
            return null

        // Make up a cache name from the URL
        val cacheName = cacheNamePattern.replace(url.toString(), "_")
        return File(theCacheDir,cacheName)
    }

    // Write a file to cache if appropriate
    protected fun cacheFile(url: Uri, data: ByteArray) {
        // If there's no cache dir or the file is local, don't cache
        if (cacheDir == null || (url.scheme == "file" && url.toFile().exists()))
            return

        try {
            cacheName(url)?.let { theCacheName ->
                FileOutputStream(theCacheName).use { fOut ->
                    fOut.write(data)
                }
            }
        } catch (e: Exception) {
            reportLoadWarning("Failed to cache file '$url'", e)
        }
    }

    /**
     * Once you're done messing with the settings, start this puppy up.
     * It'll go fetch whatever it needs to fetch until it gets everything.
     * Then it'll start the actual loader.
     */
    fun start() {
        val theControl = control.get() ?: return

        if (styleSheetJSON != null) {
            // Style sheet is already loaded, so skip that part
            processStyleSheet()
        } else {
            val theStyleURL = styleURL ?: return

            // Dev might be overriding the source
            val resolvedURL = mapboxURLFor(theStyleURL)

            // todo: reduce duplication with processStylesheet
            try {
                cacheResolve(resolvedURL)?.let { cacheFile ->
                    val json = readFile(cacheFile)
                    if (json.isNotEmpty()) {
                        styleSheetJSON = json
                        processStyleSheet()
                        checkFinished()
                        return
                    }
                }
            } catch (ex: Exception) {
                reportLoadError("Failed to load cached stylesheet", ex)
            }

            // Go get the style sheet (this will also handle local)
            val client = theControl.getHttpClient()
            val task = client.newCall(requestFor(resolvedURL).build())
            addTask(task)
            task.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    reportLoadError("Error fetching style sheet", e)
                    stop()
                }
                @Throws(IOException::class)
                override fun onResponse(call: Call, response: Response) {
                    response.use {
                        if (finished) return@use    // already done loading, how?
                        if (!response.isSuccessful) {
                            reportLoadError("JSON stylesheet request failed: ${response.code} ${response.message}")
                            return@use
                        }
                        response.body?.use {
                            // Convert to string, treat empty as null
                            val s = String(it.bytes())
                            if (s.isNotEmpty()) s else null
                        }?.let { json ->
                            styleSheetJSON = json

                            cacheFile(theStyleURL, json.toByteArray())

                            try {
                                processStyleSheet()
                            } catch (ex: Exception) {
                                reportLoadError("Failed to process JSON stylesheet", ex)
                            }
                        } ?: run {
                            reportLoadError("No response for stylesheet JSON request")
                        }
                    }
                    clearTask(task)
                    checkFinished()
                }
            })
        }

        checkFinished()
    }

    // Style sheet has been loaded
    fun processStyleSheet() {
        val theControl = control.get()
        if (theControl == null || (styleURL == null && styleSheetJSON == null))
            return
        val client = theControl.getHttpClient()
        styleSheet = MapboxVectorStyleSet(styleSheetJSON, styleSettings, displayMetrics, theControl)

        // Fetch what we need to for the sources
        val sources = if (fetchSources) styleSheet?.sources else null
        sources?.filter{ it.tileSpec == null }?.forEach { source ->
            // If the tile spec isn't embedded, we need to go get it
            if (source.url?.isEmpty() != false) {
                Log.w("Maply", "Expecting either URL or tile info for a source.  Giving up.")
                return@forEach
            }

            val url = mapboxURLFor(Uri.parse(source.url))
            try {
                cacheResolve(url)?.let { cacheFile ->
                    val json = readFile(cacheFile)
                    if (json.isNotEmpty()) {
                        processStylesheetJson(source, json)
                        return@forEach
                    }
                }
            } catch (ex: Exception) {
                reportLoadWarning("Failed to load cached stylesheet", ex)
            }

            // Go fetch the TileJSON
            val task = client.newCall(requestFor(url).build())
            addTask(task)
            task.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    reportLoadError("Error trying to fetch tileJson", e)
                    clearTask(task)
                    stop()
                }
                override fun onResponse(call: Call, response: Response) {
                    response.use {
                        if (!response.isSuccessful) {
                            reportLoadError("Tile JSON request failed: ${response.code} ${response.message}")
                            return@use
                        }
                        response.body?.use {
                            // Convert to string, treat empty as null
                            val s = String(it.bytes())
                            if (s.isNotEmpty()) s else null
                        }?.let { json ->
                            cacheFile(Uri.parse(url.toString()), json.toByteArray())
                            try {
                                processStylesheetJson(source, json)
                            } catch (ex: Exception) {
                                reportLoadError("Failed to process tile JSON", ex)
                            }
                        }
                    }
                    clearTask(task)
                    checkFinished()
                }
            })
        }

        // Load the sprite sheets
        styleSheet?.spriteURL?.let {
            loadSprites(it, client)
        }

        checkFinished()
    }

    private fun loadSprites(spriteURL: String, client: OkHttpClient) {

        val res = spriteResFor(spriteURL)
        val resStr = if (res > 1) "@${res}x" else ""
        val spriteJSONUrl = mapboxURLFor(Uri.parse("$spriteURL$resStr.json"))
        val spritePNGUrl = mapboxURLFor(Uri.parse("$spriteURL$resStr.png"))
        try {
            cacheResolve(spriteJSONUrl)?.let { cacheFile ->
                spriteJSON = readFile(cacheFile)
            }
        } catch (ex: Exception) {
            reportLoadWarning("Failed to load cached sprite sheet", ex)
        }

        if (spriteJSON == null && fetchSprites) {
            val task1 = client.newCall(requestFor(spriteJSONUrl).build())
            addTask(task1)
            task1.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    reportLoadError("Error fetching sprite sheet", e)
                    stop()
                }
                override fun onResponse(call: Call, response: Response) {
                    response.use {
                        if (finished) return@use
                        if (!response.isSuccessful) {
                            reportLoadError("Sprite sheet request failed: ${response.code} ${response.message}")
                            return@use
                        }
                        response.body?.use {
                            // Convert to string, treat empty as null
                            val s = String(it.bytes())
                            if (s.isNotEmpty()) s else null
                        }?.let { json ->
                            // todo: validate that it's not some kind of error message
                            cacheFile(spriteJSONUrl, json.toByteArray())
                            spriteJSON = json
                        }
                    }
                    clearTask(task1)
                    checkFinished()
                }
            })
        }

        // Look for the PNG in the cache
        try {
            cacheResolve(spritePNGUrl)?.let { cacheFile ->
                FileInputStream(cacheFile).use {
                    spritePNG = BitmapFactory.decodeStream(it)
                }
            }
        } catch (ex: Exception) {
            reportLoadWarning("Failed to load cached sprite image", ex)
        }

        if (spritePNG == null && fetchSprites) {
            val task2 = client.newCall(requestFor(spritePNGUrl).build())
            addTask(task2)
            task2.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    reportLoadError("Error fetching sprite image", e)
                    stop()
                }
                override fun onResponse(call: Call, response: Response) {
                    response.use {
                        if (finished) return@use
                        if (!response.isSuccessful) {
                            reportLoadError("Sprite image request failed with ${response.code}: ${response.message}")
                            return@use
                        }
                        spritePNG = response.body?.use { body ->
                            try {
                                val bytes = body.bytes()
                                BitmapFactory.decodeByteArray(bytes, 0, bytes.size)?.also {
                                    // Parsed as an image, cache it
                                    cacheFile(spritePNGUrl, bytes)
                                }
                            } catch (ex: Exception) {
                                reportLoadError("Invalid sprite image response")
                                null
                            }
                        } ?: run {
                            reportLoadError("Empty sprite image response")
                            null
                        }
                    }
                    clearTask(task2)
                    checkFinished()
                }
            })
        }

        // Might have loaded from the caches
        checkFinished()
    }

    private fun processStylesheetJson(source: MapboxVectorStyleSet.Source, json: String) {
        // todo: find a better way to convert from `AttrDictionary` to `AttrDictionaryEntry`, like an `asEntry` method.
        AttrDictionary().apply {
            if (parseFromJSON("{\"tileSpec\":[$json]}")) {
                source.tileSpec = getArray("tileSpec")
            }
        }
    }

    // Everything has been fetched, so fire up the loader
    protected fun startLoader() {
        val theControl = control.get() ?: return

        // Figure out overall min/max zoom
        var minZoom = 10000
        var maxZoom = -1

        val tileInfos = ArrayList<TileInfoNew>()
        val localFetchers = ArrayList<MBTileFetcher>()

        styleSheet?.sources?.forEach { source ->
            source.tileSpec?.forEach { specItem ->
                specItem.dict?.let {
                    val itemMinZoom = it.getInt("minzoom")
                    val itemMaxZoom = it.getInt("maxzoom")
                    if (fetchSources) {
                        minZoom = (itemMinZoom ?: minZoom).coerceAtMost(minZoom)
                        maxZoom = (itemMaxZoom ?: maxZoom).coerceAtLeast(maxZoom)
                    }
                }
            }
            // Even if fetchSources is off, keep the highest maxZoom value to use in maxReportedZoom.
            source.maxZoom?.let { z ->
                sourceMaxZoom = sourceMaxZoom?.coerceAtLeast(z) ?: z
            }
        }

        if (!fetchSources) {
            localMBTiles?.forEach { item ->
                val fetcher = MBTileFetcher(theControl, item)
                maxConcurrentLoad?.let { fetcher.maxParsing = it }
                localFetchers.add(fetcher)
                fetcher.tileInfo?.also {
                    tileInfos.add(it)
                    minZoom = it.minZoom.coerceAtMost(minZoom)
                    maxZoom = it.maxZoom.coerceAtLeast(maxZoom)
                }
            }
        }

        // Sources probably weren't set up
        if (minZoom > maxZoom) {
            reportLoadError("Sources missing.  Bad zoom min/max.")
            return
        }

        // Adjustment for loading (512 vs 1024 or so)
        styleSettings.lineScale = lineScale

        // Similar adjustment for text
        styleSettings.textScale = textScale

        // And let's not forget markers and circles
        styleSettings.markerScale = markerScale

        // Parameters describing how we want a globe broken down
        val params = SamplingParams().also {
            it.coordSystem = SphericalMercatorCoordSystem()
            it.minImportance = minImportance
            it.singleLevel = true
            it.coverPoles = (theControl is GlobeController)
            it.edgeMatching = (theControl is GlobeController)
            it.minZoom = minZoom.coerceAtMost(1)
            it.maxZoom = maxZoom
            // Let the reported zoom go beyond the maximum
            it.reportedMaxZoom = (maxZoom + 1).coerceAtLeast(reportedMaxZoom ?: 0)
                                              .coerceAtLeast(sourceMaxZoom ?: 0)
        }
        sampleParams = params

        // If we don't have a solid under-layer for each tile, we can't really
        //  keep level 0 around all the time
        if (!backgroundAllPolys) {
            params.setForceMinLevel(false)
        } else {
            params.minImportanceTop = 0.0
        }

        // Image/vector hybrids draw the polygons into a background image
        if (imageVectorHybrid) {
            startHybridLoader(params, tileInfos, localFetchers)
        } else {
            startSimpleLoader(params, tileInfos, localFetchers)
        }

        // If the stylesheet has a background layer, use it to set the clear color for flat maps
        styleSheetVector?.let { ss ->
            (control.get() as? MapController)?.let { mc ->
                if (ss.hasBackgroundStyle()) {
                    // Set the background clear to the color at level 0
                    // TODO: Make this change by level
                    mc.setClearColor(ss.backgroundColorForZoom(0.0))
                }
            }
        }

        postSetup(this)
    }

    private fun startSimpleLoader(sampleParams: SamplingParams,
                                  tileInfos: ArrayList<TileInfoNew>,
                                  localFetchers: ArrayList<MBTileFetcher>) {
        val control = control.get() ?: return

        // todo: deal with sprite sheets

        val vectorStyleDict = AttrDictionary().apply {
            parseFromJSON(styleSheetJSON)
        }

        val styleSet = MapboxVectorStyleSet(vectorStyleDict, styleSettings, displayMetrics, control)
        styleSheetVector = styleSet

        // Set up the sprite sheet
        if (spriteJSON != null && spritePNG != null) {
            styleSheetVector?.addSprites(spriteJSON!!,spritePNG!!)
        }

        synchronized(this) {
            if (stopping) {
                return
            }
            mapboxInterp = MapboxVectorInterpreter(styleSet, control)
            loader = QuadPagingLoader(sampleParams, tileInfos.toTypedArray(), mapboxInterp, control).also {
                it.flipY = false
                if (localFetchers.isNotEmpty()) {
                    it.setTileFetcher(localFetchers[0])
                }
                it.debugMode = debugMode
            }
        }

        // Called after we've parsed the style sheet (again)
        synchronized(postLoadRunnables) {
            try {
                running = true
                postLoadRunnables.toTypedArray() // copy
            } finally {
                postLoadRunnables.clear()
            }
        }.forEach {
            it.run()
        }
     }

    private fun startHybridLoader(sampleParams: SamplingParams,
                                  tileInfos: ArrayList<TileInfoNew>,
                                  localFetchers: ArrayList<MBTileFetcher>) {
        val control = control.get() ?: return
        // Put together the tileInfoNew objects
        styleSheet?.sources?.mapNotNull { it.tileSpec }?.flatMap { it.asIterable() }
                           ?.mapNotNull { it.dict }?.forEach { tileSpec ->
            val minZoom = tileSpec.getInt("minzoom")
            val maxZoom = tileSpec.getInt("maxzoom")
            if (minZoom != null && maxZoom != null && minZoom < maxZoom) {
                // A tile source may list multiple URLs, but we only support one.
                tileSpec.getArray("tiles")
                        ?.mapNotNull { it.string }
                        ?.firstOrNull()?.let { tileUrl ->
                    tileInfos.add(RemoteTileInfoNew(tileUrl, minZoom, maxZoom).also { tileSource ->
                       if (cacheDir != null) {
                            val cacheName = cacheNamePattern.replace(tileUrl, "_")
                            tileSource.cacheDir = File(cacheDir, cacheName)
                        }
                    })
                }
            }
        }

        if (styleSheetJSON == null || stopping)
            return

        if (backgroundAllPolys) {
            synchronized(this) {
                if (stopping) {
                    return
                }

                // Set up an offline renderer and a Mapbox vector style handler to render to it
                val imageSize = 512
                offlineRender = RenderController(control.renderControl, imageSize, imageSize)
                val imageStyleSettings = VectorStyleSettings()
                imageStyleSettings.baseDrawPriority = styleSettings.baseDrawPriority
    
                // We only want the polygons in the image
                val imageStyleDict = AttrDictionary()
                imageStyleDict.parseFromJSON(styleSheetJSON)
                val imageLayers = imageStyleDict.getArray("layers")
                val newImageLayers = ArrayList<AttrDictionaryEntry>()
                for (layer in imageLayers ?: emptyArray()) {
                    if (layer.type == AttrDictionaryEntry.Type.DictTypeDictionary) {
                        val layerDict = layer.dict
                        val type = layerDict?.getString("type")
                        if (type != null && (type == "background" || type == "fill"))
                            newImageLayers.add(layer)
                    }
                }
                imageStyleDict.setArray("layers", newImageLayers.toTypedArray())
                styleSheetImage = MapboxVectorStyleSet(imageStyleDict, styleSettings, displayMetrics, offlineRender!!).also {
                    offlineRender!!.setClearColor(it.backgroundColorForZoom(0.0))
                }
            }
        }

        val vectorStyleDict = AttrDictionary()
        vectorStyleDict.parseFromJSON(styleSheetJSON)

        // The polygons only go into the background in this case
        if (backgroundAllPolys) {
            val vectorLayers = vectorStyleDict.getArray("layers")
            val newVectorLayers = ArrayList<AttrDictionaryEntry>()
            for (layer in vectorLayers ?: emptyArray()) {
                if (layer.type == AttrDictionaryEntry.Type.DictTypeDictionary) {
                    val layerDict = layer.dict
                    val type = layerDict?.getString("type")
                    if (type != null && (type != "background" && type != "fill"))
                        newVectorLayers.add(layer)
                }
            }
            vectorStyleDict.setArray("layers", newVectorLayers.toTypedArray())
        }

        val vecStyle = MapboxVectorStyleSet(vectorStyleDict, styleSettings, displayMetrics, control)
        val imgStyle = styleSheetImage
        styleSheetVector = vecStyle

        // Set up the sprite sheet
        if (spriteJSON != null && spritePNG != null) {
            styleSheetVector?.addSprites(spriteJSON!!,spritePNG!!)
        }
        
        synchronized(this) {
            if (stopping) {
                return
            }
            mapboxInterp = if (offlineRender != null && imgStyle != null) {
                MapboxVectorInterpreter(imgStyle, offlineRender!!, vecStyle, control)
            } else {
                MapboxVectorInterpreter(vecStyle, control)
            }
        }

        if (mapboxInterp == null) {
            reportLoadError("Failed to set up Mapbox interpreter", null)
            stop()
        }
    
        synchronized(this) {
            if (stopping) {
                return
            }
            loader = QuadImageLoader(sampleParams, tileInfos.toTypedArray(),
                    control, QuadLoaderBase.Mode.SingleFrame).apply {
                setBaseDrawPriority(styleSettings.baseDrawPriority)
                setDrawPriorityPerLevel(styleSettings.drawPriorityPerLevel)
                setLoaderInterpreter(mapboxInterp)
                setTileFetcher(localFetchers.firstOrNull() ?: RemoteTileFetcher(control, "Remote Tile Fetcher").apply {
                    debugMode = this@MapboxKindaMap.debugMode
                })
                debugMode = this@MapboxKindaMap.debugMode
            }
        }

        synchronized(postLoadRunnables) {
            try {
                running = true
                postLoadRunnables.toTypedArray() // copy
            } finally {
                postLoadRunnables.clear()
            }
        }.forEach {
            it.run()
        }
    }

    private fun reportLoadWarning(desc: String, ex: Exception? = null) = reportLoadError(desc, ex, false)
    private fun reportLoadError(desc: String, ex: Exception? = null) = reportLoadError(desc, ex, true)
    private fun reportLoadError(desc: String, ex: Exception?, fatal: Boolean) {
        Log.e("MapboxKindaMap", desc, ex)
        synchronized(loadErrorRunnables) {
            loadErrorRunnables.toTypedArray()
        }.forEach {
            it.invoke(desc, fatal, ex)
        }
    }

    // Stop trying to load data if we're doing that
    //  or shutdown the loader if we've gotten to that point
    fun stop() {
        synchronized(this) {
            stopping = true
            running = false
        }

        val theControl = control.get() ?: return
    
        // Gotta run on the main thread
        if (Looper.getMainLooper().thread != Thread.currentThread()) {
            val activity = theControl.activity
            if (activity != null) {
                activity.runOnUiThread {
                    stop()
                }
                return
            }
            Log.w("Maply", "No activity, shutting down on calling thread")
        }

        outstandingFetches.forEach {
            it?.cancel()
        }
        outstandingFetches.clear()

        loader?.shutdown()
        loader = null
        mapboxInterp = null

        offlineRender?.shutdown()
        offlineRender = null
    
        styleSheet?.shutdown()
        styleSheetVector?.shutdown()
        styleSheetImage?.shutdown()
        
        control.clear()
    }

    private fun readFile(fileName: String) = readFile(File(fileName))
    private fun readFile(file: File) =
        FileInputStream(file).use { stream->
            stream.bufferedReader().use { reader ->
                reader.readText()
            }
        }

    private val control : WeakReference<BaseController> = WeakReference<BaseController>(inControl)
    private val outstandingFetches = ArrayList<Call?>()

    private var sourceMaxZoom: Int? = null

    // Characters which we don't put in cache filenames.
    // Equals and Ampersand are valid, but get escaped by URI, and so tend to cause trouble.
    private val cacheNamePattern = Regex("[|?*<\":%@>+\\[\\]\\\\/=&]")
    private var finished = false
    
    private var stopping = false

    private val postLoadRunnables = ArrayList<Runnable>()
    private val loadErrorRunnables = ArrayList<(String,Boolean,Exception?) -> Unit>()

    init {
        val metrics = displayMetrics
        val dpi = (metrics.xdpi + metrics.ydpi) / 2.0
        lineScale = dpi / 230.0
        textScale = dpi / 150.0
        markerScale = dpi / 150.0
    }
}
