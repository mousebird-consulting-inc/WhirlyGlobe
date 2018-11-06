package com.mousebird.maply;

public class VectorStyleSettings {

    /// @brief Line widths will be scaled by this amount before display.
    private double lineScale;

    /// @brief Text sizes will be scaled by this amount before display.
    private double textScale;

    /// @brief Markers will be scaled by this amount before display.
    private double markerScale;

    /// @brief Importance for markers in the layout engine
    private double markerImportance;

    /// @brief Default marker size when none is specified
    private double markerSize;

    /** @brief The overall map scale calculations will be scaled by this amount
     *  @details We use the map scale calculations to figure out what is dispalyed and when.  Not
     *  what to load in, mind you, that's a separate, but related calculation.  This controls the
     *  scaling of those calculations.  Scale it down to load things in later, up to load them in
     *  sooner.
     */
    private double mapScale;

    /// @brief Dashed lines will be scaled by this amount before display.
    private double dashPatternScale;

    /// @brief Use widened vectors (which do anti-aliasing and such)
    private boolean useWideVectors;

    /// @brief Where we're using old vectors (e.g. not wide) scale them by this amount
    private double oldVecWidthScale;

    /// @brief If we're using widened vectors, only active them for strokes wider than this.  Defaults to zero.
    private double wideVecCutoff;

    /// @brief If set, we'll make the areal features selectable.  If not, this saves memory.
    private boolean selectable;

    /// @brief If set, icons will be loaded from this directory
    private String iconDirectory;

    /// @brief The default font family for all text
    private String fontName;


    public double getLineScale() {
        return lineScale;
    }

    public void setLineScale(double lineScale) {
        this.lineScale = lineScale;
    }

    public double getTextScale() {
        return textScale;
    }

    public void setTextScale(double textScale) {
        this.textScale = textScale;
    }

    public double getMarkerScale() {
        return markerScale;
    }

    public void setMarkerScale(double markerScale) {
        this.markerScale = markerScale;
    }

    public double getMarkerImportance() {
        return markerImportance;
    }

    public void setMarkerImportance(double markerImportance) {
        this.markerImportance = markerImportance;
    }

    public double getMarkerSize() {
        return markerSize;
    }

    public void setMarkerSize(double markerSize) {
        this.markerSize = markerSize;
    }

    public double getMapScale() {
        return mapScale;
    }

    public void setMapScale(double mapScale) {
        this.mapScale = mapScale;
    }

    public double getDashPatternScale() {
        return dashPatternScale;
    }

    public void setDashPatternScale(double dashPatternScale) {
        this.dashPatternScale = dashPatternScale;
    }

    public boolean isUseWideVectors() {
        return useWideVectors;
    }

    public void setUseWideVectors(boolean useWideVectors) {
        this.useWideVectors = useWideVectors;
    }

    public double getOldVecWidthScale() {
        return oldVecWidthScale;
    }

    public void setOldVecWidthScale(double oldVecWidthScale) {
        this.oldVecWidthScale = oldVecWidthScale;
    }

    public double getWideVecCutoff() {
        return wideVecCutoff;
    }

    public void setWideVecCutoff(double wideVecCutoff) {
        this.wideVecCutoff = wideVecCutoff;
    }

    public boolean isSelectable() {
        return selectable;
    }

    public void setSelectable(boolean selectable) {
        this.selectable = selectable;
    }

    public String getIconDirectory() {
        return iconDirectory;
    }

    public void setIconDirectory(String iconDirectory) {
        this.iconDirectory = iconDirectory;
    }

    public String getFontName() {
        return fontName;
    }

    public void setFontName(String fontName) {
        this.fontName = fontName;
    }




}
