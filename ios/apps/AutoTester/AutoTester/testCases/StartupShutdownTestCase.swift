//
//  StartupShutdownTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 10/29/19.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

import UIKit

class StartupShutdownTestCase: MaplyTestCase {

    override init() {
        super.init()

        self.name = "Repeated Startup/Shutdown"
        self.captureDelay = 4
        self.implementations = [.globe]
    }
    
    func setupLayer(_ baseVC: MaplyBaseViewController) -> MaplyQuadImageTilesLayer {
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        
        let thisCacheDir = "\(cacheDir)/stamentiles/"
        let maxZoom = Int32(16)
        let tileSource = MaplyRemoteTileSource(
            baseURL: "http://tile.stamen.com/watercolor/",
            ext: "png", minZoom: Int32(0), maxZoom: Int32(maxZoom))
        tileSource!.cacheDir = thisCacheDir
        let layer = MaplyQuadImageTilesLayer(tileSource: tileSource!)
        layer!.handleEdges = true
        layer!.drawPriority = kMaplyImageLayerDrawPriorityDefault
        //        layer!.waitLoad = imageWaitLoad
        layer!.singleLevelLoading = false

        return layer!;
    }
    
    func startGlobe() {
        globeViewController = WhirlyGlobeViewController()
        baseViewController = globeViewController
        testView?.addSubview(globeViewController!.view)
        globeViewController!.view.frame = testView!.bounds
        globeViewController!.delegate = self
        // Note: Should also be adding as a child of the view controller

        setUpWithGlobe(globeViewController!)
        
        // Shut it down in a bit
        DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
            self.stopGlobe()
        }
    }
    
    func stopGlobe() {
//        globeViewController?.teardown()
        globeViewController?.view.removeFromSuperview()
        
        // Start it back up again in a bit
        // Note: Check to see if we're still valid here
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.startGlobe()
        }
    }

    // We need to create the globe controller ourselves so we can shut it down
    override func runGlobeTest(withLock lock: DispatchGroup) {

        startGlobe()
    }

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let layer = setupLayer(globeVC)
        
        globeVC.keepNorthUp = true
        globeVC.add(layer)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
    }
}
