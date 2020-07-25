/*
 *  QuadImageLoaderBase.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
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

import java.lang.ref.WeakReference;

/**
 * Base object for Maply Quad Image loader.
 * <br>
 * Look to the subclasses for actual functionality.  This holds methods they share.
 */
public class QuadImageLoaderBase extends QuadLoaderBase
{
    public static int BaseDrawPriorityDefault = 100;
    public static int DrawPriorityPerLevelDefault = 100;

    protected QuadImageLoaderBase() { }

    protected QuadImageLoaderBase(BaseController control)
    {
        super(control);

        setBaseDrawPriority(BaseDrawPriorityDefault);
        setDrawPriorityPerLevel(DrawPriorityPerLevelDefault);
    }

    protected QuadImageLoaderBase(BaseController control,SamplingParams params,int numFrames)
    {
        super(control,params,numFrames,(numFrames <= 1 ? Mode.SingleFrame : Mode.MultiFrame));

        setBaseDrawPriority(BaseDrawPriorityDefault);
        setDrawPriorityPerLevel(DrawPriorityPerLevelDefault);
    }

    // Called after a tick after creation on the main thread
    // We want them to be able to modify settings before it starts
    public void delayedInit(final SamplingParams params)
    {
        if (tileFetcher == null) {
            tileFetcher = getController().addTileFetcher("Image Fetcher");
        }

        if (loadInterp == null) {
            loadInterp = new ImageLoaderInterpreter();
        }

        samplingLayer = new WeakReference<QuadSamplingLayer>(getController().findSamplingLayer(params,this));
        loadInterp.setLoader(this);

        delayedInitNative(getController().getScene());
    }

    protected native void delayedInitNative(Scene scene);

    /**
     * Return the tile fetcher being used.  Must be called after initialization.
     */
    public TileFetcher getTileFetcher() {
        return tileFetcher;
    }

    /**
     * Set the base priority values for produced tiles.
     * <br>
     * The system will use a range of values to deal with overlaps.
     * This is the base value.
     */
    public native void setBaseDrawPriority(int drawPriority);

    /**
     * Set the draw priority offset per level.
     * <br>
     * We use this number when calculating the draw priority for a particular
     * level.  This is multiplied by the level number.  By default it's 1.
     * <br>
     * This is useful if we want to insert geometry between the levels and have
     * it be masked by high levels.  We do this with vector tile data.
     */
    public native void setDrawPriorityPerLevel(int drawPriority);

    /**
     * Set the color used by the geometry.
     * @param color Color in Android format, including alpha.
     */
    public void setColor(final int color)
    {
        if(samplingLayer.get() == null) {
            setColor(Color.red(color) / 255.f, Color.green(color) / 255.f, Color.blue(color) / 255.f, Color.alpha(color) / 255.f, null);
        }

        samplingLayer.get().layerThread.addTask(new Runnable() {
            @Override
            public void run() {
                ChangeSet changes = new ChangeSet();
                setColor(Color.red(color) / 255.f, Color.green(color) / 255.f, Color.blue(color) / 255.f, Color.alpha(color) / 255.f, changes);
                BaseController control = getController();
                if (control != null && control.renderControl != null) {
                    changes.process(control.renderControl, control.getScene());
                    changes.dispose();
                }
            }
        });
    }

    /**
     * Set the color used by the geometry.  Color values range from 0 to 1.0.
     * You must specify all four values.  Alpha controls transparency.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    public native void setColor(float r,float g,float b,float a, ChangeSet changes);

    /**
     * Write to the z buffer when rendering.  On by default.
     */
    public native void setZBufferWrite(boolean val);

    /**
     * Read from the z buffer when rendering.  Off by default.
     */
    public native void setZBufferRead(boolean val);

    /**
     *  Shader to use for rendering the image frames.
     *
     *  If not set we'll pick the default visual shader.
     */
    public void setShader(Shader shader) {
        setShaderID(shader.getID());
    }

    /**
     *  Shader to use for rendering the image frames.
     *
     *  If not set we'll pick the default visual shader.
     */
    public native void setShaderID(long shaderID);

    /**
     *  An optional render target for this loader.
     *  <br>
     *  The loader can draw to a render target rather than to the screen.
     *  You use this in a multi-pass rendering setup.
     */
    public void setRenderTarget(RenderTarget target) {
        setRenderTargetID(target.renderTargetID);
    }

    /**
     *  An optional render target for this loader.
     *  <br>
     *  The loader can draw to a render target rather than to the screen.
     *  You use this in a multi-pass rendering setup.
     */
    public native void setRenderTargetID(long targetID);

    /**
     *  Set the image format for internal imagery storage.
     *  <br>
     *  OpenGL ES offers us several image formats that are more efficient than 32 bit RGBA, but they're not always appropriate.  This property lets you choose one of them.  The 16 or 8 bit ones can save a huge amount of space and will work well for some imagery, most maps, and a lot of weather overlays.
     *  <br>
     *  Be sure to set this at layer creation, it won't do anything later on.
     */
    public void setImageFormat(RenderController.ImageFormat imageFormat) {
        setImageFormatNative(imageFormat.ordinal());
    }

    protected native void setImageFormatNative(int imageFormat);

    /**
     *  Number of border texels to set up around image tiles.
     *  <br>
     *  For matching image tiles along borders in 3D (probably the globe) we resample the image slightly smaller than we get and make up a boundary around the outside.  This number controls that border size.
     *  <br>
     *  By default this is 1.  It's safe to set it to 0 for 2D maps and some overlays.
     */
    public native void setBorderTexel(int borderTexel);

    protected LoaderReturn makeLoaderReturn()
    {
        return new ImageLoaderReturn(this.getGeneration());
    }

    /**
     * Change the tile sources all at once.  This also forces a reload.
     */
    protected void changeTileInfo(final TileInfoNew[] newTileInfo)
    {
        if(samplingLayer.get() == null)
            return;

        samplingLayer.get().layerThread.addTask(new Runnable() {
            @Override
            public void run() {
                tileInfos = newTileInfo;
                ChangeSet changes = new ChangeSet();
                reloadNative(changes);
                samplingLayer.get().layerThread.addChanges(changes);
            }
        });
    }

    /**
     * Forces a reload of all currently loaded tiles.
     */
    public void reload()
    {
        if (samplingLayer.get() == null)
            return;

        samplingLayer.get().layerThread.addTask(new Runnable() {
            @Override
            public void run() {
                ChangeSet changes = new ChangeSet();
                reloadNative(changes);
                samplingLayer.get().layerThread.addChanges(changes);
            }
        });
    }
}
