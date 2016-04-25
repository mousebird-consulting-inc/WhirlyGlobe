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
 * The particle system defines what the objects are and how they're controlled.
 * Actual data is handled through the ParticleBatch.
 * You set up a particle system and then add ParticleBatches via a MaplyBaseController.
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

    private ArrayList<Bitmap> images = new ArrayList<>();

    private ParticleSystem()
    {}

    /**
     * Construct with the name of the particle system.
     * @param name Name is used internally for reference.
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
     * All the textures will be handed over to the shader in the order they are defined.
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
     * Set the name of the particle system.  This is used internally.
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
     * Particles systems will generate either points or rectangles.
     */
    public void setParticleSystemType(ParticleSystem.STATE type)
    {
        setParticleSystemTypeNative(type.getValue());
    }

    native void setParticleSystemTypeNative(int particleSystemType);

    /**
     * Set the shader by ID.  There are times this is useful, but in
     * general you should call setShader with the shader.
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
     */
    public native void setLifetime(double lifetime);

    /**
     * Particles move over time, but current time is a large number.
     * This is the base time you can use to subtract from current time values.
     */
    public native void setBasetime(double basetime);

    /**
     * Particles move over time, but current time is a large number.
     * This is the base time you can use to subtract from current time values.
     */
    public native double getBasetime();

    /**
     * Size of the individual batches you add when adding particles.  The total number of
     * particles should be a multiple of this.
     */
    public native void setBatchSize(int batchSize);

    /**
     * Size of the individual batches you add when adding particles.
     */
    public native int getBatchSize();

    /**
     * The total number of particles to display at once.  These will be broken up into
     * batchSized chunks.
     */
    public native void setTotalParticles(int totalParticles);

    ArrayList<String> names = new ArrayList<String>();
    ArrayList<Integer> types = new ArrayList<Integer>();

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

}
