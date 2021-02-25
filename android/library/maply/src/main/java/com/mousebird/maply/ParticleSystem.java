/*
 *  ParticleSystem.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 18/1/16
 *  Copyright 2011-2016 mousebird consulting
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

import java.util.ArrayList;
import java.util.Date;

/**
 * A particle system is used to spawn large numbers of small moving objects.
 * <br>
 * The particle system defines what the objects are and how they're controlled.
 * Actual data is handled through the ParticleBatch.
 * <br>
 * You set up a particle system and then add ParticleBatches via a view controller.
 */
public class ParticleSystem {

    public enum Type {
        /**
         * The particles are defined as points.
         */
        Point,
        /**
         * Particles are defined as rectangles.
         */
        Rectangle;
    }

    private ParticleSystem() {
    }

    /**
     * The particle system name is used for performance debugging.
     * @param name Name of the particle system.
     */
    public ParticleSystem(String name) {
        initialise();
        this.setName(name);
        this.setParticleSystemType(Type.Point);
        this.setLifetime(5.0);
        this.setBatchSize(2000);
        this.setTotalParticles(100000);
        this.setBasetime(new Date().getTime() / 1000.0);
    }

    public void finalize() {
        dispose();
    }

    native private void setName(String name);

    /**
     * Returns the unique ID used by the renderer.
     */
    public native long getID();

    /**
     * At present particle systems are just point geometry.
     * @param type The type of the particle system.
     */
    public void setParticleSystemType(ParticleSystem.Type type)
    {
        setParticleSystemTypeNative(type.ordinal());
    }

    private native void setParticleSystemTypeNative(int type);

    /**
     * Assign a shader to the position stage of this particle system.
     */
    public void setPositionShader(Shader shader)
    {
        setPositionShaderID(shader.getID());
    }

    /**
     * Assign a shader to the position stage of this particle system.
     * @param shaderID The unique ID of the shader
     */
    public native void setPositionShaderID(long shaderID);

    /**
     * Assign a shader to the render stage of this particle system.
     */
    public void setRenderShader(Shader shader)
    {
        setRenderShaderID(shader.getID());
    }

    /**
     * Assign a shader to the render stage of this particle system.
     * @param shaderID The unique ID of the shader
     */
    public native void setRenderShaderID(long shaderID);

    /**
     * Sets the particle lifetime.  The system will try to keep particles around for this long.
     * Individual particle lifetime.
     * <br>
     * The created particles will last only a certain amount of time.
     * @param lifetime
     */
    public native void setLifetime(double lifetime);

    /**
     * Particles move over time, but current time is a large number.
     * This is the base time you can use to subtract from current time values.
     * <br>
     * Individual particles will measure their own lifetime against this base value.
     * @param basetime
     */
    public native void setBasetime(double basetime);

    /**
     * Particles move over time, but current time is a large number.
     * This is the base time you can use to subtract from current time values.
     * @return The base that particle time is measured from.
     */
    public native double getBasetime();

    /**
     * The total number of particles to display at once.  These will be broken up into
     * batchSized chunks.
     * <br>
     * This is the most particles we'll have on the screen at any time.
     * Space will be allocated for them, so don't overdo it.
     * @param totalParticles Total number of particles to be represented at once.
     */
    public native void setTotalParticles(int totalParticles);

    /**
     * Total number of particles to display at once.
     */
    public native int getTotalParticles();

    /**
     * Size of the individual batches you add when adding particles.  The total number of
     * particles should be a multiple of this.
     * <br>
     * Particles need to be created in large batches for efficiency.
     * This is the size of individual batches.
     * @param batchSize Batch size for ParticleBatch.
     */
    public native void setBatchSize(int batchSize);

    /**
     * Size of the individual batches you add when adding particles.
     * @return Total number of particles to be represented at once.
     */
    public native int getBatchSize();

    /**
     * Turn the continuous render on or off.  By default the particle system is going to
     * move all the particles all the time.  This means the renderer must execute every frame.
     * There are times where this is not appropriate (e.g. stars) and so we can turn it off.
     */
    public native void setContinuousUpdate(boolean cRender);

    /**
     * Set the draw priority for the particles
     */
    public native void setDrawPriority(int drawPriority);

    /**
     * Set the point size if the particle system is points.
     */
    public native void setPointSize(float pointSize);

    /**
     * Add an attribute that will appear in each batch.  Attributes are data values that
     * go into each and every particle.
     * @param name The name of the attribute we'll look for in each batch.
     * @param type Type of the attribute.
     */
    public void addAttribute(String name,Shader.AttributeType type)
    {
        addAttributeNative(name,type.ordinal());
    }

    private native void addAttributeNative(String name, int type);

    /**
     * For two stage shaders, these are the varying outputs from one shader to the next.
     * <br>
     * Two stage shaders run a position shader and then a regular render shader
     * from the position output.  Add any varying values you want to share per
     * vertex from the former to the latter.
     * @param name The name of the varying output
     * @param inputName Name of the input to the next shader
     * @param type Data type of the varying
     */
    public void addVarying(String name,String inputName,Shader.AttributeType type)
    {
        addVaryingNative(name,inputName,type.ordinal());
    }

    private native void addVaryingNative(String name,String inputName,int type);

    /**
     * Add a texture to the particle system.
     * <br>
     * All the textures will be handed over to the shader.
     */
    public void addTexture(MaplyTexture tex) {
        addTextureID(tex.texID);
    }

    /**
     * Add a texture to the shader by ID.
     */
    public native void addTextureID(long texID);

    /**
     * If set we'll compare against the z buffer.
     */
    public native void setZBufferRead(boolean zBufferRead);

    /**
     * If set, we'll write to the z buffer.
     */
    public native void setZBufferWrite(boolean zBufferWrite);

    /**
     * If the particle system is drawing to a render target, set it here.
     */
    public void setRenderTarget(RenderTarget target)
    {
        setRenderTargetNative(target.renderTargetID);
    }

    private native void setRenderTargetNative(long targetID);

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();

    private long nativeHandle;
}
