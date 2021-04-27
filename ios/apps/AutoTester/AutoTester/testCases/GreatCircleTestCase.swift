//
//  GreatCircleTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 3/11/16.
//  Copyright © 2016-2021 mousebird consulting. All rights reserved.
//

import Foundation

class GreatCircleTestCase: MaplyTestCase {
    
    override init() {
        super.init()
        self.name = "Great Circles"
        self.implementations = [.globe,.map]
    }

    func addLongRoute(_ viewC: MaplyBaseViewController) -> [MaplyComponentObject] {

        let isGlobe = viewC is WhirlyGlobeViewController
        var compObjs = [MaplyComponentObject]()
        
        if (true) {
            let x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
            let y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);
            let v0 = MaplyVectorObject(lineString:[x, y], numCoords:2, attributes:nil);

    //        UIImage *alcohol = [UIImage imageNamed:@"alcohol-shop-24@2x"];
    //        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc]init];
    //        marker.image = alcohol;
    //        marker.loc = v0.centroid;
    //        marker.selectable = true;
    //        marker.layoutImportance = 1.0;

            if (isGlobe) {
                v0.subdivide(toGlobeGreatCircle: 0.0001)
            } else {
                v0.subdivide(toFlatGreatCircle: 0.0001)
            }

            let desc = [
                kMaplyColor: UIColor.red,
                kMaplyEnable: true,
                kMaplyVecWidth: 30.0,
                kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 1,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v0], desc:desc) {
                compObjs.append(obj)
            }
    //        [compObjs addObject:[viewC addScreenMarkers:@[marker] desc:nil]];
        }

        // Precise/ellipsoidal subdivide
        if (true) {
            let x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
            let y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);
            let v0 = MaplyVectorObject(lineString:[x, y], numCoords:2, attributes:nil);

            if (isGlobe) {
                v0.subdivide(toGlobeGreatCirclePrecise: 0.01)
            } else {
                v0.subdivide(toFlatGreatCirclePrecise: 0.01)
            }
            
            // Drop a marker right in the middle
            let inv = GeoLibCalcInverseF(x, y)
            let midPt = GeoLibCalcDirectF(x, inv.azimuth1, inv.distance / 2.0)
            let marker = MaplyScreenMarker()
            marker.image = UIImage(named: "alcohol-shop-24@2x")
            marker.loc = midPt
            marker.size = CGSize(width: 64.0, height: 64.0)
            marker.selectable = true;
            marker.layoutImportance = MAXFLOAT;

            let desc = [
                kMaplyColor: UIColor.black,
                kMaplyEnable: true,
                kMaplyVecWidth: 20.0,
                kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 2,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v0], desc:desc) {
                compObjs.append(obj)
            }
            if let obj = viewC.addScreenMarkers([marker], desc: nil) {
                compObjs.append(obj)
            }
        }

        // Precise/ellipsoidal subdivide using attributes
        if (true) {
            let x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
            let y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);
            let v0 = MaplyVectorObject(lineString:[x, y], numCoords:2, attributes:nil);

            let desc = [
                kMaplyColor: UIColor.magenta,
                kMaplyEnable: true,
                kMaplyVecWidth: 10.0,
                kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 3,
                kMaplySubdivType: kMaplySubdivGreatCirclePrecise,
                kMaplySubdivEpsilon: 0.01,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v0], desc:desc) {
                compObjs.append(obj)
            }
        }

        // Thinner, manually-subdivided line on top of the previous using Maply GeoLib wrappers
        if (true) {
            let x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
            let y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);

            let inv = GeoLibCalcInverseF(x, y);

            let numSegs = Int(inv.distance / 50000 /* 50 km */);
            let segLen = inv.distance / Double(numSegs)
            var pts = [MaplyCoordinate]()
            pts.append(x)
            for i in 1..<numSegs {
                pts.append(GeoLibCalcDirectF(x, inv.azimuth1, segLen * Double(i)));
            }
            pts.append(y);

            let v0 = MaplyVectorObject(lineString:pts, numCoords:Int32(pts.count), attributes:nil);
            let desc = [
                kMaplyColor: UIColor.white,
                kMaplyEnable: true,
                kMaplyVecWidth: 5.0,
                kMaplyDrawPriority: kMaplyVectorDrawPriorityDefault + 4,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v0], desc:desc) {
                compObjs.append(obj)
            }
        }

        if (true) {
            let x = MaplyCoordinateMakeWithDegrees(150.0, 0.0);
            let y = MaplyCoordinateMakeWithDegrees(-150, 0.0);
            let v1 = MaplyVectorObject(lineString:[x, y], numCoords:2, attributes:nil);

            if (isGlobe) {
                v1.subdivide(toGlobeGreatCircle:0.001);
            } else {
                v1.subdivide(toFlatGreatCircle:0.001);
            }

            let desc = [
                kMaplyColor: UIColor.blue,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v1], desc:desc) {
                compObjs.append(obj)
            }
        }

        if (true) {
            let a = MaplyCoordinateMakeWithDegrees(-176.4624, -44.3040);
            let c = MaplyCoordinateMakeWithDegrees(171.2303, 44.3040);
            let v2 = MaplyVectorObject(lineString:[a, c], numCoords:2, attributes:nil);

            if (isGlobe) {
                v2.subdivide(toGlobeGreatCircle:0.001);
            } else {
                v2.subdivide(toFlatGreatCircle:0.001);
            }

            let desc = [
                kMaplyColor: UIColor.green,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v2], desc:desc) {
                compObjs.append(obj)
            }
        }

        // Stop at -180/+180
        if (/* DISABLES CODE */ (false)) {
            let a = MaplyCoordinateMakeWithDegrees(-176.4624, -44.3040);
            let b1 = MaplyCoordinateMakeWithDegrees(-180.0, 0.0);
            let b2 = MaplyCoordinateMakeWithDegrees(180.0, 0.0);
            let c = MaplyCoordinateMakeWithDegrees(171.2303, 44.3040);
            let v0 = MaplyVectorObject(lineString:[a, b1], numCoords:2, attributes:nil);
            let v1 = MaplyVectorObject(lineString:[b2, c], numCoords:2, attributes:nil);
            
            if (isGlobe) {
                v0.subdivide(toGlobeGreatCircle:0.001)
                v1.subdivide(toGlobeGreatCircle:0.001)
            } else {
                v0.subdivide(toFlatGreatCircle:0.001)
                v1.subdivide(toFlatGreatCircle:0.001)
            }

            var desc = [
                kMaplyColor: UIColor.orange,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v0], desc:desc) {
                compObjs.append(obj)
            }

            desc[kMaplyColor] = UIColor.yellow

            if let obj = viewC.addWideVectors([v1], desc:desc) {
                compObjs.append(obj)
            }
        }
        
        if (true) {
            let a = MaplyCoordinateMakeWithDegrees(-179.686999,-24.950296);
            let c = MaplyCoordinateMakeWithDegrees(179.950328,-22.180086);
            let v2 = MaplyVectorObject(lineString:[a, c], numCoords:2, attributes:nil)

            if (isGlobe) {
                v2.subdivide(toGlobeGreatCircle:0.001)
            } else {
                v2.subdivide(toFlatGreatCircle:0.001)
            }

            let desc = [
                kMaplyColor: UIColor.purple,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v2], desc:desc) {
                compObjs.append(obj)
            }
        }

        if (true) {
            let a = MaplyCoordinateMakeWithDegrees(-175.0,-33.092222);
            let c = MaplyCoordinateMakeWithDegrees(177.944183,-34.845333);
            let v2 = MaplyVectorObject(lineString:[a, c], numCoords:2, attributes:nil)

            if (isGlobe) {
                v2.subdivide(toGlobeGreatCircle:0.001)
            } else {
                v2.subdivide(toFlatGreatCircle:0.001)
            }

            let desc = [
                kMaplyColor: UIColor.brown,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v2], desc:desc) {
                compObjs.append(obj)
            }
        }

        if (true) {
            let a = MaplyCoordinateMakeWithDegrees(177.747519,-34.672406);
            let c = MaplyCoordinateMakeWithDegrees(-175.0,-31.833547);
            let v2 = MaplyVectorObject(lineString:[a, c], numCoords:2, attributes:nil)

            if (isGlobe) {
                v2.subdivide(toGlobeGreatCircle:0.001)
            } else {
                v2.subdivide(toFlatGreatCircle:0.001)
            }

            let desc = [
                kMaplyColor: UIColor.brown,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v2], desc:desc) {
                compObjs.append(obj)
            }
        }

        if (true) {
            let a = MaplyCoordinateMakeWithDegrees(-175.0,-31.833547);
            let c = MaplyCoordinateMakeWithDegrees(178.394161,-35.357856);
            let v2 = MaplyVectorObject(lineString:[a, c], numCoords:2, attributes:nil)

            if (isGlobe) {
                v2.subdivide(toGlobeGreatCircle:0.001)
            } else {
                v2.subdivide(toFlatGreatCircle:0.001)
            }

            let desc = [
                kMaplyColor: UIColor.brown,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v2], desc:desc) {
                compObjs.append(obj)
            }
        }

        // Line to see where the division is
        if (true) {
            let x = MaplyCoordinateMakeWithDegrees(180.0, -89.9);
            let y = MaplyCoordinateMakeWithDegrees(180.0, 89.9);
            let v3 = MaplyVectorObject(lineString:[x, y], numCoords:2, attributes:nil)

            if (isGlobe) {
                v3.subdivide(toGlobeGreatCircle:0.001)
            } else {
                v3.subdivide(toFlatGreatCircle:0.001)
            }

            let desc = [
                kMaplyColor: UIColor.white,
                kMaplyEnable: true,
                kMaplyVecWidth: 6.0,
            ] as [String : Any]

            if let obj = viewC.addWideVectors([v3], desc:desc) {
                compObjs.append(obj)
            }
        }
        
        return compObjs;
    }

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        
        globeVC.height = 0.25;
        globeVC.keepNorthUp = false;

        objs.append(contentsOf: addLongRoute(globeVC))

        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(2.548,49.010), height:1.0, heading:0, time:0.5)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)

        mapVC.height = 0.25;

        objs.append(contentsOf: addLongRoute(mapVC))

        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(2.548,49.010), height:1.0, heading:0, time:0.5)
    }

    override func stop() {
        baseViewController?.remove(objs, mode: MaplyThreadMode.current);
        objs.removeAll()
        baseCase.stop()
        super.stop()
    }

    let baseCase = CartoDBLightTestCase()
    var objs = [MaplyComponentObject]()
}

