/*
 *  SLDIsLikeOperator.java
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

import java.util.regex.Pattern;
import java.util.regex.Matcher;

import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.sld.sldexpressions.SLDExpression;
import com.mousebird.maply.sld.sldexpressions.SLDExpressionFactory;
import com.mousebird.maply.sld.sldexpressions.SLDPropertyNameExpression;
import com.mousebird.maply.sld.sldexpressions.SLDLiteralExpression;

/**
 *
 * Class corresponding to the ogc:PropertyIsLike elements
 * @see http://schemas.opengis.net/filter/1.1.0/filter.xsd for SLD v1.1.0
 * @see http://schemas.opengis.net/filter/1.0.0/filter.xsd for SLD v1.0.0
 */
public class SLDIsLikeOperator extends SLDOperator {

    private String wildCard, singleChar, escapeChar;
    private boolean matchCase;

    private Pattern pattern;
    private SLDPropertyNameExpression propertyExpression;


    public SLDIsLikeOperator(XmlPullParser xpp) throws XmlPullParserException, IOException {

        wildCard = xpp.getAttributeValue(null, "wildCard");
        singleChar = xpp.getAttributeValue(null, "singleChar");
        escapeChar = xpp.getAttributeValue(null, "escapeChar");

        matchCase = true;
        String matchCaseStr = xpp.getAttributeValue(null, "matchCase");
        if ((matchCaseStr != null) && (matchCaseStr.equals("false") || matchCaseStr.equals("0")))
            matchCase = false;

        SLDLiteralExpression literalExpression = null;

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }

            SLDExpression expression = SLDExpressionFactory.expressionForNode(xpp);
            if (expression instanceof  SLDPropertyNameExpression) {
                this.propertyExpression = (SLDPropertyNameExpression)expression;
            } else if (expression instanceof  SLDLiteralExpression) {
                literalExpression = (SLDLiteralExpression)expression;
            } else if (expression == null) {
                SLDParseHelper.skip(xpp);
            }

        }

        if (literalExpression != null && wildCard != null && wildCard.length() == 1 && singleChar != null && singleChar.length() == 1 && escapeChar != null && escapeChar.length() == 1) {

            char oldWildCardChar = wildCard.charAt(0);
            char oldSingleChar = singleChar.charAt(0);
            char oldEscapeChar = escapeChar.charAt(0);

            String newWildCardChar = ".*";
            String newSingleChar = ".?";

            Object literalValueObj = literalExpression.getLiteral();
            if (literalValueObj instanceof String) {
                String likeStr = (String)literalValueObj;

                String newLikeStr = new String();
                int oldLen = likeStr.length();

                char bufc;

                for (int i=0; i<oldLen; i++) {
                    bufc = likeStr.charAt(i);
                    if (bufc == oldSingleChar)
                        newLikeStr = newLikeStr.concat(newSingleChar);
                    else if (bufc == oldWildCardChar)
                        newLikeStr = newLikeStr.concat(newWildCardChar);
                    else if (bufc == oldEscapeChar) {
                        if (i == oldLen-1)
                            continue;
                        newLikeStr = newLikeStr.concat(Pattern.quote(likeStr.substring(i+1,i+2)));
                        i++;
                    } else {
                        newLikeStr = newLikeStr.concat(Pattern.quote(likeStr.substring(i,i+1)));
                    }
                }

                if (matchCase)
                    pattern = Pattern.compile(newLikeStr);
                else
                    pattern = Pattern.compile(newLikeStr, Pattern.CASE_INSENSITIVE);
            }
        }
    }

    public static boolean matchesElementNamed(String elementName) {
        if (elementName.equals("PropertyIsLike"))
            return true;
        return false;
    }


    public boolean evaluateWithAttrs(AttrDictionary attrs) {
        Object propertyValueObj = propertyExpression.evaluateWithAttrs(attrs);
        if (!(propertyValueObj instanceof String))
            return false;

        String propertyValue = (String)propertyValueObj;

        Matcher m = pattern.matcher(propertyValue);
        return m.matches();
    }

}
