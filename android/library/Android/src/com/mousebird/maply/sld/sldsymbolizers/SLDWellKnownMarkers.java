/*
 *  SLDWellKnownMarkers.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 7/10/17.
 *  Copyright 2011-2017 mousebird consulting
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
package com.mousebird.maply.sld.sldsymbolizers;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;

public class SLDWellKnownMarkers {

    static Bitmap getBitmap(String wellKnownName, Integer strokeColor, Integer fillColor, int size) {
        if (wellKnownName.equals("square"))
            return getSquareBitmap(strokeColor, fillColor, size);
        else if (wellKnownName.equals("circle"))
            return getCircleBitmap(strokeColor, fillColor, size);
        else if (wellKnownName.equals("triangle"))
            return getTriangleBitmap(strokeColor, fillColor, size);
        else if (wellKnownName.equals("star"))
            return getStarBitmap(strokeColor, fillColor, size);
        else if (wellKnownName.equals("cross"))
            return getCrossBitmap(strokeColor, fillColor, size);
        else if (wellKnownName.equals("x"))
            return getXBitmap(strokeColor, fillColor, size);
        else
            return null;
    }

    static Bitmap getCircleBitmap(Integer strokeColor, Integer fillColor, int size) {

        Bitmap b = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.drawColor(Color.TRANSPARENT);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(fillColor);

        c.drawCircle(size/2, size/2 , size/2, paint);
        return b;
    }

    static Bitmap getSquareBitmap(Integer strokeColor, Integer fillColor, int size) {
        Bitmap b = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.drawColor(Color.TRANSPARENT);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(fillColor);

        c.drawRect(0f, 0f, size, size, paint);
        return b;
    }

    static Bitmap getTriangleBitmap(Integer strokeColor, Integer fillColor, int size) {
        Bitmap b = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.drawColor(Color.TRANSPARENT);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(fillColor);

        Path path = new Path();
        path.moveTo(0, size * 0.93f);
        path.lineTo(size, size * 0.93f);
        path.lineTo(size/2, size * 0.07f);
        path.close();
        c.drawPath(path, paint);
        return b;
    }

    static Bitmap getStarBitmap(Integer strokeColor, Integer fillColor, int size) {
        Bitmap b = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.drawColor(Color.TRANSPARENT);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(fillColor);

        Path path = new Path();
        path.moveTo(size * 0.2f, size * 0.95f);
        path.lineTo(size * 0.3f, size * 0.615f);
        path.lineTo( 0, size * 0.395f);
        path.lineTo( size * 0.38f, size * 0.395f);
        path.lineTo(size * 0.5f, size * 0.03f);
        path.lineTo( size * 0.62f, size * 0.395f);
        path.lineTo( size, size * 0.395f);
        path.lineTo(size * 0.7f, size * 0.615f);
        path.lineTo( size * 0.8f, size * 0.95f);
        path.lineTo(size * 0.5f, size * 0.755f);
        path.close();
        c.drawPath(path, paint);
        return b;    }

    static Bitmap getCrossBitmap(Integer strokeColor, Integer fillColor, int size) {
        Bitmap b = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.drawColor(Color.TRANSPARENT);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(fillColor);

        Path path = new Path();
        path.moveTo(size * 0.05f, size * 0.65f);
        path.lineTo( size * 0.05f, size * 0.35f);
        path.lineTo( size * 0.35f, size * 0.35f);
        path.lineTo( size * 0.35f, size * 0.05f);
        path.lineTo( size * 0.65f, size * 0.05f);
        path.lineTo( size * 0.65f, size * 0.35f);
        path.lineTo( size * 0.95f, size * 0.35f);
        path.lineTo( size * 0.95f, size * 0.65f);
        path.lineTo( size * 0.65f, size * 0.65f);
        path.lineTo( size * 0.65f, size * 0.95f);
        path.lineTo( size * 0.35f, size * 0.95f);
        path.lineTo( size * 0.35f, size * 0.65f);
        path.close();
        c.drawPath(path, paint);
        return b;    }

    static Bitmap getXBitmap(Integer strokeColor, Integer fillColor, int size) {
        Bitmap b = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.drawColor(Color.TRANSPARENT);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(fillColor);

        Path path = new Path();
        path.moveTo(size * 0.08f, size * 0.29f);
        path.lineTo( size * 0.29f, size * 0.08f);
        path.lineTo( size * 0.5f, size * 0.29f);
        path.lineTo( size * 0.71f, size * 0.08f);
        path.lineTo( size * 0.92f, size * 0.29f);
        path.lineTo( size * 0.71f, size * 0.5f);
        path.lineTo( size * 0.92f, size * 0.71f);
        path.lineTo( size * 0.71f, size * 0.92f);
        path.lineTo( size * 0.5f, size * 0.71f);
        path.lineTo( size * 0.29f, size * 0.92f);
        path.lineTo( size * 0.08f, size * 0.71f);
        path.lineTo( size * 0.29f, size * 0.5f);
        path.close();
        c.drawPath(path, paint);
        return b;    }

}
