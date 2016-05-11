/*
 *  ParticleSystem.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 18/1/16
 *  Copyright 2011-2015 mousebird consulting
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
        ParticleSystemPoint(0),
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
        this.setParticleSystemType(STATE.ParticleSystemPoint.getValue());
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

    public native void setDrawPriority(int drawPriority);

    public native void setPointSize(float pointSize);

    /**
     * The type of the particle system.
     * <br>
     * At present particle systems are just point geometry.
     * @param particleSystemType The type of the particle system.
     */
    public native void setParticleSystemType(int particleSystemType);

    /**
     * Name of the shader to use for the particles.
     * This should be a shader already registered with the toolkit.
     * @param shaderID
     */
    public native void setShaderID(long shaderID);

    /**
     * Individual particle lifetime.
     * <br>
     * The created particles will last only a certain amount of time.
     * @param lifetime
     */
    public native void setLifetime(double lifetime);

    /**
     * The base that particle time is measured from.
     * <br>
     * Individual particles will measure their own lifetime against this base value.
     * @param basetime
     */
    public native void setBasetime(double basetime);

    /**
     * @return The base that particle time is measured from.
     */
    public native double getBasetime();

    /**
     * Batch size for ParticleBatch.
     * <br>
     * Particles need to be created in large batches for efficiency.
     * This is the size of individual batches.
     * @param batchSize Batch size for ParticleBatch.
     */
    public native void setBatchSize(int batchSize);

    /**
     * @return Total number of particles to be represented at once.
     */
    public native int getBatchSize();

    /**
     * Total number of particles to be represented at once.
     * <br>
     * This is the most particles we'll have on the screen at any time.
     * Space will be allocated for them, so don't overdo it.
     * @param totalParticles Total number of particles to be represented at once.
     */
    public native void setTotalParticles(int totalParticles);

    /**
     * Add an attribute we'll be expecting in each batch.
     * <br>
     * Adds an attribute name and type which will be present in each batch.
     */
    public void addParticleSystemAttribute(String name,ParticleSystemAttribute.MaplyShaderAttrType type)
    {
        int which = names.size();
        names.add(name);
        types.add(type.getValue());
        addParticleSystemAttributeNative(name,type.ordinal());
    }

    public native void addParticleSystemAttributeNative(String name, int type);

    public native void addTexID(long texID);

    public ParticleSystemAttribute[] getAttrs() {
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
