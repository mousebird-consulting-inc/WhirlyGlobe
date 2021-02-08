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
    var points = [String: (loc: CLLocationCoordinate2D, count: Int, marker: MaplyScreenMarker?)]()

    func addPoint(_ loc: CLLocation) {
        let coord = loc.coordinate
        let ptAsStr = String(format: "%.4f_%.4f", coord.longitude, coord.latitude)
        
        if let val = points[ptAsStr] {
            points[ptAsStr] = (coord, val.count+1, nil)
        } else {
            points[ptAsStr] = (coord, 1, nil)
        }
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
            markerTextures.append(viewC.addTexture(UIImage(named: "laundry-24@2x.png")!, desc: nil, mode: .current)!)
            markerTextures.append(viewC.addTexture(UIImage(named: "rocket-24@2x.png")!, desc: nil, mode: .current)!)
            markerTextures.append(viewC.addTexture(UIImage(named: "prison-24@2x.png")!, desc: nil, mode: .current)!)
            markerTextures.append(viewC.addTexture(UIImage(named: "shop-24@2x.png")!, desc: nil, mode: .current)!)
            markerTextures.append(viewC.addTexture(UIImage(named: "slaughterhouse-24@2x.png")!, desc: nil, mode: .current)!)
            markerTextures.append(viewC.addTexture(UIImage(named: "star-stroked-24@2x.png")!, desc: nil, mode: .current)!)

            let graphBuilder = GraphBuilder()
            
            // Work through the airway identifying start and end points
            for airwayObj in vecObj.splitVectors() {
                if let locArr = airwayObj.asCLLocationArrays()?.first as? [CLLocation],
                   let locStart = locArr.first,
                   let locEnd = locArr.last {
                    graphBuilder.addPoint(locStart)
                    graphBuilder.addPoint(locEnd)
                }
            }
            
            // One marker for each node
            var markers: [MaplyScreenMarker] = []
            for pt in graphBuilder.points {
                let marker = MaplyScreenMarker()
                marker.loc = MaplyCoordinateMakeWithDegrees(Float(pt.value.loc.longitude), Float(pt.value.loc.latitude))
                marker.image = markerTextures[min(pt.value.count,markerTextures.count-1)]
                marker.layoutImportance = MAXFLOAT
                marker.size = CGSize(width: 24.0, height: 24.0)
                markers.append(marker)
            }
            viewC.addScreenMarkers(markers, desc: nil)
            
            viewC.addVectors([vecObj], desc: [kMaplyVecWidth: 4.0, kMaplyColor: UIColor.blue], mode: .current)
        }
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        
        setupAirways(globeVC)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        
        setupAirways(mapVC)
    }
}
