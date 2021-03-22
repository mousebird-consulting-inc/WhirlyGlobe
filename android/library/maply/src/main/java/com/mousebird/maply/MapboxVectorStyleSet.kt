/*  MapboxVectorStyleSet.kt
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford
 *  Copyright 2011-2021 mousebird consulting
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

import android.graphics.*
import android.util.DisplayMetrics
import android.util.Log
import com.mousebird.maply.RenderController.EmptyIdentity
import java.lang.ref.WeakReference
import java.util.*
import java.util.concurrent.ConcurrentMap
import java.util.concurrent.ConcurrentSkipListMap
import java.util.regex.Pattern
import kotlin.math.ceil

/**
 * Mapbox Vector Style Set.
 * This parses a Mapbox style sheet and interfaces with the vector parser
 */
class MapboxVectorStyleSet : VectorStyleInterface {

    constructor(
        styleJSON: String?,
        inSettings: VectorStyleSettings,
        inDisplayMetrics: DisplayMetrics,
        inControl: RenderControllerInterface
    ) {
        val styleDict = AttrDictionary()
        require(styleDict.parseFromJSON(styleJSON)) { "Bad JSON for style sheet in MapboxVectorStyleSet" }
        combinedInit(styleDict, inSettings, inDisplayMetrics, inControl)
    }

    // Construct with the JSON data from a string
    constructor(
        styleDict: AttrDictionary,
        inSettings: VectorStyleSettings,
        inDisplayMetrics: DisplayMetrics,
        inControl: RenderControllerInterface
    ) {
        combinedInit(styleDict, inSettings, inDisplayMetrics, inControl)
    }

    /**
     * Allows the application to override the handling of typeface lookups.
     */
    interface TypefaceDelegate {
        /**
         * Provide a specific typeface from whatever source in response to a name from the styles.
         * The result is cached.
         */
        fun getTypeface(name: String?): Typeface?

        /**
         * If not providing a Typeface, modify the name passed to the Typeface.create
         */
        fun mapTypefaceName(name: String?): String?
    }

    fun setTypefaceDelegate(delegate: TypefaceDelegate?) {
        typefaceDelegate = delegate
    }

    // Used by both constructors
    private fun combinedInit(
        styleDict: AttrDictionary,
        inSettings: VectorStyleSettings?,
        inDisplayMetrics: DisplayMetrics,
        inControl: RenderControllerInterface
    ) {
        // Fault in the ComponentObject native implementation.
        // Because the first time it can be called in this case is C++ side
        val testObj = ComponentObject()

        control = WeakReference(inControl)
        settings = inSettings ?: VectorStyleSettings()
        spriteURL = styleDict.getString("sprite")
        displayMetrics = inDisplayMetrics

        // Sources tell us where to get tiles
        styleDict.getDict("sources")?.let { sourcesDict ->
            val keys = sourcesDict.keys
            for (key in keys) {
                try {
                    sources.add(Source(key, sourcesDict.getDict(key), this))
                } catch (e: Exception) {
                    Log.w(javaClass.simpleName, "Error while adding source '" + key + "' : " + e.message)
                }
            }
        }
        initialise(inControl.scene, inControl.coordSystem, settings, styleDict)
    }

    /**
     * Set this to override the regular fill shader.
     * Useful if you're going to mix something else into the polygons.
     */
    fun setArealShader(shader: Shader) {
        setArealShaderNative(shader.id)
    }

    // Calculate an appropriate background color given the zoom level
    override fun backgroundColorForZoom(zoom: Double): Int {
        return backgroundColorForZoomNative(zoom)
    }

