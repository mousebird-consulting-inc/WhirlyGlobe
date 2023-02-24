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

    private func widths(_ vc: MaplyBaseViewController, perf: Bool) -> [MaplyComponentObject?] {
        initTex(vc)

        return (0..<6).flatMap { j -> [MaplyComponentObject] in
            (0..<20).flatMap { i -> [MaplyComponentObject] in
                let vsep = Float(0.1)
                let hsep = Float(3.0)
                let lat = Float(60.0) + vsep * Float(i)
                let lon = Float(140.0) + (perf ? hsep * 1.1 : 0) + (Float(j) * hsep * 2.2)
                var coords = [
                    MaplyCoordinateMakeWithDegrees(lon + 0.0, lat),
                    MaplyCoordinateMakeWithDegrees(lon + hsep, lat),
                ]

                let vecObj = MaplyVectorObject(lineString: &coords, numCoords: Int32(coords.count))
                subdiv(vc, vecObj, 0.0001)

                let w = Float(i) / 4
                let e = Float(j) / 2
                
                let desc = [
                    kMaplyVecWidth: w,
                    kMaplyWideVecEdgeFalloff: e,
                    kMaplyColor: UIColor.red.withAlphaComponent(0.8),
                    kMaplyEnable: true,
                    kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
                    kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
                    kMaplyDrawableName: String(format: "WideVec-width%d%@", i, perf ? "-perf" : "")
                ] as [AnyHashable: Any]

                let lblDesc = [
                    kMaplyTextColor: UIColor.magenta,
                    kMaplyEnable: true,
                    kMaplyFont: UIFont.systemFont(ofSize: 8.0),
                    kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
                ] as [AnyHashable: Any]
                let lbl = MaplyScreenLabel()
                lbl.loc = MaplyCoordinateMakeWithDegrees(lon, lat + vsep / 3)
                lbl.text = String(format: "%.2f/%.2f%@", w, e, perf ? " P" : " L")
                
                return [
                    vc.addWideVectors([vecObj], desc: desc, mode: .current),
                    vc.addScreenLabels([lbl], desc: lblDesc, mode: .current)
                ].compactMap { $0 }
            }
        }
    }
    
    private func joins(_ vc: MaplyBaseViewController, bound: MaplyBoundingBox, slot: Int,
                       join: Int, perf: Bool, close: Bool) -> [MaplyComponentObject?] {

        // Legacy doesn't support these, don't bother with them
        if (!perf && [1,2,4].contains(join)) {
            return []
        }

        let vsep = 2.5;
        let lat = Float(30.0 + (perf ? 2*vsep : 0.0) + (close ? vsep : 0.0))
        let lon = Float(-145.0) + Float(join) * 3.0
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

        self.subdiv(vc, vecObj, 0.0001)

        let props = [
            joinAttr(join) ?? "",
            perf ? "perf" : "",
            close ? "closed" : "",
            joinClip ? "clip" : "",
        ]
        
        let desc = [
            kMaplyEnable: false,
            kMaplyColor: UIColor.green.withAlphaComponent(0.5),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 2,
        ] as [AnyHashable: Any]
        let wideDesc = [
            kMaplyEnable: false,
            kMaplyZoomSlot: slot,
            kMaplyVecWidth: perf ? ["stops":[[2,5],[12,80]]] : 20,
            kMaplyColor: (perf ? (joinClip ? UIColor.magenta : UIColor.red) : UIColor.blue).withAlphaComponent(0.35),
            kMaplyWideVecOffset: perf ? ["stops":[[12,0],[13,-50],[14,50],[15,0]]] : 0,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecFallbackMode: joinClip ? kMaplyWideVecFallbackClip : kMaplyWideVecFallbackNone,
            kMaplyWideVecJoinType: joinAttr(join) ?? NSNull(),
            kMaplyWideVecMiterLimit: 1.0,
            kMaplyDrawableName: "WideVec-" + props.joined(separator: "-"),
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: false,
            kMaplyFont: UIFont.systemFont(ofSize: 8.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = props.joined(separator: "\n")
        
        return [
            vc.addVectors([vecObj], desc: desc, mode: .current),
            vc.addWideVectors([vecObj], desc: wideDesc, mode: .current),
            vc.addScreenLabels([lbl], desc: lblDesc, mode: .current)
        ]
    }

    private func joins(_ vc: MaplyBaseViewController, slot: Int,
                       bound: MaplyBoundingBox) -> [MaplyComponentObject?] {
        (0..<6).flatMap { join in
            [ true, false ].flatMap { perf in
                [ true, false ].flatMap { close in
                    joins(vc, bound: bound, slot: slot, join: join, perf: perf, close: close)
                }
            }
        }
    }

    private func step() {
        joinN += joinD
        if (joinN < -joinSteps) {
            joinClip = !joinClip
        }
        if (joinN > joinSteps || joinN < -joinSteps) {
            joinD = -joinD
            joinN += joinD
        }
    }

    private func caps(_ vc: MaplyBaseViewController, bound: MaplyBoundingBox, slot: Int,
                       cap: Int, perf: Bool, close: Bool) -> [MaplyComponentObject?] {

        // Legacy doesn't support these, don't bother showing them
        if (!perf) {
            return []
        }

        let vsep = 2.5;
        let lat = Float(15.0 + (perf ? 2*vsep : 0.0) + (close ? vsep : 0.0))
        let lon = Float(-140.0) + Float(cap) * 3.5
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

        let desc = [
            kMaplyZoomSlot: slot,
            kMaplyVecWidth: perf ? ["stops":[[2,5],[12,80]]] : 40,
            kMaplyColor: perf ? UIColor.red.withAlphaComponent(0.35) : UIColor.blue.withAlphaComponent(0.35),
            kMaplyEnable: false,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: kMaplyWideVecBevelJoin,
            kMaplyWideVecFallbackMode: kMaplyWideVecFallbackClip,
            kMaplyWideVecLineCapType: capAttr(cap) ?? NSNull(),
            kMaplyWideVecOffset: perf ? ["stops":[[12,0],[13,-50],[14,50],[15,0]]] : 0,
            kMaplyVecTexture: dashTex ?? NSNull(),
            kMaplyWideVecTexRepeatLen: close ? 50 : length(coords, close: false) / 15000,
            kMaplyDrawableName: String(format: "WideVec-%@%@%@",
                                       capAttr(cap) ?? "", perf ? "-perf" : "",
                                       close ? "-closed" : "")
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: false,
            kMaplyFont: UIFont.systemFont(ofSize: 8.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = String(format: "%@%@%@",
                          capAttr(cap) ?? "", perf ? "\nperf" : "",
                          close ? "\nclosed" : "")
        
        return [
            vc.addWideVectors([vecObj], desc: desc, mode: .current),
            vc.addScreenLabels([lbl], desc: lblDesc, mode: .current)
        ]
    }

    private func caps(_ vc: MaplyBaseViewController, slot: Int,
                      bound: MaplyBoundingBox) -> [MaplyComponentObject?] {

        let yn = [ true, false ]
        let objs = (0..<3).flatMap { cap in
            yn.flatMap { perf in
                yn.flatMap { close in
                    yn.flatMap { subdiv in
                        caps(vc, bound: bound, slot: slot, cap: cap, perf: perf, close: close)
                    }
                }
            }
        }

        return objs
    }

    private func length(_ pts: [MaplyCoordinate], close: Bool) -> Double {
        zip(close ? pts : pts.dropLast(), pts.dropFirst() + (close ? [pts.first!] : []))
        .reduce(0.0) { (dist: Double, p) in dist + GeoLibDistanceF(p.0, p.1) }
    }
    
    private func joinAttr(_ n: Int) -> String? {
        switch (n) {
        case 0: return kMaplyWideVecMiterJoin;
        case 1: return kMaplyWideVecMiterClipJoin;
        case 2: return kMaplyWideVecMiterSimpleJoin;
        case 3: return kMaplyWideVecRoundJoin;
        case 4: return kMaplyWideVecBevelJoin;
        case 5: return kMaplyWideVecNoneJoin;
        default: return nil
        }
    }

    private func capAttr(_ n: Int) -> String? {
        switch (n) {
        case 0: return kMaplyWideVecButtCap;
        case 1: return kMaplyWideVecRoundCap;
        case 2: return kMaplyWideVecSquareCap;
        default: return nil
        }
    }

    private func initSlot() {
        if zoomSlot == nil,
           let slot = baseViewController?.retainZoomSlotMinZoom(0, maxHeight: 2, maxZoom: 20, minHeight: 0.001) {
            
            zoomSlot = Int(exactly: slot)
        }
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
                      slot: Int, perf: Bool) -> [MaplyComponentObject?] {
        initTex(vc);

        // Note vary in lon rather than lat so that they are projected identically.
        let lat = Float(30.0)
        let lon = Float(-160.0) + (perf ? 3.0 : 0.0)
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
            kMaplyVecWidth: perf ? ["stops":[[5,3],[12,50]]] : 20,
            kMaplyColor: perf ? ["stops":[[3,UIColor.blue.withAlphaComponent(0.75)],
                                          [8,UIColor.red.withAlphaComponent(0.75)]]] :
                                UIColor.red.withAlphaComponent(0.75),
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
            kMaplyWideVecMiterLimit: 5.0,
            kMaplyVecTexture: dashTex ?? NSNull(),
            kMaplyWideVecTexRepeatLen: 64,
            kMaplyWideVecOffset: 0,
            kMaplyWideVecTexOffsetX: 0.0,
            kMaplyWideVecTexOffsetY: 0.1 * Double(texY) / (perf ? 1 : 10),  // todo: why is this needed?
            kMaplyDrawableName: "WideVec-Tex",
            kMaplyTexWrapX: true,
            kMaplyTexWrapY: true,
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: false,
            kMaplyFont: UIFont.systemFont(ofSize: 8.0),
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

    private func offset(_ vc: MaplyBaseViewController, slot: Int, perf: Bool, fudge: Bool) -> [MaplyComponentObject?] {
        initTex(vc);

        if (fudge && !perf) {
            return []
        }

        // Note vary in lon rather than lat so that they are projected identically.
        let lat = Float(35.0) + (fudge ? 2.0 : 0.0)
        let lon = Float(-160.0) + (perf ? 5.0 : 0.0)
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
            kMaplyVecWidth: perf ? ["stops":[[5,3],[15,60]]] : 20,
            kMaplyColor: UIColor.red.withAlphaComponent(0.2),
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            kMaplyWideVecImpl: perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
            kMaplyVecTexture: dashTex ?? NSNull(),
            kMaplyWideVecTexRepeatLen: 64,
            kMaplyWideVecOffset: 0,
            kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
            kMaplyWideVecMiterLimit: 4,
            kMaplyWideVecFallbackMode: fudge ? kMaplyWideVecFallbackClip : kMaplyWideVecFallbackNone,
            kMaplyDrawableName: "WideVec-Offset-Baseline",
        ] as [AnyHashable: Any]

        // Offset line
        let wideDescOffs = wideDesc.merging([
            kMaplyColor: UIColor.blue.withAlphaComponent(0.6),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 2,
            kMaplyWideVecOffset: perf ? ["stops":[[8,0],[10,-70],[12,70]]] : 40,
            kMaplyDrawableName: String(format: "WideVec-Offset%@%@", perf ? "-perf" : "", fudge ? "-clip" : ""),
        ], uniquingKeysWith: { (a,b) in b })

        let wideDescOffs2 = wideDescOffs.merging([kMaplyWideVecOffset: -40], uniquingKeysWith: { (a,b) in b })

        // Centerline
        let desc = [
            kMaplyColor: UIColor.green.withAlphaComponent(0.5),
            kMaplyEnable: true,
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 3,
        ] as [AnyHashable: Any]

        let lblDesc = [
            kMaplyTextColor: UIColor.magenta,
            kMaplyEnable: true,
            kMaplyFont: UIFont.systemFont(ofSize: 8.0),
            kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault,
        ] as [AnyHashable: Any]
        let lbl = MaplyScreenLabel()
        lbl.loc = MaplyCoordinateMakeWithDegrees(lon - 0.2, lat - 0.2)
        lbl.text = String(format: "offset%@%@", perf ? "-perf" : "", fudge ? "-clip" : "")
        lbl.layoutImportance = MAXFLOAT

        return [
            vc.addWideVectors([vecObj], desc: wideDesc, mode: .current),
            vc.addWideVectors([vecObj], desc: wideDescOffs, mode: .current),
            vc.addWideVectors(perf ? [] : [vecObj], desc: wideDescOffs2, mode: .current),
            vc.addVectors([vecObj], desc: desc, mode: .current),
            vc.addScreenLabels([lbl], desc: lblDesc, mode: .current),
        ]
    }

    private func exprs(_ vc: MaplyBaseViewController, slot: Int, perf: Bool) -> [MaplyComponentObject?] {
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
        let slot = zoomSlot ?? -1

        add(to: &timerObjs, joins(vc, slot: slot, bound: curBound))
        add(to: &timerObjs, caps(vc, slot: slot, bound: curBound))

        if let slot = zoomSlot {
            add(to: &timerObjs, [true, false].flatMap {
                texs(vc, bound: curBound, slot: slot, perf: $0)
            })
        }
        vc.enable(timerObjs, mode: .current)
        vc.remove(oldObjs, mode: .current)
        
        step()
    }

    private func wideLineTest(_ vc: MaplyBaseViewController) {
        initSlot()

        overlap(vc);
        vecColors(vc);

        if let slot = zoomSlot {
            add(to: &self.objs, [true, false].flatMap {
                offset(vc, slot: slot, perf: $0, fudge: false) +
                offset(vc, slot: slot, perf: $0, fudge: true) +
                exprs(vc, slot: slot, perf: $0)
            })
        }

        DispatchQueue.main.async {
            self.joinTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { [weak self] _ in
                if let vc = self?.baseViewController {
                    self?.timerTick(vc)
                }
            }
        }

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

        add(to: &objs, [true, false].flatMap {
            widths(vc, perf: $0)
        })

        loadShapeFile(vc)
    }

    override func setUpWithGlobe(_ vc: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(vc)
        DispatchQueue.global(qos: .background).async {
            self.wideLineTest(vc)
        }
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), height: 0.01, heading: 0.0, time: 0.1)
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-140.0, 42.0), height: 0.2, heading: 0.0, time: 0.1)
    }
    
    override func setUpWithMap(_ vc: MaplyViewController) {
        baseCase.setUpWithMap(vc)
        DispatchQueue.global(qos: .background).async {
            self.wideLineTest(vc)
        }
        vc.animate(toPosition: MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793), height: 0.01, heading: 0.0, time: 0.1)
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
                vc.releaseZoomSlotIndex(Int32(slot))
                zoomSlot = nil
            }
        }
        baseCase.stop()
    }

    private func add(to a: inout [MaplyComponentObject], _ items: [MaplyComponentObject]) {
        a.append(contentsOf: items)
    }
    private func add(to a: inout [MaplyComponentObject], _ items: [MaplyComponentObject?]) {
        a.append(contentsOf: items.compactMap { $0 })
    }

    private var zoomSlot: Int?
    private var dashTex: MaplyTexture?
    private var objs = [MaplyComponentObject]()
    private var joinTimer: Timer?
    private var timerObjs = [MaplyComponentObject]()
    private var texY = 0
    private var joinN = 15
    private var joinD = 1
    private var joinClip = true
    private let joinSteps = 20
    private let baseCase = GeographyClassTestCase()
}
