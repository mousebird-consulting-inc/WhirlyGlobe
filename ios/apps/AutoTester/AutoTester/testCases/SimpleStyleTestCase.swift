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
    
    static func prop(_ name: String, _ value: String?, _ quote: Bool) -> String? {
        (value != nil) ? ("\"\(name)\": " + (quote ? "\"\(value!)\"" : value!)) : nil
    }
    static func marker(_ title: String, _ lat: Double, _ lon: Double, m: String? = nil,
                       bg: String? = nil, c: Bool? = nil, mC: String? = nil, fC: String? = nil,
                       fA: Double? = nil, s: Double? = nil, sC: String? = nil,
                       sA: Double? = nil, mSz: String? = nil) -> String {
        """
        {
          "type": "Feature",
          "properties": {
        """ +
            [prop("title", title, true),
             prop("marker-size", mSz ?? "large", true),
            prop("marker-color", mC, true),
            prop("marker-symbol", m, true),
            prop("marker-background-symbol", bg, true),
            prop("marker-circle", "\(!(c ?? true))", false),
            prop("marker-color", mC, true),
            prop("fill-color", fC, true),
            prop("fill-opacity", (fA != nil) ? "\(fA!)" : nil, false),
            prop("stroke-width", (s != nil) ? "\(s!)" : nil, false),
            prop("stroke-color", sC, true),
            prop("stroke-opacity", (sA != nil) ? "\(sA!)" : nil, false)
            ].compactMap { $0 }
            .joined(separator: ",") +
        """
          },
          "geometry": { "type": "Point", "coordinates": [ \(lon), \(lat) ] }
        }
        """
    }

    static func markers() -> String {
        var lat = -30.0, lon = 142.0, n = 0
        return [nil, "bar"].flatMap { m in
            [nil, "marker-stroked"].flatMap { bg in
                [true, false].flatMap { c in
                    [0.0, 0.8].flatMap { fA in
                        [0.0, 2.0].map { s -> String in
                            lon += 0.1
                            if (n % 8) == 0 { lat -= 0.1; lon = 140.0 }
                            n += 1
                            return marker("marker", lat, lon, m: m, bg: bg, c: c, mC: "0a0",
                                          fC: "050", fA: fA, s: s, sC: "020", sA: 0.8)
                        }
                    }
                }
            }
        }.joined(separator: ",")
    }
    let geoJSON = """
    {
      "type": "FeatureCollection",
      "features": [
        \(markers()),
        {
          "type": "Feature",
          "properties": {
            "title": "poly",
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
            "title": "line",
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

    func runExamples(_ vc: MaplyBaseViewController)
    {
        let styleMan = MaplySimpleStyleManager(viewC: vc)
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
