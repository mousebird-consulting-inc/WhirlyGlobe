//
//  AnimationDelegateTestCase.swift
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-29.
//  Copyright © 2016-2021 mousebird consulting.
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
        globeVC.height = 1.0;
        globeVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height:0.01, heading:Float.pi/4.0, time:5.0);
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        mapVC.height = 1.0;
        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222), height:0.01, heading:Float.pi/4.0, time:5.0);
    }

}
