//
//  LocationTrackingTestCase.swift
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-23.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

import UIKit

class LocationTrackingTestCase: MaplyTestCase, MaplyLocationTrackerDelegate {

    override init() {
        super.init()
        
        self.name = "Location Tracking Test Case"
        self.captureDelay = 4
        self.implementations = [.globe, .map]
    }
    
    func setupLocationTracking(baseVC: MaplyBaseViewController) {
        baseVC.startLocationTracking(with: self)
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = StamenWatercolorRemote()
        baseLayer.setUpWithGlobe(globeVC)
        
        setupLocationTracking(baseVC: globeVC)
        
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = StamenWatercolorRemote()
        baseLayer.setUpWithMap(mapVC)
        
        setupLocationTracking(baseVC: mapVC)
        
        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
        mapVC.setZoomLimitsMin(0.01, max: 4.0)
    }
    
    func locationManager(_ manager: CLLocationManager!, didFailWithError error: Error!) {
        print("Location Manager Error", error)
    }
    
    func locationManager(_ manager: CLLocationManager!, didChange status: CLAuthorizationStatus) {
        print("Location Manager status change", status);
    }
    
}
