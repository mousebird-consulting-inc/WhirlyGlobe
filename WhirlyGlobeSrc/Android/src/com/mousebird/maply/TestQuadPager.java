package com.mousebird.maply;

import java.util.ArrayList;

import android.util.Log;

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
	
	public TestQuadPager(int inMinZoom,int inMaxZoom)
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
				vecInfo.setColor(1.f, 0.f, 0.f, 1.f);
				vecInfo.setEnable(false);
				ComponentObject compObj = layer.maplyControl.addVector(vecObj, vecInfo,MaplyController.ThreadMode.ThreadAny);
				compObjs.add(compObj);
				
				// Label right in the middle
				ScreenLabel label = new ScreenLabel();
				label.loc = new Point2d((mbr.ll.getX()+mbr.ur.getX())/2.0,(mbr.ll.getY()+mbr.ur.getY())/2.0);
				label.text = tileID.level + ": (" + tileID.x + "," + tileID.y + ")";
				LabelInfo labelInfo = new LabelInfo();
				labelInfo.setEnable(false);
				compObj = layer.maplyControl.addScreenLabel(label, labelInfo,MaplyController.ThreadMode.ThreadAny);
				compObjs.add(compObj);
						
				layer.addData(compObjs, tileID);
				
				Log.d("Maply","Loaded tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");

				layer.tileDidLoad(tileID);				
			}
		},true);
	}

}
