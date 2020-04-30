package com.mousebird.maply;

/**
 Settings that control how vector tiles look in relation to their styles.

 These are set based on the sort of device we're on, particularly retina vs. non-retina.  They can be manipulated directly as well for your needs.

 This is the object backing the ObjC and Android versions.
 */
public class VectorStyleSettings {

    public VectorStyleSettings() {
        initialise();
    }

    /// Line widths will be scaled by this amount before display.
    native public double getLineScale();
    native public void setLineScale(double scale);

    /// Text sizes will be scaled by this amount before display.
    native public double getTextScale();
    native public void setTextScale(double scale);

    /// Markers will be scaled by this amount before display.
    native public double getMarkerScale();
    native public void setMarkerScale(double scale);

    /// Importance for markers in the layout engine
    native public double getMarkerImportance();
    native public void setMarkerImportance(double importance);

    /// Default marker size when none is specified
    native public double getMarkerSize();
    native public void setMarkerSize(double size);

    /// Importance for labels in the layout engine
    native public double getLabelImportance();
    native public void setLabelImportance(double importance);

    /// If set we'll use the zoom levels defined in the style
    native public boolean getUseZoomLevels();
    native public void setUseZoomLabels(boolean useZoomLabels);

    /// For symbols we'll try to pull a UUID out of this field to stick in the marker and label uniqueID
    native public String getUuidField();
    native public void setUuidField(String uuidField);

    /// Draw priority calculated as offset from here
    native public int getBaseDrawPriority();
    native public void setBaseDrawPriority(int drawPriority);

    /// Offset between levels
    native public int getDrawPriorityPerLevel();
    native public void setDrawPriorityPerLevel(int drawPriorityPerLevel);


    /** @brief The overall map scale calculations will be scaled by this amount
     *  @details We use the map scale calculations to figure out what is displayed and when.  Not
     *  what to load in, mind you, that's a separate, but related calculation.  This controls the
     *  scaling of those calculations.  Scale it down to load things in later, up to load them in
     *  sooner.
     */
    native public double getMapScaleScale();
    native public void setMapScaleScale(double scale);

    /// @brief Dashed lines will be scaled by this amount before display.
    native public double getDashPatternScale();
    native public void setDashPatternScale(double scale);

    /// @brief Use widened vectors (which do anti-aliasing and such)
    native public boolean getUseWideVectors();
    native public void setUseWideVectors(boolean useWideVectors);

    /// @brief Where we're using old vectors (e.g. not wide) scale them by this amount
    native public double getOldVecWidthScale();
    native public void setOldVecWidthScale(double widthScale);

    /// @brief If we're using widened vectors, only active them for strokes wider than this.  Defaults to zero.
    native public double getWideVecCutoff();
    native public void setWideVecCutoff(double cutoff);

    /// @brief If set, we'll make the areal features selectable.  If not, this saves memory.
    native public boolean getSelectable();
    native public void setSelectable(boolean selectable);

    /// @brief If set, icons will be loaded from this directory
    native public String getIconDirectory();
    native public void setIconDirectory(String iconDirectory);

    /// @brief The default font family for all text
    native public String getFontName();
    native public void setFontName(String fontName);

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    native void initialise();
    native void dispose();
    private static native void nativeInit();
    protected long nativeHandle;
}
