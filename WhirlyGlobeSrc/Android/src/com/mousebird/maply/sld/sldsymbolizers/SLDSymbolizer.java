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
import java.io.InputStream;
import java.util.List;
import java.util.ArrayList;


import android.graphics.Color;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Base64;
import android.util.Log;

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

            } else
                SLDParseHelper.skip(xpp);
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

    public static SLDGraphicParams graphicParamsForGraphicNode(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        SLDGraphicParams graphicParams = new SLDGraphicParams();

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }

            if (xpp.getName().equals("ExternalGraphic")) {

                parseMarkOrExternalGraphic(xpp, graphicParams, symbolizerParams);

            } else if (xpp.getName().equals("Mark")) {

                parseMarkOrExternalGraphic(xpp, graphicParams, symbolizerParams);

            } else if (xpp.getName().equals("Size")) {

                String size = SLDParseHelper.stringForLiteralInNode(xpp);
                if (size != null && SLDParseHelper.isStringNumeric(size)) {
                    double d = Double.valueOf(size).doubleValue();
                    graphicParams.setWidth((int)d);
                    graphicParams.setHeight((int)d);
                }

            }
        }


        return graphicParams;
    }


    public static void parseMarkOrExternalGraphic(XmlPullParser xpp, SLDGraphicParams graphicParams, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        String format = null;
        String href = null;
        String type = null;
        String encoding = null;
        String contents = null;
        String wellKnownName = null;
        boolean isMark = xpp.getName().equals("Mark");
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG)
                continue;

            if (xpp.getName().equals("WellKnownName") && isMark) {

                wellKnownName = SLDParseHelper.stringForLiteralInNode(xpp);

            } else if (xpp.getName().equals("OnlineResource")) {

                href = xpp.getAttributeValue(null, "href");

                type = xpp.getAttributeValue(null, "type");

                SLDParseHelper.skip(xpp);

            } else if (xpp.getName().equals("InlineContent")) {

                encoding = xpp.getAttributeValue(null, "encoding");

                contents = SLDParseHelper.stringForLiteralInNode(xpp);

                SLDParseHelper.skip(xpp);

            } else if (xpp.getName().equals("Format"))
                format = SLDParseHelper.stringForLiteralInNode(xpp);

            else if ((xpp.getName().equals("Fill") || xpp.getName().equals("Stroke")) && isMark) {

                boolean isFill = xpp.getName().equals("Fill");
                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.START_TAG)
                        continue;
                    if (xpp.getName().equals("SvgParameter") || xpp.getName().equals("CssParameter")) {

                        String name = xpp.getAttributeValue(null, "name");
                        String value = null;
                        while (xpp.next() != XmlPullParser.END_TAG) {
                            if (xpp.getEventType() != XmlPullParser.TEXT)
                                value = xpp.getText();
                            else if (xpp.getEventType() == XmlPullParser.START_TAG)
                                SLDParseHelper.skip(xpp);
                        }
                        if (name != null && value != null) {
                            if (isFill)
                                graphicParams.setFillColor(Integer.valueOf(Color.parseColor(value)));
                            else
                                graphicParams.setStrokeColor(Integer.valueOf(Color.parseColor(value)));
                        }
                    } else
                        SLDParseHelper.skip(xpp);
                }

            } else if (xpp.getName().equals("Stroke") && isMark) {

            }
        }

        if (format == null || (!format.equals("image/png") && !format.equals("image/gif")))
            return;
        // TODO: handle wellKnownName
        if (href != null) {
            if (type == null || type.equals("simple")) {
                InputStream inputStream = null;
                try {
                    inputStream = symbolizerParams.getAssetManager().open(symbolizerParams.getBasePath() + href);
                } catch (IOException e) {
                    Log.e("SLDSymbolizer", "parseMarkOrExternalGraphic", e);
                }
                if (inputStream != null) {
                    //Bitmap bmp = BitmapFactory.decodeFile(symbolizerParams.getBasePath() + href);
                    Bitmap bmp = BitmapFactory.decodeStream(inputStream);
                    graphicParams.setBitmap(bmp);
                }
            }
        } else if (contents != null) {
            if (encoding != null && encoding.equals("base64")) {
                byte[] base64converted=Base64.decode(contents,Base64.DEFAULT);
                Bitmap bmp=BitmapFactory.decodeByteArray(base64converted,0,base64converted.length);
                graphicParams.setBitmap(bmp);
            }

        }
    }

}
