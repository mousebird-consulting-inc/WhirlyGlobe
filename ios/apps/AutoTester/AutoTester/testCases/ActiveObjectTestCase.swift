//
//  ActiveObjectTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 1/28/22.
//  Copyright 2022 mousebird consulting. All rights reserved.
//

import Foundation
import WhirlyGlobe

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
        markerTex = inTex
    }

    override func hasUpdate() -> Bool {
        return true
    }
    
    override func update(forFrame frameInfo: UnsafeMutableRawPointer?) {
        guard let vc = viewC else { return }

        let oldObjs = [MaplyComponentObject](compObjs)
        compObjs.removeAll()

        let offset = MaplyCoordinateMakeWithDegrees(Float(-1.0 + 2.0 * drand48()),
                                                    Float(-1.0 + 2.0 * drand48()))
        markerCoord = MaplyCoordinate(x: markerCoord.x + offset.x,
                                      y: markerCoord.y + offset.y)

        let marker = MaplyScreenMarker()
        marker.loc = markerCoord
        marker.image = markerTex
        marker.size = CGSize(width: 64, height: 64)
        marker.color = UIColor.init(white: 1.0, alpha: 0.5)
        marker.layoutImportance = MAXFLOAT

        let desc = [
            kMaplyFade: false,
            kMaplyEnable: false,
        ];

        if let co = vc.addScreenMarkers([marker], desc: desc, mode: .current) {
            compObjs.append(co)
        }

        for i in 0..<2 {
            for j in 0..<2 {
                let lat = 50.0 + Float(i + 2 * j)
                let lon = Float(-110.0)
                var pts = [
                    MaplyCoordinateMakeWithDegrees(lon, lat),
                    MaplyCoordinateMakeWithDegrees(lon + 1, lat),
                    MaplyCoordinateMakeWithDegrees(lon + Float(wideLinePos), lat + 0.1),
                    MaplyCoordinateMakeWithDegrees(lon, lat),
                ]

                let vec = MaplyVectorObject(lineString: &pts, numCoords: (j==0) ? 3 : 4)
                let wideDesc = [
                    kMaplyEnable: false,
                    kMaplyColor: UIColor.red.withAlphaComponent(0.5),
                    kMaplyVecWidth: 20,
                    kMaplyWideVecEdgeFalloff: 5,
                    kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                    kMaplyWideVecMiterLimit: miterLimit,
                    kMaplyWideVecImpl: (i==0) ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
                ]  as [String : Any]
                if let co = vc.addWideVectors([vec], desc: wideDesc, mode: .current) {
                    compObjs.append(co)
                }
            }
        }

        wideLinePos += wideLineDir * wideLineDif
        if wideLinePos > 2 || wideLinePos < -2 {
            wideLineDir = -wideLineDir
            wideLinePos += wideLineDir * wideLineDif
        }
        
        
        vc.enable(compObjs, mode: .current)
        vc.remove(oldObjs, mode: .current)
    }

    private var compObjs = [MaplyComponentObject]()
    private var markerTex: MaplyTexture?
    private var markerCoord = MaplyCoordinate(x: 0, y: 0)
    private var wideLinePos = 0.0
    private let wideLineDif = 0.01
    private var wideLineDir = -1.0
    private var miterLimit = 0.0
}