    // Return a label info
    // Called from JNI
    fun labelInfoForFont(fontName: String, fontSize: Float): LabelInfo {
        labelInfoMap[SizedTypeface(fontName, fontSize)]?.let { return it }

        // Give the delegate a chance to produce a typeface or modify the name.
        val typeface = typefaceDelegate?.getTypeface(fontName) ?:
                typefaceDelegate?.mapTypefaceName(fontName)?.let {
                    Typeface.create(it, Typeface.NORMAL)
            } ?: run {
                // We need to create something directly from the style, so try to do something
                // reasonable for the style parameter.
                // todo: would it work better to remove bold, italic, normal, regular, etc., from the name?
                var style = Typeface.NORMAL
                if (boldPattern.matcher(fontName).matches()) {
                    style = style or Typeface.BOLD
                }
                if (italicPattern.matcher(fontName).matches()) {
                    style = style or Typeface.ITALIC
                }
                Typeface.create(fontName, style)
            }

        // Add the typeface to the cache map.  If it's already there, a concurrent thread beat
        // us to it; use that result instead for consistent results.

        val labelInfo = LabelInfo()
        labelInfo.typeface = typefaceMap.putIfAbsent(fontName, typeface) ?: typeface
        labelInfo.setFontSize(fontSize)
        labelInfo.fontName = fontName

        // Same with the size-specific label info
        return labelInfoMap.putIfAbsent(SizedTypeface(fontName, fontSize), labelInfo) ?: labelInfo
    }

    // Calculate text width based on the typeface
    // Called from JNI
    fun calculateTextWidth(text: String, labelInfo: LabelInfo): Double {
        val paint = Paint()
        paint.textSize = labelInfo.fontSize
        paint.typeface = labelInfo.typeface
        val bounds = Rect()
        paint.getTextBounds(text, 0, text.length, bounds)
        return (bounds.right - bounds.left).toDouble()
    }

    fun makeCircleTexture(inRadius: Double,
                          fillColor: Int,
                          strokeColor: Int,
                          inStrokeWidth: Float,
                          /* out */ circleSize: Point2d?): Long /*Identity*/ {
        val control = control?.get() ?: return EmptyIdentity

        // We want the texture a bit bigger than specified
        val scale = (settings?.markerScale ?: 1.0) * 2.0

        // Build an image for the circle
        val buffer = 1.0
        val radius = inRadius * scale
        val strokeWidth = inStrokeWidth * scale
        val size = ceil((buffer + radius + strokeWidth) * 2.0).toInt()
    
        circleSize?.setValue(size / 2.0, size / 2.0)

        val bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        bitmap.eraseColor(Color.TRANSPARENT)
        
        val canvas = Canvas(bitmap)
    
        val paint = Paint(Paint.FILTER_BITMAP_FLAG.or(Paint.ANTI_ALIAS_FLAG))

        // Outer stroke
        if (strokeWidth > 0) {
            paint.style = Paint.Style.FILL
            paint.color = strokeColor
            canvas.drawCircle(size/2.0f,size/2.0f,(radius+strokeWidth).toFloat(),paint)
        }
        
        // Inner circle
        paint.style = Paint.Style.FILL
        paint.color = fillColor
        canvas.drawCircle(size/2.0f,size/2.0f, radius.toFloat(),paint)

        val texSettings = RenderControllerInterface.TextureSettings().apply {
            filterType = RenderControllerInterface.TextureSettings.FilterType.FilterLinear
            imageFormat = RenderController.ImageFormat.MaplyImage4Layer8Bit
        }
        val tex = control.addTexture(bitmap, texSettings, ThreadMode.ThreadCurrent)
        return tex?.texID ?: EmptyIdentity
    }

    fun makeLineTexture(comp: DoubleArray): Long /*Identity*/ {
//        NSMutableArray *dashComp = [NSMutableArray array];
//        for (double comp: inComp)
//        [dashComp addObject:[NSNumber numberWithDouble:comp]];
//
//        MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
//        [lineTexBuilder setPattern:dashComp];
//        UIImage *lineImage = [lineTexBuilder makeImage];
//        MaplyTexture *tex = [viewC addTexture:lineImage
//                desc:@{kMaplyTexFormat: @(MaplyImageIntRGBA),
//                       kMaplyTexWrapY: @(MaplyImageWrapY)
//        }
//        mode:MaplyThreadCurrent];
//        textures.push_back(tex);
//
//        return tex.texID;
        return EmptyIdentity
    }
    
