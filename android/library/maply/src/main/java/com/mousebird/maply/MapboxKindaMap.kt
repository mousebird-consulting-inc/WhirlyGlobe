package com.mousebird.maply

import android.graphics.Bitmap
import android.net.Uri
import android.os.Looper
import android.util.Log
import androidx.core.net.toFile
import okhttp3.Call
import okhttp3.Callback
import okhttp3.Request
import okhttp3.Response
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.OutputStream
import java.lang.ref.WeakReference
import java.net.URL

/**
 * Convenience class for loading a Mapbox-style vector tiles-probably kinda map.
 * You give it a style sheet and it figures out the rest.
 * Set the various settings before it gets going to modify how it works.
 * Callbacks control various pieces that might need to be intercepted.
 */
public open class MapboxKindaMap {
    var styleURL : Uri? = null
    var control : WeakReference<BaseController>? = null

    /* If set, we build an image/vector hybrid where the polygons go into
     *  the image layer and the linears and points are represented as vectors
     * Otherwise, it's all put in a PagingLayer as vectors.  This is better for an overlay.
     */
    public var imageVectorHybrid = true

    /* If set, we'll sort all polygons into the background
     * Works well zoomed out, less enticing zoomed in
     */
    public var backgroundAllPolys = true

    // If set, a top level directory where we'll cache everything
    public var cacheDir: File? = null

    /* You can override the file loaded for a particular purpose.
     * This includes: the TileJSON files, sprite sheets, and the style sheet itself
     * For example, if you want to load from the bundle, but not have to change
     *  anything in the style sheet, just do this
     */
    public open fun mapboxURLFor(file: Uri) : Uri {
        return file
    }

    /* You can override the font to use for a given
     *  font name in the style.  Font names in the style often don't map
     *  directly to local font names.
     */
    public open fun mapboxFontFor(name: String) : String {
        return name
    }

    /* This is the importance value used in the sampler for loading
     * It's roughly the maximum number of pixels you want a tile to be on the screen
     *  before you load its children.  1024 is good for vector tiles, 256 good for image tiles
     */
    public var minImportance = 1024.0 * 1024.0

    constructor(styleURL: Uri, control: BaseController) {
        this.control = WeakReference<BaseController>(control)
        this.styleURL = styleURL
        styleSettings.baseDrawPriority = QuadImageLoaderBase.BaseDrawPriorityDefault+1000
        styleSettings.drawPriorityPerLevel = 1
    }

    constructor(styleJSON: String, control: BaseController) {
        this.control = WeakReference<BaseController>(control)
        this.styleSheetJSON = styleJSON
        styleSettings.baseDrawPriority = QuadImageLoaderBase.BaseDrawPriorityDefault+1000
        styleSettings.drawPriorityPerLevel = 1
    }

    public var styleSettings = VectorStyleSettings()
    public var styleSheet: MapboxVectorStyleSet? = null
    public var styleSheetImage: MapboxVectorStyleSet? = null
    public var styleSheetVector: MapboxVectorStyleSet? = null
    public var styleSheetJSON: String? = null
    public var spriteJSON: ByteArray? = null
    public var spritePNG: Bitmap? = null

    // Information about the sources as we fetch them
    protected var outstandingFetches: ArrayList<Call?> = ArrayList<Call?>()
    private fun addTask(task: Call) {
        val theControl = control?.get()
        if (theControl == null)
            return

        theControl.getActivity().runOnUiThread {
            outstandingFetches.add(task)
        }
    }

    private fun clearTask(task: Call) {
        val theControl = control?.get()
        if (theControl == null)
            return

        theControl.getActivity().runOnUiThread {
            outstandingFetches.remove(task)
        }
    }

    private var finished = false

    // Check if we've finished loading stuff
    protected fun checkFinished() {
        if (finished)
            return

        val theControl = control?.get()
        if (theControl == null)
            return

        theControl.getActivity().runOnUiThread(object: Runnable {
            override fun run() {
                if (finished)
                    return

                var done = true

                // If any of the outstanding fetches are running, don't start the map
                outstandingFetches.forEach {
                    if (it != null)
                        done = false
                }

                // All done, so start
                if (done) {
                    finished = true
                    startLoader()
                }
            }
        })
    }

    public var mapboxInterp: MapboxVectorInterpreter? = null
    public var loader: QuadLoaderBase? = null
    public var offlineRender: RenderController? = null

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
        val theCacheDir = cacheDir
        if (theCacheDir == null)
            return null

        // If the cache dir doesn't exist, we need to create it
        if (!theCacheDir.exists() && !theCacheDir.createNewFile())
            return null

        // It's already local
        if (url.scheme == "file" && url.toFile().exists())
            return url.toFile()

