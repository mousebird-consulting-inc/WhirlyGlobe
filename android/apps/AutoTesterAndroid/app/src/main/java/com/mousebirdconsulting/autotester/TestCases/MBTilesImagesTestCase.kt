package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.content.Context
import android.content.ContextWrapper
import android.util.Log
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException

class MBTilesImagesTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Image MBTiles")
        implementation = TestExecutionImplementation.Both
    }


    private val gestureDelegate = object : GlobeController.GestureDelegate {
        override fun userDidSelect(controller: GlobeController, objs: Array<SelectedObject>, loc: Point2d, screenLoc: Point2d) {
            // Intentionally blank
        }

        override fun userDidTap(controller: GlobeController, loc: Point2d, screenLoc: Point2d) {
            // Intentionally blank
        }

        override fun userDidTapOutside(globeControl: GlobeController, screenLoc: Point2d) {
            Log.d("Maply", "User tapped outside globe.")
        }

        override fun userDidLongPress(globeController: GlobeController, selObjs: Array<SelectedObject>, loc: Point2d, screenLoc: Point2d) {
            // Intentionally blank
        }

        override fun globeDidStartMoving(controller: GlobeController, userInitiated: Boolean) {
            // Intentionally blank
        }

        override fun globeDidStopMoving(controller: GlobeController, corners: Array<Point3d>, userInitiated: Boolean) {

            val center = controller.positionGeo

            Log.v(TAG, String.format("Globe did stop moving (lat: %.6f° lon: %.6f° z: %.6f)",
                    center!!.y * RAD_TO_DEG, center.x * RAD_TO_DEG, center.z))

        }

        override fun globeDidMove(controller: GlobeController, corners: Array<Point3d>, userInitiated: Boolean) {

            //            Point3d center = controller.getPosition();
            //
            //            Log.v(TAG, String.format("Globe did move to (lat: %.6f° lon: %.6f° z: %.6f)",
            //                    center.getY() * RAD_TO_DEG, center.getX() * RAD_TO_DEG, center.getZ()));

            val bb = controller.currentViewGeo
            if (bb != null) {
                val center = bb.middle().toDegrees()
                val span = bb.span().toDegrees()

                Log.v(TAG, String.format("Globe did move to (lat: %.6f° lon: %.6f°), span (lat: %.6f° lon: %.6f°))",
                        center.y * RAD_TO_DEG, center.x * RAD_TO_DEG,
                        span.y * RAD_TO_DEG, span.x * RAD_TO_DEG))
            }
        }
    }


    init {
        setTestName("MBTiles Image Test")
        setDelay(1000)
        this.implementation = MaplyTestCase.TestExecutionImplementation.Both
    }

    @Throws(Exception::class)
    private fun setupImageLayer(baseController: MaplyBaseController, testType: ConfigOptions.TestType): QuadImageTileLayer {

        // We need to copy the file from the asset so that it can be used as a file
        val mbTiles = this.getMbTileFile("mbtiles/geography-class.mbtiles", "geography-class.mbtiles")

        if (!mbTiles.exists()) {
            throw FileNotFoundException(String.format("Could not copy MBTiles asset to \"%s\"", mbTiles.absolutePath))
        }

        Log.d(TAG, String.format("Obtained MBTiles SQLLite database \"%s\"", mbTiles.absolutePath))

        val mbTilesFile = MBTiles(mbTiles)
        val tileSource = MBTilesImageSource(mbTilesFile)
        val imageLayer = QuadImageTileLayer(baseController, tileSource.coordSys, tileSource)
        imageLayer.setCoverPoles(true)
        imageLayer.setHandleEdges(true)

        return imageLayer
    }

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        globeVC.addLayer(setupImageLayer(globeVC, ConfigOptions.TestType.GlobeTest))

        globeVC.gestureDelegate = gestureDelegate

        return true
    }

    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        mapVC.addLayer(setupImageLayer(mapVC, ConfigOptions.TestType.MapTest))
        return true
    }

    // Copy an MBTiles file out of the package for direct use
    @Throws(IOException::class)
    private fun getMbTileFile(assetMbTile: String, mbTileFilename: String): File {

        val wrapper = ContextWrapper(activity)
        val mbTilesDirectory = wrapper.getDir(MBTILES_DIR, Context.MODE_PRIVATE)

        val inStream = activity.assets.open(assetMbTile)
        val of = File(mbTilesDirectory, mbTileFilename)

        if (of.exists()) {
            return of
        }

        val os = FileOutputStream(of)
        val mBuffer = ByteArray(1024)
        var length = inStream.read(mBuffer)
        while (length > 0) {
            os.write(mBuffer, 0, length)
            length = inStream.read(mBuffer)
        }
        os.flush()
        os.close()
        inStream.close()

        return of
    }

    companion object {

        private val RAD_TO_DEG = 180.0 / Math.PI

        private val TAG = "AutoTester"
        private val MBTILES_DIR = "mbtiles"
    }


}
