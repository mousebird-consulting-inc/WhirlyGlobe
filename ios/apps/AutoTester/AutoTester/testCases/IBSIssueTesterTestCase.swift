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
        info[kMaplyFont] = UIFont.init(name: "Helvetica-Bold", size: 16.0)

        vc.addScreenLabels([label], desc: info, mode: MaplyThreadMode.current)


        let loc2 = MaplyCoordinateMakeWithDegrees(5, 45)

        // Adding some "warning" text
        label = MaplyScreenLabel()
        label.text = "A black & white marker should show here"
        label.loc = loc2

        info = Dictionary<AnyHashable, Any>()

        info[kMaplyTextColor] = UIColor.red
        info[kMaplyTextOutlineColor] = UIColor.white
        info[kMaplyTextOutlineSize] = 2.0
        info[kMaplyDrawPriority] = 50
        info[kMaplyEnable] = true
        info[kMaplyFont] = UIFont.init(name: "Helvetica-Bold", size: 16.0)

        vc.addScreenLabels([label], desc: info, mode: MaplyThreadMode.current)


        // Filtering the image
        let colorMatrixFilter = CIFilter(name: "CIPhotoEffectMono")!
        colorMatrixFilter.setDefaults()
        colorMatrixFilter.setValue(CIImage(image: UIImage(named:"Smiley_Face_Avatar_by_PixelTwist")!)!, forKey: kCIInputImageKey)

        if let ciImage = colorMatrixFilter.outputImage {
            let bwImage = UIImage(ciImage: ciImage, scale: 1.0, orientation: UIImageOrientation.up)

            marker = MaplyScreenMarker()
            marker.image = bwImage
            marker.loc = loc2
            marker.size = CGSize(width: 50, height: 50)

            var info = Dictionary<AnyHashable, Any>()

            info[kMaplyDrawPriority] = 20

            vc.addScreenMarkers([marker], desc: info, mode: MaplyThreadMode.current)
        } else {
            print("*** ciImage is nil ***")
        }
    }
}
