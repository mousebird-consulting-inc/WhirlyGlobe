package com.mousebird.maply

import android.content.res.Resources
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.Looper
import android.util.DisplayMetrics
import android.util.Log
import androidx.core.net.toFile
import okhttp3.Call
import okhttp3.Callback
import okhttp3.Request
import okhttp3.Response
import java.io.*
import java.lang.ref.WeakReference
import java.net.URL
import kotlin.collections.ArrayList

/**
 * Convenience class for loading a Mapbox-style vector tiles-probably kinda map.
 * You give it a style sheet and it figures out the rest.
 * Set the various settings before it gets going to modify how it works.
 * Callbacks control various pieces that might need to be intercepted.
 */
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

    var styleSheet: MapboxVectorStyleSet? = null
    var styleSheetImage: MapboxVectorStyleSet? = null
    var styleSheetVector: MapboxVectorStyleSet? = null
    var spriteJSON: String? = null
    var spritePNG: Bitmap? = null
    var mapboxInterp: MapboxVectorInterpreter? = null
    var loader: QuadLoaderBase? = null
    var offlineRender: RenderController? = null
    var lineScale = 0.0
    var textScale = 0.0
    var markerScale = 0.0
    var maxConcurrentLoad: Int? = null

    // These are run after a successful load of all the style sheet pieces
    var postLoadRunnables = ArrayList<Runnable>()

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
     * You can override the font to use for a given font name in the style.
     * Font names in the style often don't map directly to local font names.
     */
    var mapboxFontFor: (String) -> String = {
        name: String ->
        name
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
    
    var sampleParams: SamplingParams? = null; private set
    
    private val displayMetrics: DisplayMetrics get() =
        control.get()?.activity?.resources?.displayMetrics ?: Resources.getSystem().displayMetrics

    // Information about the sources as we fetch them
    private fun addTask(task: Call) {
        control.get()?.getActivity()?.runOnUiThread {
            outstandingFetches.add(task)
        }
    }

    private fun clearTask(task: Call) {
        control.get()?.getActivity()?.runOnUiThread {
            outstandingFetches.remove(task)
        }
    }

    // Check if we've finished loading stuff
    protected fun checkFinished() {
        // Start the map if no outstanding fetches are running
        control.get()?.getActivity()?.runOnUiThread {
            if (!finished && outstandingFetches.all { it == null }) {
                finished = true
                startLoader()
            }
        }
    }

    // If we're using a cache dir, look for the file there
    protected fun cacheResolve(url: Uri) : URL {
        val fileRef = cacheName(url)
        if (fileRef != null && fileRef.exists()) {
            return URL(Uri.fromFile(fileRef).toString())
        }

        return URL(url.toString())
    }

    // Generate a workable cache file path
    protected fun cacheName(url: Uri) : File? {
        // It's already local
        if (url.scheme == "file" && url.toFile().exists())
            return url.toFile()

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
            Log.w("Maply", "Failed to cache file $e")
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
                val cacheUrl = cacheResolve(resolvedURL)
                if (cacheUrl.toString().startsWith("file:")) {
                    FileInputStream(cacheUrl.file).use {
                        val json = it.bufferedReader().readText()
                        if (json.isNotEmpty()) {
                            styleSheetJSON = json
                            processStyleSheet()
                            checkFinished()
                            return
                        }
                    }
                }
            } catch (ex: Exception) {
                Log.e("MapboxKindaMap", "Failed to load cached stylesheet", ex)
            }

            // Go get the style sheet (this will also handle local)
            val client = theControl.getHttpClient()
            val builder = Request.Builder().url(resolvedURL.toString())
            val task = client.newCall(builder.build())
            addTask(task)
            task.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    Log.w("Maply", "Error fetching style sheet",e)
                    stop()
                }
                @Throws(IOException::class)
                override fun onResponse(call: Call, response: Response) {
                    response.use {
                        if (!finished) {
                            it.body?.use { body ->
                                val bytes = body.bytes()
                                if (bytes.isNotEmpty()) {
                                    styleSheetJSON = String(bytes)
                                    cacheFile(theStyleURL, bytes)
                                    processStyleSheet()
                                }
                            }
                        }
                    }
                    clearTask(task)
                    checkFinished()
                }
            })
        }

        // TODO: Look for the sprite sheets

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
                val cacheUrl = cacheResolve(url)
                if (cacheUrl.protocol == "file" && File(cacheUrl.file).isFile) {
                    FileInputStream(cacheUrl.file).use {
                        val json = it.bufferedReader().readText()
                        if (json.isNotEmpty()) {
                            processStylesheetJson(source, json)
                            checkFinished()
                            return@forEach
                        }
                    }
                }
            } catch (ex: Exception) {
                Log.e("MapboxKindaMap", "Failed to load cached stylesheet", ex)
            }

            // Go fetch the TileJSON
            val task = client.newCall(Request.Builder().url(url.toString()).build())
            addTask(task)
            task.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    Log.w("MapboxKindaMap","Error trying to fetch tileJson",e)
                    clearTask(task)
                    stop()
                }
                override fun onResponse(call: Call, response: Response) {
                    response.use {
                        it.body?.use { body ->
                            val bytes = body.bytes()
                            if (bytes.isNotEmpty()) {
                                Log.d("MapboxKindaMap",
                                    "Received ${bytes.size} bytes of ${body.contentType()} for $url")
                                val json = String(bytes)
                                if (json.isNotEmpty()) {
                                    processStylesheetJson(source, json)
                                    cacheFile(Uri.parse(url.toString()), bytes)
                                }
                            }
                        }
                    }
                    clearTask(task)
                    checkFinished()
                }
            })
        }

        // Load the spread sheets
        val spriteURL = styleSheet?.spriteURL
        if (spriteURL != null) {
            val spriteJSONurl = mapboxURLFor(Uri.parse(spriteURL + "@2x.json"))
            val spritePNGurl = mapboxURLFor(Uri.parse(spriteURL + "@2x.png"))
            val client = theControl.getHttpClient()
            try {
                val cacheUrl = cacheResolve(spriteJSONurl)
                if (cacheUrl.protocol == "file" && File(cacheUrl.file).isFile) {
                    FileInputStream(cacheUrl.file).use {
                        val json = it.bufferedReader().readText()
                        if (json.isNotEmpty()) {
                            spriteJSON = json
                        }
                    }
                }
            } catch (ex: Exception) {
                Log.e("MapboxKindaMap", "Failed to load cached sprite sheet", ex)
            }

            if (spriteJSON == null) {
                val task1 = client.newCall(Request.Builder().url(spriteJSONurl.toString()).build())
                addTask(task1)
                task1.enqueue(object : Callback {
                    override fun onFailure(call: Call, e: IOException) {
                        Log.w("Maply", "Error fetching sprite sheet", e)
                        stop()
                    }

                    override fun onResponse(call: Call, response: Response) {
                        response.use {
                            if (!finished) {
                                it.body?.use { body ->
                                    val bytes = body.bytes()
                                    if (bytes.isNotEmpty()) {
                                        spriteJSON = String(bytes)
                                        cacheFile(spriteJSONurl, bytes)
                                    }
                                }
                            }
                        }
                        clearTask(task1)
                        checkFinished()
                    }
                })
            }

            // Look for the PNG in the cache
            try {
                val cacheUrl = cacheResolve(spritePNGurl)
                if (cacheUrl.protocol == "file" && File(cacheUrl.file).isFile) {
                    FileInputStream(cacheUrl.file).use {
                        spritePNG = BitmapFactory.decodeFile(cacheUrl.path)
                    }
                }
            } catch (ex: Exception) {
                Log.e("MapboxKindaMap", "Failed to load cached sprite image", ex)
            }

            if (spritePNG == null) {
                val task2 = client.newCall(Request.Builder().url(spritePNGurl.toString()).build())
                addTask(task2)
                task2.enqueue(object : Callback {
                    override fun onFailure(call: Call, e: IOException) {
                        Log.w("Maply", "Error fetching sprite image", e)
                        stop()
                    }

                    override fun onResponse(call: Call, response: Response) {
                        response.use {
                            if (!finished) {
                                it.body?.use { body ->
                                    val bytes = body.bytes()
                                    if (bytes.isNotEmpty()) {
                                        spritePNG =
                                            BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
                                        cacheFile(spritePNGurl, bytes)
                                    }
                                }
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
    }

    private fun processStylesheetJson(source: MapboxVectorStyleSet.Source, json: String) {
        // todo: find a better way to convert from `AttrDictionary` to `AttrDictionaryEntry`, like an `asEntry` method.
        AttrDictionary().apply {
            if (parseFromJSON("{\"tileSpec\":[$json]}")) {
                source.tileSpec = getArray("tileSpec")
            }
        }
        checkFinished()
    }

    // Everything has been fetched, so fire up the loader
    protected fun startLoader() {
        val theControl = control.get() ?: return

        // Figure out overall min/max zoom
        var minZoom = 10000
        var maxZoom = -1

        val tileInfos = ArrayList<TileInfoNew>()
        val localFetchers = ArrayList<MBTileFetcher>()
        if (fetchSources) {
            styleSheet?.sources?.forEach { source ->
                source.tileSpec?.forEach { specItem ->
                    specItem.dict?.let {
                        minZoom = (it.getInt("minzoom") ?: minZoom).coerceAtMost(minZoom)
                        maxZoom = (it.getInt("maxzoom") ?: maxZoom).coerceAtLeast(maxZoom)
                    }
                }
            }
        } else {
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
            Log.w("Maply", "Sources missing.  Bad zoom min/max.")
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
            it.minZoom = minZoom
            it.maxZoom = maxZoom
            // Let the reported zoom go beyond the maximum
            it.reportedMaxZoom = 21
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

        styleSheetVector = MapboxVectorStyleSet(vectorStyleDict, styleSettings, displayMetrics, control)

        // Set up the sprite sheet
        if (spriteJSON != null && spritePNG != null) {
            styleSheetVector?.addSprites(spriteJSON!!,spritePNG!!)
        }

        // Called after we've parsed the style sheet (again)
        postLoadRunnables.forEach {
            it.run()
        }

        mapboxInterp = MapboxVectorInterpreter(styleSheetVector, control)
        loader = QuadPagingLoader(sampleParams, tileInfos.toTypedArray(), mapboxInterp, control).also {
            it.flipY = false
            if (localFetchers.isNotEmpty()) {
                it.setTileFetcher(localFetchers[0])
            }
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
            if (minZoom < maxZoom) {
                // A tile source may list multiple URLs, but we only support one.
                tileSpec.getArray("tiles")
                        .mapNotNull { it.string }
                        .firstOrNull()?.let { tileUrl ->
                    tileInfos.add(RemoteTileInfoNew(tileUrl, minZoom, maxZoom).also { tileSource ->
                       if (cacheDir != null) {
                            val cacheName = cacheNamePattern.replace(tileUrl, "_")
                            tileSource.cacheDir = File(cacheDir, cacheName)
                        }
                    })
                }
            }
        }

        if (styleSheetJSON == null)
            return

        if (backgroundAllPolys) {
            // Set up an offline renderer and a Mapbox vector style handler to render to it
            val imageSizeWidth = 512
            val imageSizeHeight = 512
            val offlineRender = RenderController(control.renderControl, imageSizeWidth,imageSizeHeight)
            this.offlineRender = offlineRender
            val imageStyleSettings = VectorStyleSettings()
            imageStyleSettings.baseDrawPriority = styleSettings.baseDrawPriority
            // TODO: Do we need this?
//                imageStyleSettings.arealShaderName = kMaplyShaderDefaultTriNoLighting

            // We only want the polygons in the image
            val imageStyleDict = AttrDictionary()
            imageStyleDict.parseFromJSON(styleSheetJSON)
            val imageLayers =  imageStyleDict.getArray("layers")
            val newImageLayers = ArrayList<AttrDictionaryEntry>()
            for (layer in imageLayers) {
                if (layer.type == AttrDictionaryEntry.Type.DictTypeDictionary) {
                    val layerDict = layer.dict
                    val type = layerDict.getString("type")
                    if (type != null && (type == "background" || type == "fill"))
                        newImageLayers.add(layer)
                }
            }
            imageStyleDict.setArray("layers",newImageLayers.toTypedArray())
            styleSheetImage = MapboxVectorStyleSet(imageStyleDict, styleSettings, displayMetrics, offlineRender).also {
                offlineRender.setClearColor(it.backgroundColorForZoom(0.0))
            }
        }

        val vectorStyleDict = AttrDictionary()
        vectorStyleDict.parseFromJSON(styleSheetJSON)

        // Called after we've parsed the style sheet (again)
        postLoadRunnables.forEach {
            it.run()
        }

        // The polygons only go into the background in this case
        if (backgroundAllPolys) {
            val vectorLayers = vectorStyleDict.getArray("layers")
            val newVectorLayers = ArrayList<AttrDictionaryEntry>()
            for (layer in vectorLayers) {
                if (layer.type == AttrDictionaryEntry.Type.DictTypeDictionary) {
                    val layerDict = layer.dict
                    val type = layerDict.getString("type")
                    if (type != null && (type != "background" && type != "fill"))
                        newVectorLayers.add(layer)
                }
            }
            vectorStyleDict.setArray("layers", newVectorLayers.toTypedArray())
        }
        styleSheetVector = MapboxVectorStyleSet(vectorStyleDict, styleSettings, displayMetrics, control)

        // Set up the sprite sheet
        if (spriteJSON != null && spritePNG != null) {
            styleSheetVector?.addSprites(spriteJSON!!,spritePNG!!)
        }

        mapboxInterp = if (offlineRender != null && styleSheetImage != null) {
            MapboxVectorInterpreter(styleSheetImage, offlineRender, styleSheetVector, control)
        } else {
            MapboxVectorInterpreter(styleSheetVector, control)
        }

        if (mapboxInterp == null) {
            Log.w("Maply", "Failed to set up Mapbox interpreter.  Nothing will appear.")
            stop()
        }

        loader = QuadImageLoader(sampleParams, tileInfos.toTypedArray(), control, QuadLoaderBase.Mode.SingleFrame).apply {
            setBaseDrawPriority(styleSettings.baseDrawPriority)
            setDrawPriorityPerLevel(styleSettings.drawPriorityPerLevel)
            setLoaderInterpreter(mapboxInterp)
            setTileFetcher(localFetchers.firstOrNull() ?:
                RemoteTileFetcher(control,"Remote Tile Fetcher").apply {
                    setDebugMode(false)
                })
            setDebugMode(false)
        }
    }

    // Stop trying to load data if we're doing that
    //  or shutdown the loader if we've gotten to that point
    fun stop() {
        val theControl = control.get() ?: return
    
        // Gotta run on the main thread
        if (Looper.getMainLooper().thread != Thread.currentThread()) {
            theControl.getActivity().runOnUiThread { stop() }

            return
        }

        outstandingFetches.forEach {
            it?.cancel()
        }
        outstandingFetches.clear()

        styleSheet?.shutdown()
        styleSheetVector?.shutdown()
        styleSheetImage?.shutdown()

        loader?.shutdown()
        loader = null
        mapboxInterp = null
        control.clear()
    }

    private val control : WeakReference<BaseController> = WeakReference<BaseController>(inControl)
    private val outstandingFetches = ArrayList<Call?>()
    private val cacheNamePattern = Regex("[|?*<\":%@>+\\[\\]\\\\/]")
    private var finished = false

    init {
        val metrics = displayMetrics
        val dpi = (metrics.xdpi + metrics.ydpi) / 2.0
        lineScale = dpi / 230.0
        textScale = dpi / 150.0
        markerScale = dpi / 150.0
    }
}
