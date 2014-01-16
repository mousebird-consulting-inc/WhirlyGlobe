package com.mousebirdconsulting.maply;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.MotionEvent;

public class MaplySurfaceView extends GLSurfaceView
{
	public MaplySurfaceView(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
	}

	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		Log.i("Maply", "motion");
		
		switch (event.getAction())
		{
		case (MotionEvent.ACTION_MOVE) :
			Log.i("Maply", "Action was MOVE");
			return true;
		}

		return true;
	}
}
