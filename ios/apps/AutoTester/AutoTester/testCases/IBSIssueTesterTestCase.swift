//
//  IBSIssueTesterTestCase.swift
//  AutoTester
//
//  Created by Jean-Michel Cazaux on 11/04/2017.
//  Copyright © 2017 mousebird consulting. All rights reserved.
//

import Foundation

class IBSIssueTesterTestCase: MaplyTestCase, MaplyTileSource {


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


    func minZoom() -> Int32 {
        return 7
    }


    func maxZoom() -> Int32 {
        return 7
    }


    func tileSize() -> Int32 {
        return 256
    }


    func tileIsLocal(_ tileID: MaplyTileID, frame: Int32) -> Bool {
        return true
    }


    func coordSys() -> MaplyCoordinateSystem {
        return MaplySphericalMercator()
    }

    func startFetchLayer(_ layer: Any, tile tileID: MaplyTileID) {

        let tilesLayer = layer as! MaplyQuadImageTilesLayer

        DispatchQueue.global().async { [unowned self] in


            if (tileID.x >= 62 && tileID.x <= 66 && tileID.y >= 80 && tileID.y <= 85) {
                UIGraphicsBeginImageContextWithOptions(CGSize(width: 256, height: 256), false, 1)
                let context = UIGraphicsGetCurrentContext()
                context?.setFillColor(red: 1, green: 0, blue: 0, alpha: 0.2)
                context?.fill(CGRect(x: 20, y: 20, width: 216, height: 216))

                let image = UIGraphicsGetImageFromCurrentImageContext()

                tilesLayer.loadedImages(image!, forTile: tileID)

                print(String(format: "Tile found  for [%d, %d, %d]", tileID.x, tileID.y, tileID.level))

                return
            }

            // Uncomment the lines below to re-establish the loading of existing tiles
//            UIGraphicsBeginImageContextWithOptions(CGSize(width: 256, height: 256), false, 1)
//            let context = UIGraphicsGetCurrentContext()
//            context?.setFillColor(red: 0, green: 1, blue: 0, alpha: 0.1)
//            context?.fill(CGRect(x: 20, y: 20, width: 216, height: 216))
//
//            let image = UIGraphicsGetImageFromCurrentImageContext()
//
//            tilesLayer.loadedImages(image!, forTile: tileID)

            print(String(format: "*** No Tile found  for [%d, %d, %d]", tileID.x, tileID.y, tileID.level))
        }
    }

    




    // MARK: - Public methods

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = GeographyClassTestCase()
        baseLayer.setUpWithGlobe(globeVC)

        globeVC.add(self.setupLayer(forController: globeVC)!)


        globeVC.keepNorthUp = true
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(3.84, 43), time: 1.0)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = GeographyClassTestCase()
        baseLayer.setUpWithMap(mapVC)

        mapVC.add(self.setupLayer(forController: mapVC)!)

        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(3.84, 43), height: 0.4, time: 1.0)
        mapVC.setZoomLimitsMin(0.01, max: 4.0)
    }
    
    




    // MARK: - Private methods


    private func setupLayer(forController vc: MaplyBaseViewController) -> MaplyQuadImageTilesLayer? {

        let coordinateSystem = MaplySphericalMercator.init()

        if let layer = MaplyQuadImageTilesLayer.init(coordSystem: coordinateSystem, tileSource: self){
            layer.handleEdges = false
            layer.coverPoles = false
            layer.requireElev = false
            layer.waitLoad = false
            layer.singleLevelLoading = true
            layer.useTargetZoomLevel = true
            layer.numSimultaneousFetches = 8

            return layer
        }

        return nil
    }
}
