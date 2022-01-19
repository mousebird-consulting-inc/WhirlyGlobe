//
//  LayerStartupShutdownTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 10/30/19.
//  Copyright © 2019-2022 mousebird consulting. All rights reserved.
//

import UIKit

class LayerStartupShutdownTestCase: MaplyTestCase {

    override init() {
        super.init()

        self.name = "Repeated Layer Startup/Shutdown"
        self.implementations = testCase.implementations
    }
    
//    var testCase = StamenWatercolorRemote()
//    var testCase = GeographyClassTestCase()
    var testCase = VectorMBTilesTestCase()

    var run = true

    func startGlobeLayer() {
        self.testCase.globeViewController = globeViewController
        self.testCase.baseViewController = globeViewController
        self.testCase.setUpWithGlobe(self.globeViewController!)

        run = true

        // Shut it down in a bit
        DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
            if self.run {
                self.stopGlobeLayer()
            }
        }
    }
    
    func stopGlobeLayer() {
        self.testCase.stop()

        // Start it back up again in a bit
        // Note: Check to see if we're still valid here
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            if self.run {
                self.startGlobeLayer()
            }
        }
    }
    
    func startMapLayer() {
        self.testCase.mapViewController = mapViewController
        self.testCase.baseViewController = mapViewController
        self.testCase.setUpWithMap(self.mapViewController!)

        if let vc = baseViewController,
           let image = UIImage(named: "marker-stroked-24@2x"),
           let tex = vc.addTexture(image, desc: nil, mode: .current) {
            for i in (0..<100) {
                let marker = MaplyScreenMarker()
                marker.loc = MaplyCoordinateMakeWithDegrees(Float(i), Float(i))
                marker.size = CGSize(width: 32, height: 32)
                marker.image = tex
                marker.layoutImportance = Float.infinity
                if let co = vc.addScreenMarkers([marker], desc: nil, mode: .any) {
                    vc.remove([co], mode: .any)
                }
            }
        }

        // Shut it down in a bit
        DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
            if self.run {
                self.stopMapLayer()
            }
        }
    }
    
    func stopMapLayer() {
        self.testCase.stop()
        
        // Start it back up again in a bit
        // Note: Check to see if we're still valid here
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            if self.run {
                self.startMapLayer()
            }
        }
    }

    // We need to create the globe controller ourselves so we can shut it down
    override func startGlobe(_ nav: UINavigationController) {
        globeViewController = WhirlyGlobeViewController()
        baseViewController = globeViewController
        nav.pushViewController(baseViewController!, animated: true)
        _ = baseViewController!.view
        globeViewController!.delegate = self
        // Note: Should also be adding as a child of the view controller

        startGlobeLayer()
    }
    
    // Likewise for the map
    override func startMap(_ nav: UINavigationController) {
        mapViewController = MaplyViewController()
        baseViewController = mapViewController
        nav.pushViewController(baseViewController!, animated: true)
        _ = baseViewController!.view
        mapViewController!.delegate = self
        // Note: Should also be adding as a child of the view controller
        
        startMapLayer()
    }

    override func stop() {
        run = false
    }
}
