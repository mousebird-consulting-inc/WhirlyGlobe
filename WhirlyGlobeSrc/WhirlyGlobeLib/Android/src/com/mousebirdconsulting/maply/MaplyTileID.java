package com.mousebirdconsulting.maply;

// Represents a simple Tile ID for paging
public class MaplyTileID implements Comparable<MaplyTileID>
{
	MaplyTileID() { }
	MaplyTileID(int inX,int inY,int inLevel)
	{
		x = inX;
		y = inY;
		level = inLevel;
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
	
	public int level = 0;
	public int x = 0;
	public int y = 0;
}
