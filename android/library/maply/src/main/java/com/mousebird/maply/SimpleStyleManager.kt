/* SimpleStyleManager.kt
 * WhirlyGlobeLib
 *
 * Created by Tim Sylvester on 11/02/2021
 * Copyright © 2021 mousebird consulting, inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations under the License.
 */

package com.mousebird.maply

import android.content.Context
import android.content.res.AssetManager
import android.graphics.*
import android.util.Log
import android.util.Size
import androidx.annotation.ColorInt
import androidx.collection.LruCache
import androidx.core.graphics.*
import java.io.*
import java.lang.ref.WeakReference
import java.util.*
import kotlin.math.ceil
import kotlin.math.min

class SimpleStyleManager(context: Context, vc: RenderControllerInterface, assetManager: AssetManager? = null) {
    
    var smallSize = Point2d(16.0, 16.0)
    var medSize = Point2d(32.0, 32.0)
    var largeSize = Point2d(64.0, 64.0)
    
    @ColorInt
    var defaultColor = 0xFF555555.toInt()
    var filterAlpha = 127
    var filterMode = PorterDuff.Mode.MULTIPLY

    var defaultMarkerStrokeWidth = 2.0f

    var markerScale = 1.0
    var markerBackgroundScale = 1.0

    var sharedCacheSize: Int
        get() { return Shared.cacheSize }
        set(value) { Shared.cacheSize = value }
    
    interface StyleObjectLocator {
        fun locate(name: String): Collection<String>
    }
    var objectLocator: StyleObjectLocator = object : StyleObjectLocator {
        override fun locate(name: String): Collection<String> {
            return listOf(name, "$name.png")
        }
    }

    // Callbacks give the caller a chance to intervene after the ScreenMarker, etc.,
    // is created but before it's added to the map.  Return false to skip the item.
    var onAddMarker: ((VectorObject,ScreenMarker,MarkerInfo,SimpleStyle) -> Boolean)? = null
    var onAddLabel: ((VectorObject,ScreenLabel,LabelInfo,SimpleStyle) -> Boolean)? = null
    var onAddLine: ((VectorObject,WideVectorInfo,SimpleStyle) -> Boolean)? = null
    var onAddArea: ((VectorObject,VectorInfo,SimpleStyle) -> Boolean)? = null
    
    /**
     * Add a styled object, splitting it up if necessary.
     */
    fun addFeatures(obj: VectorObject, mode: ThreadMode = threadCurrent): Sequence<ComponentObject> {
        return addFeatures(listOf(obj), null, mode)
    }
    
    /**
     * Add a collection of styled objects, splitting each if necessary.
     */
    fun addFeatures(objs: Collection<VectorObject>, mode: ThreadMode = threadCurrent): Sequence<ComponentObject> {
        return addFeatures(objs, null, mode)
    }
    
    /**
     * Add an object with a specific style, ignoring its attributes, splitting it if necessary
     */
    fun addFeatures(obj: VectorObject, style: SimpleStyle, mode: ThreadMode = threadCurrent): Sequence<ComponentObject> {
        return addFeatures(listOf(obj), style, mode)
    }
    
    /**
     * Add objects with a specific style, ignoring its attributes, splitting it if necessary
     */
    fun addFeatures(objs: Collection<VectorObject>, style: SimpleStyle? = null, mode: ThreadMode = threadCurrent): Sequence<ComponentObject> {
        return sequence {
            for (obj in objs) {
                val split = obj.splitVectors()
                if (split != null) {
                    for (vec: VectorObject in split) {
                        yieldAll(addFeaturesInternal(vec, style, mode))
                    }
                } else {
                    yieldAll(addFeaturesInternal(obj, style, mode))
                }
            }
        }
    }
    
    fun addFeatures(markers: Sequence<ScreenMarker>, attrs: AttrDictionary, info: MarkerInfo? = null, mode: ThreadMode = threadCurrent): ComponentObject? =
            addFeatures(markers, makeStyle(attrs), info, mode)
    
    fun addFeatures(markers: Sequence<ScreenMarker>, style: SimpleStyle, inInfo: MarkerInfo? = null, mode: ThreadMode = threadCurrent): ComponentObject? {
        val vc = this.vc.get() ?: return null
        val tex = style.markerTexture ?: return null
        markers.forEach { marker ->
            marker.tex = tex
            marker.size = style.markerSize
            style.markerOffset?.let {
                marker.offset = it
            }
        }
        val info = inInfo ?: MarkerInfo().apply {
            enable = true
        }
        return vc.addScreenMarkers(markers.toList(), info, mode)
    }
    
