/*
 *  Atmosphere.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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

import java.util.ArrayList;
import java.util.List;

/** Sets up the objects and shaders to implement an atmosphere.
 * <br>
 * This object sets up a shader implementation of the simple atmosphere from GPU Gems 2
 * http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
 */
public class Atmosphere {

    public static final String  k_v3CameraPos = "u_v3CameraPos";
    public static final String k_v3LightPos = "u_v3LightPos";
    public static final String k_fCameraHeight = "u_fCameraHeight";
    public static final String k_fCameraHeight2 = "u_fCameraHeight2";
    public static final String k_fInnerRadius = "u_fInnerRadius";
    public static final String k_fInnerRadius2 = "u_fInnerRadius2";
    public static final String k_fOuterRadius ="u_fOuterRadius";
    public static final String k_fOuterRadius2 ="u_fOuterRadius2";
    public static final String k_fScale = "u_fScale";
    public static final String k_fScaleDepth ="u_fScaleDepth";
    public static final String k_fScaleOverScaleDepth ="u_fScaleOverScaleDepth";
    public static final String k_Kr = "u_Kr";
    public static final String k_Kr4PI = "u_Kr4PI";
    public static final String k_Km = "u_Km";
    public static final String k_Km4PI ="u_Km4PI";
    public static final String k_ESun = "u_ESun";
    public static final String k_KmESun = "u_KmESun";
    public static final String k_KrESun = "u_KrESun";
    public static final String k_v3InvWavelength ="u_v3InvWavelength";
    public static final String k_fSamples ="u_fSamples";
    public static final String k_nSamples = "u_nSamples";
    public static final String k_g = "g";
    public static final String k_g2 = "g2";
    public static final String k_fExposure = "fExposure";

    public static final String vertexShaderAtmosTri =
            "precision highp float;\n"+
            "\n"+
            "uniform mat4  u_mvpMatrix;\n"+
            "uniform vec3 u_v3CameraPos;\n"+
            "uniform float u_fCameraHeight2;\n"+
            "uniform vec3 u_v3LightPos;\n"+
            "\n"+
            "uniform float u_fInnerRadius;\n"+
            "uniform float u_fInnerRadius2;\n"+
            "uniform float u_fOuterRadius;\n"+
            "uniform float u_fOuterRadius2;\n"+
            "uniform float u_fScale;\n"+
            "uniform float u_fScaleDepth;\n"+
            "uniform float u_fScaleOverScaleDepth;\n"+
            "\n"+
            "uniform float u_Kr;\n"+
            "uniform float u_Kr4PI;\n"+
            "uniform float u_Km;\n"+
            "uniform float u_Km4PI;\n"+
            "uniform float u_ESun;\n"+
            "uniform float u_KmESun;\n"+
            "uniform float u_KrESun;\n"+
            "uniform vec3 u_v3InvWavelength ;\n"+
            "uniform float u_fSamples;\n"+
            "uniform int u_nSamples;\n"+
            "\n"+
            "attribute vec3 a_position;\n"+
            "\n"+
            "varying highp vec3 v3Direction;"+
            "varying highp vec3 v3RayleighColor;\n"+
            "varying highp vec3 v3MieColor;\n"+
            "\n"+
            "float scale(float fCos)\n"+
            "{\n"+
            "  float x = 1.0 - fCos;\n"+
            "  return u_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));\n"+
            "}\n"+
            "\n"+
            "void main()\n"+
            "{"+
            "   vec3 v3Pos = a_position.xyz;\n"+
            "   vec3 v3Ray = v3Pos - u_v3CameraPos;\n"+
            "   float fFar = length(v3Ray);\n"+
            "   v3Ray /= fFar;\n"+
            "\n"+
            "  float B = 2.0 * dot(u_v3CameraPos, v3Ray);\n"+
            "  float C = u_fCameraHeight2 - u_fOuterRadius2;\n"+
            "  float fDet = max(0.0, B*B - 4.0 * C);\n"+
            "  float fNear = 0.5 * (-B - sqrt(fDet));\n"+
            "\n"+
            "   vec3 v3Start = u_v3CameraPos + v3Ray * fNear;\n"+
            "   fFar -= fNear;\n"+
            "\n"+
            "   float fStartAngle = dot(v3Ray, v3Start) / u_fOuterRadius;\n"+
            "   float fStartDepth = exp(-1.0/u_fScaleDepth);\n"+
            "   float fStartOffset = fStartDepth * scale(fStartAngle);\n"+
            "\n"+
            "   float fSampleLength = fFar / u_fSamples;\n"+
            "   float fScaledLength = fSampleLength * u_fScale;\n"+
            "   vec3 v3SampleRay = v3Ray * fSampleLength;\n"+
            "   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;\n"+
            "\n"+
            "   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);\n"+
            "   vec3 v3Attenuate;\n"+
            "   for (int i=0; i<u_nSamples; i++)\n"+
            "   {\n"+
            "     float fHeight = length(v3SamplePoint);\n"+
            "     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));\n"+
            "     float fLightAngle = dot(u_v3LightPos, v3SamplePoint) / fHeight;\n"+
            "     float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;\n"+
            "     float fScatter = (fStartOffset + fDepth *(scale(fLightAngle) - scale(fCameraAngle)));\n"+
            "     v3Attenuate = exp(-fScatter * (u_v3InvWavelength * u_Kr4PI + u_Km4PI));\n"+
            "     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);\n"+
            "     v3SamplePoint += v3SampleRay;\n"+
            "   }\n"+
            "\n"+
            "   v3MieColor = v3FrontColor * u_KmESun;\n"+
            "   v3RayleighColor = v3FrontColor * (u_v3InvWavelength * u_KrESun + u_Km4PI);\n"+
            "   v3Direction = u_v3CameraPos - v3Pos;\n"+
            "\n"+
            "   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"+
            "}\n";

