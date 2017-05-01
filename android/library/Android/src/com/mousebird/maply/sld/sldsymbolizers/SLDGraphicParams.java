package com.mousebird.maply.sld.sldsymbolizers;

import android.graphics.Bitmap;
import android.graphics.Color;

public class SLDGraphicParams {

    int width;
    int height;
    Bitmap bitmap;
    Integer fillColor;
    Integer strokeColor;

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public Bitmap getBitmap() {
        return bitmap;
    }

    public void setBitmap(Bitmap bitmap) {
        this.bitmap = bitmap;
    }

    public Integer getFillColor() {
        return fillColor;
    }

    public void setFillColor(Integer fillColor) {
        this.fillColor = fillColor;
    }

    public Integer getStrokeColor() {
        return strokeColor;
    }

    public void setStrokeColor(Integer strokeColor) {
        this.strokeColor = strokeColor;
    }


    public SLDGraphicParams() {
        fillColor = null;
        strokeColor = null;
    }
}