    fun addFeatures(labels: Sequence<ScreenLabel>, attrs: AttrDictionary, info: LabelInfo? = null, mode: ThreadMode = threadCurrent): ComponentObject? =
            addFeatures(labels, makeStyle(attrs), info, mode)
    
    fun addFeatures(labels: Sequence<ScreenLabel>, style: SimpleStyle, inInfo: LabelInfo? = null, mode: ThreadMode = threadCurrent): ComponentObject? {
        val vc = this.vc.get() ?: return null
        if (style.labelColor != null && style.labelSize != null && (style.title ?: "").isNotEmpty()) {
            labels.forEach { label ->
                label.text = style.title
                style.labelOffset?.let {
                    label.offset = it
                }
                label.layoutImportance = Float.MAX_VALUE
            }
            val info = inInfo ?: LabelInfo()
            style.labelSize?.let {
                info.fontSize = it
            }
            style.labelColor?.let {
                info.textColor = it
            }
            return vc.addScreenLabels(labels.toList(), info, mode)
        }
        return null
    }
    
    fun makeStyle(dict: AttrDictionary): SimpleStyle {
        val style = SimpleStyle()
        
        style.title = dict.getString("title")
        style.description = dict.getString("description")
        
        style.markerSize = medSize
        dict.getString("marker-size")?.let {
            when (it) {
                "small" -> style.markerSize = smallSize
                "large" -> style.markerSize = largeSize
            }
        }
        
        style.markerSymbol = dict.getString("marker-symbol")
        style.backgroundSymbol = dict.getString("marker-background-symbol")
        
        style.markerSymbol?.let {
            if (it.length == 1) {
                style.markerString = it
                style.markerSymbol = null
            }
        }

        val markerAlpha = ((dict.getDouble("marker-opacity") ?: 1.0) * 255).toInt()
        style.markerColor = parseColor(dict.getString("marker-color"), markerAlpha)
        
        style.strokeColor = parseColor(dict.getString("stroke"))
        style.strokeWidth = dict.getString("stroke-width")?.toFloatOrNull()
        style.strokeOpacity = dict.getString("stroke-opacity")?.toFloatOrNull()
        
        style.fillColor = parseColor(dict["fill"]?.toString())
        style.fillOpacity = dict.getString("fill-opacity")?.toFloatOrNull()
    
        val cx = dict.getString("marker-center-x")?.toDoubleOrNull()
        val cy = dict.getString("marker-center-y")?.toDoubleOrNull()
        if (cx != null || cy != null) {
            style.markerCenter = Point2d(cx ?: 0.0, cy ?: 0.0)
        }
    
        val bcx = dict.getString("marker-background-center-x")?.toDoubleOrNull()
        val bcy = dict.getString("marker-background-center-y")?.toDoubleOrNull()
        if (bcx != null || bcy != null) {
            style.backgroundCenter = Point2d(bcx ?: 0.0, bcy ?: 0.0)
        }
        
        val ox = dict.getString("marker-offset-x")?.toDoubleOrNull()
        val oy = dict.getString("marker-offset-y")?.toDoubleOrNull()
        if ((ox != null || oy != null) && style.markerSize != null) {
            style.markerSize?.let {
                style.markerOffset = Point2d((it.x * (ox ?: 0.0)),
                                             (it.y * (oy ?: 0.0)))
            }
        }
    
        style.markerScale = dict.getString("marker-scale")?.toDoubleOrNull()
        style.backgroundScale = dict.getString("marker-background-scale")?.toDoubleOrNull()

        style.labelColor = parseColor(dict.getString("label"))
        style.labelSize = dict.getString("label-size")?.toFloatOrNull()
        
        val lx = dict.getString("label-offset-x")?.toDoubleOrNull()
        val ly = dict.getString("label-offset-y")?.toDoubleOrNull()
        if (lx != null || ly != null) {
            style.labelSize?.let {
                style.labelOffset = Point2d((it * (lx ?: 0.0)),
                                            (it * (ly ?: 0.0)))
            }
        }
        
        style.clearBackground = parseBool(dict.getString("marker-circle"))
        
        style.markerTexture = textureForStyle(style)

        return style
    }
    
