/*
 *  RemoteTileSource.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
package com.mousebird.maply;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;

import com.squareup.okhttp.CertificatePinner;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.ArrayList;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

/**
 * The remote tile source fetches individual tiles from a remote image basemap
 * on request.  It needs basic information about the remote tile source to do this,
 * such as base URL, file extension and min and max zoom level.
 * 
 * This object works on conjuction with a QuadImageTileLayer.
 *
 * This class is an extension of the regular RemoteTileSource, it allows
 * trusted SSL connections (via certificate pinning, this *requires okhttp v2.3*)
 * which are authenticated via SSL client certificates (in a keystore).
 * Note that none of these measures provides perfect security, it just makes it a
 * bit harder to access the tile server from outside.
 *
 */
public class SSLRemoteTileSource implements QuadImageTileLayer.TileSource
{
	ArrayList<String> baseURLs = new ArrayList<String>();
	String ext = null;
	int minZoom = 0;
	int maxZoom = 0;
	public CoordSystem coordSys = new SphericalMercatorCoordSystem();
	OkHttpClient client = new OkHttpClient();

	/**
	 * The tile source delegate will be called back when a tile loads
	 * or fails to load.
	 * <p>
	 * You use these for tracking what the remote tile source is up to.
	 * They're not required, but they are handy for capturing failures.
	 *
	 */
	public interface TileSourceDelegate
	{
		/**
		 * Tile successfully loaded.
		 * @param tileSource Tile source that just loaded the tile.
		 * @param tileID Tile ID
		 */
		public void tileDidLoad(SSLRemoteTileSource tileSource, MaplyTileID tileID);
		/**
		 * Tile failed to load.
		 * @param tileSource Tile source that failed to load the tile.
		 * @param tileID Tile ID.
		 */
		public void tileDidNotLoad(SSLRemoteTileSource tileSource, MaplyTileID tileID);
	}

	/**
	 * Set this delegate to get callbacks when tiles load or fail to load.
	 */
	public TileSourceDelegate delegate = null;

