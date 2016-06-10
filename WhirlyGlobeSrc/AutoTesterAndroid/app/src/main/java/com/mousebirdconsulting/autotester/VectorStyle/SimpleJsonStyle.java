package com.mousebirdconsulting.autotester.VectorStyle;

import android.content.Context;
import android.graphics.Color;
import android.util.Log;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.VectorObject;
import com.mousebird.maply.VectorStyle;
import com.mousebirdconsulting.autotester.Utils.GraphicUtils;

import java.util.Arrays;
import java.util.List;

/**
 * Project AutoTesterAndroid
 * Created by jmc on 10/06/16.
 *
 * A simple implementation of VectorStyle that is initialized from a json file.
 * The JSON object can have the following properties :
 *       "name": name of this style (used for debugging only)
 *       "type": FILL | STROKE | LABEL | MARKER type of feature (MARKER unsupported yet)
 *       "minLevel": minimum level this style should be applied
 *       "maxLevel": maximum level this style should be applied
 *       "relativeDrawPriority": relative drawPriority of the items (added to baseDrawPriority)
 *       "relativeLayoutImportance": relative layoutImportance of the items (added to baseLayoutImportance)
 *       "colorArgb": color of the features (hexa string as "#aarrggbb" or "#rrggbb")
 *       "lineWidth": line width in DPs (only used when type is STROKE)
 *       "dashes": array of integers describing the dashes *UNSUPPORTED*
 *       "labelProperty": in the case of labels, name of the property that contains the labels
 *       "typeFace": name of the type to be used for labels
 *       "fontSize": fontsize of labels in DPs
 *
 */
public class SimpleJsonStyle implements VectorStyle {

    //***********************************************************************//
    //                           Inner classes                               //
    //***********************************************************************//

    public enum Type {
        FILL,
        STROKE,
        MARKER,
        LABEL
    }


    //***********************************************************************//
    //                          Class variables                              //
    //***********************************************************************//

    private static final String TAG = "AutoTester";


    //***********************************************************************//
    //                         Instance variables                            //
    //***********************************************************************//

    private String name = "default";
    private Type type = Type.STROKE;
    private int minLevel = 0;
    private int maxLevel = 22;
    private int relativeDrawPriority;
    private int relativeLayoutImportance;
    @JsonIgnore
    private int color;
    private String labelProperty;
    private String typeFace;
    private double fontSize;
    private double lineWidth;
    private double[] dashes;
    private String marker;
    private double markerSize;

    @JsonIgnore
    private int baseDrawPriority;
    @JsonIgnore
    private int baseLayoutImportance;

    @JsonIgnore
    private double lineWidthDp = -1;
    @JsonIgnore
    private double fontSizeDp = -1;


    //***********************************************************************//
    //                            Constructors                               //
    //***********************************************************************//


    //***********************************************************************//
    //                         Getters and setters                           //
    //***********************************************************************//


    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public Type getType() {
        return type;
    }

    public void setType(Type type) {
        this.type = type;
    }

    public int getMinLevel() {
        return minLevel;
    }

    public void setMinLevel(int minLevel) {
        this.minLevel = minLevel;
    }

    public int getMaxLevel() {
        return maxLevel;
    }

    public void setMaxLevel(int maxLevel) {
        this.maxLevel = maxLevel;
    }

    public int getRelativeDrawPriority() {
        return relativeDrawPriority;
    }

    public void setRelativeDrawPriority(int relativeDrawPriority) {
        this.relativeDrawPriority = relativeDrawPriority;
    }

    public int getRelativeLayoutImportance() {
        return relativeLayoutImportance;
    }

    public void setRelativeLayoutImportance(int relativeLayoutImportance) {
        this.relativeLayoutImportance = relativeLayoutImportance;
    }

    public int getColor() {
        return color;
    }

    public void setColor(int color) {
        this.color = color;
    }

