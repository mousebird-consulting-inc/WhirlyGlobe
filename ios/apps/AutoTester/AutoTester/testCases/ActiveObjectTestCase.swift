//
//  ActiveObjectTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 1/28/22.
//  Copyright 2022 mousebird consulting. All rights reserved.
//

import Foundation

class ActiveObjectTestCase : MaplyTestCase {
    override init() {
        super.init(name: "Active Object", supporting: [.globe, .map])
    }

    func setupActiveObject(_ viewC: MaplyBaseViewController?) {
        guard let vc = viewC else { return }

        if tex == nil, let img = UIImage(named: "beer-24@2x.png") {
            tex = vc.addTexture(img, desc: nil, mode: .current)
        }
        guard let tex = tex else { return }

        let obj = SimpleActiveObject(viewController: vc, image: tex)
        activeObject = obj
        vc.add(obj)
    }

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        setupActiveObject(globeVC)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        setupActiveObject(mapVC)
    }

    override func stop() {
        if let obj = activeObject {
            baseViewController?.remove(obj)
            activeObject = nil
        }
        if let tex = tex {
            baseViewController?.remove(tex, mode: .any)
            self.tex = nil
        }
        baseCase.stop()
    }

    private var activeObject: SimpleActiveObject?
    private let baseCase = CartoDBLightTestCase()
    private var tex: MaplyTexture?
}

class SimpleActiveObject: MaplyActiveObject {
    init(viewController viewC: MaplyBaseViewController, image inTex: MaplyTexture?) {
        super.init(viewController: viewC)
        tex = inTex
    }

    override func hasUpdate() -> Bool {
        return true
    }
    
    override func update(forFrame frameInfo: UnsafeMutableRawPointer?) {
        guard let vc = viewC else { return }
        // Delete it
        if let compObj = compObj {
            vc.remove([compObj], mode: .current)
            self.compObj = nil
        }

        let offset = MaplyCoordinateMakeWithDegrees(Float(-1.0 + 2.0 * drand48()),
                                                    Float(-1.0 + 2.0 * drand48()))
        coord = MaplyCoordinate(x: coord.x + offset.x, y: coord.y + offset.y)

        let marker = MaplyScreenMarker()
        marker.loc = coord
        marker.image = tex
        marker.size = CGSize(width: 64, height: 64)
        marker.color = UIColor.init(white: 1.0, alpha: 0.5)
        marker.layoutImportance = MAXFLOAT

        let desc = [
            kMaplyFade: false
        ];

        compObj = vc.addScreenMarkers([marker], desc: desc, mode: .current)
    }

    private var coord = MaplyCoordinate(x: 0, y: 0)
    private var compObj: MaplyComponentObject?
    private var tex: MaplyTexture?
}
