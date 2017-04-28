package com.mousebird.maply.sld.sldsymbolizers;

import android.content.res.AssetManager;

import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorStyleSettings;

import java.net.URL;
import java.util.HashMap;


public class SLDSymbolizerParams {

    // Assigned once per SLD
    private MaplyBaseController baseController;
    private VectorStyleSettings vectorStyleSettings;
    private AssetManager assetManager;
    private String basePath;

    public MaplyBaseController getBaseController() {
        return baseController;
    }
    public VectorStyleSettings getVectorStyleSettings() {
        return vectorStyleSettings;
    }
    public AssetManager getAssetManager() {
        return assetManager;
    }
    public String getBasePath() {
        return basePath;
    }


    public SLDSymbolizerParams(MaplyBaseController baseController, AssetManager assetManager, VectorStyleSettings vectorStyleSettings, String basePath, int relativeDrawPriority) {
        this.baseController = baseController;
        this.assetManager = assetManager;
        this.vectorStyleSettings = vectorStyleSettings;
        this.basePath = basePath;
        this.relativeDrawPriority = relativeDrawPriority;
    }


    // Assigned once per Rule
    private Number minScaleDenominator;
    private Number maxScaleDenominator;
    private HashMap<String, Object> crossSymbolizerParams;

    public void resetRuleParams() {
        this.minScaleDenominator = null;
        this.maxScaleDenominator = null;
        crossSymbolizerParams = new HashMap<String, Object>();
    }

    public Number getMinScaleDenominator() {
        return minScaleDenominator;
    }
    public void setMinScaleDenominator(Number minScaleDenominator) {
        this.minScaleDenominator = minScaleDenominator;
    }

    public Number getMaxScaleDenominator() {
        return maxScaleDenominator;
    }
    public void setMaxScaleDenominator(Number maxScaleDenominator) {
        this.maxScaleDenominator = maxScaleDenominator;
    }

    public HashMap<String, Object> getCrossSymbolizerParams() {
        return crossSymbolizerParams;
    }


    // Increments once per Symbolizer
    private int relativeDrawPriority;

    public int getRelativeDrawPriority() {
        return relativeDrawPriority;
    }

    public void incrementRelativeDrawPriority() {
        relativeDrawPriority = relativeDrawPriority + 1;
    }

}
