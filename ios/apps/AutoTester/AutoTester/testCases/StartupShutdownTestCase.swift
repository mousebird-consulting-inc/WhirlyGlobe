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
        self.implementations = [.globe,.map]
    }
    
    var testCase = VectorMBTilesTestCase()
        
    func startGlobe() {
        globeViewController = WhirlyGlobeViewController()
        baseViewController = globeViewController
        testView?.addSubview(globeViewController!.view)
        globeViewController!.view.frame = testView!.bounds
        globeViewController!.delegate = self
        // Note: Should also be adding as a child of the view controller

        testCase.setUpWithGlobe(globeViewController!)
        
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
    
    func startMap() {
        mapViewController = MaplyViewController()
        baseViewController = mapViewController
        testView?.addSubview(mapViewController!.view)
        mapViewController!.view.frame = testView!.bounds
        mapViewController!.delegate = self
        // Note: Should also be adding as a child of the view controller

        testCase.setUpWithMap(mapViewController!)
        
        // Shut it down in a bit
        DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
            self.stopMap()
        }
    }
    
    func stopMap() {
        //        globeViewController?.teardown()
        mapViewController?.view.removeFromSuperview()
        
        // Start it back up again in a bit
        // Note: Check to see if we're still valid here
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.startMap()
        }
    }

    // We need to create the globe controller ourselves so we can shut it down
    override func runGlobeTest(withLock lock: DispatchGroup) {
        startGlobe()
    }
    
    // Likewise for the map
    override func runMapTest(withLock lock: DispatchGroup) {
        startMap()
    }

}