    public static final String fragmentShaderAtmosTri =
            "precision highp float;\n"+
            "\n"+
            "uniform float g;\n"+
            "uniform float g2;\n"+
            "uniform float fExposure;\n"+
            "uniform vec3 u_v3LightPos;\n"+
            "\n"+
            "varying highp vec3 v3Direction;"+
            "varying highp vec3 v3RayleighColor;\n"+
            "varying highp vec3 v3MieColor;\n"+
            "\n"+
            "void main()\n"+
            "{\n"+
            "  float fCos = dot(u_v3LightPos, normalize(v3Direction)) / length(v3Direction);\n"+
            "  float fCos2 = fCos*fCos;\n"+
            "  float rayPhase = 0.75 + 0.75*fCos2;\n"+
            "  float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);\n"+
            "  vec3 color = rayPhase * v3RayleighColor + miePhase * v3MieColor;\n"+
            "  color = 1.0 - exp(color * -fExposure);"+
            "  gl_FragColor = vec4(color,color.b);\n"+
            "}\n";

    public static final String kAtmosphereShader = "Atmosphere Shader";

    public static final String vertexShaderGroundTri =
            "precision highp float;\n"+
            "\n"+
            "uniform mat4  u_mvpMatrix;\n"+
            "uniform vec3 u_v3CameraPos;\n"+
            "uniform float u_fCameraHeight2;\n"+
            "uniform vec3 u_v3LightPos;\n"+
            "\n"+
            "uniform float u_fInnerRadius;\n"+
            "uniform float u_fInnerRadius2;\n"+
            "uniform float u_fOuterRadius;\n"+
            "uniform float u_fOuterRadius2;\n"+
            "uniform float u_fScale;\n"+
            "uniform float u_fScaleDepth;\n"+
            "uniform float u_fScaleOverScaleDepth;\n"+
            "\n"+
            "uniform float u_Kr;\n"+
            "uniform float u_Kr4PI;\n"+
            "uniform float u_Km;\n"+
            "uniform float u_Km4PI;\n"+
            "uniform float u_ESun;\n"+
            "uniform float u_KmESun;\n"+
            "uniform float u_KrESun;\n"+
            "uniform vec3 u_v3InvWavelength ;\n"+
            "uniform float u_fSamples;\n"+
            "uniform int u_nSamples;\n"+
            "\n"+
            "attribute vec3 a_position;\n"+
            "attribute vec3 a_normal;\n"+
            "attribute vec2 a_texCoord0;\n"+
            "attribute vec2 a_texCoord1;\n"+
            "\n"+
            "varying mediump vec3 v_color;\n"+
            "varying mediump vec3 v_v3attenuate;\n"+
            "varying mediump vec2 v_texCoord0;"+
            "varying mediump vec2 v_texCoord1;\n"+
            "\n"+
            "float scale(float fCos)\n"+
            "{\n"+
            "  float x = 1.0 - fCos;\n"+
            "  return u_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));\n"+
            "}\n"+
            "\n"+
            "void main()\n"+
            "{"+
            "   vec3 v3Pos = a_normal.xyz;\n"+
            "   vec3 v3Ray = v3Pos - u_v3CameraPos;\n"+
            "   float fFar = length(v3Ray);\n"+
            "   v3Ray /= fFar;\n"+
            "\n"+
            "  float B = 2.0 * dot(u_v3CameraPos, v3Ray);\n"+
            "  float C = u_fCameraHeight2 - u_fOuterRadius2;\n"+
            "  float fDet = max(0.0, B*B - 4.0 * C);\n"+
            "  float fNear = 0.5 * (-B - sqrt(fDet));\n"+
            "\n"+
            "   vec3 v3Start = u_v3CameraPos + v3Ray * fNear;\n"+
            "   fFar -= fNear;\n"+
            "\n"+
            "   float fDepth = exp((u_fInnerRadius - u_fOuterRadius) / u_fScaleDepth);\n"+
            "   float fCameraAngle = dot(-v3Ray, v3Pos) / length (v3Pos);\n"+
            "   float fLightAngle = dot(u_v3LightPos, v3Pos) / length(v3Pos);\n"+
            "   float fCameraScale = scale(fCameraAngle);\n"+
            "   float fLightScale = scale(fLightAngle);\n"+
            "   float fCameraOffset = fDepth*fCameraScale;\n"+
            "   float fTemp = (fLightScale + fCameraScale);\n"+
            "\n"+
            "   float fSampleLength = fFar / u_fSamples;\n"+
            "   float fScaledLength = fSampleLength * u_fScale;\n"+
            "   vec3 v3SampleRay = v3Ray * fSampleLength;\n"+
            "   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;\n"+
            "\n"+
            "   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);\n"+
            "   vec3 v3Attenuate;\n"+
            "   for (int i=0; i<u_nSamples; i++)\n"+
            "   {\n"+
            "     float fHeight = length(v3SamplePoint);\n"+
            "     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));\n"+
            "     float fScatter = fDepth*fTemp - fCameraOffset;\n"+
            "     v3Attenuate = exp(-fScatter * (u_v3InvWavelength * u_Kr4PI + u_Km4PI));\n"+
            "     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);\n"+
            "     v3SamplePoint += v3SampleRay;\n"+
            "   }\n"+
            "\n"+
            "   v_v3attenuate = v3Attenuate;\n"+
            "   v_color = v3FrontColor * (u_v3InvWavelength * u_KrESun + u_KmESun);\n"+
            "   v_texCoord0 = a_texCoord0;\n"+
            "   v_texCoord1 = a_texCoord1;\n"+
            "\n"+
            "   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"+
            "}\n";

