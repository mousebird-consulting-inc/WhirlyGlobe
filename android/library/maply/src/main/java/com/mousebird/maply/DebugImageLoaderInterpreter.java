/*  DebugImageLoaderInterpreter.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/20/19.
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

package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.Log;

import java.lang.reflect.Field;

/**
 *  This loader interpreter makes up an image for the given frame/tile
 *  and returns that.  It doesn't use any returned data.
 **/
public class DebugImageLoaderInterpreter extends ImageLoaderInterpreter
{
    DebugImageLoaderInterpreter()
    {
    }

    @Override  public void setLoader(QuadLoaderBase loader)
    {
    }

    int pixelsPerSide = 256;
    int alpha = 255;

    static int MaxDebugColors = 10;
    static int debugColors[] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

    // Make up an image to cover this tile
    @Override public void dataForTile(LoaderReturn inLoadReturn,QuadLoaderBase loader)
    {
        ImageLoaderReturn loadReturn = (ImageLoaderReturn)inLoadReturn;

        TileID tileID = loadReturn.getTileID();
        int frame = loadReturn.getFrame();

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
    }
}
