/*
 *  SLDFeatureTypeStyle.java
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

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.sld.sldstyleset.SLDRule;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizerParams;


import android.util.Log;

public class SLDFeatureTypeStyle {

    private List<SLDRule> rules = new ArrayList<SLDRule>();;

    public SLDFeatureTypeStyle(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDFeatureTypeStyle", xpp.getName());
            if (xpp.getName().equals("Rule")) {
                rules.add(new SLDRule(xpp, symbolizerParams));
            } else {
                SLDParseHelper.skip(xpp);
            }
        }
    }

    public List<VectorTileStyle> getStyles() {
        List<VectorTileStyle> styles = new ArrayList<VectorTileStyle>();
        for (SLDRule rule : rules) {
            styles.addAll(rule.getStyles());
        }
        return styles;
    }

    public List<VectorTileStyle> stylesForFeatureAttributes(AttrDictionary attrs) {
        List<VectorTileStyle> styles = new ArrayList<VectorTileStyle>();
        for (SLDRule rule : rules) {
            styles.addAll(rule.stylesForFeatureAttributes(attrs));
        }
        return styles;
    }
}
