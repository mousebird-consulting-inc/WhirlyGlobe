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

    private func joins(_ vc: MaplyBaseViewController,
                       bound: MaplyBoundingBox,
                       join: Int, perf: Bool, close: Bool, subdiv: Bool) -> [MaplyComponentObject?] {

        let vsep = 2.5;
        let lat = Float(30.0 + (perf ? 4*vsep : 0.0) + (subdiv ? 2*vsep : 0.0) + (close ? vsep : 0.0))
        let lon = Float(-140.0) + Float(join) * 3.5
        var coords = [
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + 1.5 * Float(joinN) / Float(joinSteps)),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + -0.5 * Float(joinN) / Float(joinSteps)),
            MaplyCoordinateMakeWithDegrees(lon + 2.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + (close ? 0.0 : 2.0), lat),
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))

        if !isVisible(vecObj.boundingBox(), ext: bound) {
            return []
        }

        if (subdiv) {
            self.subdiv(vc, vecObj, 0.0001)
        }

        let desc = [
            kMaplyVecWidth: 40.0,
            kMaplyColor: perf ? UIColor.red.withAlphaComponent(0.35) : UIColor.blue.withAlphaComponent(0.35),
            kMaplyEnable: false,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: joinAttr(join) ?? NSNull(),
            kMaplyDrawableName: String(format: "WideVec-%@%@%@%@",
                                       joinAttr(join) ?? "", perf ? "-perf" : "",
                                       subdiv ? "-subdiv" : "", close ? "-closed" : "")
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: false,
            kMaplyFont: UIFont.systemFont(ofSize: 10.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = String(format: "%@%@%@%@",
                          joinAttr(join) ?? "", perf ? "\nperf" : "",
                          subdiv ? "\nsubdiv" : "", close ? "\nclosed" : "")
        
        return [
            vc.addWideVectors([vecObj], desc: desc, mode: .current),
            vc.addScreenLabels([lbl], desc: lblDesc, mode: .current)
        ]
    }

    private func joinAttr(_ n: Int) -> String? {
        switch (n) {
        case 0: return kMaplyWideVecMiterJoin;
        case 1: return kMaplyWideVecMiterClipJoin;
        case 2: return kMaplyWideVecRoundJoin;
        case 3: return kMaplyWideVecBevelJoin;
        case 4: return kMaplyWideVecNoneJoin;
        default: return nil
        }
    }

    private func joins(_ vc: MaplyBaseViewController,
                       bound: MaplyBoundingBox) -> [MaplyComponentObject?] {

        let yn = [ true, false ]
        let objs = (0..<5).flatMap { join in
            yn.flatMap { perf in
                yn.flatMap { close in
                    yn.flatMap { subdiv in
                        joins(vc, bound: bound, join: join, perf: perf, close: close, subdiv: subdiv)
                    }
                }
            }
        }

        joinN += joinD
        if (joinN > joinSteps || joinN < -joinSteps) {
            joinD = -joinD
            joinN += joinD
        }
        
        return objs
    }

    private func initTex(_ vc: MaplyBaseViewController) {
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
    }

    private func texs(_ vc: MaplyBaseViewController,
                      bound: MaplyBoundingBox,
                      slot: Int32, perf: Bool) -> [MaplyComponentObject?] {
        initTex(vc);

        // Note vary in lon rather than lat so that they are projected identically.
        let lat = Float(30.0)
        let lon = Float(-150.0) + (perf ? 3.0 : 0.0)
        var coords = [
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + 1.0),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + 0.5),
            MaplyCoordinateMakeWithDegrees(lon + 2.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))

        if !isVisible(vecObj.boundingBox(), ext: bound) {
            return []
        }

        subdiv(vc, vecObj, 0.0001)

        let wideDesc = [
            kMaplyZoomSlot: slot,
            kMaplyVecWidth: perf ? ["stops":[[5,3],[12,50]]] : 50,
            kMaplyColor: ["stops":[[0,UIColor.blue.withAlphaComponent(0.75)],[6,UIColor.red.withAlphaComponent(0.75)]]],
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
            kMaplyVecTexture: dashTex ?? NSNull(),
            kMaplyWideVecTexRepeatLen: 64,
            kMaplyWideVecOffset: 0,
            kMaplyWideVecTexOffsetX: 0.0,
            kMaplyWideVecTexOffsetY: 0.1 * Double(texY),
            kMaplyTexWrapX: true,
            kMaplyTexWrapY: true,
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: false,
            kMaplyFont: UIFont.systemFont(ofSize: 10.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = String(format: "tex perf=%d", perf)

        texY += 1

        return [
            vc.addWideVectors([vecObj], desc: wideDesc, mode: .current),
            vc.addScreenLabels([lbl], desc: lblDesc, mode: .current),
        ]
    }

    private func offset(_ vc: MaplyBaseViewController, slot: Int32, perf: Bool) -> [MaplyComponentObject?] {
        initTex(vc);

        // Note vary in lon rather than lat so that they are projected identically.
        let lat = Float(32.0)
        let lon = Float(-150.0) + (perf ? 3.0 : 0.0)
        var coords = [
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + 1.0),
            MaplyCoordinateMakeWithDegrees(lon + 1.0, lat + 0.5),
            MaplyCoordinateMakeWithDegrees(lon + 2.0, lat),
            MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))

        subdiv(vc, vecObj, 0.0001)

        // Baseline with no offset
        let wideDesc = [
            kMaplyZoomSlot: slot,
            kMaplyVecWidth: perf ? ["stops":[[5,3],[15,60]]] : 50,
            kMaplyColor: UIColor.red.withAlphaComponent(0.2),
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
            kMaplyVecTexture: dashTex ?? NSNull(),
            kMaplyWideVecTexRepeatLen: 64,
            kMaplyWideVecOffset: 0,
        ] as [AnyHashable: Any]

        // Offset line
        let wideDescOffs = wideDesc.merging([
            kMaplyColor: UIColor.blue.withAlphaComponent(0.6),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 2,
            kMaplyWideVecOffset: perf ? ["stops":[[8,0],[10,-70],[12,70]]] : 5,
        ], uniquingKeysWith: { (a,b) in b })

        // Centerline
        let desc = [
            kMaplyColor: UIColor.green.withAlphaComponent(0.5),
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 3,
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: true,
            kMaplyFont: UIFont.systemFont(ofSize: 10.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = String(format: "offset perf=%d", perf)
        lbl.layoutImportance = MAXFLOAT

        return [
            vc.addWideVectors([vecObj], desc: wideDesc, mode: .current),
            vc.addWideVectors([vecObj], desc: wideDescOffs, mode: .current),
            vc.addVectors([vecObj], desc: desc, mode: .current),
            vc.addScreenLabels([lbl], desc: lblDesc, mode: .current),
        ]
    }

    private func exprs(_ vc: MaplyBaseViewController, slot: Int32, perf: Bool) -> [MaplyComponentObject?] {
        var coords = [
            MaplyCoordinateMakeWithDegrees(-130 + (perf ? 0 : 30), 60),
            MaplyCoordinateMakeWithDegrees(-140 + (perf ? 0 : 30), 61),
            MaplyCoordinateMakeWithDegrees(-150 + (perf ? 0 : 30), 62),
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))
        subdiv(vc, vecObj, 0.0001)

        let desc = [
            kMaplyColor: UIColor.red,
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 2,
        ] as [AnyHashable: Any];

        let c1 = UIColor.magenta.withAlphaComponent(0.8)
        let c2 = UIColor.blue.withAlphaComponent(0.8)
        let wideDesc = [
            kMaplyDrawPriority:       kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecEdgeFalloff: 1,
            kMaplyZoomSlot:           slot,
            kMaplyVecWidth:           perf ? ["stops":[[2,2],[12,20]]] : 20,
            kMaplyWideVecOffset:      perf ? ["stops":[[2,-20],[6,20]]] : 20,
            kMaplyOpacity:            ["stops":[[2,0.2],[12,0.9]]],
            kMaplyColor:              ["stops":[[2,c1],[12,c2]]],
            kMaplyWideVecImpl:        perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
        ] as [AnyHashable: Any]

        return [
            vc.addVectors([vecObj], desc: desc, mode: .current),
            vc.addWideVectors([vecObj], desc: wideDesc, mode: .current)
        ]
    }

    private func isVisible(_ box: MaplyBoundingBox, ext: MaplyBoundingBox) -> Bool {
        return (ext.ll.x == ext.ur.x) || MaplyBoundingBoxesOverlap(box, ext)
    }

    private func curBound(_ vc: MaplyBaseViewController) -> MaplyBoundingBox {
        if let gv = (vc as? WhirlyGlobeViewController) {
            return gv.getCurrentExtents()
        } else if let mv = (vc as? MaplyViewController) {
            return mv.getCurrentExtents()
        }
        return kMaplyNullBoundingBox;
    }

    private func isVisible(_ vc: MaplyBaseViewController, _ box: MaplyBoundingBox) -> Bool {
        return isVisible(box, ext: curBound(vc))
    }

    private func subdiv(_ vc: MaplyBaseViewController, _ obj: MaplyVectorObject, _ e: Float) {
        if vc is WhirlyGlobeViewController {
            obj.subdivide(toGlobeGreatCircle: e)
        } else {
            obj.subdivide(toFlatGreatCircle: e)
        }
    }

    private func addBox(_ vc: MaplyBaseViewController, _ box: MaplyBoundingBox) -> [MaplyComponentObject?] {
        var coords: [MaplyCoordinate] = [
            box.ll,
            MaplyCoordinateMake(box.ur.x, box.ll.y),
            box.ur,
            MaplyCoordinateMake(box.ll.x, box.ur.y),
            box.ll,
        ]

        let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))
        subdiv(vc, vecObj, 0.01)

        let desc = [
            kMaplyColor: UIColor.red.withAlphaComponent(0.5),
            kMaplyEnable: false,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any];

        return [ vc.addVectors([vecObj], desc: desc, mode: .current) ]
    }

    private func timerTick(_ vc: MaplyBaseViewController) {
        let oldObjs = timerObjs
        timerObjs.removeAll()
        
        let curBound = curBound(vc)

        timerObjs.append(contentsOf: joins(vc, bound: curBound).compactMap { $0 })

        if let slot = zoomSlot {
            timerObjs.append(contentsOf: [true, false].flatMap {
                texs(vc, bound: curBound, slot: slot, perf: $0)
            }.compactMap { $0 })
        }
        vc.enable(timerObjs, mode: .current)
        vc.remove(oldObjs, mode: .current)
    }

    private func wideLineTest(_ vc: MaplyBaseViewController) {
        if zoomSlot == nil {
            zoomSlot = baseViewController?.retainZoomSlotMinZoom(0, maxHeight: 2, maxZoom: 20, minHeight: 0.001);
        }

//        addGeoJson("sawtooth.geojson", dashPattern: nil, width: 50.0, edge: 20.0, simple: false, viewC: vc);
//        addGeoJson("moving-lawn.geojson", viewC: vc);
//        addGeoJson("spiral.geojson", viewC: vc);
//        addGeoJson("square.geojson", dashPattern: [2, 2], width: 10.0, viewC: vc);
//        addGeoJson("track.geojson", viewC: vc);
//        //addGeoJson("uturn2.geojson", dashPattern:[16, 16], width:40, viewC:vc);
//        addGeoJson("USA.geojson", viewC:vc);
//        //addGeoJson("testJson.json", viewC:vc);
//        //addGeoJson("straight.geojson", viewC:vc);
//        //addGeoJson("uturn.geojson", viewC:vc);
//
//        overlap(vc);
//        vecColors(vc);

        if let slot = zoomSlot {
            self.objs.append(contentsOf: [true, false].flatMap {
                offset(vc, slot: slot, perf: $0) +
                exprs(vc, slot: slot, perf: $0)
            }.compactMap { $0 })
        }

        joinTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { [weak self] _ in
            if let vc = self?.baseViewController {
                self?.timerTick(vc)
            }
        }
    }

    override func setUpWithGlobe(_ vc: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(vc)
        wideLineTest(vc)
        //loadShapeFile(vc)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-100.0, 40.0), height: 1.0, heading: 0.0, time: 0.1)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-148.0, 32.0), height: 0.03, heading: 0.0, time: 0.1)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-133.0, 39.0), height: 0.25, heading: 0.0, time: 0.1)
        //vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), height: 0.01, heading: 0.0, time: 0.1)
    }
    
    override func setUpWithMap(_ vc: MaplyViewController) {
        baseCase.setUpWithMap(vc)
        wideLineTest(vc)
        //loadShapeFile(vc)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-100.0, 40.0), height: 1.0, heading: 0.0, time: 0.1)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-148.0, 32.0), height: 0.03, heading: 0.0, time: 0.1)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-133.0, 39.0), height: 0.25, heading: 0.0, time: 0.1)
        //vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), height: 0.01, heading: 0.0, time: 0.1)
    }

    override func stop() {
        joinTimer?.invalidate()
        if let vc = baseViewController {
            vc.remove(objs + timerObjs, mode: .current)
            objs.removeAll()
            timerObjs.removeAll()

            if let tex = dashTex {
                vc.remove(tex, mode: .current)
                dashTex = nil
            }
            
            if let slot = zoomSlot {
                vc.releaseZoomSlotIndex(slot)
                zoomSlot = nil
            }
        }
        baseCase.stop()
    }

    private var zoomSlot: Int32?
    private var dashTex: MaplyTexture?
    private var objs = [MaplyComponentObject]()
    private var joinTimer: Timer?
    private var timerObjs = [MaplyComponentObject]()
    private var texY = 0
    private var joinN = 15
    private var joinD = 1
    private let joinSteps = 30
    private let baseCase = GeographyClassTestCase()
}
