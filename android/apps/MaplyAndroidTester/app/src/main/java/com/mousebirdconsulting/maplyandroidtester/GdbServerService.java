package com.mousebirdconsulting.maplyandroidtester;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import java.io.IOException;

/**
 * Created by sjg on 9/9/15.
 */
public class GdbServerService extends android.app.Service
{

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        startGdbServer();
        return Service.START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

    private void startGdbServer() {
        Thread thread = new Thread()
        {
            @Override
            public void run()
            {
                try {
                    new ProcessBuilder()
                            .command(getFilesDir().getParent() + "/lib/gdbserver.so", "tcp:5039", "--attach" ," " + android.os.Process.myPid())
                            .redirectErrorStream(true)
                            .start();
                } catch (IOException e) {
                    Log.e("Maply", "IOException failed to start gdbserver");
                }
            }
        };

        thread.start();
    }
}
