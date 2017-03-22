package com.mousebird.maply;

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

    public RenderTarget()
    {
        renderTargetID = Identifiable.genID();
    }
}
