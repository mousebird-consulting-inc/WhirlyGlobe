/*
 *  SLDBinaryComparisonOperator.java
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
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import java.util.List;

import android.util.Log;

import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.sld.sldexpressions.SLDExpression;
import com.mousebird.maply.sld.sldexpressions.SLDExpressionFactory;

public class SLDBinaryComparisonOperator extends SLDOperator {

    private enum ComparisonType { EqualTo, NotEqualTo, LessThan, GreaterThan, LessThanOrEqualTo, GreaterThanOrEqualTo };

    private ComparisonType comparisonType;
    private boolean matchCase;
    private SLDExpression leftExpression;
    private SLDExpression rightExpression;

    public SLDBinaryComparisonOperator(XmlPullParser xpp) throws XmlPullParserException, IOException {

        String operatorName = xpp.getName();
        if (operatorName.equals("PropertyIsEqualTo"))
            comparisonType = ComparisonType.EqualTo;
        else if (operatorName.equals("PropertyIsNotEqualTo"))
            comparisonType = ComparisonType.NotEqualTo;
        else if (operatorName.equals("PropertyIsLessThan"))
            comparisonType = ComparisonType.LessThan;
        else if (operatorName.equals("PropertyIsGreaterThan"))
            comparisonType = ComparisonType.GreaterThan;
        else if (operatorName.equals("PropertyIsLessThanOrEqualTo"))
            comparisonType = ComparisonType.LessThanOrEqualTo;
        else if (operatorName.equals("PropertyIsGreaterThanOrEqualTo"))
            comparisonType = ComparisonType.GreaterThanOrEqualTo;

        matchCase = true;
        String matchCaseStr = xpp.getAttributeValue(null, "matchCase");
        if ((matchCaseStr != null) && (matchCaseStr.equals("false") || matchCaseStr.equals("0")))
            matchCase = false;

        ArrayList<SLDExpression> expressions = new ArrayList<SLDExpression>();
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }

            Log.i("SLDBinaryComparisonOper", xpp.getName());
            SLDExpression expression = SLDExpressionFactory.expressionForNode(xpp);
            if (expression != null) {
                expressions.add(expression);
            } else {
                SLDParseHelper.skip(xpp);
            }

        }
        if (expressions.size() == 2) {
            this.leftExpression = expressions.get(0);
            this.rightExpression = expressions.get(1);
        }
    }

    public static boolean matchesElementNamed(String elementName) {
        if (elementName.equals("PropertyIsEqualTo"))
            return true;
        else if (elementName.equals("PropertyIsNotEqualTo"))
            return true;
        else if (elementName.equals("PropertyIsLessThan"))
            return true;
        else if (elementName.equals("PropertyIsGreaterThan"))
            return true;
        else if (elementName.equals("PropertyIsLessThanOrEqualTo"))
            return true;
        else if (elementName.equals("PropertyIsGreaterThanOrEqualTo"))
            return true;
        return false;
    }

    public boolean evaluateWithAttrs(AttrDictionary attrs) {

        Object leftResult = leftExpression.evaluateWithAttrs(attrs);
        Object rightResult = rightExpression.evaluateWithAttrs(attrs);

        if ((leftResult instanceof Number) || (rightResult instanceof Number)) {

            Number leftNumber, rightNumber;
            if (leftResult instanceof Number)
                leftNumber = (Number)leftResult;
            else if ((leftResult instanceof String) && SLDParseHelper.isStringNumeric((String)leftResult))
                leftNumber = Double.valueOf((String)leftResult);
            else
                return false;

            if (rightResult instanceof Number)
                rightNumber = (Number)rightResult;
            else if ((rightResult instanceof String) && SLDParseHelper.isStringNumeric((String)rightResult))
                rightNumber = Double.valueOf((String)rightResult);
            else
                return false;

            double leftDouble = leftNumber.doubleValue();
            double rightDouble = rightNumber.doubleValue();

            if (comparisonType == ComparisonType.EqualTo)
                return (leftDouble == rightDouble);
            else if (comparisonType == ComparisonType.NotEqualTo)
                return (leftDouble != rightDouble);
            else if (comparisonType == ComparisonType.LessThan)
                return (leftDouble < rightDouble);
            else if (comparisonType == ComparisonType.GreaterThan)
                return (leftDouble > rightDouble);
            else if (comparisonType == ComparisonType.LessThanOrEqualTo)
                return (leftDouble <= rightDouble);
            else if (comparisonType == ComparisonType.GreaterThanOrEqualTo)
                return (leftDouble >= rightDouble);

        } else if ((leftResult instanceof String) && (rightResult instanceof String)) {
            String leftString = (String)leftResult;
            String rightString = (String)rightResult;

            int compareResult;
            if (matchCase)
                compareResult = leftString.compareTo(rightString);
            else
                compareResult = leftString.compareToIgnoreCase(rightString);

            if (comparisonType == ComparisonType.EqualTo)
                return (compareResult == 0);
            else if (comparisonType == ComparisonType.NotEqualTo)
                return (compareResult != 0);
            else if (comparisonType == ComparisonType.LessThan)
                return (compareResult < 0);
            else if (comparisonType == ComparisonType.GreaterThan)
                return (compareResult > 0);
            else if (comparisonType == ComparisonType.LessThanOrEqualTo)
                return (compareResult <= 0);
            else if (comparisonType == ComparisonType.GreaterThanOrEqualTo)
                return (compareResult >= 0);
        }

        return false;
    }


}
