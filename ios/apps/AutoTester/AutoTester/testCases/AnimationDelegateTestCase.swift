//
//  AnimationDelegateTestCase.swift
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-29.
//  Copyright © 2016-2017 mousebird consulting.
//

import UIKit

class AnimationDelegateTestCase: MaplyTestCase {
    override init() {
        super.init()
        
        self.name = "Animation Delegate"
        self.implementations = [.globe, .map]
    }
    
    var baseCase = StamenWatercolorRemote()
   
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        
        globeVC.keepNorthUp = false
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.290,37.7793), height: 0.005, heading: 3.0*Float(Double.pi)/Float(4.0), time: 3.0)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.290,37.7793), height: 0.005, heading: 3.0*Float(Double.pi)/Float(4.0), time: 3.0)
        
    }

}
