package com.mousebirdconsulting.autotester.Utils;

import android.content.Context;
import android.util.DisplayMetrics;
import android.util.TypedValue;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 10/06/16.
 */
public abstract class GraphicUtils {

    /**
     * Converts a value in dip (Dot Independent Pixels) to pixels
     * @param context the context in which the value is measured
     * @param dipValue the value to convert
     * @return the value converted to pixels
     */
    public static float dipToPixels(Context context, float dipValue) {
        return GraphicUtils.toPixels(context, TypedValue.COMPLEX_UNIT_DIP, dipValue);
    }

    /**
     * Converts a value in sp (Scale Independent Pixels) to pixels
     * @param context the context in which the value is measured
     * @param spValue the value to convert
     * @return the value converted to pixels
     */
    public static float spToPixels(Context context, float spValue) {
        return GraphicUtils.toPixels(context, TypedValue.COMPLEX_UNIT_SP, spValue);
    }


    /**
     * Converts a value expressed in an arbitrary unit to pixels
     * @param context the context in which the value is measured
     * @param from the current unit of the value (see #android.util.TypedValue)
     * @param value the value to convert
     * @return the value converted to pixels
     */
    public static float toPixels(Context context, int from, float value) {
        DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        return TypedValue.applyDimension(from, value, metrics);
    }


    /**
     * Converts a value in pixels to dips (Dot Independent Pixels)
     * @param context the context in which the value is measured
     * @param pixelValue the value to convert
     * @return the value converted to pixels
     */
    public static float pixelToDips(Context context, float pixelValue) {
        DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_PX, pixelValue, metrics);
    }



}
