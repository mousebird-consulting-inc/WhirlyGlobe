/*
 *  SLDParseHelper.java
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
import java.util.AbstractMap;
import java.util.HashMap;
import java.util.regex.Pattern;

public class SLDParseHelper {

    public static void skip(XmlPullParser parser) throws XmlPullParserException, IOException {
//        if (parser.getEventType() != XmlPullParser.START_TAG) {
//            throw new IllegalStateException();
//        }
        int depth = 1;
        while (depth != 0) {
            switch (parser.next()) {
                case XmlPullParser.END_TAG:
                    depth--;
                    break;
                case XmlPullParser.START_TAG:
                    depth++;
                    break;
            }
        }
    }

    public static String nodeTextValue(XmlPullParser parser) throws XmlPullParserException, IOException {
        String textValue = null;
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.TEXT) {
                continue;
            }
            textValue = parser.getText();
        }
        return textValue;
    }

    public static String stringForLiteralInNode(XmlPullParser parser) throws XmlPullParserException, IOException {
        String textValue = null;

        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() == XmlPullParser.START_TAG) {
                if (parser.getName().equals("Literal")) {
                    while (parser.next() != XmlPullParser.END_TAG) {
                        if (parser.getEventType() == XmlPullParser.TEXT)
                            textValue = parser.getText();
                        else if (parser.getEventType() == XmlPullParser.START_TAG)
                            skip(parser);
                    }
                } else
                    skip(parser);
            } else if (parser.getEventType() == XmlPullParser.TEXT)
                textValue = parser.getText();

        }
        return textValue;

    }

    public static boolean isStringNumeric(String s) {

        // https://developer.android.com/reference/java/lang/Double.html#valueOf(java.lang.String)

        final String Digits     = "(\\p{Digit}+)";
        final String HexDigits  = "(\\p{XDigit}+)";
        // an exponent is 'e' or 'E' followed by an optionally
        // signed decimal integer.
        final String Exp        = "[eE][+-]?"+Digits;
        final String fpRegex    =
                ("[\\x00-\\x20]*"+  // Optional leading "whitespace"
                        "[+-]?(" + // Optional sign character
                        "NaN|" +           // "NaN" string
                        "Infinity|" +      // "Infinity" string

                        // A decimal floating-point string representing a finite positive
                        // number without a leading sign has at most five basic pieces:
                        // Digits . Digits ExponentPart FloatTypeSuffix
                        //
                        // Since this method allows integer-only strings as input
                        // in addition to strings of floating-point literals, the
                        // two sub-patterns below are simplifications of the grammar
                        // productions from section 3.10.2 of
                        // The Java™ Language Specification.

                        // Digits ._opt Digits_opt ExponentPart_opt FloatTypeSuffix_opt
                        "((("+Digits+"(\\.)?("+Digits+"?)("+Exp+")?)|"+

                        // . Digits ExponentPart_opt FloatTypeSuffix_opt
                        "(\\.("+Digits+")("+Exp+")?)|"+

                        // Hexadecimal strings
                        "((" +
                        // 0[xX] HexDigits ._opt BinaryExponent FloatTypeSuffix_opt
                        "(0[xX]" + HexDigits + "(\\.)?)|" +

                        // 0[xX] HexDigits_opt . HexDigits BinaryExponent FloatTypeSuffix_opt
                        "(0[xX]" + HexDigits + "?(\\.)" + HexDigits + ")" +

                        ")[pP][+-]?" + Digits + "))" +
                        "[fFdD]?))" +
                        "[\\x00-\\x20]*");// Optional trailing "whitespace"

        return (Pattern.matches(fpRegex, s));
    }

}
