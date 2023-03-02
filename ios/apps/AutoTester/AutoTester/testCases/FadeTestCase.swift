//
//  FadeTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 02/07/22.
//  Copyright 2022 mousebird consulting. All rights reserved.
//

import Foundation
import WhirlyGlobe

class FadeTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        self.name = "Fade"
        self.implementations = [.globe,.map]
    }

    func setup() {
        if let vc = baseViewController {
            addStuff(vc)
        }
        timer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { [weak self] _ in
            if let self = self, let vc = self.baseViewController {
                self.addStuff(vc)
            }
        }
    }
    
    func addStuff(_ vc: MaplyBaseViewController) {
        let oldObjs = [MaplyComponentObject](self.objs)
        var newObjs = [MaplyComponentObject]()
        let fillDesc = [
            kMaplyEnable: false,
            kMaplyColor: UIColor.red.withAlphaComponent(0.5),
            kMaplyFilled: true
        ] as [AnyHashable: Any]
        let lineDesc = [
            kMaplyEnable: false,
            kMaplyColor: UIColor.magenta.withAlphaComponent(0.75),
            kMaplyFilled: false
        ] as [AnyHashable: Any]

        do {
            var coords = [
                MaplyCoordinateMakeWithDegrees(-90, 40+offset),
                MaplyCoordinateMakeWithDegrees(-89, 42+offset),
                MaplyCoordinateMakeWithDegrees(-88, 40+offset),
            ]
            let area = MaplyVectorObject.init(areal: &coords, numCoords: Int32(coords.count), attributes: nil)
            // Fade in over two seconds, fade out over two seconds when removed
            let desc = [ kMaplyFade: 2.0 ] as [AnyHashable: Any]
            if let obj = vc.addVectors([area], desc: fillDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
            if let obj = vc.addVectors([area], desc: lineDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
        }

        do {
            var coords = [
                MaplyCoordinateMakeWithDegrees(-88, 40+offset),
                MaplyCoordinateMakeWithDegrees(-87, 42+offset),
                MaplyCoordinateMakeWithDegrees(-86, 40+offset),
            ]
            let area = MaplyVectorObject.init(areal: &coords, numCoords: Int32(coords.count), attributes: nil)
            // Fade in over two seconds
            let desc = [ kMaplyFadeIn: 2.0 ] as [AnyHashable: Any]
            if let obj = vc.addVectors([area], desc: fillDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
            if let obj = vc.addVectors([area], desc: lineDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
        }

        do {
            var coords = [
                MaplyCoordinateMakeWithDegrees(-86, 40+offset),
                MaplyCoordinateMakeWithDegrees(-85, 42+offset),
                MaplyCoordinateMakeWithDegrees(-84, 40+offset),
            ]
            let area = MaplyVectorObject.init(areal: &coords, numCoords: Int32(coords.count), attributes: nil)
            // Fade out over two seconds (when removed)
            let desc = [ kMaplyFadeOut: 2.0 ] as [AnyHashable: Any]
            if let obj = vc.addVectors([area], desc: fillDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
            if let obj = vc.addVectors([area], desc: lineDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
        }

        do {
            var coords = [
                MaplyCoordinateMakeWithDegrees(-84, 40+offset),
                MaplyCoordinateMakeWithDegrees(-83, 42+offset),
                MaplyCoordinateMakeWithDegrees(-82, 40+offset),
            ]
            let area = MaplyVectorObject.init(areal: &coords, numCoords: Int32(coords.count), attributes: nil)
            let desc = [
                // Fade out over two seconds, starting one second from now
                kMaplyFadeOut: 2.0,
                kMaplyFadeOutTime: CFAbsoluteTimeGetCurrent() + 1.0,
            ] as [AnyHashable: Any]
            if let obj = vc.addVectors([area], desc: fillDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
            if let obj = vc.addVectors([area], desc: lineDesc.merging(desc) { $1 }, mode: .current) { newObjs.append(obj) }
        }

        // todo: Add other stuff that supports fades: wide vectors, labels, markers, shapes, ...

        offset = (offset == 0.0) ? 2.0 : 0.0

        vc.enable(newObjs, mode: .current)
        vc.remove(oldObjs)
        objs = newObjs
    }

    override func preSetUp(withGlobe globeVC: WhirlyGlobeViewController) {
        globeVC.fastGestures = true
    }
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-80, 40), height: 0.5, heading: 0, time: 3)
        setup()
    }

    override func preSetUp(withMap mapVC: MaplyViewController) {
        mapVC.fastGestures = true
    }
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        setup()
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-80, 40), height: 0.5, time: 3)
    }

    override func stop() {
        timer?.invalidate()
        timer = nil
        baseViewController?.remove(objs, mode: MaplyThreadMode.current);
        objs.removeAll()
        baseCase.stop()
        super.stop()
    }

    let baseCase = CartoDBLightTestCase()
    var objs = [MaplyComponentObject]()
    var timer: Timer?
    var offset = Float(0.0)
}

