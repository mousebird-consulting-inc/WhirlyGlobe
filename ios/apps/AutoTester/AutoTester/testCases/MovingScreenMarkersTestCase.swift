//
//  MovingScreenMarkersTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 9 Feb. 2021
//  Copyright © 2021 mousebird consulting.
//

import Foundation

class MovingScreenMarkersTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Moving Screen Markers"
        self.implementations = [.globe, .map]
    }
    
    let baseCase : VectorsTestCase = VectorsTestCase()
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        setUp(globeVC)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(0, 10), height:0.5, heading:0, time:0.5)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        setUp(mapVC)
        mapVC.setPosition(MaplyCoordinateMakeWithDegrees(0, 10), height: 0.5)
    }
    
    func setUp(_ vc: MaplyBaseViewController) {
        self.markerObj = makeMarkers()
        timer = Timer.scheduledTimer(withTimeInterval: duration, repeats: true) { [weak self] _ in
            guard let self = self else { return }
            self.clearMarkers()
            self.markerObj = self.makeMarkers()
        }
    }
    
    func clearMarkers() {
        if let obj = markerObj {
            baseViewController?.remove(obj)
            markerObj = nil
        }
    }
    
    func makeMarkers() -> MaplyComponentObject? {
        let pts = [
            MaplyCoordinateMakeWithDegrees(0.0, 0.0),
            MaplyCoordinateMakeWithDegrees(10.0, 10.0),
            MaplyCoordinateMakeWithDegrees(0.0, 20.0),
            MaplyCoordinateMakeWithDegrees(-10.0, 10.0)
        ]
        let images = [
            UIImage(named: "marker-24@2x")!,
            UIImage(named: "marker-stroked-24@2x")!
        ]
        let markers = (0..<pts.count).map { (i) -> MaplyMovingScreenMarker in
            let marker = MaplyMovingScreenMarker()
            marker.duration = duration
            marker.period = duration / 2
            marker.loc = pts[i]
            marker.endLoc = pts[(i + 1) % pts.count]
            marker.size = CGSize(width: 32, height: 32)
            marker.images = images
            marker.layoutImportance = Float.greatestFiniteMagnitude
            return marker
        }
        return baseViewController?.addScreenMarkers(markers, desc: [:])
    }
    
    override func stop() {
        timer?.invalidate()
        timer = nil
        clearMarkers()
    }
    
    let duration = 5.0
    var timer: Timer?
    var markerObj: MaplyComponentObject?
}
