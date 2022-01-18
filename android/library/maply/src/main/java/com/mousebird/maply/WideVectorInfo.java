/*  WideVectorInfo.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */
package com.mousebird.maply;

import android.graphics.Color;

import androidx.annotation.ColorInt;

/**
 * The Wide Vector Info class holds visual information related to groups of vectors.
 * For efficiency's sake we put visual info in this class, rather than on
 * attributes in the vectors themselves.  There may be attributes that can
 * override these, however.
 */
@SuppressWarnings("unused")
public class WideVectorInfo extends BaseInfo
{
    /**
     * Default draw priority for vector features.
     */
    public static int WideVectorPriorityDefault = 51000;

    public WideVectorInfo() {
        initialise();
        setDrawPriority(WideVectorPriorityDefault);
    }

    public void finalize() {
        dispose();
    }

    /**
     * Set the color used by the geometry.
     * @param color Color in Android format, including alpha.
     */
    public void setColor(int color) {
        setColorInt(Color.red(color),Color.green(color),Color.blue(color),Color.alpha(color));
    }
    public native @ColorInt int getColor();

    /**
     * Set the color used by the geometry.  Color values range from 0 to 1.0.
     * You must specify all four values.  Alpha controls transparency.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    public native void setColor(float r,float g,float b,float a);

    /**
     * Set the color used by the geometry.  Color values range from 0 to 1.0.
     * You must specify all four values.  Alpha controls transparency.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    public native void setColorInt(int r,int g,int b,int a);

    /**
     * This is the line width for vector features.  By default this is 1.0.
     */
    public native void setLineWidth(float lineWidth);
    public native float getLineWidth();

    /**
     * This is the repeat size for a texture applied along the widened line.
     * The value is in pixels for screen widths.
     */
    public native void setTextureRepeatLength(double repeatLen);
    public native double getTextureRepeatLength();

    /**
     * Number of pixels to use in blending the edges of the wide vectors.
     */
    public native void setEdgeFalloff(double falloff);
    public native double getEdgeFalloff();

    public enum JoinType {MiterJoin,RoundJoin,BevelJoin};

    /**
     * When lines meet in a join there are several options for representing them.
     * These include MiterJoin, which is a simple miter join and BevelJoin which is a more complicated bevel.
     * See http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty for how these look.
     */
    public void setJoinType(JoinType joinType) { setJoinTypeNative(joinType.ordinal()); }
    public JoinType getJoinType() { return JoinType.values()[getJoinTypeNative()]; }

    native void setJoinTypeNative(int joinType);
    native int getJoinTypeNative();

    /**
     * When using miter joins you can trigger them at a certain threshold.
     */
    public native void setMitreLimit(double mitreLimit);
    public native double getMitreLimit();

    /**
     * This the texture to be applied to the widened vector.
     */
    public void setTexture(MaplyTexture tex) {
        setTexID(tex.texID);
    }

    /**
     * Set the texture to be applied by ID.
     */
    public native void setTexID(long texID);
    public native long getTexID();

    /**
     * Whether the items are selectable
     */
    public native void setSelectable(boolean enable);
    public native boolean getSelectable();

    /**
     * Offset to the left or right of the center-line
     */
    public native void setOffset(double pixels);
    public native double getOffset();

    /**
     * Close any un-closed areal features when drawing lines for them
     */
    public native void setCloseAreals(boolean close);
    public native boolean getCloseAreals();

    static {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
