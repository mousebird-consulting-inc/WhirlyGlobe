//
//  LocationTrackingTestCase.swift
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-23.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

import UIKit

class LocationTrackingTestCase: MaplyTestCase, MaplyLocationTrackerDelegate {

    var segCtrl: UISegmentedControl?
    
    override init() {
        super.init()
        
        self.name = "Location Tracking Test Case"
        self.captureDelay = 4
        self.implementations = [.globe, .map]
    }
    
    func setupLocationTracking(baseVC: MaplyBaseViewController) {
        baseVC.startLocationTracking(with: self, useHeading: true, useCourse: true, lockType: MaplyLocationLockNone)
        
        segCtrl = UISegmentedControl(items: ["No Lock", "North Up", "Heading Up", "Heading Up Forward"])
        segCtrl?.selectedSegmentIndex = 0
        segCtrl?.frame = CGRect(x: 20, y: 90, width: 640, height: 40)
        segCtrl?.addTarget(self, action: #selector(onSegChange), for: .valueChanged)
        
        let font = UIFont.boldSystemFont(ofSize: 16)
        segCtrl?.setTitleTextAttributes([NSFontAttributeName: font], for: UIControlState.normal)
        
        segCtrl?.tintColor = UIColor.white
        baseVC.view.addSubview(segCtrl!)
        
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = StamenWatercolorRemote()
        baseLayer.setUpWithGlobe(globeVC)
        globeVC.keepNorthUp = false
        
        setupLocationTracking(baseVC: globeVC)
        
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), time: 1.0)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = StamenWatercolorRemote()
        baseLayer.setUpWithMap(mapVC)
        
        setupLocationTracking(baseVC: mapVC)
        
        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056), height: 1.0, time: 1.0)
        //mapVC.setZoomLimitsMin(0.01, max: 4.0)
        mapVC.setZoomLimitsMin(0.0005, max: 4.0)
    }
    
    func onSegChange() {
        if (segCtrl?.selectedSegmentIndex == 0) {
            baseViewController?.changeLocationTrackingLockType(MaplyLocationLockNone)
        } else if (segCtrl?.selectedSegmentIndex == 1) {
            baseViewController?.changeLocationTrackingLockType(MaplyLocationLockNorthUp)
        } else if (segCtrl?.selectedSegmentIndex == 2) {
            baseViewController?.changeLocationTrackingLockType(MaplyLocationLockHeadingUp)
        } else {
            baseViewController?.changeLocationTrackingLockType(MaplyLocationLockHeadingUpOffset)
        }
    }
    
    func locationManager(_ manager: CLLocationManager!, didFailWithError error: Error!) {
        print("Location Manager Error", error)
    }
    
    func locationManager(_ manager: CLLocationManager!, didChange status: CLAuthorizationStatus) {
        print("Location Manager status change", status);
    }
}
