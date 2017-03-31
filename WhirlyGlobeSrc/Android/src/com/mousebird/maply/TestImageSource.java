/*
 *  TestImageSource.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

/**
 * Test Maply's image paging by creating an image per tile with the tile
 * ID in the middle.  Useful for debugging and nothing else.
 * 
 */
public class TestImageSource implements QuadImageTileLayer.TileSource
{
	int minZoom = 0;
	int maxZoom = 16;
	int pixelsPerSide = 256;
	public int alpha = 255;
	Looper mainLooper = null;
	
	public TestImageSource(Looper inMainLooper,int inMinZoom,int inMaxZoom)
	{		
		mainLooper = inMainLooper;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
	}
	
	@Override
	public int minZoom() 
	{
		return minZoom;
	}

	@Override
	public int maxZoom() 
	{
		return maxZoom;
	}

	@Override
	public int pixelsPerSide() { return pixelsPerSide; }

	public boolean validTile(MaplyTileID tileID,Mbr tileBounds)
	{
		return true;
	}

	static int MaxDebugColors = 10;
	static int debugColors[] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

	@Override
	public void startFetchForTile(final QuadImageTileLayerInterface layer, final MaplyTileID tileID, final int frame)
	{
		// Note: Porting.  Do something with the frame
		
		Handler handler = new Handler(mainLooper);
		handler.post(new Runnable()
		{
		@Override
		public void run()
		{		
			// Render the tile ID into a bitmap
			String text = null;
			if (frame == -1)
				text = tileID.toString();
			else
				text = tileID.toString() + " " + frame;
			Paint p = new Paint();
			p.setTextSize(24.f);
			p.setColor(Color.WHITE);
			Rect bounds = new Rect();
			p.getTextBounds(text, 0, text.length(), bounds);
			int textLen = bounds.right;
//			int textHeight = -bounds.top;
	
			// Draw into a bitmap
			int sizeX = pixelsPerSide,sizeY = pixelsPerSide;
			Bitmap bitmap = Bitmap.createBitmap(sizeX, sizeY, Bitmap.Config.ARGB_8888);
			Canvas c = new Canvas(bitmap);
			Paint p2 = new Paint();
			p2.setStyle(Paint.Style.FILL);
	        int hexColor = debugColors[tileID.level % MaxDebugColors];
	        int red = (((hexColor) >> 16) & 0xFF);
	        int green = (((hexColor) >> 8) & 0xFF);
	        int blue = (((hexColor) >> 0) & 0xFF);
			p2.setARGB(alpha, red, green, blue);
			c.drawRect(0, 0, sizeX, sizeY, p2);
			Paint p3 = new Paint();
			p3.setStyle(Paint.Style.STROKE);
			p3.setColor(Color.WHITE);
			c.drawRect(2,2,sizeX-2,sizeY-2,p3);
			c.drawText(text, (sizeX - textLen) / 2.f, sizeY / 2.f, p);

			Log.d("Maply", "Loaded fake tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")" + " " + frame);

			layer.loadedTile(tileID, frame, new MaplyImageTile(bitmap));
		}
		});
	}

	@Override
	public void clear(QuadImageTileLayerInterface layer)
	{
	}
}
