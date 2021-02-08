//
//  AirwayTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 2/8/21.
//  Copyright © 2021 mousebird consulting. All rights reserved.
//

import Foundation

// A very dumb graph builder
class GraphBuilder {
    // Sort endpoints (gross)
    var points = [String: (loc: CLLocationCoordinate2D, count: Int, uuid: String)]()
    
    // Convert point to a simple string for caching (barf)
    func stringPoint(_ loc: CLLocationCoordinate2D) -> String {
        return String(format: "%.4f_%.4f", loc.longitude, loc.latitude)
    }

    // Add an entry for the given point
    func addPoint(_ loc: CLLocation) {
        let coord = loc.coordinate
        let ptAsStr = stringPoint(coord)
        
        if let val = points[ptAsStr] {
            points[ptAsStr] = (coord, val.count+1, val.uuid)
        } else {
            points[ptAsStr] = (coord, 1, UUID().uuidString)
        }
    }
    
    // Look for the given point
    func getPoint(_ loc: CLLocation) -> (loc: CLLocationCoordinate2D, count: Int, uuid: String)? {
        let ptAsStr = stringPoint(loc.coordinate)
        return points[ptAsStr]
    }
}

class AirwayTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        
        self.name = "Airways & Airspaces"
        self.implementations = [.globe, .map]
    }
    
    let baseCase = StamenWatercolorRemote()
    
    func setupAirways(_ viewC: MaplyBaseViewController) {
        DispatchQueue.global(qos: .default).async {
            guard let vecObj = MaplyVectorObject(fromShapeFile: "ATS_Route") else {
                print("Failed to load ATS_Route shapefile")
                return
            }
            
            var markerTextures: [MaplyTexture] = []
            markerTextures.append(viewC.addTexture(UIImage(named: "rocket-24@2x.png")!, desc: nil, mode: .current)!)         // 0, not used
            markerTextures.append(viewC.addTexture(UIImage(named: "rocket-24@2x.png")!, desc: nil, mode: .current)!)         // 1
            markerTextures.append(viewC.addTexture(UIImage(named: "star-stroked-24@2x.png")!, desc: nil, mode: .current)!)   // 2
            markerTextures.append(viewC.addTexture(UIImage(named: "shop-24@2x.png")!, desc: nil, mode: .current)!)           // 3
            markerTextures.append(viewC.addTexture(UIImage(named: "london-underground-24@2x.png")!, desc: nil, mode: .current)!) // 4
            markerTextures.append(viewC.addTexture(UIImage(named: "post-24@2x.png")!, desc: nil, mode: .current)!) // 5
            markerTextures.append(viewC.addTexture(UIImage(named: "town-hall-24@2x.png")!, desc: nil, mode: .current)!) // 6

            let graphBuilder = GraphBuilder()
            
            // Work through the airway identifying start and end points
            var segments = [MaplyVectorObject]()
            for airwayObj in vecObj.splitVectors() {
                if let locArr = airwayObj.asCLLocationArrays()?.first as? [CLLocation],
                   let locStart = locArr.first,
                   let locEnd = locArr.last {
                    graphBuilder.addPoint(locStart)
                    graphBuilder.addPoint(locEnd)
                    segments.append(airwayObj)
                }
            }
            
            // One marker for each node
            var markers: [MaplyScreenMarker] = []
            for pt in graphBuilder.points {
                let marker = MaplyScreenMarker()
                marker.loc = MaplyCoordinateMakeWithDegrees(Float(pt.value.loc.longitude), Float(pt.value.loc.latitude))
                marker.image = markerTextures[min(pt.value.count,markerTextures.count-1)]
//                marker.layoutImportance = MAXFLOAT
                marker.size = CGSize(width: 24.0, height: 24.0)
//                marker.maskID = pt.value.uuid
                markers.append(marker)
            }
            viewC.addScreenMarkers(markers, desc: nil)
            
            // For each segment, we want to add the two endpoints as masks
            for seg in segments {
                if let locArr = seg.asCLLocationArrays()?.first as? [CLLocation],
                   let locStart = locArr.first,
                   let locEnd = locArr.last {
                    if let markStart = graphBuilder.getPoint(locStart),
                       let markEnd = graphBuilder.getPoint(locEnd) {
                        seg.attributes?["maskID 0"] = markStart.uuid
                        seg.attributes?["maskID 1"] = markEnd.uuid
                    }
                }
            }

            viewC.addVectors(segments, desc: [kMaplyVecWidth: 4.0, kMaplyColor: UIColor.blue], mode: .current)
        }
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)

        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-110.0, 40.5023056), time: 1.0)

        setupAirways(globeVC)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-110.0, 40.5023056), time: 1.0)
        
        setupAirways(mapVC)
    }
}
