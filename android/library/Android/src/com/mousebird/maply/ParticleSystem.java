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

    public enum STATE {
        /**
         * The particles are defined as points.
         */
        ParticleSystemPoint(0),
        /**
         * Particles are defined as rectangles.
         */
        ParticleSystemRectangle(1);

        private final int value;

        STATE(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }
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
        this.setParticleSystemType(STATE.ParticleSystemPoint);
        this.setLifetime(5.0);
        this.setBatchSize(2000);
        this.setTotalParticles(100000);
        this.setBasetime(new Date().getTime()/1000.0);
    }

    /**
     * Add a texture to the particle system.
     * <br>
     * All the textures will be handed over to the shader.
     * @param texture
     */
    public void addTexture(Bitmap texture) {
        images.add(texture);
    }

    /**
     * Return the list of textures being used by the shader.
     * @return
     */
    public ArrayList<Bitmap> getTextures() {
        return images;
    }

    public void finalize() {
        dispose();
    }

    public native long getIdent();

    /**
     * The particle system name is used for performance debugging.
     * @param name Name of the particle system.
     */
    public native void setName(String name);

    /**
     * Set the draw priority for the particles
     */
    public native void setDrawPriority(int drawPriority);

    /**
     * Set the point size if the particle system is points.
     */
    public native void setPointSize(float pointSize);

    /**
     * Set the shader by ID.  There are times this is useful, but in
     * general you should call setShader with the shader.
     * The type of the particle system.
     * <br>
     * At present particle systems are just point geometry.
     * @param particleSystemType The type of the particle system.
     */
    public void setParticleSystemType(ParticleSystem.STATE type)
    {
        setParticleSystemTypeNative(type.getValue());
    }

    native void setParticleSystemTypeNative(int particleSystemType);

    /**
     * Name of the shader to use for the particles.
     * This should be a shader already registered with the toolkit.
     * @param shaderID
     */
    public native void setShaderID(long shaderID);

    /**
     * Assign a shader to this particle system.
     * The shader contains all the code to implement the system.
     */
    public void setShader(Shader shader)
    {
        setShaderID(shader.getID());
    }

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
     * The total number of particles to display at once.  These will be broken up into
     * batchSized chunks.
     * <br>
     * This is the most particles we'll have on the screen at any time.
     * Space will be allocated for them, so don't overdo it.
     * @param totalParticles Total number of particles to be represented at once.
     */
    public native void setTotalParticles(int totalParticles);

    /**
     * Turn the continuous render on or off.  By default the particle system is going to
     * move all the particles all the time.  This means the renderer must execute every frame.
     * There are times where this is not appropriate (e.g. stars) and so we can turn it off.
     */
    public native void setContinuousRender(boolean cRender);

    /**
     * Add an attribute that will appear in each batch.  Attributes are data values that
     * go into each and every particle.
     * @param name The name of the attribute we'll look for in each batch.
     * @param type Type of the attribute.
     */
    public void addParticleSystemAttribute(String name,ParticleSystemAttribute.MaplyShaderAttrType type)
    {
        int which = names.size();
        names.add(name);
        types.add(type.getValue());
        addParticleSystemAttributeNative(name,type.ordinal());
    }

    native void addParticleSystemAttributeNative(String name, int type);

    native void addTexID(long texID);

    /**
     * Returns a list of particle attributes.  For use internally.
     */
    public ParticleSystemAttribute [] getAttrs() {
        if (names.size() != types.size()) {
            return null;
        }

        ParticleSystemAttribute attrsList[] = new ParticleSystemAttribute[names.size()];
        for (int i = 0 ; i < attrsList.length; i++) {
            attrsList[i] = new ParticleSystemAttribute();
            attrsList[i].setName(names.get(i));
            attrsList[i].setType(ParticleSystemAttribute.MaplyShaderAttrType.values()[types.get(i)]);
        }

        return attrsList;
     }

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

    private ArrayList<String> names = new ArrayList<String>();
    private ArrayList<Integer> types = new ArrayList<Integer>();
    private ArrayList<Bitmap> images = new ArrayList<>();

}
