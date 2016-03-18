/*
 *  SunUpdater.java
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


public class SunUpdater implements ActiveObject {

    private boolean changed;
    private boolean started;
    private Point3d sunPos;
    private Shader shader;
    private Shader groundShader;
    private Atmosphere atm;

    public GlobeController viewC;

    public SunUpdater(Shader inShader, Shader inGroundShader, Atmosphere inAtmosphere,  GlobeController viewC) {
        this.shader = inShader;
        this.groundShader = inGroundShader;
        this.atm = inAtmosphere;
        this.viewC = viewC;
        changed = true;
        started = false;
    }

    public void setSunPosition(Point3d inSunPos) {
        this.sunPos = inSunPos;
        this.changed = true;
    }

    @Override
    public void activeUpdate() {
        //TODO REVIEW
        Point3d cameraPos = viewC.getPosition();
        Point4d sunDir4d = new Point4d(sunPos.getX(), sunPos.getY(), sunPos.getZ(), 1.0);
        Point3d sunDir3d = new Point3d(sunDir4d.getX(), sunDir4d.getY(), sunDir4d.getZ());

        sunDir3d.normalize();

        double cameraHeight = cameraPos.norm();

        float scale = 1.0f / (atm.getOuterRadius() - 1.f);
        float scaleDepth = 0.25f;

        float[] wavelength = atm.getWaveLength();
        for (int ii = 0; ii <3; ii++){
            wavelength[ii] = (float)(1.0/ Math.pow(wavelength[ii], 4.0));
        }

        Shader[] shaders = {shader, groundShader};

        for (int ii = 0; ii < 2; ii++){
            Shader thisShader = shaders[ii];
            thisShader.setUniform(Atmosphere.k_v3CameraPos, cameraPos);
            thisShader.setUniform(Atmosphere.k_fCameraHeight, (float) cameraHeight);
            thisShader.setUniform(Atmosphere.k_fCameraHeight2, (float)(cameraHeight*cameraHeight));
            thisShader.setUniform(Atmosphere.k_v3LightPos, sunDir3d);

            thisShader.setUniform(Atmosphere.k_fInnerRadius, 1.f);
            thisShader.setUniform(Atmosphere.k_fInnerRadius2, 1.f);
            thisShader.setUniform(Atmosphere.k_fOuterRadius, atm.getOuterRadius());
            thisShader.setUniform(Atmosphere.k_fOuterRadius2, atm.getOuterRadius()*atm.getOuterRadius());
            thisShader.setUniform(Atmosphere.k_fScale, scale);
            thisShader.setUniform(Atmosphere.k_fScaleDepth, scaleDepth);
            thisShader.setUniform(Atmosphere.k_fScaleOverScaleDepth, scale/scaleDepth);

            thisShader.setUniform(Atmosphere.k_Kr, atm.getKr());
            thisShader.setUniform(Atmosphere.k_Kr4PI, (float) (atm.getKr() * 4.0 * Math.PI ));
            thisShader.setUniform(Atmosphere.k_Km, atm.getKm());
            thisShader.setUniform(Atmosphere.k_Km4PI, (float) (atm.getKm() * 4.0 * Math.PI));
            thisShader.setUniform(Atmosphere.k_ESun, atm.geteSun());
            thisShader.setUniform(Atmosphere.k_KmESun, atm.getKm() * atm.geteSun());
            thisShader.setUniform(Atmosphere.k_KrESun, atm.getKr() * atm.geteSun());
            thisShader.setUniform(Atmosphere.k_v3InvWavelength, new Point3d(wavelength[0], wavelength[1], wavelength[2]));
            thisShader.setUniform(Atmosphere.k_fSamples, (float) atm.getNumSamples());
            thisShader.setUniform(Atmosphere.k_nSamples, atm.getNumSamples());

            thisShader.setUniform(Atmosphere.k_g, atm.getG());
            thisShader.setUniform(Atmosphere.k_g2, atm.getG());
            thisShader.setUniform(Atmosphere.k_fExposure, atm.getExposure());
        }

        this.changed = false;
        this.started = true;
    }
}
