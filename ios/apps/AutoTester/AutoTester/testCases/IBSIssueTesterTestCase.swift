//
//  IBSIssueTesterTestCase.swift
//  AutoTester
//
//  Created by Jean-Michel Cazaux on 11/04/2017.
//  Copyright © 2017 mousebird consulting. All rights reserved.
//

import Foundation

class IBSIssueTesterTestCase: MaplyTestCase {


    // MARK: - Constants



    // MARK: - Private variables



    // MARK: - Public variables



    // MARK: - Initializers

    override init() {
        super.init()

        self.name = "Iron Bird Software issues tester"
        self.captureDelay = 5
        self.implementations = [.globe, .map]
    }




    // MARK: - Protocols

    // MARK: Protocol XyZAbdc



    // MARK: - Public methods

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = GeographyClassTestCase()
        baseLayer.setUpWithGlobe(globeVC)

//        globeVC.add(setupLayer(baseVC: globeVC))

        self.addObjects(toController: globeVC)

        globeVC.keepNorthUp = true
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(3.84, 43), time: 1.0)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = GeographyClassTestCase()
        baseLayer.setUpWithMap(mapVC)

//        mapVC.add(setupLayer(baseVC: mapVC))

        self.addObjects(toController: mapVC)

        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(3.84, 43), height: 0.5, time: 1.0)
        mapVC.setZoomLimitsMin(0.01, max: 4.0)
    }
    
    




    // MARK: - Private methods


    private func addObjects(toController vc: MaplyBaseViewController) {

        let loc = MaplyCoordinateMakeWithDegrees(4, 44)


        // Adding a standard marker
        var marker = MaplyScreenMarker()
        marker.image = UIImage(named:"Smiley_Face_Avatar_by_PixelTwist")
        marker.loc = loc
        marker.size = CGSize(width: 50, height: 50)

        var info = Dictionary<AnyHashable, Any>()

        info[kMaplyDrawPriority] = 20

        vc.addScreenMarkers([marker], desc: info, mode: MaplyThreadMode.current)

        var label = MaplyScreenLabel()
        label.text = "A colored marker shows here"
        label.loc = loc

        info = Dictionary<AnyHashable, Any>()

        info[kMaplyTextColor] = UIColor.blue
        info[kMaplyTextOutlineColor] = UIColor.white
        info[kMaplyTextOutlineSize] = 2.0
        info[kMaplyDrawPriority] = 50
        info[kMaplyEnable] = true
        info[kMaplyFont] = UIFont.init(name: "Helvetica-Bold", size: 12.0)

        vc.addScreenLabels([label], desc: info, mode: MaplyThreadMode.current)


        let loc2 = MaplyCoordinateMakeWithDegrees(5, 45)

        // Adding some "warning" text
        label = MaplyScreenLabel()
        label.text = "A black & white marker shows here"
        label.loc = loc2

        info = Dictionary<AnyHashable, Any>()

        info[kMaplyTextColor] = UIColor.red
        info[kMaplyTextOutlineColor] = UIColor.white
        info[kMaplyTextOutlineSize] = 2.0
        info[kMaplyDrawPriority] = 50
        info[kMaplyEnable] = true
        info[kMaplyFont] = UIFont.init(name: "Helvetica-Bold", size: 12.0)

        vc.addScreenLabels([label], desc: info, mode: MaplyThreadMode.current)


        // Filtering the image
        let uiSmiley = UIImage(named: "Smiley_Face_Avatar_by_PixelTwist")!
        let ciSmiley = CIImage(image: uiSmiley)

        if let bwSmiley = ciSmiley?.applyingFilter("CIPhotoEffectNoir", withInputParameters: nil) {

            UIGraphicsBeginImageContextWithOptions(uiSmiley.size, false, uiSmiley.scale)
            defer {
                UIGraphicsEndImageContext()
            }

            UIImage(ciImage: bwSmiley).draw(in: CGRect(origin: .zero, size: uiSmiley.size))

            guard let image = UIGraphicsGetImageFromCurrentImageContext() else {
                print("*** Could not get image from context ***")
                return
            }

            marker = MaplyScreenMarker()
            marker.image = image
            marker.loc = loc2
            marker.size = CGSize(width: 50, height: 50)

            var info = Dictionary<AnyHashable, Any>()

            info[kMaplyDrawPriority] = 20

            vc.addScreenMarkers([marker], desc: info, mode: MaplyThreadMode.current)
        } else {
            print("*** bwSmiley is nil ***")
        }
    }
}
