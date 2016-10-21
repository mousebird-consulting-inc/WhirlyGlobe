package com.mousebird.maply;

import android.renderscript.Matrix4f;

/**
 * Rather than add a single 3D point we assume you want to add a lot of them all at once.  This object lets you do that and lets you assign the various data values to input attributes in your custom shader.
 * All the cool kids have custom shaders.
 */
public class Points
{
    public Points()
    {
        mat = new Matrix4f();
        mat.loadIdentity();
    }

    GeometryRawPoints rawPoints = new GeometryRawPoints();

    /**
     * If the matrix is set the points will be transformed by this matrix first.
     */
    public Matrix4d mat = null;

    /**
     * Add an array of integer values.
     * @param name Name of attribute in shader
     * @param intVals Integer values
     * @param numVals Number of values
     */
    public void addIntValues(String name,int[] intVals,int numVals)
    {
        rawPoints.addIntValues(name,intVals,numVals);
    }

    /**
     * Add an array of floating values.
     * @param name Name of attribute in shader
     * @param floatVals Floating point values
     * @param numVals Number of values
     */
    public void addFloatValues(String name,float[] floatVals,int numVals)
    {
        rawPoints.addFloatValues(name,floatVals,numVals);
    }

    /**
     * Add an array of 2 floating point values.
     * @param name Name of attribute in shader
     * @param pt2fVals 2*numPts floating point values
     * @param numPts Number of points
     */
    public void addPoint2fValues(String name,float[] pt2fVals,int numPts)
    {
        rawPoints.addPoint2fValues(name,pt2fVals,numPts);
    }

    /**
     * Add an array of 3 floating point values
     * @param name Name of attribute in shader
     * @param pt3fVals 3*numPts floating point values
     * @param numPts Number of points
     */
    public void addPoint3fValues(String name,float[] pt3fVals,int numPts)
    {
        rawPoints.addPoint3fValues(name,pt3fVals,numPts);
    }

    /**
     * Add an array of 3 double values
     * @param name Name of attribute in shader
     * @param pt3dVals 3*numPts double values
     * @param numPts Number of points
     */
    public void addPoint3dValues(String name,double[] pt3dVals,int numPts)
    {
        rawPoints.addPoint3dValues(name,pt3dVals,numPts);
    }

    /**
     * Add an array of 4 floating point values
     * @param name Name of attribute in shader
     * @param pt4fVals 4*numPts floating values
     * @param numPts Number of points
     */
    public void addPoint4fValues(String name,float[] pt4fVals,int numPts)
    {
        rawPoints.addPoint4fValues(name,pt4fVals,numPts);
    }

    /**
     * Add an attribute of the given type.  The name corresponds to an attribute in the shader.
     * @param name Name of attribute in shader
     * @param type Type of shader attribute
     */
    public void addAttribute(String name,GeometryRawPoints.Type type)
    {
        rawPoints.addAttribute(name,type);
    }
}
