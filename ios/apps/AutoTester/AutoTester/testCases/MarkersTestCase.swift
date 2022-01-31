//
//  MarkersTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright 2015-2022 mousebird consulting.
//

import UIKit
import WhirlyGlobe

class MarkersTestCase: MaplyTestCase {

	override init() {
		super.init()

		self.name = "Markers"
		self.implementations = [.globe, .map]
	}
    let startImage = UIImage(named: "airfield-24@2x")

	func insertMarkers (_ arrayComp: NSArray, theViewC: MaplyBaseViewController) {
		let size = CGSize(width: 0.05, height: 0.05);
		var markers = [MaplyMarker]()
		for i in 0 ..< arrayComp.count {
			let object = arrayComp[i]
			let marker = MaplyMarker()
			marker.image = startImage
			marker.loc = (object as! MaplyVectorObject).center()
			marker.size = size
            marker.userObject = (object as! MaplyVectorObject).attributes?["title"]
			markers.append(marker)
		}
		theViewC.addMarkers(markers, desc: nil)
	}
    
    let baseCase = VectorsTestCase()

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
		insertMarkers(baseCase.vecList!, theViewC: globeVC)
		globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)

        addLongPressGesture()
	}

	override func setUpWithMap(_ mapVC: MaplyViewController) {
		baseCase.setUpWithMap(mapVC)
		insertMarkers(baseCase.vecList!, theViewC: mapVC)
		mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)

        addLongPressGesture()
	}
}

// MARK: - Long Press

extension MarkersTestCase {

    private func addLongPressGesture() {

        let gesture = UILongPressGestureRecognizer(target: self, action: #selector(longPressed(_:)))
        baseViewController?.view.addGestureRecognizer(gesture)
    }

    @objc
    private func longPressed(_ gesture: UILongPressGestureRecognizer) {

        let point = gesture.location(in: gesture.view)
        var loc: MaplyCoordinate?

        if let mapViewController = mapViewController {
            loc = mapViewController.geo(fromScreenPoint: point)
        } else if let globeViewController = globeViewController {
            loc = globeViewController.geoPoint(fromScreen: point)?.maplyCoordinateValue
        }
        guard let loc = loc else {
            return
        }
        showObjectsCount(at: loc)
    }

    private func showObjectsCount(at coord: MaplyCoordinate) {

        guard let viewC = baseViewController else {
            return
        }
        let objs = viewC.objects(atCoord: coord) ?? []
        let labelsAndMarkers = viewC.labelsAndMarkers(atCoord: coord) ?? []

        let vc = UIAlertController(title: nil, message: "\(objs.count) selectable vector objects and \(labelsAndMarkers.count) selectable labels and markers found at tapped location", preferredStyle: .alert)
        viewC.present(vc, animated: true, completion: nil)
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            viewC.dismiss(animated: true, completion: nil)
        }
    }
}
