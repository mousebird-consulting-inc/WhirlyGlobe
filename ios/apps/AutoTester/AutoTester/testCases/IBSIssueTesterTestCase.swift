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

        let lat: Float = 44
        let lon: Float = 4
        let loc = MaplyCoordinateMakeWithDegrees(lon, lat)

        // Adding the text at the bottom
        var label = MaplyScreenLabel()
        label.text = "The quick brown fox is at the bottom"
        label.loc = loc

        var info = Dictionary<AnyHashable, Any>()

        info[kMaplyTextColor] = UIColor.brown
        info[kMaplyTextOutlineColor] = UIColor.white
        info[kMaplyTextOutlineSize] = 2.0
        info[kMaplyDrawPriority] = 10
        info[kMaplyEnable] = true
        info[kMaplyFont] = UIFont.init(name: "Helvetica-Bold", size: 24.0)

        vc.addScreenLabels([label], desc: info, mode: MaplyThreadMode.current)


        // Adding a marker above the text
        let marker = MaplyScreenMarker()
        marker.image = UIImage(named:"Smiley_Face_Avatar_by_PixelTwist")
        marker.loc = loc
        marker.size = CGSize(width: 100, height: 100)

        info = Dictionary<AnyHashable, Any>()

        info[kMaplyDrawPriority] = 20

        vc.addScreenMarkers([marker], desc: info, mode: MaplyThreadMode.current)



        // Adding a cross made of wide vectors above the marker
        let ur = MaplyCoordinateMakeWithDegrees(lon + 1, lat + 1)
        let ll = MaplyCoordinateMakeWithDegrees(lon - 1, lat - 1)
        let ul = MaplyCoordinateMakeWithDegrees(lon - 1, lat + 1)
        let lr = MaplyCoordinateMakeWithDegrees(lon + 1, lat - 1)
        var ls1 = [ur, ll]
        let wv1 = MaplyVectorObject(lineString: &ls1, numCoords: 2, attributes: nil)
        var ls2 = [ul, lr]
        let wv2 = MaplyVectorObject(lineString: &ls2, numCoords: 2, attributes: nil)

        var lineInfo = Dictionary<AnyHashable, Any>()
        lineInfo[kMaplyVecWidth] = 50
        lineInfo[kMaplyColor] = UIColor.red
        lineInfo[kMaplyDrawPriority] = 30
        lineInfo[kMaplyEnable] = true
        lineInfo[kMaplyFilled] = false
        lineInfo[kMaplyWideVecCoordType] = kMaplyWideVecCoordTypeScreen

        vc.addWideVectors([wv1, wv2], desc: lineInfo, mode: MaplyThreadMode.current)




        // Adding some text above
        label = MaplyScreenLabel()
        label.text = "The lazzy dog is at the top"
        label.loc = MaplyCoordinateMakeWithDegrees(lon, lat)

        info = Dictionary<AnyHashable, Any>()

        info[kMaplyTextColor] = UIColor.blue
        info[kMaplyTextOutlineColor] = UIColor.white
        info[kMaplyTextOutlineSize] = 1.0
        info[kMaplyDrawPriority] = 50
        info[kMaplyEnable] = true
        info[kMaplyFont] = UIFont.init(name: "Helvetica-Bold", size: 16.0)

        vc.addScreenLabels([label], desc: info, mode: MaplyThreadMode.current)
        
    }


}