    public static final String fragmentShaderGroundTri =
            "precision mediump float;\n"+
            "\n"+
            "uniform sampler2D s_baseMap0;\n"+
            "uniform sampler2D s_baseMap1;\n"+
            "\n"+
            "varying vec3      v_color;\n"+
            "varying vec2      v_texCoord0;\n"+
            "varying vec2      v_texCoord1;\n"+
            "varying vec3      v_v3attenuate;\n"+
            "\n"+
            "void main()\n"+
            "{\n"+
            "  vec3 dayColor = texture2D(s_baseMap0, v_texCoord0).xyz * v_v3attenuate;\n"+
            "  vec3 nightColor = texture2D(s_baseMap1, v_texCoord1).xyz * (1.0 - v_v3attenuate);\n"+
            "  gl_FragColor = vec4(v_color, 1.0) + vec4(dayColor + nightColor, 1.0);\n"+
            "}\n";

    public static final String kAtmosphereGroundShader = "Atmosphere Ground Shader";

    public static final int kMaplyAtmosphereDrawPriorityDefault = 10;
    private float kr;
    private float km;
    private float eSun;
    private int numSamples;
    private float outerRadius;
    private float g;
    private float exposure;

    private Shader shader;
    private Shader groundShader;

    private GlobeController viewC;
    private ComponentObject comObj;
    private SunUpdater sunUpdater;
    float[] waveLength;

