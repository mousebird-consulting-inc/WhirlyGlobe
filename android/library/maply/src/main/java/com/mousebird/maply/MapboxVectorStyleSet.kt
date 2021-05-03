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
import android.util.*
import androidx.annotation.ColorInt
import androidx.core.util.component1
import androidx.core.util.component2
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
        @Suppress("UNUSED_VARIABLE") val testObj = ComponentObject()

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
                    Log.w("Maply", "Error while adding source '$key'", e)
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

    // Return true if the stylesheet contains a background style layer
    external fun hasBackgroundStyle(): Boolean

    // Calculate an appropriate background color given the zoom level
    override fun backgroundColorForZoom(zoom: Double): Int {
        return backgroundColorForZoomNative(zoom)
    }

    // Return a label info
    @Suppress("unused") // called from JNI
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
        // We scale the font size up double
        labelInfo.setFontSize(fontSize)
        labelInfo.fontName = fontName
        // TODO: See if we could get this from the typeface
        labelInfo.fontPointSize = 32.0f;

        // Same with the size-specific label info
        return labelInfoMap.putIfAbsent(SizedTypeface(fontName, fontSize), labelInfo) ?: labelInfo
    }

    // Calculate text width based on the typeface
    @Suppress("unused") // called from JNI
    fun calculateTextWidth(text: String, labelInfo: LabelInfo): Double {
        val paint = Paint()
        paint.textSize = labelInfo.fontSize
        paint.typeface = labelInfo.typeface
        val bounds = Rect()
        paint.getTextBounds(text, 0, text.length, bounds)
        return (bounds.right - bounds.left).toDouble()
    }

    @Suppress("unused") // called from JNI
    fun makeCircleTexture(
            inRadius: Double,
            fillColor: Int,
            strokeColor: Int,
            inStrokeWidth: Float,
            /* out */ circleSize: Point2d?
    ): Long /*Identity*/ {
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
            canvas.drawCircle(size / 2.0f, size / 2.0f, (radius + strokeWidth).toFloat(), paint)
        }
        
        // Inner circle
        paint.style = Paint.Style.FILL
        paint.color = fillColor
        canvas.drawCircle(size / 2.0f, size / 2.0f, radius.toFloat(), paint)

        val texSettings = RenderControllerInterface.TextureSettings().apply {
            filterType = RenderControllerInterface.TextureSettings.FilterType.FilterLinear
            imageFormat = RenderController.ImageFormat.MaplyImage4Layer8Bit
        }
        val tex = control.addTexture(bitmap, texSettings, ThreadMode.ThreadCurrent)
        return tex?.texID ?: EmptyIdentity
    }

    @Suppress("unused") // called from JNI
    fun makeLineTexture(comp: DoubleArray): Long /*Identity*/ {
        val control = control?.get() ?: return EmptyIdentity

        // We want the texture a bit bigger than specified
        val scale = (settings?.markerScale ?: 1.0) * 2.0

        // Need to scale these elements to the texture size
        var eleSum = 0.0
        comp.forEach {
            eleSum += it
        }
        val size = eleSum.toInt()
        if (size == 0)
            return EmptyIdentity;

        val width = 1
        val bitmap = Bitmap.createBitmap(width, size, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        bitmap.eraseColor(Color.TRANSPARENT)

        val paint = Paint(Paint.FILTER_BITMAP_FLAG.or(Paint.ANTI_ALIAS_FLAG))
        paint.style = Paint.Style.FILL_AND_STROKE
        paint.color = Color.WHITE

        // Work our way through the elements
        var curY = 0
        var onOrOff = true
        comp.forEach {
            val eleLen = it.toInt()
            if (onOrOff) {
                for (jj in 0..(eleLen-1)) {
                    val startY = (curY+jj)/eleSum.toFloat() * size.toFloat()
                    canvas.drawLine(0.0f, startY, width.toFloat(), startY+1.0f, paint)
                }
            }
            onOrOff = !onOrOff
            curY += eleLen
        }

        // Create and return a texture
        // TODO: Are we releasing the texture somewhere?
        val texSettings = RenderControllerInterface.TextureSettings().apply {
            filterType = RenderControllerInterface.TextureSettings.FilterType.FilterLinear
            imageFormat = RenderController.ImageFormat.MaplyImage4Layer8Bit
            wrapV = true
        }
        val tex = control.addTexture(bitmap, texSettings, ThreadMode.ThreadCurrent)
        return tex?.texID ?: EmptyIdentity
    }

    @ColorInt var legendBorderColor = Color.BLACK
    var legendBorderSize = 1

    /**
     * Returns a dictionary containing a flexible legend for the layers contained in this style.
     * Each layer is rendered as a representative image at the given size.  Layer names that start
     * with the same "<name>_" will be grouped together in the hierarchy if the group parameter is
     * set.  Otherwise they'll be flat
     */
    fun getLayerLegend(imageSize: Size, useGroups: Boolean): Collection<LegendEntry>? {
        val styles = getStyleInfo(0.0f) ?: return null

        val legend = ArrayList<LegendEntry>(styles.size)
        val groupMap = HashMap<String, LegendEntry>()
        
        for (style in styles) {
            // If the style has a representation value, it's an alternate version of another style,
            // e.g., "selected", so ignore it.
            if (style.getString("representation")?.isNotEmpty() == true) {
                continue
            }

            val ident = style.getString("ident") ?: continue
            val (group, name) = if (useGroups) parseIdent(ident) else Pair(null, ident)

            val color = style.getInt("legendColor")?: Color.TRANSPARENT
            
            val bitmap = when (style.getString("type")) {
                "background" -> getSolidImage(imageSize, color)
                "symbol" -> getSymbolImage(imageSize, color, style.getString("legendText"))
                "circle" -> getCircleImage(imageSize, color)
                "line" -> getLineImage(imageSize, color)
                "fill" -> getSolidImage(imageSize, color)
                else -> null
            }

            if (group?.isNotEmpty() == true) {
                groupMap[group]?.let {
                    it.entries = it.entries.plus(LegendEntry(name, ident, bitmap, emptyList()))
                } ?: run {
                    val entry = LegendEntry(group, ident, null, listOf(
                            LegendEntry(name, ident, bitmap, emptyList())))
                    groupMap[group] = entry
                    legend.add(entry)
                }
            } else {
                legend.add(LegendEntry(name, ident, bitmap, emptyList()))
            }
        }

        return legend
    }

    private fun parseIdent(ident: String): Pair<String?, String> {
        // todo: are we handling leading underscores correctly?
        val items = ident.split('_', limit = 2)
        return if (items.size > 1)  Pair(items[0], items[1]) else Pair(null, items[0])
    }

    private fun getSolidImage(size: Size, @ColorInt color: Int): Bitmap? {
        val image = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888)
        image.eraseColor(color)
        if (legendBorderSize > 0) {
            val canvas = Canvas(image)
            val paint = Paint(Paint.ANTI_ALIAS_FLAG)
            paint.style = Paint.Style.STROKE
            paint.strokeWidth = legendBorderSize.toFloat()
            paint.color = legendBorderColor
            canvas.drawRect(Rect(0, 0, size.width, size.height), paint)
        }
        return image
    }
    
    private fun getSymbolImage(size: Size, @ColorInt color: Int, text: String?): Bitmap? {
        val image = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888)
        image.eraseColor(Color.TRANSPARENT)
        val canvas = Canvas(image)
        if (text?.isNotEmpty() == true) {
            // todo: get sprite
        } // else {
        val paint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.HINTING_ON).or(Paint.SUBPIXEL_TEXT_FLAG))
        paint.color = color
        paint.typeface = Typeface.create("Arial", Typeface.BOLD)
        paint.textSize = (size.height - 2 * legendBorderSize).toFloat()
        paint.textAlign = Paint.Align.CENTER
        canvas.drawText("T",
                size.width / 2f + legendBorderSize.toFloat(),
                (size.height - paint.descent() - paint.ascent()) / 2f,
                paint)
        if (legendBorderSize > 0) {
            paint.style = Paint.Style.STROKE
            paint.strokeWidth = legendBorderSize.toFloat()
            paint.color = legendBorderColor
            canvas.drawRect(Rect(0, 0, size.width, size.height), paint)
        }
        return image
    }
    
    private fun getLineImage(size: Size, @ColorInt color: Int): Bitmap? {
        val image = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888)
        image.eraseColor(Color.TRANSPARENT)
        val canvas = Canvas(image)
        val paint = Paint(Paint.ANTI_ALIAS_FLAG)
        paint.style = Paint.Style.STROKE
        paint.strokeWidth = (size.width / 10f).coerceAtLeast(1f)
        paint.color = color
        canvas.drawLine(0f, size.height.toFloat(), size.width.toFloat(), 0f, paint)
        // no border?
        return image
    }
    
    private fun getCircleImage(size: Size, @ColorInt color: Int): Bitmap? {
        val image = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888)
        image.eraseColor(Color.TRANSPARENT)
        val canvas = Canvas(image)
        val paint = Paint(Paint.ANTI_ALIAS_FLAG)
        paint.style = Paint.Style.FILL
        paint.color = color
        canvas.drawOval(RectF(0f, 0f, size.width.toFloat(), size.height.toFloat()), paint)
        if (legendBorderSize > 0) {
            paint.style = Paint.Style.STROKE
            paint.strokeWidth = legendBorderSize.toFloat()
            paint.color = legendBorderColor
            canvas.drawOval(RectF(0f, 0f, size.width.toFloat(), size.height.toFloat()), paint)
        }
        return image
    }

    private var spriteTex: MaplyTexture? = null
    /**
     * Add the sprites
     */
    public fun addSprites(spriteJSON: String, spriteSheet: Bitmap) {
        val control = control?.get() ?: return

        val spriteTex = control.addTexture(spriteSheet, RenderControllerInterface.TextureSettings(),
                RenderControllerInterface.ThreadMode.ThreadCurrent) ?: return
        this.spriteTex = spriteTex
        addSpritesNative(spriteJSON, spriteTex.texID, spriteSheet.width, spriteSheet.height)
    }

    external fun addSpritesNative(spriteJSON: String, texID: Long, width: Int, height: Int): Boolean

    // Set a named layer visible or invisible
    public external fun setLayerVisible(layerName: String, visible: Boolean)

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
     * These are actually implemented on the C++ side, which communicates with itself.
     * But we need to here to appear to be using the standard interface.
     */
    external override fun stylesForFeature(
            attrs: AttrDictionary,
            tileID: TileID,
            layerName: String,
            controller: RenderControllerInterface): Array<VectorStyle>?
    external override fun allStyles(): Array<VectorStyle>?
    external override fun layerShouldDisplay(layerName: String, tileID: TileID): Boolean
    external override fun styleForUUID(uuid: Long, controller: RenderControllerInterface): VectorStyle?

    /**
     * Get the zoom slot, or -1
     */
    external override fun getZoomSlot(): Int

    /**
     * Capture the zoom slot if you're going use it
     */
    external override fun setZoomSlot(inZoomSlot: Int)

    /**
     * Clean up resources associated with the vector style
     */
    public fun shutdown() {
        val control = control?.get() ?: return

        spriteTex = spriteTex?.let {
            control.removeTexture(it, RenderControllerInterface.ThreadMode.ThreadAny)
            null
        }
    }

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
    
    private external fun getStyleInfo(zoom: Float): Array<AttrDictionary>?
    
    private external fun initialise(
            scene: Scene?,
            coordSystem: CoordSystem?,
            settings: VectorStyleSettings?,
            styleDict: AttrDictionary?
    )

    external fun dispose()
    protected var nativeHandle: Long = 0
}
