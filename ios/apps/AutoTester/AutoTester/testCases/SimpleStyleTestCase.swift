//
//  SimpleStyleTestCase.swift
//  AutoTester
//
//  Created by Steve Gifford on 3/31/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

import UIKit

class SimpleStyleTestCase: MaplyTestCase {

    override init() {
        super.init()

        self.name = "SimpleStyle"
        self.implementations = [.globe, .map]
    }
    
    let baseCase = StamenWatercolorRemote()
    
    let geoJSON = """
    {
      "type": "FeatureCollection",
      "features": [
        {
          "type": "Feature",
          "properties": {
            "marker-color": "#00aa00",
            "marker-size": "large",
            "marker-symbol": "bar"
          },
          "geometry": {
            "type": "Point",
            "coordinates": [
              151.211111,
              -33.859972
            ]
          }
        },
        {
          "type": "Feature",
          "properties": {
            "fill": "#ff0000",
            "stroke": "#ffffff"
          },
          "geometry": {
            "type": "Polygon",
            "coordinates": [
              [
                [
                  150.40283203125,
                  -33.504759069226075
                ],
                [
                  151.10595703125,
                  -33.504759069226075
                ],
                [
                  151.10595703125,
                  -33.16974360021616
                ],
                [
                  150.40283203125,
                  -33.16974360021616
                ],
                [
                  150.40283203125,
                  -33.504759069226075
                ]
              ]
            ]
          }
        },
        {
          "type": "Feature",
          "properties": {
            "stroke": "#0000ff",
            "stroke-width": 10.0
          },
          "geometry": {
            "type": "LineString",
            "coordinates": [
              [
                150.0677490234375,
                -32.97871614600332
              ],
              [
                150.14190673828125,
                -32.97641208290518
              ],
              [
                150.5731201171875,
                -32.95797741405951
              ],
              [
                150.98236083984375,
                -33.01096671579776
              ],
              [
                150.75164794921875,
                -33.08463802391686
              ],
              [
                149.91943359375,
                -33.27084277265288
              ],
              [
                149.77935791015625,
                -32.9049563191375
              ],
              [
                150.24078369140625,
                -32.669436832605314
              ],
              [
                150.55389404296875,
                -32.764181375100804
              ]
            ]
          }
        }
      ]
    }
    """

    var styleMan : MaplySimpleStyleManager? = nil

    func runExamples(_ vc: MaplyBaseViewController)
    {
        styleMan = MaplySimpleStyleManager(viewC: vc)
        guard let styleMan = styleMan else {
            return
        }
        
        if let data = geoJSON.data(using: .utf8),
            let vecObj = MaplyVectorObject(fromGeoJSON: data) {
            styleMan.addFeatures([vecObj], mode: .current)
        }
    }
    

    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        baseCase.setUpWithGlobe(globeVC)
        runExamples(globeVC)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        baseCase.setUpWithMap(mapVC)
        runExamples(mapVC)
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }

}
