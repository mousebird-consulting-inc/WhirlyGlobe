/*
 *  SLDTextSymbolizer.java
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


import android.graphics.Color;
import android.graphics.Typeface;
import android.util.Log;

import com.mousebird.maply.LabelInfo;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.VectorTileTextStyle;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.Point2d;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

public class SLDTextSymbolizer extends SLDSymbolizer {

    private VectorTileTextStyle vectorTileTextStyle;

    public SLDTextSymbolizer(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {

        MaplyBaseController viewC = symbolizerParams.getBaseController();
        VectorStyleSettings vectorStyleSettings = symbolizerParams.getVectorStyleSettings();

        LabelInfo labelInfo = new LabelInfo();
        Point2d offset = new Point2d(0.0, 0.0);
        String textField = null;

        if (symbolizerParams.getMinScaleDenominator() != null) {
            if (symbolizerParams.getMaxScaleDenominator() == null)
                labelInfo.setMaxVis(Float.MAX_VALUE);
            labelInfo.setMinVis((float) viewC.heightForMapScale(symbolizerParams.getMinScaleDenominator().floatValue()));
        }
        if (symbolizerParams.getMaxScaleDenominator() != null) {
            if (symbolizerParams.getMinScaleDenominator() == null)
                labelInfo.setMinVis(0.0f);
            labelInfo.setMaxVis((float) viewC.heightForMapScale(symbolizerParams.getMaxScaleDenominator().floatValue()));
        }
        
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDTextSymbolizer", xpp.getName());
            if (xpp.getName().equals("Label")) {

                textField = getLabel(xpp);

            } else if (xpp.getName().equals("Font")) {

                getFontParams(xpp, labelInfo);

            } else if (xpp.getName().equals("LabelPlacement")) {

                getLabelPlacementParams(xpp, labelInfo, offset);

            } else if (xpp.getName().equals("Halo")) {

                getHaloParams(xpp, labelInfo);

            } else if (xpp.getName().equals("Fill")) {

                getFillParams(xpp, labelInfo);

            } else if (xpp.getName().equals("VendorOption")) {

                getVendorOptions(xpp);

            } else {
                SLDParseHelper.skip(xpp);
            }
        }

        if (offset.getX() == 0.0 && offset.getY() == 0.0)
            offset = null;

        vectorTileTextStyle = new VectorTileTextStyle(labelInfo, offset, textField, vectorStyleSettings, viewC);
    }

    private String getLabel(XmlPullParser xpp) throws XmlPullParserException, IOException {
        String label = null;
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("PropertyName")) {
                label = "[" + SLDParseHelper.nodeTextValue(xpp) + "]";
            } else {
                label = SLDParseHelper.stringForLiteralInNode(xpp);
            }
        }
        return label;
    }

    private void getFontParams(XmlPullParser xpp, LabelInfo labelInfo) throws XmlPullParserException, IOException {
        boolean italic = false;
        boolean bold = false;

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

                if (name.equals("font-family")) {

                    // TODO: Is this of any use on Android?

                } else if (name.equals("font-style")) {

                    if (value.equals("italic") || value.equals("oblique"))
                        italic = true;

                } else if (name.equals("font-weight")) {

                    if (!(value.equals("normal") || value.equals("lighter")))
                        bold = true;

                } else if (name.equals("font-size")) {
                    String fontSize = value.trim();
                    if (fontSize.equals("xx-small"))
                        labelInfo.setFontSize(8.0f);
                    else if (fontSize.equals("x-small"))
                        labelInfo.setFontSize(10.0f);
                    else if (fontSize.equals("small"))
                        labelInfo.setFontSize(11.0f);
                    else if (fontSize.equals("medium"))
                        labelInfo.setFontSize(12.0f);
                    else if (fontSize.equals("large"))
                        labelInfo.setFontSize(18.0f);
                    else if (fontSize.equals("x-large"))
                        labelInfo.setFontSize(24.0f);
                    else if (fontSize.equals("xx-large"))
                        labelInfo.setFontSize(36.0f);
                    else {
                        String numericalFontSize = fontSize;
                        if (fontSize.endsWith("px"))
                            numericalFontSize = fontSize.replaceAll("px", "");
                        else if (fontSize.endsWith("em"))
                            numericalFontSize = fontSize.replaceAll("em", "");
                        else if (fontSize.endsWith("%"))
                            numericalFontSize = fontSize.replaceAll("%", "");

                        if (SLDParseHelper.isStringNumeric(numericalFontSize)) {
                            float size = Float.valueOf(numericalFontSize).floatValue();
                            if (fontSize.endsWith("px"))
                                labelInfo.setFontSize(size);
                            else if (fontSize.endsWith("em"))
                                labelInfo.setFontSize(size * 12.0f);
                            else if (fontSize.endsWith("%"))
                                labelInfo.setFontSize(size * 100.0f * 12.0f);
                            else
                                labelInfo.setFontSize(size);
                        }
                    }
                }

            } else {
                SLDParseHelper.skip(xpp);
            }
        }
        labelInfo.setTypeface(Typeface.create(Typeface.DEFAULT, bold ? (italic ? Typeface.BOLD_ITALIC : Typeface.BOLD) : (italic ? Typeface.ITALIC : Typeface.NORMAL)));

    }

    private void getLabelPlacementParams(XmlPullParser xpp, LabelInfo labelInfo, Point2d offset) throws XmlPullParserException, IOException {
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("PointPlacement")) {

                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.START_TAG)
                        continue;
                    if (xpp.getName().equals("AnchorPoint")) {

                        float ax = 0.0f;
                        float ay = 0.5f;

                        while (xpp.next() != XmlPullParser.END_TAG) {
                            if (xpp.getEventType() != XmlPullParser.START_TAG)
                                continue;
                            if (xpp.getName().equals("AnchorPointX")) {
                                String sAx = SLDParseHelper.stringForLiteralInNode(xpp);
                                if (SLDParseHelper.isStringNumeric(sAx))
                                    ax = Float.valueOf(sAx).floatValue();

                            } else if (xpp.getName().equals("AnchorPointY")) {
                                String sAy = SLDParseHelper.stringForLiteralInNode(xpp);
                                if (SLDParseHelper.isStringNumeric(sAy))
                                    ay = Float.valueOf(sAy).floatValue();
                            } else
                                SLDParseHelper.skip(xpp);
                        }

                        if (ax <= 0.33f)
                            labelInfo.setLayoutPlacement(LabelInfo.LayoutLeft);
                        else if (ax > 0.67)
                            labelInfo.setLayoutPlacement(LabelInfo.LayoutRight);
                        else {
                            if (ay <= 0.33f)
                                labelInfo.setLayoutPlacement(LabelInfo.LayoutBelow);
                            else if (ay > 0.67f)
                                labelInfo.setLayoutPlacement(LabelInfo.LayoutAbove);
                            else
                                labelInfo.setLayoutPlacement(LabelInfo.LayoutCenter);
                        }

                    } else if (xpp.getName().equals("Displacement")) {

                        float dx = 0.0f;
                        float dy = 0.0f;

                        while (xpp.next() != XmlPullParser.END_TAG) {
                            if (xpp.getEventType() != XmlPullParser.START_TAG)
                                continue;
                            if (xpp.getName().equals("DisplacementX")) {
                                String sDx = SLDParseHelper.stringForLiteralInNode(xpp);
                                if (SLDParseHelper.isStringNumeric(sDx))
                                    dx = Float.valueOf(sDx).floatValue();
                            } else if (xpp.getName().equals("DisplacementY")) {
                                String sDy = SLDParseHelper.stringForLiteralInNode(xpp);
                                if (SLDParseHelper.isStringNumeric(sDy))
                                    dy = Float.valueOf(sDy).floatValue();
                            } else
                                SLDParseHelper.skip(xpp);
                        }

                        offset.setValue(dx, dy);

                    } else
                        SLDParseHelper.skip(xpp);
                }


            } else if (xpp.getName().equals("LinePlacement")) {
                float dy = 0.0f;
                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.START_TAG)
                        continue;
                    if (xpp.getName().equals("PerpendicularOffset")) {
                        String perpendicularOffset = SLDParseHelper.stringForLiteralInNode(xpp);
                        if (SLDParseHelper.isStringNumeric(perpendicularOffset))
                            dy = Float.valueOf(perpendicularOffset).floatValue();
                    } else
                        SLDParseHelper.skip(xpp);
                }

                offset.setValue(offset.getX(), dy);

            } else {
                SLDParseHelper.skip(xpp);
            }

        }
    }



    private void getHaloParams(XmlPullParser xpp, LabelInfo labelInfo) throws XmlPullParserException, IOException {

        while (xpp.next() != XmlPullParser.END_TAG) {

            if (xpp.getEventType() != XmlPullParser.START_TAG)
                continue;

            if (xpp.getName().equals("Radius")) {

                String sRadius = SLDParseHelper.stringForLiteralInNode(xpp);
                if (SLDParseHelper.isStringNumeric(sRadius))
                    labelInfo.setOutlineSize(Float.valueOf(sRadius).floatValue());

            } else if (xpp.getName().equals("Fill")) {

                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.START_TAG)
                        continue;
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

                        if (name.equals("fill")) {
                            try {
                                int color = Integer.valueOf(Color.parseColor(value)).intValue();
                                labelInfo.setOutlineColor(color);

                            } finally {
                            }
                        }
                    } else
                        SLDParseHelper.skip(xpp);
                }
            } else
                SLDParseHelper.skip(xpp);
        }
    }

    private void getFillParams(XmlPullParser xpp, LabelInfo labelInfo) throws XmlPullParserException, IOException {

        Integer fillColor = null;
        Float fillOpacity = null;

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG)
                continue;

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

                if (name.equals("fill")) {
                    try {
                        fillColor = Integer.valueOf(Color.parseColor(value));
                    } finally {
                    }
                } else if (name.equals("fill-opacity")) {
                    try {
                        fillOpacity = Float.valueOf(value);
                    } finally {
                    }
                }

            } else
                SLDParseHelper.skip(xpp);
        }

        if (fillColor != null) {
            int color = fillColor.intValue();
            if (fillOpacity != null)
                color = Color.argb(Math.round(fillOpacity.floatValue()*255.f), Color.red(color), Color.green(color), Color.blue(color));

            labelInfo.setTextColor(color);
        }

    }

    private void getVendorOptions(XmlPullParser xpp) throws XmlPullParserException, IOException {
        //TODO: implement this
        SLDParseHelper.skip(xpp);
    }


    public VectorTileStyle[] getStyles() {
        if (vectorTileTextStyle != null)
            return new VectorTileStyle[]{vectorTileTextStyle};
        return new VectorTileStyle[]{};
    }


    public static boolean matchesSymbolizerNamed(String symbolizerName) {
        if (symbolizerName.equals("TextSymbolizer"))
            return true;
        return false;
    }
}
