//
//  GlobeRotationTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 2/9/21.
//  Copyright © 2021 mousebird consulting. All rights reserved.
//

import Foundation

class GlobeRotationTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Globe Rotation (#1286)"
        self.implementations = [.globe]
    }
    
    let baseCase : VectorsTestCase = VectorsTestCase()
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(0, 20), height:0.75, heading:0, time:0.5)
    }
    
    override func stop() {
    }
}
