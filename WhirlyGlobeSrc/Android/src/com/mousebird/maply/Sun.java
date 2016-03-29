package com.mousebird.maply;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

/**
 *
 */
public class Sun
{
    /**
     * Set up the sun with the current time.
     */
    Sun()
    {
        initialise();
        Date date = new Date();
        setDate(date);
    }

    /**
     * Set up the sun with the given date/time.
     */
    Sun(Date date)
    {
        initialise();
        setDate(date);
    }

    /**
     * Make a Light from the current time.
     */
    Light makeLight()
    {
        Light light = new Light();
        light.setPos(getDirection());
        light.setAmbient(0.1f,0.1f,0.1f,1.f);
        light.setDiffuse(0.8f,0.8f,0.8f,1.f);
        light.setViewDependent(true);

        return light;
    }

    public void setDate(Date date)
    {
        Calendar cal = GregorianCalendar.getInstance();
        cal.setTime(date);
        double year = cal.get(Calendar.YEAR);
        double month = cal.get(Calendar.MONTH);
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

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
