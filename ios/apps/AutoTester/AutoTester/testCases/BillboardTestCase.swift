//
//  BillboardTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 6/14/19.
//  Copyright 2019-2022 mousebird consulting. All rights reserved.
//

import Foundation
import WhirlyGlobe

class BillboardTestCase : MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Billboards"
        self.implementations = [.globe]
    }
    
    func insertBillboards(_ arrayComp: [MaplyVectorObject], theViewC: MaplyBaseViewController) {
        let size = CGSize(width: 0.05, height: 0.05);
        var billboards = [MaplyBillboard]()
        guard let image0 = UIImage(named: "marker-24@2x") else {
            return
        }
        let tex0 = theViewC.addTexture(image0, desc: nil, mode: .current)
        for i in 0 ..< arrayComp.count {
            let vecObj = arrayComp[i]
            guard let bboard = MaplyBillboard.init(image: tex0 as Any, color: UIColor.white, size: size) else {
                continue
            }
            let centroid = vecObj.centroid()
            bboard.center = MaplyCoordinate3dMake(centroid.x, centroid.y, 0.0)
            bboard.userObject = vecObj.attributes?["title"]
            billboards.append(bboard)
        }
        theViewC.addBillboards(billboards, desc: [kMaplyBillboardOrient: kMaplyBillboardOrientEye], mode: .any)
    }
    
    let baseCase = VectorsTestCase()
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        insertBillboards(baseCase.vecList as! [MaplyVectorObject], theViewC: globeVC)
        globeVC.setTiltMinHeight(0.001, maxHeight: 0.1, minTilt: -80*Float.pi/180, maxTilt: 0.0)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }
}
