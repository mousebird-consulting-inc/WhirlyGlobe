//
//  AnimationDelegateTestCase.swift
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-29.
//  Copyright 2016-2022 mousebird consulting.
//

import UIKit
import WhirlyGlobe

class AnimationDelegateTestCase: MaplyTestCase {
    override init() {
        super.init()
        
        self.name = "Animation Delegate"
        self.implementations = [.globe, .map]
    }
    
    var baseCase = StamenWatercolorRemote()
    let initPos = MaplyCoordinateMakeWithDegrees(12.5037997, 41.893988)
    let pos = MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222)

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        
        let minZ: Float = 0.001 //globeVC.getZoomLimitsMin()
        let maxZ: Float = 2.0 // globeVC.getZoomLimitsMax()

        globeVC.setPosition(initPos)    // this doesn't seem to work?
        globeVC.height = maxZ
        globeVC.keepNorthUp = false
        globeVC.animate(toPosition:pos, height:minZ, heading:Float.pi/4.0, time:9.0);

        // Do it again with the old, linear zoom and keep-north-up
        Timer.scheduledTimer(withTimeInterval: 10, repeats: false) { [self] _ in
            globeVC.keepNorthUp = true
            globeVC.animationZoomEasing = { (z0,z1,t) in z0 + (z1 - z0) * t }
            globeVC.animate(toPosition: initPos, height: maxZ, heading: Float.pi/4.0, time: 5.0)
        }
        Timer.scheduledTimer(withTimeInterval: 15, repeats: false) { [self] _ in
            globeVC.animate(toPosition: pos, height: minZ, heading: 0.0, time: 5.0)
        }

        // And again, with flair!
        Timer.scheduledTimer(withTimeInterval: 22, repeats: false) { [self] _ in
            globeVC.animationZoomEasing = easeOut
            globeVC.animate(toPosition: initPos, height: maxZ, heading: 0.0, time: 5.0)
        }
        Timer.scheduledTimer(withTimeInterval: 28, repeats: false) { [self] _ in
            globeVC.animationZoomEasing = easeIn
            globeVC.animate(toPosition: pos, height: minZ, heading: 0.0, time: 5.0)
        }
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)

        let minZ = mapVC.getMinZoom()
        let maxZ = Float(2.0)

        // Start high, zoom in
        mapVC.setPosition(initPos)
        mapVC.height = maxZ;
        mapVC.animate(toPosition:pos, height:minZ, heading:Float.pi/4.0, time:9.0);

        // Back out, and in again, with the old, linear zoom
        Timer.scheduledTimer(withTimeInterval: 10, repeats: false) { [self] _ in
            mapVC.animationZoomEasing = { (z0,z1,t) in z0 + (z1 - z0) * t }
            mapVC.animate(toPosition: initPos, height: maxZ, heading: 0.0, time: 5.0)
        }
        Timer.scheduledTimer(withTimeInterval: 15, repeats: false) { [self] _ in
            mapVC.animate(toPosition: pos, height: minZ, heading: 0.0, time: 5.0)
        }

        // And again, with flair!
        Timer.scheduledTimer(withTimeInterval: 22, repeats: false) { [self] _ in
            mapVC.animationZoomEasing = easeOut
            mapVC.animate(toPosition: initPos, height: maxZ, heading: 0.0, time: 5.0)
        }
        Timer.scheduledTimer(withTimeInterval: 28, repeats: false) { [self] _ in
            mapVC.animationZoomEasing = easeIn
            mapVC.animate(toPosition: pos, height: minZ, heading: 0.0, time: 5.0)
        }
    }

    private let easeIn = linearize({ (z0,z1,t) in z0+(z1-z0)*easeInBack(t) })
    private let easeOut = linearize({ (z0,z1,t) in z0+(z1-z0)*(1-easeInBack(1-t)) })

    private static func linearize(_ f: @escaping (Double,Double,Double)->Double) -> ((Double,Double,Double)->Double) {
        return { (z0,z1,t) in exp(f(log(z0),log(z1),t)) }
    }
    // https://easings.net/#easeInBack
    private static func easeInBack(_ t: Double) -> Double {
        let c = 1.70158;
        return ((c + 1) * t - c) * t * t;
    }
}
