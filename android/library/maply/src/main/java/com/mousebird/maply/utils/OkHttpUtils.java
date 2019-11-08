package com.mousebird.maply.utils;

import okhttp3.Call;
import okhttp3.OkHttpClient;

/**
 * @author António Bastião (antonio.bastiao@igenius.ai) on 2019-10-14
 */
public class OkHttpUtils {
    public static void cancel(OkHttpClient client, Object tag) {
        for (Call call : client.dispatcher().queuedCalls()) {
            if (tag.equals(call.request().tag())) call.cancel();
        }
        for (Call call : client.dispatcher().runningCalls()) {
            if (tag.equals(call.request().tag())) call.cancel();
        }
    }
}
