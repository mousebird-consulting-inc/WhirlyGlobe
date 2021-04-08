/*  SLDSymbolizer.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/14/17.
 *  Copyright 2011-2021 mousebird consulting
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
 */
package com.mousebird.maply.sld.sldsymbolizers;

import com.mousebird.maply.LinearTextureBuilder;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.RenderController;
import com.mousebird.maply.RenderControllerInterface;
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

/**
 *
 * Base class for Symbolizer elements
 * see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 * see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
public abstract class SLDSymbolizer {

    public abstract VectorTileStyle[] getStyles();

    public static VectorTileLineStyle vectorTileLineStyleFromStrokeNode(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {

        RenderControllerInterface viewC = symbolizerParams.getBaseController();
        VectorStyleSettings vectorStyleSettings = symbolizerParams.getVectorStyleSettings();

        boolean useWideVectors = vectorStyleSettings.getUseWideVectors();
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
        baseInfo.setEnable(false);
        if (symbolizerParams.getMinScaleDenominator() != null) {
            if (symbolizerParams.getMaxScaleDenominator() == null)
                baseInfo.setMaxVis(Float.MAX_VALUE);
            baseInfo.setMinVis((float) viewC.heightForMapScale(symbolizerParams.getMinScaleDenominator().floatValue()));
        }
        if (symbolizerParams.getMaxScaleDenominator() != null) {
            if (symbolizerParams.getMinScaleDenominator() == null)
                baseInfo.setMinVis(0.0f);
            baseInfo.setMaxVis((float) viewC.heightForMapScale(symbolizerParams.getMaxScaleDenominator().floatValue()));
        }

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
                    strokeColor = Color.parseColor(value);

                } else if (name.equals("stroke-width") && SLDParseHelper.isStringNumeric(value)) {
                    float strokeWidth = Double.valueOf(value).floatValue();
                    if (useWideVectors)
                        wideVectorInfo.setLineWidth(strokeWidth);
                    else
                        vectorInfo.setLineWidth(strokeWidth);

                } else if (name.equals("stroke-opacity") || name.equals("opacity")) {
                    strokeOpacity = Float.valueOf(value);

                } else if (name.equals("stroke-dasharray")) {
                    dashArray = value;
                }

            } else
                SLDParseHelper.skip(xpp);
        }

        if (strokeColor != null) {
            int color = strokeColor;
            if (strokeOpacity != null)
                color = Color.argb(Math.round(strokeOpacity *255.f), Color.red(color), Color.green(color), Color.blue(color));

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
                List<Integer> componentNumbers = new ArrayList<>();
                for (String s : componentStrings) {
                    Integer i = Integer.valueOf(s);
                    componentNumbers.add(i);
                }
                if (componentNumbers.size() > 0) {
                    int[] pattern = new int[componentNumbers.size()];
                    repeatLength = 0.0;
                    int which = 0;
                    for (Integer ival : componentNumbers) {
                        pattern[which] = ival;
                        repeatLength += (double) ival;
                        which++;
                    }
                    texBuild.setPattern(pattern);
                } else {
                    texBuild.setPattern(new int[]{4});
                }
            } else {
                texBuild.setPattern(new int[]{4});
            }
            Bitmap patternImage = texBuild.makeImage();
            RenderController.TextureSettings texSet = new RenderController.TextureSettings();
            texSet.wrapU = true;  texSet.wrapV = true;
            MaplyTexture tex = viewC.addTexture(patternImage,new RenderController.TextureSettings(), RenderController.ThreadMode.ThreadCurrent);
            wideVectorInfo.setTexture(tex);
            wideVectorInfo.setTextureRepeatLength(repeatLength);
        }

        baseInfo.setDrawPriority(symbolizerParams.getRelativeDrawPriority() + RenderController.FeatureDrawPriorityBase);
        return new VectorTileLineStyle(null,null,baseInfo, vectorStyleSettings, viewC);

    }

    public static SLDGraphicParams graphicParamsForGraphicNode(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        SLDGraphicParams graphicParams = new SLDGraphicParams();

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }

            switch (xpp.getName()) {
                case "ExternalGraphic":
                case "Mark":
                    parseMarkOrExternalGraphic(xpp, graphicParams, symbolizerParams);
                    break;
                case "Size":
                    String size = SLDParseHelper.stringForLiteralInNode(xpp);
                    if (size != null && SLDParseHelper.isStringNumeric(size)) {
                        double d = Double.parseDouble(size);
                        graphicParams.setWidth((int) d);
                        graphicParams.setHeight((int) d);
                    }
                    break;
            }
        }

        String wellKnownName = graphicParams.getWellKnownName();
        if (wellKnownName != null) {
            Integer strokeColor = graphicParams.getStrokeColor();
            Integer fillColor = graphicParams.getFillColor();

            if (strokeColor == null)
                strokeColor = Color.DKGRAY;
            if (fillColor == null)
                fillColor = Color.WHITE;

            Integer width = graphicParams.getWidth();
            Integer height = graphicParams.getHeight();
            if (width == null || width < 8)
                width = 8;
            if (height == null || height < 8)
                height = 8;



            Bitmap bmp = SLDWellKnownMarkers.getBitmap(wellKnownName, strokeColor, fillColor, Math.min(width, height));
            graphicParams.setBitmap(bmp);
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
                            if (xpp.getEventType() == XmlPullParser.TEXT)
                                value = xpp.getText();
                            else if (xpp.getEventType() == XmlPullParser.START_TAG)
                                SLDParseHelper.skip(xpp);
                        }

                        if (name != null && value != null) {
                            if (isFill)
                                graphicParams.setFillColor(Color.parseColor(value));
                            else
                                graphicParams.setStrokeColor(Color.parseColor(value));
                        }
                    } else
                        SLDParseHelper.skip(xpp);
                }
            }
        }
        if ((wellKnownName == null) &&
                (format == null || (!format.equals("image/png") && !format.equals("image/gif"))))
            return;

        if (wellKnownName != null)
            graphicParams.setWellKnownName(wellKnownName);
        else if (href != null) {
            if (type == null || type.equals("simple")) {
                InputStream inputStream = null;
                String graphicPath;
                String basePath = symbolizerParams.getBasePath();
                if (basePath == null || basePath.equals(""))
                    graphicPath = href;
                else
                    graphicPath = symbolizerParams.getBasePath() + "/" + href;
                try {
                    inputStream = symbolizerParams.getAssetWrapper().open(graphicPath);
                } catch (IOException e) {
                    Log.e("SLDSymbolizer", "parseMarkOrExternalGraphic", e);
                }
                if (inputStream != null) {
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
