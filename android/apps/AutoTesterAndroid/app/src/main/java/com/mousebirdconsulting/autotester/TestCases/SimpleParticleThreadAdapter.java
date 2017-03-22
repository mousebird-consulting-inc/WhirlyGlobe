package com.mousebirdconsulting.autotester.TestCases;

import android.os.Handler;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;

import java.util.Date;

/**
 * Created by sjg on 2/11/16.
 */
public class SimpleParticleThreadAdapter
{
    private ParticleSystem partSys;
    private MaplyBaseController viewC;
    private ComponentObject partSysObj;
    private float locs[];
    private float dirs[];
    private float colors[];
    private float time[];
    private double particleLifeTime;
    private int numParticles;
    private double updateInterval;
    private float velocityScale;
    private Runnable updateTask;
    LayerThread particleThread = null;

    public SimpleParticleThreadAdapter(MaplyBaseController inViewC, LayerThread inThread)
    {
        particleThread = inThread;

        viewC = inViewC;
        this.updateInterval = 1.0;
        this.particleLifeTime = 4.0;
        this.numParticles = 12000;
        this.velocityScale = 0.1f;

        this.partSys = new ParticleSystem("Test Particle System");
        this.partSys.setParticleSystemType(ParticleSystem.STATE.ParticleSystemPoint);
        this.partSys.setLifetime(this.particleLifeTime);
        this.partSys.setTotalParticles(this.numParticles);
        this.partSys.setDrawPriority(101000);
        this.partSys.setPointSize(8.f);
        this.partSys.setBatchSize((int) (this.numParticles / (this.particleLifeTime / this.updateInterval)));
        this.partSys.addParticleSystemAttribute("a_position", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
        this.partSys.addParticleSystemAttribute("a_dir", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
        this.partSys.addParticleSystemAttribute("a_startTime", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT);
        this.partSys.addParticleSystemAttribute("a_color", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT4);

        this.partSysObj = this.viewC.addParticleSystem(this.partSys, MaplyBaseController.ThreadMode.ThreadCurrent);

        scheduleUpdateTask();
    }

    void scheduleUpdateTask()
    {
        Runnable run = new Runnable() {
            @Override
            public void run() {
                try {
                    generateParticles();
                }
                catch (Exception e)
                {
                }

                scheduleUpdateTask();
            }
        };
        Handler handler = new Handler(particleThread.getLooper());
        handler.postDelayed(run, (long)(updateInterval*1000));
    }



    public void generateParticles() throws InterruptedException {
        double now = ((double) new Date().getTime()/1000.0 ) - partSys.getBasetime();

        // Data arrays for particles
        int batchSize = this.partSys.getBatchSize();

        if (locs == null) {
            this.locs = new float[this.partSys.getBatchSize()*3];
            this.dirs = new float[this.partSys.getBatchSize()*3];
            this.colors = new float[this.partSys.getBatchSize()*4];
            this.time = new float[this.partSys.getBatchSize()];
        }

        // Make up some random particles
        for (int ii = 0; ii< this.partSys.getBatchSize(); ii++) {
            //Random location
            float x = (float)Math.random()*2-1;
            float y = (float)Math.random()*2-1;
            float z = (float)Math.random()*2-1;
            float sum = (float)Math.sqrt(x*x+y*y+z*z);
            x /= sum;  y /= sum;  z /= sum;
            locs[ii*3] = x; locs[ii*3+1] = y; locs[ii*3+2] = z;

            //Random direction
            x = (float) Math.random()*2-1;  y = (float) Math.random()*2-1;  z = (float) Math.random()*2-1;
            sum = (float)Math.sqrt(x*x+y*y+z*z)*100;
            x /= sum;  y /= sum;  z /= sum;
            dirs[ii*3] = x; dirs[ii*3+1] = y; dirs[ii*3+2] = z;

            // A couple of simple colors
            float r,g,b;
            if (ii % 2 == 0)
            {
                r = 1.f;  g = 0;  b = 0;
            } else {
                r = 0;  g = 1.f;  b = 0;
            }
            colors[ii*4] = r; colors[ii*4+1] = g; colors[ii*4+2] = b; colors[ii*4+3] = 1.f;

            time[ii] = (float)now;
        }

        ParticleBatch batch = new ParticleBatch(this.partSys);
        batch.addAttribute("a_position", this.locs);
        batch.addAttribute("a_dir", this.dirs);
        batch.addAttribute("a_startTime", this.time);
        batch.addAttribute("a_color", this.colors);
        // Note: Can't do this on the current thread.  Need to fix that.
        viewC.addParticleBatch(batch, MaplyBaseController.ThreadMode.ThreadCurrent);
    }
}
