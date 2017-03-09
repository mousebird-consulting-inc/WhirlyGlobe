package com.mousebird.maply;

import android.graphics.Color;
import android.graphics.Point;

/**
 * The Wide Vector Info class holds visual information related to groups of vectors.
 * For efficiency's sake we put visual info in this class, rather than on
 * attributes in the vectors themselves.  There may be attributes that can
 * override these, however.
 *
 */
public class WideVectorInfo extends BaseInfo
{
    /**
     * Default draw priority for vector features.
     */
    public static int WideVectorPriorityDefault = 51000;

    public WideVectorInfo()
    {
        setColor(Color.WHITE);
        setLineWidth(2.0f);
        setJoinType(JoinType.MiterJoin);
        setMitreLimit(2.0f);
        setTexId(0);
        setEdgeFalloff(1.0f);
    }

    public void finalize()
    {
        dispose();
    }

    enum JoinType {MiterJoin,BevelJoin};

    /**
     * When lines meet in a join there are several options for representing them.
     * These include MiterJoin, which is a simple miter join and BevelJoin which is a more complicated bevel.
     * See http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty for how these look.
     */
    public void setJoinType(JoinType joinType)
    {
        setJoinTypeNative(joinType.ordinal());
    }

    native void setJoinTypeNative(int joinType);

    /**
     * When using miter joins you can trigger them at a certain threshold.
     */
    public native void setMitreLimit(double mitreLimit);

    /**
     * This the texture to be applied to the widened vector.
     */
    public void setTexture(MaplyTexture tex)
    {
        setTexId(tex.texID);
    }

    native void setTexId(long texID);

    /**
     * This is the repeat size for a texture applied along the widened line.
     * The value is in pixels for screen widths.
     */
    public native void setTextureRepeatLength(double repeatLen);

    /**
     * Number of pixels to use in blending the edges of the wide vectors.
     */
    public native void setEdgeFalloff(double falloff);

    /**
     * Set the color used by the geometry.
     * @param color Color in Android format, including alpha.
     */
    public void setColor(int color)
    {
        setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

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
     * This is the line width for vector features.  By default this is 1.0.
     */
    public native void setLineWidth(float lineWidth);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
