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
import java.util.List;
import java.util.ArrayList;

import com.mousebird.maply.sld.sldstyleset.SLDFilter;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizer;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizerFactory;

import android.util.Log;

public class SLDRule {

    private List<SLDFilter> filters;
    private List<SLDFilter> elseFilters;
    private List<SLDSymbolizer> symbolizers;


    public SLDRule(XmlPullParser xpp) throws XmlPullParserException, IOException {
        this.filters = new ArrayList<SLDFilter>();
        this.elseFilters = new ArrayList<SLDFilter>();
        this.symbolizers = new ArrayList<SLDSymbolizer>();
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDRule", xpp.getName());
            if (xpp.getName().equals("Filter")) {
                this.filters.add(new SLDFilter(xpp));
            } else if (xpp.getName().equals("ElseFilter")) {
                this.elseFilters.add(new SLDFilter(xpp));
            } else if (xpp.getName().equals("MinScaleDenominator")) {
            } else if (xpp.getName().equals("MaxScaleDenominator")) {
            } else {
                SLDSymbolizer symbolizer = SLDSymbolizerFactory.symbolizerForNode(xpp);
                if (symbolizer != null) {
                    this.symbolizers.add(symbolizer);
                } else {
                    SLDParseHelper.skip(xpp);
                }
            }
        }
    }
}