    public String getLabelProperty() {
        return labelProperty;
    }

    public void setLabelProperty(String labelProperty) {
        this.labelProperty = labelProperty;
    }

    public String getTypeFace() {
        return typeFace;
    }

    public void setTypeFace(String typeFace) {
        this.typeFace = typeFace;
    }

    public double getFontSize() {
        return fontSize;
    }

    public void setFontSize(double fontSize) {
        this.fontSize = fontSize;
    }

    public double getLineWidth() {
        return lineWidth;
    }

    public void setLineWidth(double lineWidth) {
        this.lineWidth = lineWidth;
    }

    public double[] getDashes() {
        return dashes;
    }

    public void setDashes(double[] dashes) {
        this.dashes = dashes;
    }

    public String getMarker() {
        return marker;
    }

    public void setMarker(String marker) {
        this.marker = marker;
    }

    public double getMarkerSize() {
        return markerSize;
    }

    public void setMarkerSize(double markerSize) {
        this.markerSize = markerSize;
    }

    public int getBaseDrawPriority() {
        return baseDrawPriority;
    }

    public void setBaseDrawPriority(int baseDrawPriority) {
        this.baseDrawPriority = baseDrawPriority;
    }

    public int getBaseLayoutImportance() {
        return baseLayoutImportance;
    }

    public void setBaseLayoutImportance(int baseLayoutImportance) {
        this.baseLayoutImportance = baseLayoutImportance;
    }


    /* User by Jackson */
    public String getColorArgb() {
        return Integer.toHexString(this.getColor());
    }

    public void setColorArgb(String argbColor) {
        this.setColor(Color.parseColor(argbColor));
    }


    //***********************************************************************//
    //                               Interfaces                              //
    //***********************************************************************//

    /* Implements TheInterface */


    //***********************************************************************//
    //                               Overrides                               //
    //***********************************************************************//


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        SimpleJsonStyle that = (SimpleJsonStyle) o;

