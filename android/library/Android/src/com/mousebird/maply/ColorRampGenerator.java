package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import java.util.ArrayList;

/**
 * The color ramp generator will take a set of color values
 * and generate a linear ramp of those colors in an output
 * image.  You typically feed the color ramp image into a shader.
 */
public class ColorRampGenerator
{
    ArrayList<Integer> colors = new ArrayList<Integer>();

    /**
     * If set we'll stretch the colors out to the whole image
     * On by default
     */
    public boolean stretch = true;

    /**
     * Add a color as a hex value.
     */
    public void addHexColor(int hexColor)
    {
        int red = (((hexColor) >> 16) & 0xFF);
        int green = (((hexColor) >> 8) & 0xFF);
        int blue = (((hexColor) >> 0) & 0xFF);

        int color = Color.rgb(red,green,blue);
        colors.add(color);
    }

    /**
     * This color has an alpha too.
     */
    public void addHexColorWithAlpha(int hexColor)
    {
        int alpha = (((hexColor) >> 24) & 0xFF);
        int red = (((hexColor) >> 16) & 0xFF);
        int green = (((hexColor) >> 8) & 0xFF);
        int blue = (((hexColor) >> 0) & 0xFF);

        int color = Color.argb(alpha,red,green,blue);
        colors.add(color);
    }

    /**
     * Add a single color to the end.
     */
    public void addColor(int color)
    {
        colors.add(color);
    }

    // Interpolate a color between values, potentially
    int interpColor(float where)
    {
        if (colors.size() == 0)
            return 0;

        if (colors.size() == 1)
            return colors.get(0);

        float pos = where*colors.size();
        int aIdx = (int)Math.floor(pos);
        int bIdx = (int)Math.ceil(pos);
        if (bIdx >= colors.size())
            bIdx = colors.size()-1;
        if (aIdx >= colors.size())
            aIdx = colors.size()-1;
        float t = pos-aIdx;

        int a = colors.get(aIdx);
        int b = colors.get(bIdx);
        int[] aVals = new int[4];
        int[] bVals = new int[4];
        aVals[0] = Color.red(a);  aVals[1] = Color.green(a);  aVals[2] = Color.blue(a);  aVals[3] = Color.alpha(4);
        bVals[0] = Color.red(b);  bVals[1] = Color.green(b);  bVals[2] = Color.blue(b);  bVals[3] = Color.alpha(b);

        int[] iVals = new int[4];
        for (int ii=0;ii<4;ii++)
            iVals[ii] = (int)((bVals[ii]-aVals[ii])*t + aVals[ii]);

        return Color.argb(iVals[3],iVals[0],iVals[1],iVals[2]);
    }

    /**
     * Generate the image with the color ramp in it.
     */
    public Bitmap makeImage(int width,int height)
    {
        Bitmap bitmap = Bitmap.createBitmap(width,height,Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        Paint p = new Paint();

        // Work our way through the pixels by height
        for (int xx=0;xx<width;xx++)
        {
            int color = 0;
            if (stretch)
                color = interpColor(xx/(float)(width-1));
            else {
                if (xx >= colors.size())
                    color = 0;
                else
                    color = colors.get(xx);
            }
//            p.setColor(color);
            p.setColor(Color.RED);
//            canvas.drawRect((float)xx,0.f,(float)xx+1,(float)height,p);
            canvas.drawRect(0,0,width,height,p);
        }

        return bitmap;
    }
}
