package com.mousebirdconsulting.maply;

import java.util.Iterator;

// Handles iterating over the pieces of a vector object
public class VectorIterator implements Iterator<VectorObject>
{
	VectorIterator(VectorObject vecObj)
	{
		initialise(vecObj);
	}

	public native void initialise(VectorObject vecObj);
	public native void dispose();

	private long nativeHandle;

	@Override
	public native boolean hasNext();

	@Override
	public native VectorObject next();

	@Override
	public native void remove();
}