    /**
     * Call when done with all the generated objects.
     */
    fun shutdown() {
        synchronized(textureCache) {
            vc.get()?.removeTextures(textureCache.elements().toList(), threadCurrent)
            textureCache.clear()
        }
    }
    
    private fun addFeaturesInternal(obj: VectorObject, optStyle: SimpleStyle? = null, mode: ThreadMode = threadCurrent): Sequence<ComponentObject> {
        val vc = this.vc.get() ?: return sequenceOf()
        val style = optStyle ?: makeStyle(obj.attributes)
        when (obj.vectorType) {
            VectorObject.MaplyVectorObjectType.MaplyVectorPointType -> {
                var markerObj: ComponentObject? = null
                var labelObj: ComponentObject? = null
                if (style.markerTexture != null) {
                    val marker = ScreenMarker().apply {
                        loc = obj.center()
                        tex = style.markerTexture
                        size = style.markerSize
                        offset = style.markerOffset ?: Point2d(0.0, 0.0)
                    }
                    val info = MarkerInfo().apply {
                        enable = true
                    }
                    if (onAddMarker?.invoke(obj,marker,info,style) != false) {
                        markerObj = vc.addScreenMarkers(listOf(marker), info, mode)
                    }
                }
                if (style.labelColor != null && style.labelSize != null && (style.title ?: "").isNotEmpty()) {
                    val label = ScreenLabel().apply {
                        text = style.title
                        loc = obj.center()
                        offset = style.labelOffset ?: Point2d(0.0, 0.0)
                        layoutImportance = Float.MAX_VALUE
                    }
                    val info = LabelInfo().apply {
                        fontSize = style.labelSize ?: 10f
                        textColor = style.labelColor ?: defaultColor
                    }
                    if (onAddLabel?.invoke(obj,label,info,style) != false) {
                        labelObj = vc.addScreenLabels(listOf(label), info, mode)
                    }
                }
                return sequenceOf(markerObj, labelObj).filterNotNull()
            }
            VectorObject.MaplyVectorObjectType.MaplyVectorLinearType -> {
                val info = WideVectorInfo()
                info.setLineWidth(style.strokeWidth ?: 2f)
                info.setColor(resolveColor(style.strokeColor, style.strokeOpacity))
                if (onAddLine?.invoke(obj,info,style) != false) {
                    return sequenceOf(vc.addWideVectors(listOf(obj), info, threadCurrent))
                }
            }
            VectorObject.MaplyVectorObjectType.MaplyVectorArealType -> {
                val info = VectorInfo()
                info.setColor(resolveColor(style.fillColor, style.fillOpacity))
                info.setFilled(true)
                if (onAddArea?.invoke(obj,info,style) != false) {
                    return sequenceOf(vc.addVectors(listOf(obj), info, mode))
                }
            }
            else -> return sequenceOf()
        }
        return sequenceOf()
    }

    private fun tryLoadImage(stream: InputStream): Bitmap {
        return BufferedInputStream(stream).use { BitmapFactory.decodeStream(it) }
    }
    
    private fun tryLoadAssetImage(name: String): Bitmap {
        return assets.open(name).use { tryLoadImage(it) }
    }

    private fun tryLoadFileImage(name: String): Bitmap {
        return File(name).inputStream().use { tryLoadImage(it) }
    }
    
    private fun loadImage(name: String): Bitmap? {
        try {
            for (location in objectLocator.locate(name)) {
                try { return tryLoadFileImage(location) } catch (e: FileNotFoundException) {
                    Log.d(logTag, "Failed to load file $location: ${e.message}")
                }
                try { return tryLoadAssetImage(location) } catch (e: FileNotFoundException) {
                    Log.d(logTag, "Failed to load asset $location: ${e.message}")
                }
            }
        } catch (e: Exception) {
            Log.w(logTag, String.format("Failed to load '%s': '%s'", name, e.localizedMessage))
        }
        return null
    }
    
    private fun loadImageCached(name: String?): Bitmap? {
        if (name == null || name.isEmpty()) {
            return null
        }
    
        synchronized(Shared.imageCache) {
            Shared.imageCache[name]?.let { return it }
        }

        return try {
            loadImage(name)
        } catch (e: Exception) {
            Log.w(logTag, String.format("Failed to load '%s': '%s'", name, e.localizedMessage))
            null
        }?.also {
            synchronized(Shared.imageCache) {
                Shared.imageCache.put(name, it)
            }
        }
    }
    
