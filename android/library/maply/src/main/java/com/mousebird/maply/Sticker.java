package com.mousebird.maply;

import android.graphics.Bitmap;

import java.util.ArrayList;

/**
 * The Sticker class is used to represent a rectangular area we stick on top
 * of the globe or map.
 *
 * @author sjg
 */
public class Sticker
{
    /**
     * Construct with empty values.
     */
    public Sticker() { initialise(); }

    public void finalize()
    {
        dispose();
    }

    /**
     * Set the lower left corner of the sticker.
     * This is in the given coordinate system.
     */
    public native void setLowerLeft(Point2d pt);

    /**
     * Set the upper right corner of the sticker.
     * This is in the given coordinate system.
     */
    public native void setUpperRight(Point2d pt);

    /**
     * Set the rotation in radians.  0 by default.
     */
    public native void setRotation(double rot);

    /**
     * Set the coordinate system.  By default this is just geographic.
     */
    public native void setCoordSys(CoordSystem coordSys);

    /**
     * Set the sampling in X and Y for the sticker.
     */
    public native void setSampling(int sampleX,int sampleY);

    /**
     * If not doing static sampling, break it down until its no farther than this from the globe.
     * The min and max values control the sampling.
     * @param eps Epsilon value
     * @param minSampleX Minimum samples in X
     * @param minSampleY Minimum samples in Y
     * @param maxSampleX Maximum samples in X
     * @param maxSampleY Maximum samples in Y
     */
    public native void setEpsilon(double eps,int minSampleX,int minSampleY,int maxSampleX,int maxSampleY);
    
    /**
     * Images to display on the Sticker.  This may include an image or a blending of
     * multiple, depending on the shader.
     */
    public void setTextures(ArrayList<MaplyTexture> textures)
    {
        long[] texIDs = new long[textures.size()];
        int which = 0;
        for (MaplyTexture tex : textures)
        {
            texIDs[which] = tex.texID;
            which++;
        }

        setTextureIDs(texIDs);
    }

    native void setTextureIDs(long[] texIDs);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
