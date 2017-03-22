package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.Color;

/** The Maply Linear Texture Builder is used to construct linear textures
 for use on widened vectors.
 <br>
 Linear textures of this type are used to represent dotted and dashed lines.
 These may come from Mapnik configuration files or you can just make them up yourself.
 <br>
 After creating an image with this object, you'll want to pass it as a parameter to the
 widened vector add method.
 */
public class LinearTextureBuilder
{
    public LinearTextureBuilder()
    {
    }

    int[] elements = null;

    /** Set the pattern of dots and empty spaces.
     * <br>
     * This is an array of NSNumbers (treated as integers) that specify how
     * big an element in the given pattern is.  The first element is on, the next off and so forth.
     */
    public void setPattern(int[] inElements)
    {
        elements = inElements;
    }

    /** Build the image from the size and elements specified.
     * <br>
     * If you've set a reasonable size and added a pattern,
     * this will render the pattern into the image and return it.
     * If the size of the image differs from the size of the elements,
     * they will be scaled to the image.
     */
    public Bitmap makeImage()
    {
        if (elements == null)
            return null;

        int eleSum = 0;
        for (int ele : elements)
            eleSum += ele;
        int size = eleSum;

        if (size == 0)
            return null;

        int[] rawColors = new int[size];
        for (int ii=0;ii<size;ii++)
            rawColors[ii] = Color.BLACK;

        int width = 1;

        // Work our way through the elements
        int curY = 0;
        boolean onOrOff = true;
        for (int ii=0;ii<elements.length;ii++)
        {
            int eleLen = elements[ii];
            if (onOrOff) {
                for (int jj = 0; jj < eleLen; jj++) {
                    rawColors[curY] = Color.BLACK;
                    curY++;
                }
            } else
                curY += eleLen;
            onOrOff = !onOrOff;
        }

        return Bitmap.createBitmap(rawColors,width,size,Bitmap.Config.ARGB_8888);
    }
}
