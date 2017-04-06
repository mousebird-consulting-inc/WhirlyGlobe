/*
 *  SLDSymbolizer.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/14/17.
 *  Copyright 2011-2017 mousebird consulting
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
package com.mousebird.maply.sld.sldsymbolizers;

import com.mousebird.maply.LinearTextureBuilder;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.VectorTileLineStyle;
import com.mousebird.maply.BaseInfo;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.WideVectorInfo;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.Vector;
import java.util.List;
import java.util.ArrayList;


import android.graphics.Color;
import android.graphics.Bitmap;

public abstract class SLDSymbolizer {

    public abstract VectorTileStyle[] getStyles();

    public static VectorTileLineStyle vectorTileLineStyleFromStrokeNode(XmlPullParser xpp, MaplyBaseController viewC, VectorStyleSettings vectorStyleSettings) throws XmlPullParserException, IOException {

        boolean useWideVectors = vectorStyleSettings.isUseWideVectors();
        BaseInfo baseInfo;
        VectorInfo vectorInfo = null;
        WideVectorInfo wideVectorInfo = null;
        if (useWideVectors) {
            wideVectorInfo = new WideVectorInfo();
            baseInfo = wideVectorInfo;
        } else {
            vectorInfo = new VectorInfo();
            baseInfo = vectorInfo;
        }
        baseInfo.disposeAfterUse = true;
        baseInfo.setEnable(false);
        //baseInfo.setDrawPriority(???)

        Integer strokeColor = null;
        Float strokeOpacity = null;
        String dashArray = null;

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("SvgParameter") || xpp.getName().equals("CssParameter")) {

                String name = xpp.getAttributeValue(null, "name");
                String value = null;
                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.TEXT) {
                        continue;
                    }
                    value = xpp.getText();
                }
                if (name == null || value == null)
                    continue;

                if (name.equals("stroke")) {
                    try {
                        strokeColor = Integer.valueOf(Color.parseColor(value));
                    } finally {
                    }

                } else if (name.equals("stroke-width") && SLDParseHelper.isStringNumeric(value)) {
                    try {
                        float strokeWidth = Double.valueOf(value).floatValue();
                        if (useWideVectors)
                            wideVectorInfo.setLineWidth(strokeWidth);
                        else
                            vectorInfo.setLineWidth(strokeWidth);
                    } finally {
                    }

                } else if (name.equals("stroke-opacity") || name.equals("opacity")) {
                    try {
                        strokeOpacity = Float.valueOf(value);
                    } finally {
                    }

                } else if (name.equals("stroke-dasharray")) {
                    dashArray = value;
                }

            }
        }

        if (strokeColor != null) {
            int color = strokeColor.intValue();
            if (strokeOpacity != null)
                color = Color.argb(Math.round(strokeOpacity.floatValue()*255.f), Color.red(color), Color.green(color), Color.blue(color));

            if (useWideVectors)
                wideVectorInfo.setColor(color);
            else
                vectorInfo.setColor(color);
        }

        if (useWideVectors) {
            LinearTextureBuilder texBuild = new LinearTextureBuilder();
            double repeatLength = 4.0;
            if (dashArray != null) {
                String[] componentStrings = dashArray.split(",");
                List<Integer> componentNumbers = new ArrayList<Integer>();
                for (String s : componentStrings) {
                    try {
                        Integer i = Integer.valueOf(s);
                        componentNumbers.add(i);
                    } finally {
                    }
                }
                if (componentNumbers.size() > 0) {
                    int[] pattern = new int[componentNumbers.size()];
                    repeatLength = 0.0;
                    for (Integer i : componentNumbers) {
                        pattern[i] = i.intValue();
                        repeatLength += (double)i.intValue();
                    }
                    texBuild.setPattern(pattern);
                } else {
                    texBuild.setPattern(new int[]{4});
                }
            } else {
                texBuild.setPattern(new int[]{4});
            }
            Bitmap patternImage = texBuild.makeImage();
            MaplyBaseController.TextureSettings texSet = new MaplyBaseController.TextureSettings();
            texSet.wrapU = true;  texSet.wrapV = true;
            MaplyTexture tex = viewC.addTexture(patternImage,new MaplyBaseController.TextureSettings(), MaplyBaseController.ThreadMode.ThreadCurrent);
            wideVectorInfo.setTexture(tex);
            wideVectorInfo.setTextureRepeatLength(repeatLength);
        }


        VectorTileLineStyle vectorTileLineStyle = new VectorTileLineStyle(baseInfo, vectorStyleSettings, viewC);
        return vectorTileLineStyle;

    }

}
