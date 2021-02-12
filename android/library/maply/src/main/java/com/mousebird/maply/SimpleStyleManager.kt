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
import java.util.*

class SimpleStyleManager {
    
    var scale = 1.0;    // [UIScreen mainScreen].scale; //?
    var smallSize = Size(16, 16);
    var medSize = Size(32, 32);
    var largeSize = Size(64, 64);
    var strokeWidthForIcons = 1.0 * scale;
    var centerIcon = true;
    
    constructor(context: Context, vc: RenderControllerInterface) {
        this.vc = vc
        this.context = context;
    }

    fun makeStyle(dict: Dictionary<String,Any>): SimpleStyle? {
        return null
    }
    
    fun addFeature(obj: VectorObject, mode: RenderControllerInterface.ThreadMode): ComponentObject? {
        return null
    }
    
    fun addFeatures(objs: Collection<VectorObject>, mode: RenderControllerInterface.ThreadMode): Collection<ComponentObject> {
        return listOf()
    }
    
    fun shutdown() {
        synchronized(textureCache) {
            vc.removeTextures(textureCache.elements().toList(), threadCurrent)
            textureCache.clear()
        }
    }
    
    private fun loadImage(name: String, cacheKey: String): Drawable? {
        if (name == null || name.isEmpty()) {
            return null
        }
    
        synchronized(Shared.drawableCache) {
            Shared.drawableCache.get(cacheKey)?.let { return it }
        }
    
        //var bitmap: Bitmap? = null
    
        // todo: detect file path?  contains a slash?
        var drawable = Drawable.createFromPath(name)
        
        // todo: try other paths?
        //fullName = [NSString stringWithFormat:@"%@-24@2x.png",symbol];
        //mainImage = [UIImage imageNamed:fullName];
        //NSString *shortName = [symbol stringByDeletingPathExtension];
        //fullName = [NSString stringWithFormat:@"%@@2x.png",shortName];
    
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
                Shared.drawableCache.put(cacheKey, drawable)
            }
        }
    
        return drawable
    }
    
    private fun iconForName(name: String, size: Size, @ColorInt color: Int,
                            @ColorInt circleColor: Int, strokeWidth: Float,
                            @ColorInt strokeColor: Int): Bitmap? {
        val cacheKey = "${name}_${size.width}_${size.height}_" +
                        "${circleColor}_${(100*strokeWidth).toInt()}_${strokeColor}"
        
        synchronized(Shared.imageCache) {
            Shared.imageCache.get(cacheKey)?.let { return it }
        }

        var drawable = loadImage(name, name)
        if (drawable == null) {
            return null
        }
        
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
    
    private fun textureForStyle(style: SimpleStyle, symbol: String, backSymbol: String,
                                strokeWidth: Float, center: Point, clearBG: Boolean): MaplyTexture? {
        val cacheKey = "${symbol}_${backSymbol}_${center.x}_" +
                        "${center.y}_${(100*strokeWidth).toInt()}_${clearBG}"
        
        synchronized(textureCache) {
            textureCache.get(cacheKey)?.let { return it }
        }
        
        val mainImage = loadImage(symbol, cacheKey + "_main")
        val backImage = loadImage(backSymbol, cacheKey + "_back")
        
        val renderSize = style.markerSize ?: return null  // * scale
    
        val bitmap = Bitmap.createBitmap(renderSize.width, renderSize.height, Bitmap.Config.ARGB_8888)
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
            if (strokeWidth > 0) {
                paint.strokeWidth = strokeWidth
                paint.color = style.strokeColor
                paint.style = Paint.Style.STROKE
                canvas.drawOval(0f, 0f, renderSize.width.toFloat(), renderSize.height.toFloat(), paint)
                // todo: ?
                //canvas.drawOval(1f, 1f, renderSize.width.toFloat() - 2, renderSize.height.toFloat() - 2, paint)
            }
            
            if (!clearBG) {
                paint.color = style.fillColor
                paint.style = Paint.Style.FILL
                val sw = strokeWidth.coerceAtLeast(0.0f)
                canvas.drawOval(sw, sw, renderSize.width - 2 * sw, renderSize.height - 2 * sw, paint)
                // todo: ?
                //canvas.drawOval(sw + 1, sw + 1, renderSize.width - 2 * sw - 2, renderSize.height - 2 *  sw - 2, paint)
            }
        }
        
        if (mainImage != null) {
            val scale = if (backImage != null) renderSize.width / backImage.intrinsicWidth else 1.0
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
    
        val tex = vc.addTexture(bitmap, RenderControllerInterface.TextureSettings(), threadCurrent)
        return tex?.let {
            synchronized(textureCache) {
                textureCache.put(cacheKey, tex)
            }
        }
    }
    
    private val vc: RenderControllerInterface
    private val context: Context
    private val textureCache = Hashtable<String, MaplyTexture>()
    
    private val threadCurrent = RenderControllerInterface.ThreadMode.ThreadCurrent
    
    private object Shared {
        var cacheSize = 4 * 1024 * 1024
        val imageCache: LruCache<String, Bitmap> by lazy { LruCache(cacheSize) }
        val drawableCache: LruCache<String, Drawable> by lazy { LruCache(cacheSize) }
    }
}