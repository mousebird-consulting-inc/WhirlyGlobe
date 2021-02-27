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

            viewC.startMaskTarget(nil)
            
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
                marker.maskID = pt.value.uuid
                markers.append(marker)
            }
            viewC.addScreenMarkers(markers, desc: nil)
            
            // For each segment, we want to add the two endpoints as masks
            var labels: [MaplyScreenLabel] = []
            for seg in segments {
                if let locArr = seg.asCLLocationArrays()?.first as? [CLLocation],
                   let locStart = locArr.first,
                   let locEnd = locArr.last {
                    if let markStart = graphBuilder.getPoint(locStart),
                       let markEnd = graphBuilder.getPoint(locEnd) {
                        seg.attributes?["maskID0"] = markStart.uuid
                        seg.attributes?["maskID1"] = markEnd.uuid
                    }
                }
                
                // Put a label along the line
                let label = MaplyScreenLabel()
                label.layoutVec = seg
                label.text = seg.attributes?["IDENT"] as? String
                label.loc = seg.center()
                label.layoutImportance = 1.0
                labels.append(label)
            }

            viewC.addWideVectors(segments, desc: [kMaplyVecWidth: 2.0,
                                                  kMaplyColor: UIColor.blue],
                                 mode: .any)
            viewC.addScreenLabels(labels, desc: [kMaplyFont: UIFont.boldSystemFont(ofSize: 24.0),
                                                 kMaplyTextColor: UIColor.purple,
                                                 kMaplyJustify: kMaplyTextJustifyCenter],
                                  mode: .any)
        }
    }
    
    func setupAirspaces(_ viewC: MaplyBaseViewController) {
        guard let vecObj = MaplyVectorObject(fromShapeFile: "Airspace_Boundary") else {
            print("Failed to load Airspace_Boundary shapefile")
            return
        }

        // Put the airspace vectors together
        var airspaceVecs = [MaplyVectorObject]()
        var labels = [MaplyScreenLabel]()
        for vec in vecObj.splitVectors() {
            var include = false
            if let highVal = vec.attributes?["US_HIGH"] as? Int {
                if highVal > 0 {
                    if vec.vectorType() == .arealType {
                        include = true
                    }
                }
            }
            
            if include {
                // Put a label in the middle
                let label = MaplyScreenLabel()
                label.layoutVec = vec
                label.loc = vec.centroid()
                label.text = vec.attributes?["NAME"] as? String
                label.layoutImportance = 1.0
                
                if label.text != nil {
                    labels.append(label)
                    airspaceVecs.append(vec)
                }
            }
        }
        
        viewC.addWideVectors(airspaceVecs, desc: [kMaplyVecWidth: 6.0,
                                                  kMaplyColor: UIColor.blue], mode: .any)
        viewC.addScreenLabels(labels, desc: [kMaplyFont: UIFont.boldSystemFont(ofSize: 18.0),
                                             kMaplyTextColor: UIColor.purple,
                                             kMaplyTextLayoutOffset: 30.0,  // Right in the center
                                             kMaplyTextLayoutSpacing: 100.0, // 100 pixels between
                                             kMaplyTextLayoutRepeat: -1,  // As many as fit
                                             ],
                              mode: .any)
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)

        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-110.0, 40.5023056), time: 1.0)

//        setupAirways(globeVC)
        setupAirspaces(globeVC)
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-110.0, 40.5023056), time: 1.0)
        
//        setupAirways(mapVC)
        setupAirspaces(mapVC)
    }
}