	/**
	 * Construct a remote tile source that fetches from a single URL.  You provide
	 * the base URL and the extension as well as min and max zoom levels.
	 *
	 * @param inBase The base URL we'll fetching tiles from
	 * @param inExt
	 * @param inMinZoom
	 * @param inMaxZoom
	 * @param clientCertKeystore Stream holding the keystore containing the client certificate
	 * @param clientCertKeystorePassword The password for the keystore containing the client certificate
	 * @param clientCertPassword The password for the client certificate
	 * @param serverPublicCertKeystore Stream holding the keystore containing the server certificate
	 * @param serverPublicCertKeystorePassword The password for the keystore containing the server certificate
	 * @param certPins Sequence of SHA1 hashes for certificate pinning
	 * @throws IllegalArgumentException A lot of stuff can go wrong while loading the certificates from the keystores
	 */
	public SSLRemoteTileSource(String inBase, String inExt, int inMinZoom, int inMaxZoom, InputStream clientCertKeystore, String clientCertKeystorePassword, String clientCertPassword, InputStream serverPublicCertKeystore, String serverPublicCertKeystorePassword, String[] certPins) throws IllegalArgumentException {
		URL inBaseURL;
		try {
			inBaseURL = new URL(inBase);
		} catch (MalformedURLException e) {
			throw new IllegalArgumentException("Malformed URL");
		}
		if(!inBaseURL.getProtocol().equals("https")) {
			throw new IllegalArgumentException("SSLRemoteTileSource requires an https URL");
		}
		baseURLs.add(inBase);
		ext = inExt;
		minZoom = inMinZoom;
		maxZoom = inMaxZoom;
		KeyStore selfsignedKeys = null;
		if(serverPublicCertKeystore!=null) {
			try {
				selfsignedKeys = KeyStore.getInstance("BKS");
			} catch (KeyStoreException e) {
				throw new IllegalArgumentException(
						"Could not open keystore BKS instance");
			}
			try {
				selfsignedKeys.load(serverPublicCertKeystore, serverPublicCertKeystorePassword.toCharArray());
			} catch (NoSuchAlgorithmException e) {
				throw new IllegalArgumentException(
						"Could not load certificate resource - no such algorithm");
			} catch (CertificateException e) {
				throw new IllegalArgumentException(
						"Could not load certificate resource - certificate exception");
			} catch (IOException e) {
				throw new IllegalArgumentException("Could not load server certificate, IO error");
			}
		}
		TrustManagerFactory trustMgr;
		try {
			trustMgr = TrustManagerFactory.getInstance(TrustManagerFactory
                                        .getDefaultAlgorithm());
		} catch (NoSuchAlgorithmException e) {
			throw new IllegalArgumentException(
                                        "Could not create trust manager instance - no such algorithm");
		}
		try {
			// if selfsignedKeys is null the default keystore _should_ be used.
			trustMgr.init(selfsignedKeys);
		} catch (KeyStoreException e) {
			throw new IllegalArgumentException(
                                        "Could not init trust manager - keystore exception");
		}
		KeyStore clientauthKeys;
		try {
			clientauthKeys = KeyStore.getInstance("BKS");
		} catch (KeyStoreException e1) {
			throw new IllegalArgumentException(
                                        "Could not get keystore instance - keystore exception");
		}
		try {
			clientauthKeys.load(clientCertKeystore, clientCertKeystorePassword.toCharArray());
		} catch (NoSuchAlgorithmException e1) {
			throw new IllegalArgumentException(
                                        "Could not load client keys - no such algorithm");
		} catch (CertificateException e1) {
			throw new IllegalArgumentException(
                                        "Could not load client keys - certificate exception");
		} catch (IOException e) {
			throw new IllegalArgumentException("Could not load client certificate, IO error");
		}
		KeyManagerFactory keyMgr;
		try {
			keyMgr = KeyManagerFactory.getInstance(KeyManagerFactory
                                        .getDefaultAlgorithm());
		} catch (NoSuchAlgorithmException e1) {
			throw new IllegalArgumentException(
                                        "Could not get key manager instance - no such algorithm");
		}
		try {
			keyMgr.init(clientauthKeys, clientCertPassword.toCharArray());
		} catch (KeyStoreException e1) {
			throw new IllegalArgumentException(
                                        "Could not init key manager - key store exception");
		} catch (NoSuchAlgorithmException e1) {
			throw new IllegalArgumentException(
                                        "Could not init key manager - no such algorithm");
		} catch (UnrecoverableKeyException e1) {
			throw new IllegalArgumentException(
                                        "Could not init key manager - unrecoverable key");
		}
		SSLContext privateSSLcontext;
		try {
			privateSSLcontext = SSLContext.getInstance("TLS");
		} catch (NoSuchAlgorithmException e1) {
			throw new IllegalArgumentException("Could not create SSL context instance - no such algorithm");
		}
		try {
			privateSSLcontext.init(keyMgr.getKeyManagers(),
                                        trustMgr.getTrustManagers(), new SecureRandom());
		} catch (KeyManagementException e1) {
			throw new IllegalArgumentException(
                                        "Could not init SSL context - key management exception");
		}
		client.setSslSocketFactory(privateSSLcontext.getSocketFactory());
		if(certPins!=null && certPins.length>0) {
			CertificatePinner certificatePinner = new CertificatePinner.Builder().add(inBaseURL.getHost(), certPins).build();
			client.setCertificatePinner(certificatePinner);
		}
	}

	File cacheDir = null;
	/**
	 * Set the cache directory for fetched images.  We'll look there first.
	 * There is no limiting or pruning going on, that directory will just get
	 * bigger and bigger.
	 * <p>
	 * By default that directory is null.
	 * 
	 * @param inCacheDir Cache directory for image tiles.
	 */
	public void setCacheDir(File inCacheDir)
	{
		cacheDir = inCacheDir;
	}

