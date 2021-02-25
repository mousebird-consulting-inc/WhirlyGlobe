package com.mousebird.maply;

import android.graphics.Color;
import android.graphics.Typeface;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

/**
 * Created by sjg on 6/2/16.
 */
public class VectorStyleSimpleGenerator implements VectorStyleInterface
{
    /**
     * Base class for our vector style implementations.
     */
    public abstract class VectorStyleSimple implements VectorStyle
    {
        long uuid = 0;

        /**
         * Draw priority for each feature.
         */
        public int drawPriority = VectorInfo.VectorPriorityDefault;

        @Override
        public long getUuid()
        {
            if (uuid == 0)
            {
                uuid = Identifiable.genID();
            }
            return uuid;
        }

        @Override
        public String getCategory()
        {
            return null;
        }

        /**
         * Set if this geometry is additive (e.g. sticks around) rather than being replaced.
         */
        @Override
        public boolean geomIsAdditive()
        {
            return false;
        }
    }

    // Note: This should be ThreadCurrent
    RenderController.ThreadMode threadMode = RenderController.ThreadMode.ThreadCurrent;

    /**
     * For points we just turn them into labels for demonstration.
     */
    public class VectorStyleSimplePoint extends VectorStyleSimple
    {
        LabelInfo labelInfo;

        VectorStyleSimplePoint(int inPriority)
        {
            drawPriority = inPriority;
            labelInfo = new LabelInfo();
            labelInfo.setFontSize(32.f);
            labelInfo.setTextColor(Color.WHITE);
            labelInfo.setTypeface(Typeface.DEFAULT);
            labelInfo.setDrawPriority(drawPriority);
            labelInfo.setEnable(false);
        }

        @Override
        public void buildObjects(VectorObject vecObjs[], VectorTileData tileData, RenderControllerInterface controller)
        {
            ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
            for (VectorObject point : vecObjs)
            {
                // Note: Do we need to split here
                AttrDictionary dict = point.getAttributes();
                String name = dict.getString("name");
                if (name == null)
                    name = dict.getString("value");
                Point2d pt = point.centroid();
                if (pt != null)
                {
                    ScreenLabel label = new ScreenLabel();
                    label.text = name != null ? name : ".";
                    label.loc = pt;

                    labels.add(label);
                }
            }

            ComponentObject compObj = controller.addScreenLabels(labels,labelInfo, threadMode);
            tileData.addComponentObject(compObj);
        }
    }

    /**
     * For linears we turn them into random linear features.
     */
    public class VectorStyleSimpleLinear extends VectorStyleSimple
    {
        double red,green,blue;

        VectorStyleSimpleLinear(int priority)
        {
            drawPriority = priority;
            red = Math.random()/2.0+0.5;
            green = Math.random()/2.0+0.5;
            blue = Math.random()/2.0+0.5;
        }

        @Override
        public void buildObjects(VectorObject vecObjs[],VectorTileData tileData,RenderControllerInterface controller)
        {
            VectorInfo vecInfo = new VectorInfo();
//            vecInfo.disposeAfterUse = true;
            vecInfo.setColor((float)red,(float)green,(float)blue,1.f);
            vecInfo.setLineWidth(4.f);
            vecInfo.setFilled(false);
            vecInfo.setDrawPriority(drawPriority);
            vecInfo.setEnable(false);

            ComponentObject compObj = controller.addVectors(new ArrayList<VectorObject>(Arrays.asList(vecObjs)),vecInfo,threadMode);
            tileData.addComponentObject(compObj);
        }
    }

    /**
     *
     */
    public class VectorStyleSimplePolygon extends VectorStyleSimple
    {
        double red,green,blue;

        VectorStyleSimplePolygon(int priority)
        {
            drawPriority = priority;
            red = Math.random()/2.0+0.5;
            green = Math.random()/2.0+0.5;
            blue = 0.0;
        }

        @Override
        public void buildObjects(VectorObject vecObjs[],VectorTileData tileData,RenderControllerInterface controller)
        {
            // TODO: Sort by layer name and take layer order into account for the drawPriority
//            Integer layerOrder = attrs.getInt("layer_order");

            VectorInfo vecInfo = new VectorInfo();
            vecInfo.disposeAfterUse = true;
            vecInfo.setColor((float)red,(float)green,(float)blue,1.f);
            vecInfo.setFilled(true);
            vecInfo.setDrawPriority(drawPriority);
            vecInfo.setEnable(false);

            for (VectorObject vecObj: vecObjs)
                vecObj.setSelectable(true);

            ComponentObject compObj = controller.addVectors(new ArrayList<VectorObject>(Arrays.asList(vecObjs)),vecInfo,threadMode);
            tileData.addComponentObject(compObj);
        }
    }

    WeakReference<RenderControllerInterface> controller;
    HashMap<Long,VectorStyleSimple> stylesByUUID = new HashMap<Long,VectorStyleSimple>();

    // One style per geometry type, but we'll tweak it a little based on layer name
    VectorStyleSimple pointStyle,lineStyle,polyStyle;

    public VectorStyleSimpleGenerator(RenderControllerInterface inControl)
    {
        controller = new WeakReference<RenderControllerInterface>(inControl);
        pointStyle = new VectorStyleSimplePoint(LabelInfo.LabelPriorityDefault);
        stylesByUUID.put(pointStyle.getUuid(),pointStyle);
        lineStyle = new VectorStyleSimpleLinear(VectorInfo.VectorPriorityDefault);
        stylesByUUID.put(lineStyle.getUuid(),lineStyle);
        polyStyle = new VectorStyleSimplePolygon(VectorInfo.VectorPriorityDefault);
        stylesByUUID.put(polyStyle.getUuid(),polyStyle);
    }

    /**
     * We'll return a point, line, or areal vector style
     */
    @Override
    public VectorStyle[] stylesForFeature(AttrDictionary attrs,TileID tileID,String layerName,RenderControllerInterface controller)
    {
        // Look for an existing style if we've already done this lookup
        Integer geomType = attrs.getInt("geometry_type");
        VectorStyle style = null;

        // Each layer gets its own style
        switch (geomType)
        {
            case MapboxVectorTileParser.GeomTypePoint:
                style = pointStyle;
                break;
            case MapboxVectorTileParser.GeomTypeLineString:
                style = lineStyle;
                break;
            case MapboxVectorTileParser.GeomTypePolygon:
                style = polyStyle;
                break;
            default:
                break;
        }

        return new VectorStyle[]{style};
    }

    /**
     * We'll display all layers
     */
    @Override
    public boolean layerShouldDisplay(String layerName,TileID tileID)
    {
        return true;
    }

    @Override
    public VectorStyle styleForUUID(long uuid,RenderControllerInterface controller)
    {
        VectorStyle style = stylesByUUID.get(uuid);
        return style;
    }

    @Override
    public VectorStyle[] allStyles()
    {
        return stylesByUUID.values().toArray(new VectorStyle[0]);
    }

    @Override
    public int backgroundColorForZoom(double zoom) {
        return Color.BLACK;
    }
}