        if (minLevel != that.minLevel) return false;
        if (maxLevel != that.maxLevel) return false;
        if (relativeDrawPriority != that.relativeDrawPriority) return false;
        if (relativeLayoutImportance != that.relativeLayoutImportance) return false;
        if (color != that.color) return false;
        if (Double.compare(that.fontSize, fontSize) != 0) return false;
        if (Double.compare(that.lineWidth, lineWidth) != 0) return false;
        if (Double.compare(that.markerSize, markerSize) != 0) return false;
        if (!name.equals(that.name)) return false;
        if (type != that.type) return false;
        if (labelProperty != null ? !labelProperty.equals(that.labelProperty) : that.labelProperty != null)
            return false;
        if (typeFace != null ? !typeFace.equals(that.typeFace) : that.typeFace != null)
            return false;
        if (!Arrays.equals(dashes, that.dashes)) return false;
        return marker != null ? marker.equals(that.marker) : that.marker == null;

    }

    @Override
    public int hashCode() {
        int result;
        long temp;
        result = name.hashCode();
        result = 31 * result + type.hashCode();
        result = 31 * result + minLevel;
        result = 31 * result + maxLevel;
        result = 31 * result + relativeDrawPriority;
        result = 31 * result + relativeLayoutImportance;
        result = 31 * result + color;
        result = 31 * result + (labelProperty != null ? labelProperty.hashCode() : 0);
        result = 31 * result + (typeFace != null ? typeFace.hashCode() : 0);
        temp = Double.doubleToLongBits(fontSize);
        result = 31 * result + (int) (temp ^ (temp >>> 32));
        temp = Double.doubleToLongBits(lineWidth);
        result = 31 * result + (int) (temp ^ (temp >>> 32));
        result = 31 * result + Arrays.hashCode(dashes);
        result = 31 * result + (marker != null ? marker.hashCode() : 0);
        temp = Double.doubleToLongBits(markerSize);
        result = 31 * result + (int) (temp ^ (temp >>> 32));
        return result;
    }

    @Override
    public String getUuid() {
        return String.valueOf(this.hashCode());
    }

    @Override
    public boolean geomIsAdditive() {
        return false;
    }

    @Override
    public ComponentObject[] buildObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {

        int level = maplyTileID.level;

        if (level < minLevel || level > maxLevel) {
            return null;
        }

        switch (this.type) {
            case FILL:
                return this.buildFilledObjects(vectors, maplyTileID, controller);

            case STROKE:
                return this.buildLineObjects(vectors, maplyTileID, controller);

            case LABEL:
                return this.buildLabelObjects(vectors, maplyTileID, controller);

            case MARKER:
                return this.buildMarkerObjects(vectors, maplyTileID, controller);
        }

        return null;
    }

    @Override
    public String toString() {
        return String.format("name='%s' - type=%s - color=(a:%d, r:%d, g:%d, b:%d)",
                name, type,
                Color.alpha(color), Color.red(color), Color.green(color), Color.blue(color));
    }

    //***********************************************************************//
    //                           Public methods                              //
    //***********************************************************************//

    /**
     * Returns whether with style component is visible at a given zoom level
     * @param level the level we want to know if the component is visible.
     * @return true if this component would display at level, false otherwise.
     */
    public boolean isVisibleAtLevel(int level) {
        return (level >= minLevel && level <= maxLevel);
    }


    //***********************************************************************//
    //                           Private methods                             //
    //***********************************************************************//

    private ComponentObject[] buildLineObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {

        if (this.lineWidth == 0 || color == Color.TRANSPARENT) {
            return null;
        }

        long start = System.currentTimeMillis();

        if (this.lineWidthDp == -1) {
            Context context = controller.getContentView().getContext();
            this.lineWidthDp = GraphicUtils.dipToPixels(context, (float) this.lineWidth);
        }


        VectorInfo vecInfo = new VectorInfo();
        vecInfo.setColor(this.color);
        vecInfo.setFilled(false);
        vecInfo.setDrawPriority(this.baseDrawPriority + this.relativeDrawPriority);
        vecInfo.setEnable(true);
        vecInfo.setLineWidth((float)lineWidthDp);
        ComponentObject component = controller.addVectors(vectors, vecInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

        String log = String.format("Style %-20s built %4d line objects in   %4d ms (tile x=%3d, y=%3d, level=%2d).",
                name, vectors.size(), System.currentTimeMillis() - start,
                maplyTileID.x, maplyTileID.y, maplyTileID.level);
        Log.i(TAG, log);

        return component != null ? new ComponentObject[]{component} : null;
    }


    private ComponentObject[] buildFilledObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {

        if (color == Color.TRANSPARENT) {
            return null;
        }

        long start = System.currentTimeMillis();

        VectorInfo vecInfo = new VectorInfo();
        vecInfo.setColor(this.color);
        vecInfo.setFilled(true);
        vecInfo.setDrawPriority(this.baseDrawPriority + this.relativeDrawPriority);
        vecInfo.setEnable(true);
        ComponentObject component = controller.addVectors(vectors, vecInfo, MaplyBaseController.ThreadMode.ThreadCurrent);

        String log = String.format("Style %-20s built %4d filled objects in %4d ms (tile x=%3d, y=%3d, level=%2d).",
                name, vectors.size(), System.currentTimeMillis() - start,
                maplyTileID.x, maplyTileID.y, maplyTileID.level);
        Log.i(TAG, log);

        return component != null ? new ComponentObject[]{component} : null;

    }

    private ComponentObject[] buildMarkerObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {
        // Not implemented yet
        return new ComponentObject[0];
    }

    private ComponentObject[] buildLabelObjects(List<VectorObject> vectors, MaplyTileID maplyTileID, MaplyBaseController controller) {
        // Not implemented yet
        return new ComponentObject[0];
    }



}
