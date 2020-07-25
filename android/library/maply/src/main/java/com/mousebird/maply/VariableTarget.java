/*
 *  MaplyVariableTarget.java
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/24/18.
 *  Copyright 2011-2019 mousebird consulting
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

import android.graphics.Color;
import android.os.Handler;
import android.os.Looper;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * A variable target manages two pass rendering for one type of variable.
 */
public class VariableTarget
{
    public boolean valid = true;
    protected boolean setup = false;

    WeakReference<RenderControllerInterface> vc = null;

    /**
     * A plausible draw priority for render targets.
     * But you really want to set this yourself.
     */
    public static final int VariableTargetDrawPriority = 60000;

    /**
     * This version can be set up explicitly later.
     */
    public VariableTarget(RenderControllerInterface inVc,boolean setupNow) {
        vc = new WeakReference<RenderControllerInterface>(inVc);
        renderTarget = new RenderTarget();

        if (setupNow) {
            Handler handler = new Handler(Looper.getMainLooper());
            handler.post(new Runnable() {
                @Override
                public void run() {
                    if (!valid)
                        return;

                    delayedSetup();
                }
            });
        }
    }

    /**
     * The variable target is actually created a tick later than this
     * so you can tweak its settings.
     */
    public VariableTarget(RenderControllerInterface inVc) {
        this(inVc,true);
    }

    /**
     * Scale the screen by this amount for the render
     */
    public double scale = 1.0;

    /**
     * Color of the rectangle used to draw the render target
     */
    public int color = Color.WHITE;

    /**
     * Draw priority of the rectangle we'll use to draw the render target to the screen
     */
    public int drawPriority = VariableTargetDrawPriority;

    public RenderController.ImageFormat imageFormat = RenderController.ImageFormat.MaplyImage4Layer8Bit;

    Shader shader = null;

    /**
     * Set the shader to use on the rectangle we render to.
     */
    public void setShader(Shader inShader) {
        if (setup)
            return;

        shader = inShader;
    }

    /** By default we'll build a rectangle to display the target
     */
    public boolean buildRectangle = true;

    // Other variable targets we're hooking up to
    private ArrayList<VariableTarget> auxTargets = new ArrayList<VariableTarget>();

    /**
     * Passing in another variable target will let us assign that target to the
     * rectangle used to render this variable target's data.  This is used if
     * you need the contents of more than one target in a shader.
     */
    public void addVariableTarget(VariableTarget target)
    {
        auxTargets.add(target);
    }


    protected MaplyTexture renderTex = null;
    public RenderTarget renderTarget = null;
    public ComponentObject compObj = null;

    /**
     * Call setup explicitly after setting values.
     */
    public void setup() {
        if (setup)
            return;
        delayedSetup();
    }

    // We let the setup go a tick so the caller and set settings
    protected void delayedSetup() {
        setup = true;

        if (vc.get() == null)
            return;

        // Set up the texture and render target
        RenderControllerInterface viewC = vc.get();
        int[] frameSize = viewC.getFrameBufferSize();
        frameSize[0] = (int)((double)frameSize[0] * scale);
        frameSize[1] = (int)((double)frameSize[1] * scale);

        RenderControllerInterface.TextureSettings settings = new RenderControllerInterface.TextureSettings();
        settings.imageFormat = imageFormat;
        renderTex = viewC.createTexture(frameSize[0], frameSize[1],settings, RenderControllerInterface.ThreadMode.ThreadCurrent);
        renderTarget.texture = renderTex;
        viewC.addRenderTarget(renderTarget);

        // Default shader
        if (shader == null) {
            shader = viewC.getShader(Shader.NoLightTriangleShader);
        }

        if (buildRectangle) {
            // Rectangle that sits over the view and pulls from the render target
            ShapeRectangle rect = new ShapeRectangle();
            rect.setPoints(new Point3d(-1.0, -1.0, 0.0), new Point3d(1.0, 1.0, 0.0));
            rect.setClipCoords(true);
            rect.addTexture(renderTex);
            for (VariableTarget target : auxTargets) {
                rect.addTexture(target.renderTex);
            }
            ArrayList<Shape> shapes = new ArrayList<Shape>();
            shapes.add(rect);

            ShapeInfo shapeInfo = new ShapeInfo();
            shapeInfo.setColor(color);
            shapeInfo.setDrawPriority(drawPriority);
            shapeInfo.setShader(shader);
            shapeInfo.setZBufferRead(false);
            shapeInfo.setZBufferWrite(false);
            compObj = viewC.addShapes(shapes, shapeInfo, RenderControllerInterface.ThreadMode.ThreadCurrent);
        }
        auxTargets = null;
    }

    /**
     * Clean up the associated render target and rectangle.
     */
    public void shutdown()
    {
        valid = false;

        if (vc.get() == null)
            return;
        RenderControllerInterface viewC = vc.get();

        if (compObj != null) {
            viewC.removeObject(compObj,RenderControllerInterface.ThreadMode.ThreadAny);
            compObj = null;
        }
        if (renderTarget != null) {
            viewC.removeRenderTarget(renderTarget);
            renderTarget = null;
        }
        if (renderTex != null) {
            viewC.removeTexture(renderTex,RenderControllerInterface.ThreadMode.ThreadAny);
            renderTex = null;
        }
    }

}
