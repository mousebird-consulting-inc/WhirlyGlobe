/*
 *  SLDIsBetweenOperator.java
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
package com.mousebird.maply.sld.sldoperators;

import com.mousebird.maply.AttrDictionary;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;

import android.util.Log;

import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.sld.sldexpressions.SLDExpression;
import com.mousebird.maply.sld.sldexpressions.SLDExpressionFactory;

public class SLDIsBetweenOperator extends SLDOperator {

    private SLDExpression subExpression;
    private SLDExpression lowerBoundaryExpression;
    private SLDExpression upperBoundaryExpression;

    public SLDIsBetweenOperator(XmlPullParser xpp) throws XmlPullParserException, IOException {
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDIsBetweenOperator", xpp.getName());

            SLDExpression expression = SLDExpressionFactory.expressionForNode(xpp);
            if (expression != null) {
                this.subExpression = expression;
            } else if (xpp.getName() == "LowerBoundary") {
                this.lowerBoundaryExpression = getBoundaryExpression(xpp);
            } else if (xpp.getName() == "UpperBoundary") {
                this.upperBoundaryExpression = getBoundaryExpression(xpp);
            } else {
                SLDParseHelper.skip(xpp);
            }
        }
    }

    private SLDExpression getBoundaryExpression(XmlPullParser xpp) throws XmlPullParserException, IOException {

        SLDExpression expression = null;

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            expression = SLDExpressionFactory.expressionForNode(xpp);
        }

        return expression;
    }

    public static boolean matchesElementNamed(String elementName) {
        if (elementName.equals("PropertyIsBetween"))
            return true;
        return false;
    }

    public boolean evaluateWithAttrs(AttrDictionary attrs) {
        return false;
    }

}
