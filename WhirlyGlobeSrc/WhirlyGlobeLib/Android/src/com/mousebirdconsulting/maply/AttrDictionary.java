package com.mousebirdconsulting.maply;

/**
 * The attribute dictionary is a collection of name/value pairs.
 * We use it to store (and modify) attributes on VectorObject structures,
 * primarily.  It's analogous to the NSDictionary class on iOS, but much
 * simpler.
 * 
 * @author sjg
 *
 */
public class AttrDictionary
{	
	/**
	 * Construct an empty attribution dictionary
	 */
	AttrDictionary()
	{
		initialise();
	}
	
	/**
	 * Return a string corresponding to the given attribute name.
	 * @param attrName Name of the attribute we're looking for.
	 * @return Returns a string for the attribute or null if there was none.
	 */
	public native String getString(String attrName);
	
	/**
	 * Return an Integer corresponding to the given attribute name.
	 * @param attrName Name of the attribute we're looking for.
	 * @return Returns an Integer for the given attribute or null if there was none.
	 */
	public native Integer getInt(String attrName);
	
	/**
	 * Fetch a double corresponding to the given attribute name.
	 * @param attrName Name of the attribute we're looking for.
	 * @return Returns a Double for the given attribute or null if there was none.
	 */
	public native Double getDouble(String attrName);	
	
	public void finalize()
	{
		dispose();
	}
	native void initialise();
	native void dispose();	
	private long nativeHandle;	
}
