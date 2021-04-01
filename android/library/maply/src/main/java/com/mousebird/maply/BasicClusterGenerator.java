/*  MaplyBasicClusterGenerator.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Build;
import android.text.TextPaint;

import androidx.annotation.ColorInt;
import androidx.annotation.RequiresApi;

import java.security.InvalidParameterException;
import java.util.HashMap;

/**
 * The basic cluster generator installed by default.
 * <p>
 * This cluster generator will make images for grouped clusters of markers/labels.
 */
public class BasicClusterGenerator extends ClusterGenerator {

    /**
     * Initialize unset, must assign colors or bitmap before use
     */
    public BasicClusterGenerator(BaseController viewC) {
        this(0,new Point2d(16.0,16.0),new Point2d(16.0,16.0),0.0f,viewC);
    }

    /**
     * Initialize with a list of colors.
     * <p>
     * Each order of magnitude will use another color.  Must provide at least 1.
     */
    public BasicClusterGenerator(int[] colors, BaseController viewC) {
        this(colors,0,new Point2d(16.0,16.0),viewC);
    }

    /**
     * Initialize with a bitmap.
     */
    public BasicClusterGenerator(Bitmap bitmap, BaseController viewC) {
        this(bitmap,0,new Point2d(16.0,16.0),0.0f,viewC);
    }

    public BasicClusterGenerator(int[] colors, int clusterNumber, Point2d markerSize, BaseController viewC, Activity activity) {
        this(colors,clusterNumber,markerSize,viewC);
    }

    public BasicClusterGenerator(Bitmap bitmap, int clusterNumber, Point2d markerSize, float textSize, BaseController viewC, Activity activity) {
        this(bitmap,clusterNumber,markerSize,textSize,viewC);
    }

    public BasicClusterGenerator(int[] colors, int clusterNumber, Point2d markerSize, BaseController viewC) {
        this(clusterNumber,markerSize,markerSize,0.0f,viewC);
        this.colors = colors;
    }

    public BasicClusterGenerator(Bitmap bitmap, int clusterNumber, Point2d markerSize, float textSize, BaseController viewC) {
        this(clusterNumber,markerSize,markerSize,textSize,viewC);
        this.bitmap = bitmap;
    }

    private BasicClusterGenerator(int clusterNumber, Point2d markerSize, Point2d layoutSize, float textSize, BaseController viewC) {
        this.baseController = viewC;
        this.size = markerSize;
        this.clusterNumber = clusterNumber;
        this.textSize = textSize;
        this.clusterLayoutSize = layoutSize;
    }

    public void shutdown()
    {
        baseController = null;
    }

    /**
     * The ID number corresponding to the cluster.
     * Every marker/label with this cluster ID will be grouped together.
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setClusterNumber(int number) { throwIfStarted(); this.clusterNumber = number; }

    /**
     * Set the colors to use for generated markers.
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setColors(@ColorInt int[] colors) { throwIfStarted(); this.colors = colors; }

    /**
     * Set the base used for computing orders of magnitude
     */
    public void setExponentBase(double base) { throwIfStarted(); this.expBase = base; }

    /**
     * Set the bitmap to use for markers.
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setBitmap(Bitmap bitmap) { throwIfStarted(); this.bitmap = bitmap; }

    /**
     * Set the size of the generated marker images.
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setMarkerSize(Point2d size) { throwIfStarted(); this.size = size; }

    /**
     * The size of the cluster that will be created.
     * This is the biggest cluster you're likely to create.
     * We use it to figure overlaps between clusters.
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setLayoutSize(Point2d size) { throwIfStarted(); this.clusterLayoutSize = size; }

    /**
     * Set the size of the text for the numbers
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setTextSize(float size) { throwIfStarted(); this.textSize = size; }

    /**
     * Set the color of the text for the numbers
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setTextColor(@ColorInt int color) { throwIfStarted(); this.textColor = color; }

    /**
     * Set the font for the numbers
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setTypeface(Typeface typeface) { throwIfStarted(); this.typeface = typeface; }

    /**
     * Set the stroke color
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setStrokeColor(@ColorInt int color) { throwIfStarted(); this.strokeColor = color; }

    /**
     * Set the font features
     * https://developer.android.com/reference/android/graphics/Paint#setFontFeatureSettings(java.lang.String)
     * Cannot be modified after the generator is added to a map controller.
     */
    public void setFontFeatures(String str) { throwIfStarted(); this.fontFeatures = str; }

    /**
     * Set the font variation
     * https://developer.android.com/reference/android/graphics/Paint#setFontVariationSettings(java.lang.String)
     * Cannot be modified after the generator is added to a map controller.
     */
    @RequiresApi(api = Build.VERSION_CODES.O)
    public void setFontSettings(String str) { throwIfStarted(); this.fontSettings = str; }

    /**
     * Flags to use on text paint
     * https://developer.android.com/reference/android/graphics/Paint#setFlags(int)
     */
    public void setTextFlags(int flags) { throwIfStarted(); textPaintFlags = flags; }

    /**
     * How long to animate markers that join and leave a cluster
     */
    public void setMarkerAnimationTime(double sec) { this.markerAnimationTime = sec; }

    /**
     * Set this if you want cluster to be user selectable.  On by default.
     */
    public void setSelectable(boolean sel) { this.selectable = sel; }

    /**
     * Enable or disable caching of generated marker images
     */
    public void cacheBitmaps(boolean enable) { this.cacheBitmaps = enable; }

    // Check that we're not already going.
    // todo: we could clear the cache to let most fields be modified, but watch out for thread safety.
    private void throwIfStarted() {
        if (started) {
            throw new InvalidParameterException("Property may not be modified");
        }
    }

