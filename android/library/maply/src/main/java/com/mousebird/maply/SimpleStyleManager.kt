/*
 * SimpleStyleManager.kt
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
import android.content.res.Resources
import android.graphics.*
import android.graphics.drawable.Drawable
import android.util.Size
import androidx.annotation.ColorInt
import androidx.collection.LruCache
import java.io.IOException
import java.lang.ref.WeakReference
import java.util.*
import kotlin.math.ceil

class SimpleStyleManager {
    
    var smallSize = Point2d(16.0, 16.0)
    var medSize = Point2d(32.0, 32.0)
    var largeSize = Point2d(64.0, 64.0)
    
    constructor(context: Context, vc: RenderControllerInterface) {
        this.vc = WeakReference(vc)
        this.context = context
    }
    
    /**
     * Add a styled object, splitting it up if necessary.
     */
    fun addFeatures(obj: VectorObject, mode: ThreadMode): Collection<ComponentObject>? {
        return addFeatures(listOf(obj), null, mode)
    }
    
    /**
     * Add a collection of styled objects, splitting each if necessary.
     */
    fun addFeatures(objs: Collection<VectorObject>, mode: ThreadMode): Collection<ComponentObject> {
        return addFeatures(objs, null, mode)
    }
    
    /**
     * Add an object with a specific style, ignoring its attributes, splitting it if necessary
     */
    fun addFeatures(obj: VectorObject, style: SimpleStyle, mode: ThreadMode): Collection<ComponentObject> {
        return addFeatures(listOf(obj), style, mode)
    }
    
    /**
     * Add objects with a specific style, ignoring its attributes, splitting it if necessary
     */
    fun addFeatures(objs: Collection<VectorObject>, style: SimpleStyle?, mode: ThreadMode): Collection<ComponentObject> {
        return objs.flatMap { obj ->
            (obj.splitVectors() ?: arrayOf()).mapNotNull { sObj ->
                addFeaturesInternal(sObj, style ?: makeStyle(sObj.attributes), mode)
            }
        }
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
        
        style.markerColor = parseColor(dict.getString("marker-color"))
        
        style.strokeColor = parseColor(dict.getString("stroke"))
        style.strokeWidth = dict.getString("stroke-width")?.toFloatOrNull()
        style.strokeOpacity = dict.getString("stroke-opacity")?.toFloatOrNull()
        
        style.fillColor = parseColor(dict["fill"]?.toString())
        style.fillOpacity = dict.getString("fill-opacity")?.toFloatOrNull()

        val cx = dict.getString("marker-background-center-x")?.toDoubleOrNull()
        val cy = dict.getString("marker-background-center-y")?.toDoubleOrNull()
        if (cx != null || cy != null) {
            style.markerCenter = Point2d(cx ?: 0.0, cy ?: 0.0)
        }
    
        val ox = dict.getString("marker-offset-x")?.toDoubleOrNull()
        val oy = dict.getString("marker-offset-y")?.toDoubleOrNull()
        if ((ox != null || oy != null) && style.markerSize != null) {
            style.markerSize?.let {
                style.markerOffset = Point2d((it.x * (ox ?: 0.0)),
                                             (it.y * (oy ?: 0.0)))
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
    
    private fun addFeaturesInternal(obj: VectorObject, style: SimpleStyle, mode: RenderControllerInterface.ThreadMode): ComponentObject? {
        val vc = this.vc.get() ?: return null
        when (obj.vectorType) {
            VectorObject.MaplyVectorObjectType.MaplyVectorPointType -> {
                if (style.markerTexture != null) {
                    val marker = ScreenMarker()
                    marker.loc = obj.center()
                    marker.tex = style.markerTexture
                    marker.size = style.markerSize
                    marker.offset = style.markerOffset
                    return vc.addScreenMarkers(listOf(marker), null, mode)
                }
            }
            VectorObject.MaplyVectorObjectType.MaplyVectorLinearType -> {
                val info = WideVectorInfo()
                info.setLineWidth(style.strokeWidth ?: 2f)
                info.setColor(resolveColor(style.strokeColor, style.strokeOpacity))
                return vc.addWideVectors(listOf(obj), info, threadCurrent)
            }
            VectorObject.MaplyVectorObjectType.MaplyVectorArealType -> {
                val info = VectorInfo()
                info.setColor(resolveColor(style.fillColor, style.fillOpacity))
                info.setFilled(true)
                return vc.addVectors(listOf(obj), info, mode)
            }
        }
        return null
    }
    
    private fun loadImage(name: String?): Drawable? {
        if (name == null || name.isEmpty()) {
            return null
        }
    
        synchronized(Shared.drawableCache) {
            Shared.drawableCache[name]?.let { return it }
        }
    
        //var bitmap: Bitmap? = null
    
        // todo: detect file path?  contains a slash?
        var drawable = Drawable.createFromPath(name)
        
        // todo: try other paths?
        //fullName = [NSString stringWithFormat:@"%@-24@2x.png",symbol]
        //mainImage = [UIImage imageNamed:fullName]
        //NSString *shortName = [symbol stringByDeletingPathExtension]
        //fullName = [NSString stringWithFormat:@"%@@2x.png",shortName]
    
        if (drawable == null) {
            try {
                val stream = context.assets.open(name)
                drawable = Drawable.createFromStream(stream, null)
            } catch (e: IOException) {
            }
        }
    
        if (drawable == null) {
            // maybe it's a resource ID?
            try {
                drawable = context.resources.getDrawable(Integer.parseInt(name), context.theme)
            } catch (e: NumberFormatException) {
            } catch (e: Resources.NotFoundException) {
            }
        }
    
        if (drawable != null) {
            synchronized(Shared.drawableCache) {
                Shared.drawableCache.put(name, drawable)
            }
        }
    
        return drawable
    }
    
    private fun iconForName(name: String, size: Size, @ColorInt color: Int,
                            @ColorInt circleColor: Int, strokeWidth: Float,
                            @ColorInt strokeColor: Int): Bitmap? {
        val cacheKey = "${name}_${size.width}_${size.height}_" +
                        "${circleColor}_${strokeWidth}_${strokeColor}"
        
        synchronized(Shared.imageCache) {
            Shared.imageCache[cacheKey]?.let { return it }
        }

        var drawable: Drawable? = loadImage(name) ?: return null
    
        val bitmap = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888)
        bitmap.eraseColor(Color.TRANSPARENT)
    
        val canvas = Canvas(bitmap)
    
        val paint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.FILTER_BITMAP_FLAG))
        //paint.color =
        //canvas.drawOval(0f, 0f, size.toFloat(), size.toFloat(), paint)
    
        synchronized(Shared.imageCache) {
            Shared.imageCache.put(cacheKey, bitmap)
        }
        
        return bitmap
    }
    
    private fun textureForStyle(style: SimpleStyle): MaplyTexture? {
        val vc = this.vc.get() ?: return null
        
        // Make a cache key from anything that can affect the way the texture looks
        val cacheKey = arrayOf(
                style.markerSymbol,
                style.backgroundSymbol,
                style.markerString,
                style.markerCenter?.x,
                style.markerCenter?.y,
                style.markerSize?.x,
                style.markerSize?.y,
                style.markerColor,
                style.clearBackground,
                style.fillOpacity,
                style.fillColor,
                style.strokeWidth,
                style.strokeOpacity,
                style.strokeColor)
                .joinToString("_") { it?.toString() ?: "" }
    
        synchronized(textureCache) {
            textureCache[cacheKey]?.let { return it }
        }
        
        val mainImage = loadImage(style.markerSymbol)
        val backImage = loadImage(style.backgroundSymbol)
        
        val renderSize = style.markerSize ?: return null
    
        val bitmap = Bitmap.createBitmap(ceil(renderSize.x).toInt(), ceil(renderSize.y).toInt(), Bitmap.Config.ARGB_8888)
        bitmap.eraseColor(Color.TRANSPARENT)
    
        val canvas = Canvas(bitmap)
        val paint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.FILTER_BITMAP_FLAG))
        
        if (backImage != null) {
            // todo: ?
            //backImage.setBounds()
            //backImage.setColorFilter()
            //backImage.setTint()
            //backImage.setTintMode()
            backImage.draw(canvas)
        } else {
            val strokeWidth = style.strokeWidth ?: 2f
            val strokeOpacity = style.strokeOpacity ?: 1f
            if (strokeWidth > 0 && strokeOpacity > 0) {
                paint.strokeWidth = strokeWidth
                paint.color = style.strokeColor ?: 0xFF555555.toInt()
                paint.style = Paint.Style.STROKE
                canvas.drawOval(0f, 0f, renderSize.x.toFloat(), renderSize.y.toFloat(), paint)
                // todo: ?
                //canvas.drawOval(1f, 1f, renderSize.width.toFloat() - 2, renderSize.height.toFloat() - 2, paint)
            }
            
            if (style.clearBackground == false) {
                paint.color = style.fillColor ?: 0xFF555555.toInt()
                paint.style = Paint.Style.FILL
                paint.alpha = ((style.fillOpacity ?: 0f) * 255f).toInt()
                if (paint.alpha > 0) {
                    canvas.drawOval(strokeWidth, strokeWidth,
                            (renderSize.x - 2 * strokeWidth).toFloat(),
                            (renderSize.y - 2 * strokeWidth).toFloat(),
                            paint)
                    // todo: ?
                    //canvas.drawOval(sw + 1, sw + 1, renderSize.width - 2 * sw - 2, renderSize.height - 2 *  sw - 2, paint)
                }
            }
        }
        
        if (mainImage != null) {
            val scale = if (backImage != null) renderSize.x / backImage.intrinsicWidth else 1.0
            // todo: Set up scale/translate ?
            //canvas.scale(scalex, scaley, centerx, centery)
            //theCenter.x = if (center.x > -1000.0) center.x * scale else renderSize.width / 2.0
            //theCenter.y = if (center.y > -1000.0 && backImage) center.y / backImage.size.height * renderSize.height else renderSize.height / 2.0
            //CGContextDrawImage(ctx, CGRectMake(theCenter.x - scale * mainImage.size.width / 2.0, renderSize.height - theCenter.y - scale * mainImage.size.height / 2.0,
            //        mainImage.size.width * scale, mainImage.size.height * scale), mainImage.CGImage)
            // setBounds?
            // setTint?
            mainImage.draw(canvas)
        }
    
        return vc.addTexture(bitmap, RenderControllerInterface.TextureSettings(), threadCurrent)?.let {
            synchronized(textureCache) {
                textureCache.put(cacheKey, it)
            }
        }
    }
    
    @ColorInt private fun resolveColor(@ColorInt color: Int?, alpha: Float?): Int {
        val c = color ?: 0x555555
        val a = alpha ?: 1.0f
        val r = (Color.red(c) * a).toInt()
        val g = (Color.green(c) * a).toInt()
        val b = (Color.blue(c) * a).toInt()
        return Color.argb((a * 255f).toInt(),r, g, b)
    }
    
    private fun parseBool(s: String?): Boolean? {
        if (s == null || s.isEmpty()) return null
        return (s == "1" || s.toLowerCase(Locale.ROOT) == "true")
    }
    
    @ColorInt private fun parseColor(s: String?): Int? {
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
                    .or(0xFF000000.toInt()) // alpha = 1
            }
            // regular color, already in the correct order for @ColorInt, add alpha and we're done
            return c.or(0xFF000000.toInt())
        } catch (e: java.lang.NumberFormatException) {
            return null
        }
    }
    
    private var vc = WeakReference <RenderControllerInterface>(null)
    private val context: Context
    private val textureCache = Hashtable<String, MaplyTexture>()
    
    private val threadCurrent = ThreadMode.ThreadCurrent
    
    private object Shared {
        var cacheSize = 4 * 1024 * 1024
        val imageCache: LruCache<String, Bitmap> by lazy { LruCache<String, Bitmap>(cacheSize) }
        val drawableCache: LruCache<String, Drawable> by lazy { LruCache<String, Drawable>(cacheSize) }
    }
}

typealias ThreadMode = RenderControllerInterface.ThreadMode
