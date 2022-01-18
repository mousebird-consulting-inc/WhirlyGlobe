//
//  MovingScreenLabelsTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015-2017 mousebird consulting.
//

import UIKit

class MovingScreenLabelsTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Moving Screen Labels"
		self.implementations = [.globe, .map]
	}
    
    let baseCase : VectorsTestCase = VectorsTestCase()

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        setUp(globeVC)
        globeVC.height = 2
        globeVC.heading = -Float.pi/2
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(0, 10), height:0.5, heading:0, time:0.5)
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        setUp(mapVC)
        mapVC.setPosition(MaplyCoordinateMakeWithDegrees(0, 10), height: 0.5)
	}
    
    func setUp(_ vc: MaplyBaseViewController) {
        self.labelObj = makeLabels()
        timer = Timer.scheduledTimer(withTimeInterval: duration, repeats: true) { [weak self] _ in
            guard let self = self else { return }
            self.clearLabels()
            self.labelObj = self.makeLabels()
        }
    }
    
    func clearLabels() {
        if let obj = labelObj {
            baseViewController?.remove(obj)
            labelObj = nil
        }
    }
    
    func makeLabels() -> MaplyComponentObject? {
        let pts = [
            MaplyCoordinateMakeWithDegrees(0.0, 0.0),
            MaplyCoordinateMakeWithDegrees(10.0, 10.0),
            MaplyCoordinateMakeWithDegrees(0.0, 20.0),
            MaplyCoordinateMakeWithDegrees(-10.0, 10.0)
        ]
        let labels = (0..<pts.count).map { (i) -> MaplyMovingScreenLabel in
            let label = MaplyMovingScreenLabel()
            label.duration = duration
            label.loc = pts[i]
            label.endLoc = pts[(i + 1) % pts.count]
            label.layoutImportance = Float.greatestFiniteMagnitude
            label.text = "Text"
            return label
        }
        return baseViewController?.addScreenLabels(labels,
                                    desc: [kMaplyFont: UIFont.systemFont(ofSize: 32.0),
                                      kMaplyTextColor: UIColor.red])
    }


    override func stop() {
        timer?.invalidate()
        timer = nil
        clearLabels()
    }

    let duration = 5.0
    var timer: Timer?
    var labelObj: MaplyComponentObject?

}