	@Override
	public int minZoom() 
	{
		return minZoom;
	}

	@Override
	public int maxZoom() 
	{
		return maxZoom;
	}
	
	// Connection task fetches the image
	private class ConnectionTask extends AsyncTask<String, Void, Bitmap>
	{
		SSLRemoteTileSource tileSource = null;
		QuadImageTileLayer layer = null;
		MaplyTileID tileID = null;
		URL url = null;
		String locFile = null;
		int frame;
		
		ConnectionTask(QuadImageTileLayer inLayer,SSLRemoteTileSource inTileSource, MaplyTileID inTileID,String inURL,String inFile,int inFrame)
		{
			tileSource = inTileSource;
			layer = inLayer;
			tileID = inTileID;
			locFile = inFile;
			frame = inFrame;
			try
			{
				url = new URL(inURL);
			}
			catch (IOException e)
			{
				
			}
		}
		
		@Override
		protected Bitmap doInBackground(String... urls)
		{
			try
			{
				// See if it's here locally
				File cacheFile = null;
				Bitmap bm = null;
				if (locFile != null)
				{
					cacheFile = new File(locFile);
					if (cacheFile.exists())
					{
						BufferedInputStream aBufferedInputStream = new BufferedInputStream(new FileInputStream(cacheFile));
			    		bm = BitmapFactory.decodeStream(aBufferedInputStream);				
//			    		Log.d("Maply","Read cached file for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
					}
				}
				
				// Wasn't cached
				if (bm == null)
				{
					// Load the data from that URL
				    Request request = new Request.Builder().url(url).build();

				    byte[] rawImage = null;
				    try
				    {
				    	Response response = client.newCall(request).execute();
				    	rawImage = response.body().bytes();
				    	bm = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length);				
				    }
				    catch (Exception e)
				    {
				    }
		    		
		    		// Save to cache
		    		if (cacheFile != null && rawImage != null)
		    		{
		    			OutputStream fOut;
		    			fOut = new FileOutputStream(cacheFile);
		    			fOut.write(rawImage);
		    			fOut.close();
		    		}
//		    		Log.d("Maply","Fetched remote file for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
				}
	    		
				// Let the layer and delegate know what happened with it
		    	if (bm != null)
		    	{
		    		MaplyImageTile imageTile = new MaplyImageTile(bm);
		    		if (tileSource.delegate != null)
		    			tileSource.delegate.tileDidLoad(tileSource,tileID);
		    		layer.loadedTile(tileID, imageTile, frame);
		    	} else {
		    		if (tileSource.delegate != null)
		    			tileSource.delegate.tileDidNotLoad(tileSource,tileID);
		    		layer.loadedTile(tileID, null,frame);
		    	}
		    	
		    	return null;
			}
			catch (IOException e)
			{
				return null;
			}
			catch (Exception e)
			{
				Log.d("Maply","Remote tile error: " + e);
				return null;
			}
		}

	    @Override
	    protected void onPostExecute(Bitmap bm) 
	    {
	    }

	}
	
	/**
	 * This is called by the quad image tile layer.  Don't call this yourself.
	 */
	@Override
	public void startFetchForTile(QuadImageTileLayer layer, MaplyTileID tileID, int frame)
	{
//		Log.d("Maply","Starting fetch for tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
		
		// Form the tile URL
		int maxY = 1<<tileID.level;
		int remoteY = maxY - tileID.y - 1;
		final String tileURL = baseURLs.get(tileID.x % baseURLs.size()) + tileID.level + "/" + tileID.x + "/" + remoteY + "." + ext;

		String cacheFile = null;
		if (cacheDir != null)
			cacheFile = cacheDir.getAbsolutePath() + "/" + tileID.level + "_" + tileID.x + "_" + tileID.y + "."  + ext;
		ConnectionTask task = new ConnectionTask(layer,this,tileID,tileURL,cacheFile,frame);
		String[] params = new String[1];
		params[0] = tileURL;
		task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR,params);
	}
	
}
