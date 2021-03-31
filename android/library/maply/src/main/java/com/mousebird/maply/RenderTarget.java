package com.mousebird.maply;

import android.graphics.Color;

/** Represents a render target (other than the screen)
 * <br>
 * Individual objects can ask to be drawn somewhere other than the screen.  This is how we do that.
 * A render target is just a link between a render every frame and a MaplyTexture.  To get at the actual image you use the MaplyTexture.
 * At the moment a render target can only draw the full screen, possibly at a lower resolution.
 */
public class RenderTarget
{
    /** The texture we'll draw into.
     * <br>
     * This is the texture we'll draw into.  Use createTexture to set it up.
     */
    public MaplyTexture texture;

    /**
     * The internal ID for the render target.  Don't mess with this.
     */
    public long renderTargetID;

    /**
     * If set, we'll clear the target textures every frame before rendering to it.
     * If not set, we won't clear the render target texture between frames.
     * True by default.
     */
    public boolean clearEveryFrame = true;

    /**
     *     Clear the render target to this value on every frame.
     *     This is for render targets that are not purely color, such as multiple floats.
     */
    public float clearVal = 0.0f;

    /**
     * Clear the render target to this color every frame.
     * Default is clear black.
     */
    public int color = 0;

    /**
     * If set, anything rendered to this render target will blend with what's there.
     * If not set, what's rendered will replace what was there before.
     * This is the way it normally works for screen rendering.
     * Set to false by default.
     */
    public boolean blend = false;

    public RenderTarget()
    {
        renderTargetID = Identifiable.genID();
    }
}