    private fun iconForName(name: String, size: Size, @ColorInt color: Int,
                            @ColorInt circleColor: Int, strokeWidth: Float,
                            @ColorInt strokeColor: Int): Bitmap? {
        val cacheKey = "${name}_${size.width}_${size.height}_" +
                        "${circleColor}_${strokeWidth}_${strokeColor}"
        
        synchronized(Shared.imageCache) {
            Shared.imageCache[cacheKey]?.let { return it }
        }

        val image = loadImageCached(name) ?: return null
    
        val bitmap = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888)
        bitmap.eraseColor(Color.TRANSPARENT)
    
        val canvas = Canvas(bitmap)
    
        val paint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.FILTER_BITMAP_FLAG))
        canvas.drawBitmap(image, 0f, 0f, paint)
    
        synchronized(Shared.imageCache) {
            Shared.imageCache.put(cacheKey, bitmap)
        }
        
        return bitmap
    }
    
    // Make a cache key from anything that can affect the way the texture looks
    private fun styleCacheKey(style: SimpleStyle): String {
        return sequenceOf(
                style.markerSymbol,
                style.backgroundSymbol,
                style.markerString,
                style.markerCenter?.x,
                style.markerCenter?.y,
                style.backgroundCenter?.x,
                style.backgroundCenter?.y,
                style.markerSize?.x,
                style.markerSize?.y,
                style.markerScale,
                style.backgroundScale,
                style.markerColor,
                style.clearBackground,
                style.fillOpacity,
                style.fillColor,
                style.strokeWidth,
                style.strokeOpacity,
                style.strokeColor)
            .joinToString("_") { it?.toString() ?: "" }
    }
    private fun textureForStyle(style: SimpleStyle): MaplyTexture? {
        val vc = this.vc.get() ?: return null

        val cacheKey = styleCacheKey(style)

        synchronized(textureCache) {
            textureCache[cacheKey]?.let { return it }
        }
        
        val mainImage = loadImageCached(style.markerSymbol)
        val backImage = loadImageCached(style.backgroundSymbol)
        
        val renderSize = style.markerSize ?: return null
    
        val bitmap = Bitmap.createBitmap(ceil(renderSize.x).toInt(), ceil(renderSize.y).toInt(), Bitmap.Config.ARGB_8888)
        bitmap.eraseColor(Color.TRANSPARENT)

        val canvas = Canvas(bitmap)
        val paint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.FILTER_BITMAP_FLAG))
    
        val strokeOpacity = style.strokeOpacity ?: 1f
        val strokeWidth = if (backImage == null && strokeOpacity > 0f) (style.strokeWidth ?: defaultMarkerStrokeWidth) else 0f

        if (backImage != null) {
            val imageSize = Point2d(backImage.getScaledWidth(canvas).toDouble(),
                                    backImage.getScaledHeight(canvas).toDouble())

            // Scale the larger dimension down into the image
            val scaleX = renderSize.x / imageSize.x
            val scaleY = renderSize.y / imageSize.y
            val scale = min(scaleX, scaleY) * markerScale * (style.markerScale ?: 1.0)
    
            // Center the scaled rectangle and apply offset
            val scaleAdjustX = renderSize.x / 2 + ((style.backgroundCenter?.x ?: 0.0) - (imageSize.x / 2)) * scale
            val scaleAdjustY = renderSize.y / 2 + ((style.backgroundCenter?.y ?: 0.0) - (imageSize.y / 2)) * scale
            
            canvas.withTranslation(scaleAdjustX.toFloat(), scaleAdjustY.toFloat()) {
                canvas.withScale(scale.toFloat(), scale.toFloat()) {
                    val markerColor = style.markerColor ?: defaultColor
                    val filterColor = Color.argb(filterAlpha, Color.red(markerColor), Color.green(markerColor), Color.blue(markerColor))
                    paint.colorFilter = PorterDuffColorFilter(filterColor, filterMode)
                    canvas.drawBitmap(backImage, 0f, 0f, paint)
                    paint.colorFilter = null
                }
            }
        } else {
            if (style.clearBackground == false) {
                paint.color = style.fillColor ?: defaultColor
                paint.style = Paint.Style.FILL
                paint.alpha = ((style.fillOpacity ?: 0f) * 255f).toInt()
                if (paint.alpha > 0) {
                    canvas.drawOval(strokeWidth + 1,
                            strokeWidth + 1,
                            (renderSize.x - strokeWidth - 1).toFloat(),
                            (renderSize.y - strokeWidth - 1).toFloat(), paint)
                }
            }

            if (strokeWidth > 0) {
                paint.strokeWidth = strokeWidth
                paint.color = style.strokeColor ?: defaultColor
                paint.style = Paint.Style.STROKE
                canvas.drawOval(0.5f * strokeWidth + 1,
                        0.5f * strokeWidth + 1,
                        renderSize.x.toFloat() - 0.5f * strokeWidth - 1,
                        renderSize.y.toFloat() - 0.5f * strokeWidth - 1, paint)
            }
        }
        
        if (mainImage != null) {
            val imageSize = Point2d(mainImage.getScaledWidth(canvas).toDouble(),
                                    mainImage.getScaledHeight(canvas).toDouble())

            // Scale the larger dimension down into the image
            val scaleX = renderSize.x / (imageSize.x + 2f * strokeWidth)
            val scaleY = renderSize.y / (imageSize.y + 2f * strokeWidth)
            val scale = min(scaleX, scaleY) * markerBackgroundScale * (style.backgroundScale ?: 1.0)

            // Center the scaled rectangle and apply offset
            val scaleAdjustX = renderSize.x / 2 + ((style.markerCenter?.x ?: 0.0) - (imageSize.x / 2)) * scale
            val scaleAdjustY = renderSize.y / 2 + ((style.markerCenter?.y ?: 0.0) - (imageSize.y / 2)) * scale
            
            canvas.withSave {
                canvas.translate(scaleAdjustX.toFloat(), scaleAdjustY.toFloat())
                canvas.scale(scale.toFloat(), scale.toFloat())
                canvas.translate(strokeWidth, strokeWidth)
                canvas.drawBitmap(mainImage, 0f, 0f, paint)
            }
        }

        val settings = RenderControllerInterface.TextureSettings().apply {
            imageFormat = RenderController.ImageFormat.MaplyImageIntRGBA    // Bitmap.Config.ARGB_8888
            filterType = RenderControllerInterface.TextureSettings.FilterType.FilterLinear
        }
        return vc.addTexture(bitmap, settings, threadCurrent).also {
            synchronized(textureCache) {
                textureCache[cacheKey] = it
            }
        }
    }
    
    @ColorInt private fun resolveColor(@ColorInt color: Int?, alpha: Float?): Int {
        val c = color ?: defaultColor
        val a = alpha ?: 1.0f
        val r = (Color.red(c) * a).toInt()
        val g = (Color.green(c) * a).toInt()
        val b = (Color.blue(c) * a).toInt()
        return Color.argb((a * 255f).toInt(), r, g, b)
    }
    
    private fun parseBool(s: String?): Boolean? {
        if (s == null || s.isEmpty()) return null
        return (s == "1" || s.toLowerCase(Locale.ROOT) == "true")
    }
    
    companion object {
        @ColorInt
        fun parseColor(s: String?, a: Int = 255): Int? {
            if (s == null || s.isEmpty()) return null
            try {
                val c = (if (s[0] == '#') s.substring(1) else s).toInt(16)
                if (s.length < 5) {
                    // handle short colors
                    val r = c.and(0xF00)
                    val g = c.and(0x0F0)
                    val b = c.and(0x00F)
                    return r.shl(12)   // R__ => R_____
                            .or(r.shl(8))  // R__ => _R____
                            .or(g.shl(8))  // _G_ => __G___
                            .or(g.shl(4))  // _G_ => ___G__
                            .or(b.shl(4))  // __B => ____B_
                            .or(b)                  // __B => _____B
                            .or(a.and(255).shl(24)) // alpha = 1
                }
                // regular color, already in the correct order for @ColorInt, add alpha and we're done
                return c.or(a.and(255).shl(24))
            } catch (e: java.lang.NumberFormatException) {
                return null
            }
        }
    }
    
    private val vc = WeakReference(vc)
    private val assets = assetManager ?: context.assets
    private val textureCache = Hashtable<String, MaplyTexture>()
    private val threadCurrent = ThreadMode.ThreadCurrent
    private val logTag = javaClass.name

    private object Shared {
        var cacheSize = 4 * 1024 * 1024
        set(value) {
            field = value
            imageCache.resize(value)
        }
        val imageCache: LruCache<String, Bitmap> by lazy { LruCache<String, Bitmap>(cacheSize) }
    }
}

typealias ThreadMode = RenderControllerInterface.ThreadMode