    /** @brief Rayleigh scattering constant (0.0025 by default)
     */
    public float getKr() {
        return kr;
    }

    /**
     * Rayleigh scattering constant (0.0025 by default)
     */
    public void setKr(float kr) {
        this.kr = kr;
    }

    /**
     * Mie scattering constant (0.0010 by default)
     */
    public float getKm() {
        return km;
    }

    /**
     * Mie scattering constant (0.0010 by default)
     */
    public void setKm(float km) {
        this.km = km;
    }

    /**
     * Brightness of the sun (20.0 by default)
     */
    public float geteSun() {
        return eSun;
    }

    /**
     * Brightness of the sun (20.0 by default)
     */
    public void seteSun(float eSun) {
        this.eSun = eSun;
    }

    /**
     * Number of samples for the ray through the atmosphere (3 by default)
     */
    public int getNumSamples() {
        return numSamples;
    }

    /**
     * Number of samples for the ray through the atmosphere (3 by default)
     */
    public void setNumSamples(int numSamples) {
        this.numSamples = numSamples;
    }

    /**
     * Outer radius of the atmosphere (1.05 by default).  Earth is radius 1.0.
     */
    public float getOuterRadius() {
        return outerRadius;
    }

    /**
     * Outer radius of the atmosphere (1.05 by default).  Earth is radius 1.0.
     */
    public void setOuterRadius(float outerRadius) {
        this.outerRadius = outerRadius;
    }

    /**
     * Constant used in the fragment shader.  Default is -0.95.
     */
    public float getG() {
        return g;
    }

    /**
     * Constant used in the fragment shader.  Default is -0.95.
     */
    public void setG(float g) {
        this.g = g;
    }

    /**
     * Exposure constant in fragment shader.  Default is 2.0.
     */
    public float getExposure() {
        return exposure;
    }

    /**
     * Exposure constant in fragment shader.  Default is 2.0.
     */
    public void setExposure(float exposure) {
        this.exposure = exposure;
    }

    /**
     * The ground shader we set up.  You need to apply it yourself.
     */
    public Shader getGroundShader() {
        return groundShader;
    }

    /**
     * Set up the atmospheric shaders and the default parameters for rendering.
     * @param inViewC The globe controller to create objects in.
     * @param mode Whether or not to work on this thread on put the work in the background.
     */
    public Atmosphere(GlobeController inViewC, MaplyBaseController.ThreadMode mode) {
        this.viewC = inViewC;
        this.kr = 0.0025f;
        this.km = 0.0010f;
        this.eSun = 20.0f;
        this.numSamples = 3;
        this.outerRadius = 1.05f;
        this.g = -0.95f;
        this.exposure = 2.0f;
        this.waveLength = new float[3];
        waveLength[0] = 0.650f;
        waveLength[1] = 0.570f;
        waveLength[2] = 0.475f;
        shader = this.setupShader();
        if (shader == null)
            return;

        this.groundShader = this.setupGroundShader();

        this.complexAtmosphere(mode);
    }

