/*  TestTileFetcher.kt
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Tim Sylvester on 4/20/2021.
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
import android.os.*
import androidx.annotation.ColorInt
import java.io.ByteArrayOutputStream
import java.lang.Exception
import java.lang.ref.WeakReference

/**
 * Test Maply's image paging by creating an image per tile with the tile
 * ID in the middle.  Useful for debugging and nothing else.
 */
class TestTileFetcher(
        inControl: BaseController,
        name: String) :
        HandlerThread(name), TileFetcher {

    constructor(inControl: BaseController,
                name: String,
                inMinZoom: Int,
                inMaxZoom: Int) : this(inControl, name) {
        minZoom = inMinZoom
        maxZoom = inMaxZoom
    }
    
    var minZoom = -1
    var maxZoom = -1
    var pixelsPerSide = 256
    var alpha = 255
    
    /**
     * We don't need to describe a remote URL, so this is
     * basically a stub that passes back the tile ID.
     */
    class TestTileInfo(inMinZoom: Int, inMaxZoom: Int) :
            TileInfoNew(inMinZoom, inMaxZoom) {
        override fun fetchInfoForTile(tileID: TileID, flipY: Boolean): Any {
            return TestTileFetchInfo(tileID)
        }
    }

    /**
     * No remote URLs to track, so we just keep the tile ID.
     */
    internal class TestTileFetchInfo(var tileID: TileID)
    
    /**
     * Name of this tile fetcher.  Used for coordinating tile sources.
     */
    override fun getFetcherName(): String {
        return name
    }
    
    /**
     * Add a bunch of requests to the queue and kick them off in a tick.
     */
    override fun startTileFetches(requests: Array<TileFetchRequest>) {
        val looper = super.getLooper() ?: Looper.myLooper()
        for (request in requests) {
            (request.fetchInfo as? TestTileFetchInfo)?.let { fetchInfo ->
                Handler(looper!!).post { startTileFetch(fetchInfo, request) }
            }
        }
    }
    
    private fun startTileFetch(fetchInfo: TestTileFetchInfo, request: TileFetchRequest) {
        try {
            request.callback.success(request, tryTileFetch(fetchInfo))
        } catch (e: Exception) {
            request.callback?.failure(request, e.message)
        }
    }

    private fun tryTileFetch(fetchInfo: TestTileFetchInfo): ByteArray {
        val width = pixelsPerSide
        val height = pixelsPerSide
        return ByteArrayOutputStream(width * height * 4 / 2).let { stream ->
            Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).also { bitmap ->
                val tileID = fetchInfo.tileID
                val text: String = tileID.toString()
                val textWidth = Paint().let { paint ->
                    paint.textSize = 24f
                    paint.color = Color.WHITE
                    val textBound = Rect()
                    paint.getTextBounds(text, 0, text.length, textBound)
                    textBound.width()
                }
    
                val color = debugColors[tileID.level % debugColors.size] and 0xffffff or (alpha shl 24)
                bitmap.eraseColor(color)
                
                Canvas(bitmap).let { canvas ->
                    //canvas.drawRect(0f, 0f, sizeX.toFloat(), sizeY.toFloat(), p2)
                    Paint().let { paint ->
                        paint.style = Paint.Style.STROKE
                        paint.color = Color.WHITE
                        //canvas.drawRect(2f, 2f, (width - 2).toFloat(), (height - 2).toFloat(), paint)
                        canvas.drawRect(0f, 0f, width.toFloat(), height.toFloat(), paint)
                        canvas.drawText(text, (width - textWidth) / 2f, height / 2f, paint)
                    }
                }
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream)
                bitmap.recycle()
            }
            stream.flush()
            stream.toByteArray()
        }
    }

    /**
     * Update an active request with a new priority and importance.
     */
    override fun updateTileFetch(fetchID: Any?, priority: Int, importance: Float): Any? = null
    
    /**
     * Cancel a group of requests at once
     * Use the object returned by the startTileFetch call (which is just a Request object)
     */
    override fun cancelTileFetches(fetchIDs: Array<out Any>?) {
    }
    
    /**
     * Kill all outstanding connections and clean up.
     */
    override fun shutdown() {
        quitSafely()
        control.clear()
    }

    @ColorInt
    private val debugColors = intArrayOf(
            0x86812D, 0x5EB9C9, 0x2A7E3E,
            0x4F256F, 0xD89CDE, 0x773B28,
            0x333D99, 0x862D52, 0xC2C653,
            0xB8583D)
    
    private val control: WeakReference<BaseController> = WeakReference(inControl)
}