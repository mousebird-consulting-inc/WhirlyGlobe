/*
 *  MaplyStarModel.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.Shader;

import org.apache.commons.io.IOUtils;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.TimeZone;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import jodd.datetime.JDateTime;


public class MaplyStarModel {

    private class SingleStar
    {
        float mag;
        float ra, dec;
    }

    private static final String vertexShaderTriPoint =
            "uniform mat4  u_mvpMatrix;"+
            "uniform float u_radius;"+
            ""+
            "attribute vec3 a_position;"+
            "attribute float a_size;"+
            ""+
            "varying vec4 v_color;"+
            ""+
            "void main()"+
            "{"+
            "   v_color = vec4(1.0,1.0,1.0,1.0);"+
            "   gl_PointSize = a_size;"+
            "   gl_Position = u_mvpMatrix * vec4(a_position * u_radius,1.0);"+
            "}";

    private static final String fragmentShaderTriPoint =
            "precision lowp float;"+
            ""+
            "varying vec4      v_color;"+
            ""+
            "void main()"+
            "{"+
            "  gl_FragColor = v_color;"+
            "}";

    private static final String fragmentShaderTexTriPoint =
            "precision lowp float;"+
            ""+
            "uniform sampler2D s_baseMap0;"+
            "varying vec4      v_color;"+
            ""+
            "void main()"+
            "{"+
            "  gl_FragColor = v_color * texture2D(s_baseMap0, gl_PointCoord);"+
            "}";

    ArrayList<SingleStar> stars;
    ParticleSystem particleSystem;
    ComponentObject particleSystemObj;
    GlobeController viewC;
    MaplyBaseController.ThreadMode addedMode;
    Bitmap image;


    public MaplyStarModel(String fileName, String imageName, Activity activity) throws IOException {

        AssetManager assetMgr = activity.getAssets();
        InputStream inputStream;
        String[] paths = assetMgr.list("maplystarmodel");
        for (String path : paths) {
            if (path.equals(imageName))
            { //image
                inputStream = assetMgr.open("maplystarmodel/" + path);
                BufferedInputStream bufferedInputStream = new BufferedInputStream(inputStream);
                image = BitmapFactory.decodeStream(bufferedInputStream);

            }
            if (path.equals(fileName))
            { //data
                this.stars = new ArrayList<SingleStar>();
                inputStream = assetMgr.open("maplystarmodel/" + path);
                String stars = IOUtils.toString(inputStream, Charset.defaultCharset());
                Pattern p = Pattern.compile("[-]?[0-9]*\\.?[0-9]+");
                Matcher m = p.matcher(stars);
                if (m.groupCount() % 3 == 0){
                    int i = 0;
                    SingleStar s = null;
                    while (m.find()) {
                        switch (i) {
                            case 0:
                                s = new SingleStar();
                                s.ra = Float.valueOf(m.group());
                                i++;
                                break;
                            case 1:
                                s.dec = Float.valueOf(m.group());
                                i++;
                                break;
                            case 2:
                                s.mag = Float.valueOf(m.group());
                                this.stars.add(s);
                                i = 0;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

    public void addToViewc (GlobeController inViewC, MaplyBaseController.ThreadMode mode) {
        this.viewC = inViewC;
        this.addedMode = mode;

        //Julian date for position calculation
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        JDateTime jdt = new JDateTime(cal.getTimeInMillis());
        double jd = jdt.getJulianDateDouble();
        double siderealTime = Greenwich_Mean_Sidereal_Deg(jd);

        //Really simple shader
        Shader shader = new Shader("Star Shader", vertexShaderTriPoint, (image != null ? fragmentShaderTexTriPoint : fragmentShaderTriPoint), viewC);
        shader.setUniform("u_radius", 6.0);
        viewC.addShaderProgram(shader, "Star Shader");

        long shaderID = viewC.getScene().getProgramIDBySceneName("Star Shader");

        //Set up a simple particle system (that doesn't move)
        particleSystem = new ParticleSystem("Stars");
        particleSystem.setParticleSystemType(ParticleSystem.STATE.ParticleSystemPoint.getValue());
        particleSystem.setLifetime(1e20);
        particleSystem.setTotalParticles(stars.size());
        particleSystem.setBatchSize(stars.size());
        particleSystem.setShaderID(shaderID);



        if (image != null){
            particleSystem.addTexture(image);
        }

        particleSystem.addParticleSystemAttribute("a_position", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
        particleSystem.addParticleSystemAttribute("a_size", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT);

        particleSystemObj = viewC.addParticleSystem(particleSystem, addedMode);

        //Data arrays for particles
        //We'll clear them out in case we don't fill them out completely
        float[] posData = new float[stars.size()*3];
        float[] sizeData = new float[stars.size()];

        for(int i =0; i < stars.size(); i++) {
            SingleStar singleStar = stars.get(i);

            //Convert the start from equatorial to useable lon/lat
            //Note: Should check this math
            double starLon = Math.toRadians(singleStar.ra - 15*siderealTime);
            double starLat = Math.toRadians(singleStar.dec);

            double z = Math.sin(starLat);
            double rad = Math.sqrt(1.0 - z * z);
            Point3d pt = new Point3d(rad*Math.cos(starLon), rad * Math.sin(starLon), z);

            posData[i*3] = (float) pt.getX();
            posData[i*3+1] = (float)  pt.getY();
            posData[i*3+2] = (float) pt.getZ();
            float mag = (float) (6.0 - singleStar.mag);
            if (mag < 0.0)
                mag = (float) 0.0;
            sizeData[i] = mag;
        }

        //Set up the particle batch
        ParticleBatch batch = new ParticleBatch(particleSystem);
        batch.addAttribute("a_position", posData);
        batch.addAttribute("a_size", sizeData);
        viewC.addParticleBatch(batch, addedMode);

    }

    public void removeFromView() {
        if (particleSystemObj != null)
            viewC.removeObject(particleSystemObj, addedMode);
    }

    private double Greenwich_Mean_Sidereal_Deg(double mjd) {
        // calculate T
        double T = (mjd-51544.5)/36525.0;

        // do calculation
        double gmst = ( (280.46061837 + 360.98564736629*(mjd-51544.5)) + 0.000387933*T*T - T*T*T/38710000.0) % 360.0;

        // make positive
        if(gmst < 0) {
            gmst += 360.0;
        }

        return gmst;
    } //Greenwich_Mean_Sidereal_Deg
}
