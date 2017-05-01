/*
 *  SLDBinaryOperatorExpression.java
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
package com.mousebird.maply.sld.sldexpressions;


import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import android.util.Log;

public class SLDBinaryOperatorExpression extends SLDExpression {

    private enum ExpressionType { ExpressionTypeAdd, ExpressionTypeSub, ExpressionTypeMul, ExpressionTypeDiv};

    private ExpressionType expressionType;
    private SLDExpression leftExpression;
    private SLDExpression rightExpression;

    public SLDBinaryOperatorExpression(XmlPullParser xpp) throws XmlPullParserException, IOException {

        String expressionName = xpp.getName();
        if (expressionName.equals("Add"))
            expressionType = ExpressionType.ExpressionTypeAdd;
        else if (expressionName.equals("Sub"))
            expressionType = ExpressionType.ExpressionTypeSub;
        else if (expressionName.equals("Mul"))
            expressionType = ExpressionType.ExpressionTypeMul;
        else if (expressionName.equals("Div"))
            expressionType = ExpressionType.ExpressionTypeDiv;

        ArrayList<SLDExpression> expressions = new ArrayList<SLDExpression>();
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDBinaryOperatorExpr", xpp.getName());

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


    public Object evaluateWithAttrs(AttrDictionary attrs) {

        Object leftObject = leftExpression.evaluateWithAttrs(attrs);
        Object rightObject = rightExpression.evaluateWithAttrs(attrs);

        if (!(leftObject instanceof  Number) || !(rightObject instanceof  Number))
            return null;
        Number leftNumber = (Number)leftObject;
        Number rightNumber = (Number)rightObject;

        if (expressionType == ExpressionType.ExpressionTypeAdd)
            return Double.valueOf(leftNumber.doubleValue() + rightNumber.doubleValue());
        else if (expressionType == ExpressionType.ExpressionTypeSub)
            return Double.valueOf(leftNumber.doubleValue() - rightNumber.doubleValue());
        else if (expressionType == ExpressionType.ExpressionTypeMul)
            return Double.valueOf(leftNumber.doubleValue() * rightNumber.doubleValue());
        else if (expressionType == ExpressionType.ExpressionTypeDiv) {
            try {
                double d = leftNumber.doubleValue() / rightNumber.doubleValue();
                return Double.valueOf(d);
            } catch (Exception e) {
                return null;
            }
        }
        return null;
    }

    public static boolean matchesElementNamed(String elementName) {
        if (elementName.equals("Add"))
            return true;
        else if (elementName.equals("Sub"))
            return true;
        else if (elementName.equals("Mul"))
            return true;
        else if (elementName.equals("Div"))
            return true;
        return false;
    }
}
