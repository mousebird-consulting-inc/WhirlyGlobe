//
//  AnimatedMarkersTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 11/9/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

import Foundation

class AnimatedMarkersTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Animated Markers"
        self.captureDelay = 4
        self.implementations = [.globe, .map]
    }
    
    func insertMarkers (_ arrayComp: NSArray, theViewC: MaplyBaseViewController) {
        let size = CGSize(width: 0.05, height: 0.05);
        let image0 = UIImage(named: "marker-24@2x")
        let image1 = UIImage(named: "marker-stroked-24@2x")
        var markers = [MaplyMarker]()
        for i in 0 ..< arrayComp.count {
            let object = arrayComp[i]
            let marker = MaplyMarker()
            marker.images = [image0!, image1!]
            marker.period = 4.0
            marker.loc = (object as! MaplyVectorObject).center()
            marker.size = size
            marker.userObject = (object as AnyObject).userObject!
            markers.append(marker)
        }
        theViewC.addMarkers(markers, desc: nil)
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer  = VectorsTestCase()
        baseLayer.setUpWithGlobe(globeVC)
        insertMarkers(baseLayer.compList!, theViewC: globeVC)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = VectorsTestCase()
        baseLayer.setUpWithMap(mapVC)
        insertMarkers(baseLayer.compList!, theViewC: mapVC)
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }
    
}
