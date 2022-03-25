//
//  WideVectorsTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 3/25/22.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

import Foundation

class WideVectorsTestCase : WideVectorsTestCaseBase
{
    override init() {
        super.init(name: "Wide Vectors", supporting: [.map, .globe])
    }

    private func joins(_ vc: MaplyBaseViewController, join: Int, perf: Bool, close: Bool, subdiv: Bool) -> [MaplyComponentObject] {
        let lat = Float(30.0) + (perf ? 2.0 : 0.0) + (subdiv ? 4.0 : 0.0) + (close ? 8.0 : 0.0)
        let lon = Float(-140.0) + Float(join) * 3.0
        var coords = [
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + Float(joinN) / 100.0),
            MaplyCoordinateMakeWithDegrees(lon + 2.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count - (close ? 0 : 1)))

        if (subdiv) {
            if vc is WhirlyGlobeViewController {
                vecObj.subdivide(toGlobeGreatCircle: 0.0001)
            } else {
                vecObj.subdivide(toFlatGreatCircle: 0.0001)
            }
        }

        let desc = [
            kMaplyVecWidth: 20.0,
            kMaplyColor: UIColor.red.withAlphaComponent(0.5),
            kMaplyEnable: false,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: joinAttr(join) ?? NSNull()
        ] as [AnyHashable: Any]

        let co = vc.addWideVectors([vecObj], desc: desc, mode: .current)

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: false,
            kMaplyFont: UIFont.systemFont(ofSize: 10.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = String(format: "%@ p=%d s=%d", joinAttr(join) ?? "", perf, subdiv)
        let lco = vc.addScreenLabels([lbl], desc: lblDesc, mode: .current)
        
        return [co, lco].compactMap { $0 }
    }

    private func joinAttr(_ n: Int) -> String? {
        switch (n) {
        case 0: return kMaplyWideVecBevelJoin;
        case 1: return kMaplyWideVecMiterJoin;
        default: return nil
        }
    }

    private func texs(_ vc: MaplyBaseViewController, perf: Bool) -> [MaplyComponentObject] {
        if dashTex == nil {
            let b = MaplyLinearTextureBuilder()
            b.setPattern([1, 1])
            if let img = b.makeImage() {
                let desc = [
                    kMaplyTexWrapY: true,
                    kMaplyTexFormat: MaplyQuadImageFormat.imageIntRGBA,
                ] as [AnyHashable: Any]
                dashTex = vc.addTexture(img, desc: desc, mode: .current)
            }
        }

        let lat = Float(30.0) + (perf ? 2.0 : 0.0)
        let lon = Float(-150.0)
        var coords = [
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + 1.0),
            MaplyCoordinateMakeWithDegrees(lon + 2.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))

        if vc is WhirlyGlobeViewController {
            vecObj.subdivide(toGlobeGreatCircle: 0.0001)
        } else {
            vecObj.subdivide(toFlatGreatCircle: 0.0001)
        }

        let desc = [
            kMaplyVecWidth: 20.0,
            kMaplyColor: UIColor.red.withAlphaComponent(0.5),
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: kMaplyWideVecBevelJoin,
            kMaplyVecTexture: dashTex ?? NSNull()
        ] as [AnyHashable: Any]

        let co = vc.addWideVectors([vecObj], desc: desc, mode: .current)

        return [co].compactMap { $0 }
    }

    private func joins(_ vc: MaplyBaseViewController, add: Bool = true) {
        let oldObjs = joinObjs
        joinObjs.removeAll()
        
        if (add) {
            let yn = [ true, false ]
            (0..<2).forEach { join in
                yn.forEach { perf in
                    yn.forEach { close in
                        yn.forEach { subdiv in
                            joinObjs.append(contentsOf: joins(vc, join: join, perf: perf, close: close, subdiv: subdiv))
                        }
                    }
                }
            }
            vc.enable(joinObjs, mode: .current)
        }
        vc.remove(oldObjs, mode: .current)

        joinN += joinD
        if (joinN > 100 || joinN < -100) {
            joinD = -joinD
            joinN += joinD
        }
    }

    private func wideLineTest(_ vc: MaplyBaseViewController) {
        addGeoJson("sawtooth.geojson", dashPattern: nil, width: 50.0, edge: 20.0, simple: false, viewC: vc);
        addGeoJson("moving-lawn.geojson", viewC: vc);
        addGeoJson("spiral.geojson", viewC: vc);
        addGeoJson("square.geojson", dashPattern: [2, 2], width: 10.0, viewC: vc);
        addGeoJson("track.geojson", viewC: vc);
        //addGeoJson("uturn2.geojson", dashPattern:[16, 16], width:40, viewC:vc);

        addGeoJson("USA.geojson", viewC:vc);

        //addGeoJson("testJson.json", viewC:vc);
        //addGeoJson("straight.geojson", viewC:vc);
        //addGeoJson("uturn.geojson", viewC:vc);

        overlap(vc);
        vecColors(vc);
        objs.append(contentsOf: texs(vc, perf: false))
        objs.append(contentsOf: texs(vc, perf: true))

        // Dynamic properties require a zoom slot, which may not be set up yet
        baseCase.getLoader()?.addPostInitBlock { [weak self] in
            guard let self = self, let loader = self.baseCase.getLoader() else { return }
            self.exprs(vc, withLoader: loader, perf: false)
            self.exprs(vc, withLoader: loader, perf: true)
        }

        joinTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
            if let self = self {
                self.joins(vc)
            }
        }
    }

    override func setUpWithGlobe(_ vc: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(vc)
        wideLineTest(vc)
        loadShapeFile(vc)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-100.0, 40.0), height: 1.0, heading: 0.0, time: 0.1)
        //vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), height: 0.01, heading: 0.0, time: 0.1)
    }
    
    override func setUpWithMap(_ vc: MaplyViewController) {
        baseCase.setUpWithMap(vc)
        wideLineTest(vc)
        loadShapeFile(vc)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-100.0, 40.0), height: 1.0, heading: 0.0, time: 0.1)
        //vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), height: 0.01, heading: 0.0, time: 0.1)
    }

    override func stop() {
        joinTimer?.invalidate()
        if let vc = baseViewController {
            joins(vc, add: false);
            vc.remove(objs, mode: .current)
            objs.removeAll()
        }
        baseCase.stop()
    }

    private var dashTex: MaplyTexture?
    private var objs = [MaplyComponentObject]()
    private var joinTimer: Timer?
    private var joinObjs = [MaplyComponentObject]()
    private var joinN = 0
    private var joinD = 1
    private let baseCase = GeographyClassTestCase()
}