    /**
     * Wavelengths of the light (RGB).  Three floats, defaults are: 0.650, 0.570, 0.475
     */
    public void setWaveLength(float [] waveLength) {
        if (waveLength == null)
            waveLength = new float[3];
        this.waveLength[0] = waveLength[0];
        this.waveLength[1] = waveLength[1];
        this.waveLength[2] = waveLength[2];
    }

    /**
     * Wavelengths of the light (RGB).  Three floats, defaults are: 0.650, 0.570, 0.475
     */
    public void setWaveLength(float redWavelength, float greenWavelength, float blueWavelength) {
        if (waveLength == null)
            waveLength = new float[3];
        this.waveLength[0] = redWavelength;
        this.waveLength[1] = greenWavelength;
        this.waveLength[2] = blueWavelength;
    }

    /**
     * Wavelengths of the light (RGB).  Three floats, defaults are: 0.650, 0.570, 0.475
     */
    public float[] getWaveLength() {
        return this.waveLength.clone();
    }

    /**
     * Wavelengths of the light (RGB).  Three floats, defaults are: 0.650, 0.570, 0.475
     */
    public float getWavelengthForComponent(short component) {
        return this.waveLength[component];
    }

    // Set up the complex atmospheric shader
    private void complexAtmosphere(MaplyBaseController.ThreadMode mode) {
        ShapeSphere sphere = new ShapeSphere();
        sphere.setLoc(new Point2d(0, 0));
        sphere.setRadius(this.outerRadius);
        sphere.setHeight(-1.0f);
        sphere.setSampleX(120);
        sphere.setSampleY(60);

        List<Shape> shapes = new ArrayList<Shape>();
        shapes.add(sphere);
        ShapeInfo shapeInfo = new ShapeInfo();
        shapeInfo.setZBufferRead(false);
        shapeInfo.setZBufferWrite(false);
        shapeInfo.setInsideOut(true);
        shapeInfo.setCenter(new Point3d(0, 0, 0));
        shapeInfo.setDrawPriority(kMaplyAtmosphereDrawPriorityDefault);
        shapeInfo.setShader(shader);
        this.comObj = this.viewC.addShapes(shapes, shapeInfo, mode);

        this.sunUpdater = new SunUpdater(this.shader, this.groundShader,this, viewC);
        this.viewC.addActiveObject(sunUpdater);
    }

    // Set up a corresponding ground shader
    private Shader setupGroundShader() {
        Shader theShader = new Shader(kAtmosphereGroundShader, vertexShaderGroundTri, fragmentShaderGroundTri, viewC);
        if (!theShader.valid())
            return null;

        viewC.addShaderProgram(theShader, kAtmosphereGroundShader);

        return theShader;
    }

    /**
     * Set the sun's position relative to the earth.  This is what comes out of MaplySun.
     */
    public void setSunPosition(Point3d sunDir) {
        if (this.sunUpdater != null){
            this.sunUpdater.setSunPosition(sunDir);
        }
    }

    /**
     * Return the shader for use by the atmosphere.
     * @return
     */
    public Shader setupShader() {
        Shader theShader = new Shader(kAtmosphereShader, vertexShaderAtmosTri, fragmentShaderAtmosTri, viewC);
        if (!theShader.valid())
            return null;

        viewC.addShaderProgram(theShader, kAtmosphereShader);

        return theShader;
    }

    /**
     * Remove any objects from the globe controller and shut down the atmosphere.
     */
    public void removeFromController() {
        if (comObj!= null)
            viewC.removeObject(comObj, MaplyBaseController.ThreadMode.ThreadAny);
        comObj = null;

        if (sunUpdater != null)
            viewC.removeActiveObject(this.sunUpdater);
        this.sunUpdater = null;
        //Note: Should remove shader
    }

}
