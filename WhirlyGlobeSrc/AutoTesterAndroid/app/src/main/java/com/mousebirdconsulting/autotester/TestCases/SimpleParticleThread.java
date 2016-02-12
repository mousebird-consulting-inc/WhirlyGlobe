package com.mousebirdconsulting.autotester.TestCases;

import android.os.Handler;
import android.os.HandlerThread;

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.ParticleBatch;
import com.mousebird.maply.ParticleSystem;
import com.mousebird.maply.ParticleSystemAttribute;

import java.util.Date;

/**
 * Created by sjg on 2/11/16.
 */
public class SimpleParticleThread extends HandlerThread
{
    private ParticleSystem partSys;
    private MaplyBaseController viewC;
    private ComponentObject partSysObj;
    private float locs[];
    private float dirs[];
    private float colors[];
    private double particleLifeTime;
    private int numParticles;
    private double updateInterval;
    private float velocityScale;
    private Runnable updateTask;

    public SimpleParticleThread(MaplyBaseController inViewC)
    {
        super("Simple Particle Thread");

        viewC = inViewC;
        this.updateInterval = 2.0;
        this.particleLifeTime = 20.0;
        this.numParticles = 10000;
        this.velocityScale = 0.1f;

        this.partSys = new ParticleSystem("Test Particle System");
        this.partSys.setLifetime(this.particleLifeTime);
        this.partSys.setTotalParticles(this.numParticles);
        this.partSys.setDrawPriority(101000);
        this.partSys.setPointSize(4);
        this.partSys.setBatchSize((int) (this.numParticles / (this.particleLifeTime / this.updateInterval)));
        this.partSys.addParticleSystemAttribute("a_position", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
//            this.partSys.addParticleSystemAttribute("a_dir", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3);
//            this.partSys.addParticleSystemAttribute("a_color", ParticleSystemAttribute.MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT4);


        this.partSysObj = this.viewC.addParticleSystem(this.partSys, MaplyBaseController.ThreadMode.ThreadCurrent);

        // Note: Test it once
        try {
            generateParticles();
        }
        catch (Exception e)
        {
        }

//        start();
//        scheduleUpdateTask();
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

//                scheduleUpdateTask();
            }
        };
        Handler handler = new Handler(getLooper());
        handler.postDelayed(run, (long)(updateInterval*1000));
    }

    public void generateParticles() throws InterruptedException {
        double now = ((double) new Date().getTime() ) - 978303600000.d;

        // Data arrays for particles
        int batchSize = this.partSys.getBatchSize();

        if (locs == null) {
            this.locs = new float[this.partSys.getBatchSize()*3];
            this.dirs = new float[this.partSys.getBatchSize()*3];
            this.colors = new float[this.partSys.getBatchSize()*4];
//            this.times = new float[batchSize];
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
//                dirs[ii*3] = (float) Math.random()*2-1; dirs[ii*3+1] = (float) Math.random()*2-1; dirs[ii*3+2] = (float) Math.random()*2-1;
            dirs[ii*3] = 0.f;  dirs[ii*3+1] = 0.f;  dirs[ii*3+2] = 0.f;

//            this.times[ii] = 0;

            colors[ii*4] = 1.0f; colors[ii*4+1] = 1.0f; colors[ii*4+2] = 1.0f; colors[ii*4+3] = 1.0f;
        }

        ParticleBatch batch = new ParticleBatch(this.partSys);
        batch.addAttribute("a_position", this.locs);
//        batch.addAttribute("a_dir", this.dirs);
//        batch.addAttribute("a_color", this.colors);
//        batch.addAttribute("a_startTime", this.times);
        viewC.addParticleBatch(batch, MaplyBaseController.ThreadMode.ThreadCurrent);
    }
}
