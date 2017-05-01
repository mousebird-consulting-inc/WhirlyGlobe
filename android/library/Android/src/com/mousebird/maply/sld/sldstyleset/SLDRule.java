/*
 *  SLDRule.java
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
package com.mousebird.maply.sld.sldstyleset;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.sld.sldstyleset.SLDFilter;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizer;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizerFactory;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizerParams;

import android.util.Log;

public class SLDRule {

    private List<SLDFilter> filters = new ArrayList<SLDFilter>();
    private List<SLDFilter> elseFilters = new ArrayList<SLDFilter>();

    public List<VectorTileStyle> getStyles() {
        return styles;
    }

    private List<VectorTileStyle> styles = new ArrayList<VectorTileStyle>();

    public SLDRule(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {

        symbolizerParams.resetRuleParams();

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDRule", xpp.getName());
            if (xpp.getName().equals("Filter")) {
                filters.add(new SLDFilter(xpp));
            } else if (xpp.getName().equals("ElseFilter")) {
                elseFilters.add(new SLDFilter(xpp));
            } else if (xpp.getName().equals("MinScaleDenominator")) {
                String value = SLDParseHelper.nodeTextValue(xpp);
                if (SLDParseHelper.isStringNumeric(value))
                    symbolizerParams.setMinScaleDenominator(Double.valueOf(value));
            } else if (xpp.getName().equals("MaxScaleDenominator")) {
                String value = SLDParseHelper.nodeTextValue(xpp);
                if (SLDParseHelper.isStringNumeric(value))
                    symbolizerParams.setMaxScaleDenominator(Double.valueOf(value));
            } else {
                SLDSymbolizer symbolizer = SLDSymbolizerFactory.symbolizerForNode(xpp, symbolizerParams);
                if (symbolizer != null) {
                    styles.addAll(Arrays.asList(symbolizer.getStyles()));
                    symbolizerParams.incrementRelativeDrawPriority();
                } else {
                    SLDParseHelper.skip(xpp);
                }
            }
        }
    }


    public List<VectorTileStyle> stylesForFeatureAttributes(AttrDictionary attrs) {
        boolean matched = false;
        if (filters.size() == 0 && elseFilters.size() == 0)
            matched = true;
        for (SLDFilter filter : filters) {
            if (filter.getOperator().evaluateWithAttrs(attrs)) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            for (SLDFilter filter: elseFilters) {
                if (filter.getOperator().evaluateWithAttrs(attrs)) {
                    matched = true;
                    break;
                }
            }
        }
        if (matched)
            return styles;
        return new ArrayList<VectorTileStyle>();
    }

}
