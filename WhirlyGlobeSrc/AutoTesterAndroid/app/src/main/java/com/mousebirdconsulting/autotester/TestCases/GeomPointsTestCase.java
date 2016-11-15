package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Color;

import com.mousebird.maply.GeometryInfo;
import com.mousebird.maply.GeometryRawPoints;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Matrix4d;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.Points;
import com.mousebird.maply.Shader;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class GeomPointsTestCase extends MaplyTestCase {

    public GeomPointsTestCase(Activity activity) {
        super(activity);
        setTestName("Geometry Points Test");
        setDelay(100);
        this.implementation = TestExecutionImplementation.Globe;
    }

    String vertProg =
            "uniform mat4  u_mvpMatrix;\n" +
            "uniform float u_pointSize;\n" +
            "\n" +
            "attribute vec3 a_position;\n" +
            "attribute vec4 a_color;\n" +
            "\n" +
            "varying vec4 v_color;\n" +
            "\n" +
            "void main()\n" +
            "{\n" +
            "   v_color = a_color;\n" +
            "   gl_PointSize = u_pointSize;\n" +
            "   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n" +
            "}\n";

    String fragProg =
            "precision mediump float;\n" +
            "\n" +
            "varying vec4      v_color;\n" +
            "\n" +
            "void main()\n" +
            "{\n" +
            "  gl_FragColor = v_color;\n" +
            "}\n";

    Shader makePointShader(MaplyBaseController control)
    {
        Shader shader = new Shader("Point Shader",vertProg,fragProg,control);
        control.addShaderProgram(shader,"Point Shader");

        return shader;
    }

    static int NumPoints = 10000;

    public void addPoints(GlobeController control)
    {
        Points points = new Points();
        Shader shader = makePointShader(control);

        // Points are in the display space so let's figure out where that is
        Point2d centerGeo = Point2d.FromDegrees(-122.416667, 37.783333);
        Point3d centerPt = control.displayPointFromGeo(new Point3d(centerGeo.getX(),centerGeo.getY(),1000));
        Matrix4d mat = Matrix4d.translate(centerPt.getX(),centerPt.getY(),centerPt.getZ());

        // The actual points is a collection of arrays
        Points pts = new Points();
        pts.addAttribute("a_position", GeometryRawPoints.Type.Float3Type);
        pts.addAttribute("a_color", GeometryRawPoints.Type.Float4Type);
        pts.setMatrix(mat);

        float ptArray[] = new float[NumPoints*3];
        float colorArray[] = new float[NumPoints*4];
        for (int ii=0;ii<NumPoints;ii++)
        {
            ptArray[3*ii+0] = (float)Math.random()*10000 / 6371000;
            ptArray[3*ii+1] = (float)Math.random()*10000 / 6371000;
            ptArray[3*ii+2] = (float)Math.random()*10000 / 6371000;
            colorArray[4*ii+0] = (float)Math.random()*0.5f+0.5f;
            colorArray[4*ii+1] = (float)Math.random()*0.5f+0.5f;
            colorArray[4*ii+2] = (float)Math.random()*0.5f+0.5f;
            colorArray[4*ii+3] = 1.f;
        }
        pts.addPoint3fValues("a_position",ptArray);
        pts.addPoint4fValues("a_color",colorArray);

        GeometryInfo geomInfo = new GeometryInfo();
        geomInfo.setColor(Color.RED);
        geomInfo.setPointSize(15.f);
        geomInfo.setDrawPriority(10000000);
        geomInfo.setZBufferRead(true);
        geomInfo.setZBufferWrite(true);
        geomInfo.setShader(shader);

        control.addPoints(pts, geomInfo, MaplyBaseController.ThreadMode.ThreadAny);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase baseTestCase = new CartoDBMapTestCase(this.getActivity());
        baseTestCase.setUpWithGlobe(globeVC);
        addPoints(globeVC);

        Point2d geoPt = Point2d.FromDegrees(-122.416667, 37.783333);
        globeVC.animatePositionGeo(geoPt.getX(),geoPt.getY(),0.01,1.0);

        return true;
    }
}
