package com.mousebird.maply;

import java.util.ArrayList;

/**
 * Test Maply's quad paging by creating a rectangle for each tile
 * with a label in the center.  This is extremely useful for debugging
 * and not much else.
 * 
 * @author sjg
 *
 */
public class TestQuadPager implements QuadPagingLayer.PagingInterface
{
	int minZoom = 0;
	int maxZoom = 16;
	
	TestQuadPager(int inMinZoom,int inMaxZoom)
	{		
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
	}
	
	@Override
	public int minZoom() 
	{
		return minZoom;
	}

	@Override
	public int maxZoom() 
	{
		return maxZoom;
	}

	@Override
	public void startFetchForTile(final QuadPagingLayer layer, final MaplyTileID tileID) 
	{
		layer.layerThread.addTask(new Runnable()
		{
			@Override
			public void run()
			{
				ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();

				// Areal that covers the tile
				Mbr mbr = layer.geoBoundsForTile(tileID);
				VectorObject vecObj = new VectorObject();
				Point2d pts[] = new Point2d[4];
				Point2d span = mbr.span();
				pts[0] = new Point2d(mbr.ll.getX()+span.getX()/10,mbr.ll.getY()+span.getY()/10);
				pts[1] = new Point2d(mbr.ur.getX()-span.getX()/10,mbr.ll.getY()+span.getY()/10);
				pts[2] = new Point2d(mbr.ur.getX()-span.getX()/10,mbr.ur.getY()-span.getY()/10);
				pts[3] = new Point2d(mbr.ll.getX()+span.getX()/10,mbr.ur.getY()-span.getY()/10);
				vecObj.addAreal(pts);

				VectorInfo vecInfo = new VectorInfo();
				vecInfo.setEnable(false);
				ComponentObject compObj = layer.maplyControl.addVector(vecObj, vecInfo);
				compObjs.add(compObj);
				
				// Label right in the middle
				ScreenLabel label = new ScreenLabel();
				label.loc = new Point2d((mbr.ll.getX()+mbr.ur.getX())/2.0,(mbr.ll.getY()+mbr.ur.getY())/2.0);
				label.text = tileID.level + ": (" + tileID.x + "," + tileID.y + ")";
				LabelInfo labelInfo = new LabelInfo();
				labelInfo.setEnable(false);
				labelInfo.fontSize = 32.f;
				compObj = layer.maplyControl.addScreenLabel(label, labelInfo);
				compObjs.add(compObj);
						
				layer.addData(compObjs, tileID);

				layer.tileDidLoad(tileID);				
			}
		},true);
	}

}