        // Make up a cache name from the URL
        val cacheName = url.toString().replace("/","_").replace(":","_").replace("?","_").replace(".","_")
        return File(theCacheDir,cacheName)
    }

    // Write a file to cache if appropriate
    protected fun cacheFile(url: Uri, data: ByteArray) {
        // If there's no cache dir or the file is local, don't cache
        if (cacheDir == null || (url.scheme == "file" && url.toFile().exists()))
            return

        var fOut: OutputStream? = null
        try {
            val theCacheName = cacheName(url)
            cacheDir?.mkdirs()

            fOut = FileOutputStream(theCacheName)
            fOut.write(data)
        } catch (e: Exception) {
            Log.w("Maply", "Failed to cache file $e")
        }
        finally {
            fOut?.close()
        }
    }

    /**
     * Once you're done messing with the settings, start this puppy up.
     * It'll go fetch whatever it needs to fetch until it gets everything.
     * Then it'll start the actual loader.
     */
    public fun start() {
        val theControl = control?.get()
        if (theControl == null)
            return

        if (styleSheetJSON != null) {
            // Style sheet is already loaded, so skip that part
            processStyleSheet()
        } else {
            val theStyleURL = styleURL
            if (theStyleURL == null)
                return
            // Dev might be overriding the source
            val resolvedURL = cacheResolve(mapboxURLFor(theStyleURL))

            // Go get the style sheet (this will also handle local)
            val client = theControl.getHttpClient()
            val builder = Request.Builder().url(resolvedURL)
            val task = client.newCall(builder.build())
            addTask(task)
            task.enqueue(object : Callback {
                override fun onFailure(call: Call, e: IOException) {
                    Log.w("Maply", "Error fetching style sheet: \n$e")

                    stop()
                }

                @Throws(IOException::class)
                override fun onResponse(call: Call, response: Response) {
                    if (finished)
                        return

                    val jsonStr = response.body()?.string()
                    if (jsonStr == null) {
                        Log.w("Maply", "Error parsing style sheet")
                        return
                    }
                    styleSheetJSON = jsonStr
                    cacheFile(theStyleURL, response.body()!!.bytes())

                    processStyleSheet()

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
        val theControl = control?.get()
        if (theControl == null || (styleURL == null && styleSheetJSON == null))
            return
        val client = theControl.getHttpClient()
        styleSheet = MapboxVectorStyleSet(styleSheetJSON, styleSettings, theControl.activity.resources.displayMetrics, theControl)

        // Fetch what we need to for the sources
        var success = true
        styleSheet?.sources?.forEach {
            val source = it
            // If the tile spec isn't embedded, we need to go get it
            if (source.tileSpec == null && success) {
                if (source.url.isEmpty()) {
                    Log.w("Maply", "Expecting either URL or tile info for a source.  Giving up.")
                    success = false
                }
                val newURI = Uri.parse(source.url)
                val url = cacheResolve(mapboxURLFor(newURI))

                // Go fetch the TileJSON
                val builder2 = Request.Builder().url(url)
                val task2 = client.newCall(builder2.build())
                addTask(task2)
                task2.enqueue(object : Callback {
                    override fun onFailure(call: Call, e: IOException) {
                        Log.w("Maply", "Error trying to fetch tileJson: \n$e")

                        stop()
                    }

                    override fun onResponse(call: Call, response: Response) {
                        val jsonStr2 = response.body()?.string()
                        val resp = AttrDictionary()
                        resp.parseFromJSON(jsonStr2)
                        source.tileSpec = resp

                        val newUri = Uri.parse(url.toString())
                        cacheFile(newUri,response.body()!!.bytes())

                        clearTask(task2)
                        checkFinished()
                    }
                })
            }
        }

        if (!success)
            stop()
    }

    // Everything has been fetched, so fire up the loader
    protected fun startLoader() {
        val theControl = control?.get()
        val theStyleSheet = styleSheet
        if (theControl == null || theStyleSheet == null)
            return

        // Figure out overall min/max zoom
        var minZoom = 10000
        var maxZoom = -1
        styleSheet?.sources?.forEach {
            val source = it
            if (source.tileSpec.hasField("minzoom"))
                minZoom = source.tileSpec.getInt("minzoom")
            if (source.tileSpec.hasField("maxzoom"))
                maxZoom = source.tileSpec.getInt("maxzoom")
        }

        // Sources probably weren't set up
        if (minZoom > maxZoom) {
            Log.w("Maply", "Sources missing.  Bad zoom min/max.")
            return
        }

        // Image/vector hybrids draw the polygons into a background image
        if (imageVectorHybrid) {
            // Put together the tileInfoNew objects
            var tileInfos: ArrayList<RemoteTileInfoNew> = ArrayList<RemoteTileInfoNew>()
            styleSheet?.sources?.forEach {
                val source = it
                if (source.tileSpec.hasField("tiles")) {
                    val tiles = source.tileSpec.getArray("tiles")
                    val tileSrc = tiles.get(0).string
                    val tileSource = RemoteTileInfoNew(tileSrc, source.tileSpec.getInt("minzoom"), source.tileSpec.getInt("maxzoom"))
                    if (cacheDir != null) {
                        tileSource.cacheDir = File(cacheDir!!,tileSrc.replace("/","_").
                                        replace(":","_").replace("?","_").replace(".","_").
                                        replace("{","_").replace("}","_"))
                    }
                    tileInfos.add(tileSource)
                } else {
                    Log.w("Maply", "TileInfo source missing tiles.  Skipping.")
                }
            }

            // Parameters describing how we want a globe broken down
            val sampleParams = SamplingParams()
            sampleParams.coordSystem = SphericalMercatorCoordSystem()
            sampleParams.minImportance = minImportance
            sampleParams.singleLevel = true
            // If we don't have a solid underlayer for each tile, we can't really
            //  keep level 0 around all the time
            if (!backgroundAllPolys) {
                sampleParams.setForceMinLevel(false)
            }
            if (control?.get() is GlobeController) {
                sampleParams.coverPoles = true
                sampleParams.edgeMatching = true
            } else {
                sampleParams.coverPoles = false
                sampleParams.edgeMatching = false
            }
            sampleParams.minZoom = minZoom
            sampleParams.maxZoom = maxZoom

            if (styleSheetJSON == null)
                return

            if (backgroundAllPolys) {
                // Set up an offline renderer and a Mapbox vector style handler to render to it
                val imageSizeWidth = 512
                val imageSizeHeight = 512
                val offlineRender = RenderController(theControl.renderControl, imageSizeWidth,imageSizeHeight)
                this.offlineRender = offlineRender
                val imageStyleSettings = VectorStyleSettings()
                imageStyleSettings.baseDrawPriority = styleSettings.baseDrawPriority
                // TODO: Do we need this?
//                imageStyleSettings.arealShaderName = kMaplyShaderDefaultTriNoLighting

                // We only want the polygons in the image
                val imageStyleDict = AttrDictionary()
                imageStyleDict.parseFromJSON(styleSheetJSON)
                val imageLayers =  imageStyleDict.getArray("layers")
                var newImageLayers = ArrayList<AttrDictionaryEntry>()
                for (layer in imageLayers) {
                    if (layer.type == AttrDictionaryEntry.Type.DictTypeDictionary) {
                        val layerDict = layer.dict
                        val type = layerDict.getString("type")
                        if (type != null && (type == "background" || type == "fill"))
                            newImageLayers.add(layer)
                    }
                }
                imageStyleDict.setArray("layers",newImageLayers.toTypedArray())
                styleSheetImage = MapboxVectorStyleSet(imageStyleDict, styleSettings, theControl.activity.resources.displayMetrics, offlineRender)
                offlineRender.setClearColor(styleSheetImage!!.backgroundColorForZoom(0.0))
            }

            val vectorStyleDict = AttrDictionary()
            vectorStyleDict.parseFromJSON(styleSheetJSON)

            // The polygons only go into the background in this case
            if (backgroundAllPolys) {
                val vectorLayers = vectorStyleDict.getArray("layers")
                var newVectorLayers = ArrayList<AttrDictionaryEntry>()
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
            styleSheetVector = MapboxVectorStyleSet(vectorStyleDict, styleSettings, theControl.activity.resources.displayMetrics, theControl)

            if (control?.get() !is GlobeController) {
                // Set the background clear to the color at level 0
                // TODO: Make this change by level
                val color = styleSheetVector?.backgroundColorForZoom(0.0)
                if (color != null)
                    theControl.setClearColor(color)
            }

            if (offlineRender != null && styleSheetImage != null) {
                mapboxInterp = MapboxVectorInterpreter(styleSheetImage, offlineRender, styleSheetVector, theControl)
            } else {
                mapboxInterp = MapboxVectorInterpreter(styleSheetVector, theControl)
            }
            if (mapboxInterp == null) {
                Log.w("Maply", "Failed to set up Mapbox interpreter.  Nothing will appear.")
                stop()
            }

            // TODO: Handle more than one source
            if (backgroundAllPolys) {
                val imageLoader = QuadImageLoader(sampleParams, tileInfos[0], theControl)
                imageLoader.setLoaderInterpreter(mapboxInterp!!)
                loader = imageLoader
            } else {
                val vecLoader = QuadPagingLoader(sampleParams, tileInfos[0], mapboxInterp, theControl)
                loader = vecLoader
            }

        } else {
            Log.w("Maply", "Non-hybrid case not currently hooked up for 3.0")
        }
    }

    // Stop trying to load data if we're doing that
    //  or shutdown the loader if we've gotten to that point
    fun stop() {
        val theControl = control?.get()
        if (theControl == null)
            return

        // Gotta run on the main thread
        if (Looper.getMainLooper().thread != Thread.currentThread()) {
            theControl.getActivity().runOnUiThread { stop() }

            return
        }

        outstandingFetches.forEach {
            it?.cancel()
        }
        outstandingFetches.clear()

        loader?.shutdown()
        loader = null
        mapboxInterp = null

        control = null
    }

}