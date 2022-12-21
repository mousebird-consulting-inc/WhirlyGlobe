/*  MaplyVariableTarget.java
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/24/18.
 *  Copyright 2011-2022 mousebird consulting
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

import com.mousebird.maply.RenderControllerInterface.ThreadMode;
import android.graphics.Color;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.ColorInt;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

/**
 * A variable target manages two pass rendering for one type of variable.
 */
@SuppressWarnings("unused")
public class VariableTarget
{
    public boolean valid = true;
    protected boolean setup = false;

    WeakReference<RenderControllerInterface> vc;

    /**
     * A plausible draw priority for render targets.
     * But you really want to set this yourself.
     */
    public static final int VariableTargetDrawPriority = 60000;

    /**
     * This version can be set up explicitly later.
     */
    public VariableTarget(RenderControllerInterface inVc, boolean setupNow) {
        vc = new WeakReference<>(inVc);
        renderTarget = new RenderTarget();

        if (setupNow) {
            new Handler(Looper.getMainLooper()).post(() -> {
                if (valid) {
                    setup();
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
    @ColorInt
    public int color = Color.WHITE;

    /**
     * Draw priority of the rectangle we'll use to draw the render target to the screen
     */
    public int drawPriority = VariableTargetDrawPriority;

    /**
     * Pixel format for the render target texture
     */
    public RenderController.ImageFormat imageFormat =
            RenderController.ImageFormat.MaplyImage4Layer8Bit;

    /**
     * Texture filtering setting for the render target texture
     */
    public RenderControllerInterface.TextureSettings.FilterType filterType =
            RenderControllerInterface.TextureSettings.FilterType.FilterLinear;

    private Shader shader = null;

    public Shader getShader() { return shader; }

    /**
     * Set the shader to use on the rectangle we render to.
     */
    public void setShader(Shader inShader) {
        if (!setup) {
            shader = inShader;
        } else if (inShader != shader) {
            Log.w("Maply", "VariableTarget shader cannot be changed after setup");
        }
    }

    /**
     * By default we'll build a rectangle to display the target
     */
    public boolean buildRectangle = true;

    private boolean rectangleEnabled = true;

    public boolean getRectangleEnabled() { return rectangleEnabled; }
    public void setRectangleEnabled(boolean b) { enableRectangle(b, ThreadMode.ThreadCurrent); }

    /**
     * Show or hide the rectangle, if it was or will be built
     */
    public void enableRectangle(boolean enable, ThreadMode mode) {
        rectangleEnabled = enable;

        final RenderControllerInterface viewC = vc.get();
        if (valid && setup && viewC != null && compObj != null) {
            if (rectangleEnabled) {
                viewC.enableObjects(Collections.singletonList(compObj), ThreadMode.ThreadCurrent);
            } else {
                viewC.disableObjects(Collections.singletonList(compObj), ThreadMode.ThreadCurrent);
            }
        }
    }

    // Other variable targets we're hooking up to
    private ArrayList<VariableTarget> auxTargets = new ArrayList<>();

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
    public RenderTarget renderTarget;
    public ComponentObject compObj = null;

    /**
     * If set, we clear the render target every frame.  Otherwise we let data accumulate.
     */
    public Boolean clearEveryFrame = true;

    /**
     * When we're clearing, use this value.  0 by default
     */
    public double clearVal = 0.0;

    /**
     * Call setup explicitly after setting values.
     */
    public void setup() {
        if (!setup) {
            delayedSetup();
        }
    }

    // We let the setup go a tick so the caller and set settings
    protected void delayedSetup() {
        final RenderControllerInterface viewC = vc.get();
        if (!valid || setup || viewC == null) {
            return;
        }
        setup = true;

        // Set up the texture and render target
        final int[] frameSize = viewC.getFrameBufferSize();
        frameSize[0] = (int)((double)frameSize[0] * scale);
        frameSize[1] = (int)((double)frameSize[1] * scale);

        final RenderControllerInterface.TextureSettings settings = new RenderControllerInterface.TextureSettings();
        settings.imageFormat = imageFormat;
        settings.filterType = filterType;
        renderTex = viewC.createTexture(frameSize[0], frameSize[1],settings, RenderControllerInterface.ThreadMode.ThreadCurrent);
        renderTarget.texture = renderTex;
        renderTarget.clearEveryFrame = clearEveryFrame;
        renderTarget.clearVal = (float)clearVal;
        viewC.addRenderTarget(renderTarget);

        // Default shader
        if (shader == null) {
            shader = viewC.getShader(Shader.NoLightTriangleShader);
        }

        if (buildRectangle) {
            // Rectangle that sits over the view and pulls from the render target
            final ShapeRectangle rect = new ShapeRectangle();
            rect.setPoints(new Point3d(-1.0, -1.0, 0.0), new Point3d(1.0, 1.0, 0.0));
            rect.setClipCoords(true);
            rect.addTexture(renderTex);
            for (VariableTarget target : auxTargets) {
                // Bit if a cheat, but it should be fine
                if (target.renderTex == null) {
                    target.delayedSetup();
                }
                rect.addTexture(target.renderTex);
            }
            final ArrayList<Shape> shapes = new ArrayList<>();
            shapes.add(rect);

            final ShapeInfo shapeInfo = new ShapeInfo();
            shapeInfo.setEnable(rectangleEnabled);
            shapeInfo.setColor(color);
            shapeInfo.setDrawPriority(drawPriority);
            shapeInfo.setShader(shader);
            shapeInfo.setZBufferRead(false);
            shapeInfo.setZBufferWrite(false);
            compObj = viewC.addShapes(shapes, shapeInfo, ThreadMode.ThreadCurrent);
        }
        auxTargets = null;
    }

    /**
     * Clean up the associated render target and rectangle.
     */
    public void shutdown()
    {
        valid = false;
        final RenderControllerInterface viewC = vc.get();
        if (viewC != null) {
            if (compObj != null) {
                viewC.removeObject(compObj, ThreadMode.ThreadAny);
            }
            if (renderTarget != null) {
                viewC.removeRenderTarget(renderTarget);
            }
            if (renderTex != null) {
                viewC.removeTexture(renderTex, ThreadMode.ThreadAny);
            }
        }
        compObj = null;
        renderTarget = null;
        renderTex = null;
    }
}
