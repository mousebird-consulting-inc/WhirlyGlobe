package com.mousebird.maply;

/**
 * A tile ID represents a single tile for paging.  Tile IDs
 * will show up when you're implementing a tile source.
 * You'll be given a tileID to fetch the data for.
 * <p>
 * We assume all tiles refer to positions with a quad tree
 * starting at (0,0,0).
 * 
 * @author sjg
 *
 */
public class MaplyTileID implements Comparable<MaplyTileID>
{
	MaplyTileID() { }
	
	/**
	 * Construct with x,y, and level.
	 * @param inX The horizontal tile
	 * @param inY Vertical tile
	 * @param inLevel Level in the quad tree.
	 */
	MaplyTileID(int inX,int inY,int inLevel)
	{
		x = inX;
		y = inY;
		level = inLevel;
	}
	
	public String toString()
	{
		return level + ": (" + x + "," + y + ")";
	}
	
	@Override
	public int compareTo(MaplyTileID that) 
	{
		if (level == that.level)
		{
			if (x == that.x)
			{
				if (y == that.y)
					return 0;
				else
					return (y < that.y) ? -1 : 1;
			} else
				return (x < that.x) ? -1 : 1;
		}
		return (level < that.level) ? -1 : 1;
	}		
	
	@Override
	public boolean equals(Object that)
	{
		if (this == that)
			return true;
		
		if (!(that instanceof MaplyTileID))
			return false;
		
		MaplyTileID lhs = (MaplyTileID)that;
		return level == lhs.level && x == lhs.x && y == lhs.y;
	}
	
	@Override
	public int hashCode()
	{
		int result = 17;
		result = 31 * result + level;
		result = 31 * result + x;
		result = 31 * result + y;
		
		return result;
	}
	
	/**
	 * Level refers to the level within the quad tree.  This starts at zero.
	 */
	public int level = 0;
	
	/**
	 * Horizontal position in the quad tree.  This starts on the left as
	 * far as Maply is concerned, but your own implementation may vary.
	 */
	public int x = 0;
	
	/**
	 * Vertical position in the quad tree.  This starts at the bottom,
	 * but your own tiling system may vary.
	 */
	public int y = 0;
}
