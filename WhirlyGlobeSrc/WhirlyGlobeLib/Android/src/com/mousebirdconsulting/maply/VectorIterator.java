package com.mousebirdconsulting.maply;

import java.util.Iterator;

/**
 * A VectorObject can contain multiple sub-vectors.  This iterator
 * lets toolkit users iterate over those sub-vectors without having
 * to access them directly.
 * 
 * @author sjg
 *
 */
public class VectorIterator implements Iterator<VectorObject>
{
	private VectorIterator()
	{
	}
	
	VectorIterator(VectorObject vecObj)
	{
		initialise(vecObj);
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(VectorObject vecObj);
	native void dispose();

	private long nativeHandle;

	@Override
	public native boolean hasNext();

	@Override
	public native VectorObject next();

	@Override
	public native void remove();
}
