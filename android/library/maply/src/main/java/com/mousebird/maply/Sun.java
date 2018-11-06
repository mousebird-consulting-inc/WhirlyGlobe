package com.mousebird.maply;
import android.util.Log;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.SimpleTimeZone;

/**
 *
 */
public class Sun
{
    /**
     * Set up the sun with the current time.
     */
    public Sun()
    {
        initialise();
        Date date = new Date();
        setDate(date);
    }

    /**
     * Set up the sun with the given date/time.
     */
    public Sun(Date date)
    {
        initialise();
        setDate(date);
    }

    /**
     * Make a Light from the current time.
     */
    public Light makeLight()
    {
        Light light = new Light();
        Point3d dir = getDirection();
        light.setPos(new Point3d(dir.getX(), dir.getZ(), dir.getY()));
//        light.setPos(new Point3d(-0.940805137, 0.13197355, 0.3121996828));
//        Log.d("Maply","Light = " + dir.getX() + " " + dir.getY() + " " + dir.getZ());
        light.setAmbient(0.1f,0.1f,0.1f,1.f);
        light.setDiffuse(0.8f,0.8f,0.8f,1.f);
        light.setViewDependent(true);

        return light;
    }

    public void setDate(Date date)
    {
        Calendar cal = GregorianCalendar.getInstance();
        cal.setTimeZone(new SimpleTimeZone(SimpleTimeZone.UTC_TIME, "UTC"));
        cal.setTime(date);
        double year = cal.get(Calendar.YEAR);
        double month = cal.get(Calendar.MONTH)+1;
        double day = cal.get(Calendar.DAY_OF_MONTH);
        double hour = cal.get(Calendar.HOUR_OF_DAY);
        double min = cal.get(Calendar.MINUTE);
        double sec = cal.get(Calendar.SECOND);

        setTime(date.getTime()/1000.0,year,month,day,hour,min,sec);
    }

    // Run the data sun calculation on startup
    native void setTime(double time,double year,double month,double day,double hour,double minute,double second);

    /**
     * Return the vector corresponding to the sun location from the earth.
     */
    public native Point3d getDirection();

    public native float[] asPosition();

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
