/*
 *  MaplyBasicClusterGenerator.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.text.TextPaint;

import java.awt.font.TextAttribute;
import java.util.HashMap;

/**
 * The basic cluster generator installed by default.
 * <p>
 * This cluster generator will make images for grouped clusters of markers/labels.
 *
 */
public class BasicClusterGenerator extends ClusterGenerator {

    /**
     * The ID number corresponding to the cluster.  Every marker/label with this cluster ID will be grouped together.
     */
    private int clusterNumber;

    /**
     * The size of the cluster that will be created.
     * <p>
     * This is the biggest cluster you're likely to create.  We use it to figure overlaps between clusters.
     */
    private Point2d clusterLayoutSize;

    /**
     * Set this if you want cluster to be user selectable.  On by default.
     */
    private boolean selectable;

    /**
     * How long to animate markers the join and leave a cluster
     */
    private double markerAnimationTime;

    /**
     * The shader to use when moving objects around
     * <p>
     * When warping objects to their new locations we use a motion shader.  Set this if you want to override the default.
     */
//    private Shader motionShader;

    private int[] colors;
    private Point2d size;

    //TODO Font still not supported
    private TextAttribute font;
    private float scale;
    private HashMap<Integer, MaplyTexture> texByNumber;
    private MaplyBaseController viewC;
    private boolean correct = false;
    private Activity activity;

    /**
     * Initialize with a list of colors.
     * <p>
     * Each order of magnitude will use another color.  Must provide at least 1.
     */
    public BasicClusterGenerator(int[] colors, int clusterNumber, Point2d markerSize, MaplyBaseController viewC, Activity activity) {
        correct = (colors.length > 0);
        if (!correct) {
            return;
        }

        this.colors = colors;
        this.size = markerSize;
        this.clusterNumber = clusterNumber;
        this.scale = 1;
        this.activity = activity;
        this.clusterLayoutSize = markerSize;
        this.selectable = true;
        this.markerAnimationTime = 0.2;
        this.viewC = viewC;
    }

    @Override
    public void startClusterGroup() {
        super.startClusterGroup();
        this.texByNumber = new HashMap<Integer, MaplyTexture>();
    }

    MaplyBaseController.TextureSettings texSettings = new MaplyBaseController.TextureSettings();

    @Override
    public ClusterGroup makeClusterGroup(ClusterInfo clusterInfo) {
        if (!correct)
            return null;

        ClusterGroup group = new ClusterGroup();
        MaplyTexture tex = this.texByNumber.get(clusterInfo.numObjects);
        if (tex == null) {
            //Note: Pick the color based on number of markers
            //Create the Bitmap
            Bitmap image = Bitmap.createBitmap((int) this.size.getX(), (int) this.size.getY(), Bitmap.Config.ARGB_8888);
            Canvas c = new Canvas(image);

            //Configure Background
            Paint background = new Paint();
            background.setColor(this.colors[0]);
            background.setStyle(Paint.Style.FILL);
            //Configure Stroke
            Paint stroke = new Paint();
            stroke.setColor(Color.WHITE);
            stroke.setStyle(Paint.Style.STROKE);
            //Configure Text
            TextPaint text = new TextPaint();
            text.setColor(Color.WHITE);
            text.setTextSize((float) (size.getX()/2));
            text.setTextAlign(Paint.Align.CENTER);

            // Compute Circle Position
            float x = (c.getWidth() / 2);
            float y = (c.getHeight()/2);
            float radius = c.getWidth()/2;

            //Draw Circle and Stroke
            c.drawCircle(x, y, radius, background);
            c.drawCircle(x,y,radius,stroke);

            // Compute Text Position
            int xPos = (c.getWidth() / 2);
            int yPos = (int) ((c.getHeight() / 2) - ((text.descent() + text.ascent()) / 2));

            //Draw Text
            c.drawText(Integer.toString(clusterInfo.numObjects), xPos, yPos, text);

            tex = baseController.addTexture(image, texSettings, MaplyBaseController.ThreadMode.ThreadCurrent);

            texByNumber.put(clusterInfo.numObjects, tex);
        }
        group.tex = tex;
        group.size = this.size;

        return group;
    }

    @Override
    public void endClusterGroup() {
        super.endClusterGroup();

        this.texByNumber.clear();
        this.texByNumber = null;
    }

    @Override
    public int clusterNumber() {
        return this.clusterNumber;
    }

    @Override
    public Point2d clusterLayoutSize() {
        return this.clusterLayoutSize;
    }

    @Override
    public boolean selectable() {
        return this.selectable;
    }

    @Override
    public double markerAnimationTime() {
        return this.markerAnimationTime;
    }

//    @Override
//    public Shader motionShader() {
//        return this.motionShader;
//    }
}