    enum class SourceType {
        Vector, Raster
    }

    // Source for vector tile (or raster) data
    inner class Source internal constructor(// Name as it appears in the file
        var name: String, styleEntry: AttrDictionary, styleSet: MapboxVectorStyleSet?
    ) {
        // Either vector or raster at present
        var type: SourceType? = null

        // TileJSON URL, if present
        var url: String?

        // If the TileJSON spec is inline, it's here
        var tileSpec: Array<AttrDictionaryEntry>?

        init {
            val typeStr = styleEntry.getString("type")
            type = if (typeStr == "vector") {
                SourceType.Vector
            } else if (typeStr == "raster") {
                SourceType.Raster
            } else {
                throw IllegalArgumentException("Unexpected type string in Mapbox Source")
            }
            url = styleEntry.getString("url")
            tileSpec = styleEntry.getArray("tiles")
            if (url == null && tileSpec == null) {
                Log.w("Maply", "Expecting either URL or tileSpec in source $name")
                throw IllegalArgumentException("Expecting either URL or tileSpec in source $name")
            }
        }
    }

    /**
     * These are actually implemented on the C++ side, which communicates
     * with itself.  But we need to here to appear to be using the standard
     * interface.
     */
    override fun stylesForFeature(
        attrs: AttrDictionary,
        tileID: TileID,
        layerName: String,
        controller: RenderControllerInterface
    ): Array<VectorStyle>? {
        return null
    }

    override fun allStyles(): Array<VectorStyle>? {
        return null
    }

    override fun layerShouldDisplay(layerName: String, tileID: TileID): Boolean {
        return false
    }

    override fun styleForUUID(uuid: Long, controller: RenderControllerInterface): VectorStyle? {
        return null
    }

    /**
     * Get the zoom slot, or -1
     */
    external override fun getZoomSlot(): Int

    /**
     * Capture the zoom slot if you're going use it
     */
    external override fun setZoomSlot(inZoomSlot: Int)

    fun finalize() {
        dispose()
    }

    companion object {
        private val boldPattern = Pattern.compile("[\\s-_]bold\\b", Pattern.CASE_INSENSITIVE)
        private val italicPattern = Pattern.compile("[\\s-_]italic\\b", Pattern.CASE_INSENSITIVE)

        @JvmStatic
        private external fun nativeInit()

        init {
            nativeInit()
        }
    }

    var sources = ArrayList<Source>()

    // If there's a sprite sheet, where it's at
    var spriteURL: String? = null

    var settings: VectorStyleSettings? = null

    private var displayMetrics: DisplayMetrics? = null

    private var control: WeakReference<RenderControllerInterface>? = null

    private var typefaceDelegate: TypefaceDelegate? = null

    private val typefaceMap: ConcurrentMap<String, Typeface> =
        ConcurrentSkipListMap(String.CASE_INSENSITIVE_ORDER)

    private data class SizedTypeface(val fontName: String, val size: Float) : Comparable<SizedTypeface> {
        override fun compareTo(other: SizedTypeface): Int {
            val result = fontName.compareTo(other.fontName)
            return if (result == 0) size.compareTo(other.size) else result
        }
    }

    private val labelInfoMap: ConcurrentMap<SizedTypeface, LabelInfo> =
        ConcurrentSkipListMap()

    // JNI stuff

    private external fun setArealShaderNative(shaderID: Long)
    private external fun backgroundColorForZoomNative(zoom: Double): Int
    private external fun initialise(
        scene: Scene?,
        coordSystem: CoordSystem?,
        settings: VectorStyleSettings?,
        styleDict: AttrDictionary?
    )

    external fun dispose()
    protected var nativeHandle: Long = 0
}