    @Override
    public void startClusterGroup() {
        started = true;
        super.startClusterGroup();
    }

    @Override
    public ClusterGroup makeClusterGroup(ClusterInfo clusterInfo) {
        if (bitmap == null && (colors == null || colors.length < 1)) {
            return null;
        }

        MaplyTexture tex = texByNumber.get(clusterInfo.numObjects);
        if (tex == null) {
            Bitmap image = cacheBitmaps ? imageByNumber.get(clusterInfo.numObjects) : null;
            if (image == null) {
                image = generateMarker(clusterInfo);
            }
            if (image != null) {
                if (cacheBitmaps) {
                    imageByNumber.put(clusterInfo.numObjects,image);
                }
                tex = baseController.addTexture(image, texSettings, RenderController.ThreadMode.ThreadCurrent);
                if (tex != null) {
                    texByNumber.put(clusterInfo.numObjects, tex);
                }
            }
        }

        if (tex != null) {
            ClusterGroup group = new ClusterGroup();
            group.tex = tex;
            group.size = size;
            return group;
        }
        return null;
    }

    @Override
    public void endClusterGroup() {
        super.endClusterGroup();
        texByNumber.clear();
    }

    @Override
    public int clusterNumber() { return clusterNumber; }

    @Override
    public Point2d clusterLayoutSize() { return clusterLayoutSize; }

    @Override
    public boolean selectable() { return selectable; }

    @Override
    public double markerAnimationTime() { return markerAnimationTime; }

    protected String getMarkerText(ClusterInfo clusterInfo) {
        return Integer.toString(clusterInfo.numObjects);
    }

    protected int getMarkerColor(ClusterInfo clusterInfo) {
        final int mag = (int)Math.floor(Math.log(clusterInfo.numObjects)/Math.log(expBase));
        final int idx = Math.max(0, Math.min(mag, colors.length - 1));
        return (idx < colors.length) ? colors[idx] : 0;
    }

    private Bitmap generateMarker(ClusterInfo clusterInfo) {
        if (bitmap == null && (colors == null || colors.length < 1)) {
            return null;
        }

        final Bitmap image = Bitmap.createBitmap((int)size.getX(), (int)size.getY(), Bitmap.Config.ARGB_8888);
        image.eraseColor(Color.TRANSPARENT);

        final Canvas c = new Canvas(image);

        if (colors != null) {
            final Paint backgroundPaint = new Paint();
            backgroundPaint.setColor(getMarkerColor(clusterInfo));
            backgroundPaint.setStyle(Paint.Style.FILL);

            final Paint strokePaint = new Paint();
            strokePaint.setColor(strokeColor);
            strokePaint.setStyle(Paint.Style.STROKE);

            // Compute Circle Position
            final float x = (c.getWidth() / 2.0f);
            final float y = (c.getHeight() / 2.0f);
            final float radius = c.getWidth() / 2.0f;

            // Draw Circle and Stroke
            c.drawCircle(x, y, radius, backgroundPaint);
            c.drawCircle(x, y, radius, strokePaint);
        } else {
            final Paint bitmapPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);
            final Rect srcRect = new Rect(0,0,bitmap.getWidth(),bitmap.getHeight());
            final Rect dstRect = new Rect(0,0,(int)size.getX(),(int)size.getY());
            c.drawBitmap(bitmap,srcRect,dstRect,bitmapPaint);
        }

        // Configure Text
        final TextPaint textPaint = new TextPaint(Paint.ANTI_ALIAS_FLAG | Paint.HINTING_ON | textPaintFlags);
        textPaint.setColor(textColor);
        textPaint.setTextSize((textSize > 0.0) ? textSize : (float)(size.getX()/2.0f));
        textPaint.setTextAlign(Paint.Align.CENTER);
        if (typeface != null) {
            textPaint.setTypeface(typeface);
        }

        if (fontFeatures != null && !fontFeatures.isEmpty()) {
            textPaint.setFontFeatureSettings(fontFeatures);
        }
        if (fontSettings != null && !fontSettings.isEmpty()) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                textPaint.setFontVariationSettings(fontSettings);
            }
        }

        // Compute Text Position
        final int xPos = (c.getWidth() / 2);
        final int yPos = (int) ((c.getHeight() / 2) - ((textPaint.descent() + textPaint.ascent()) / 2));

        // Draw Text
        c.drawText(getMarkerText(clusterInfo), xPos, yPos, textPaint);

        return image;
    }

//    @Override
//    public Shader motionShader() {
//        return motionShader;
//    }

//    /**
//     * The shader to use when moving objects around
//     * <p>
//     * When warping objects to their new locations we use a motion shader.  Set this if you want to override the default.
//     */
//    private Shader motionShader;

    private boolean started = false;
    private boolean cacheBitmaps = false;
    private int clusterNumber;
    private Point2d clusterLayoutSize;
    private boolean selectable = true;
    private double markerAnimationTime = 0.2;
    @ColorInt
    private int[] colors;
    private Bitmap bitmap;
    private Point2d size;
    private double expBase = 3.0;
    private Typeface typeface = null;
    private float textSize = 0.f;
    @ColorInt
    private int textColor = Color.WHITE;
    @ColorInt
    private int strokeColor = Color.WHITE;
    private int textPaintFlags = 0;
    private String fontFeatures;
    private String fontSettings;
    private final HashMap<Integer, MaplyTexture> texByNumber = new HashMap<>(20);
    private final HashMap<Integer, Bitmap> imageByNumber = new HashMap<>(20);
    private final RenderController.TextureSettings texSettings = new RenderController.TextureSettings();
}
