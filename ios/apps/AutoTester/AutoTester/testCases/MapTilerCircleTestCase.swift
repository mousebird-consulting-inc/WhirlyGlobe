//
//  MapTilerCircleTestCase.swift
//  AutoTester
//
//  Created by Tim Sylvester on 3/23/21.
//  Copyright © 2021 mousebird consulting. All rights reserved.
//
import UIKit

class MapTilerCircleTestCase: MapTilerTestCase {

    init() {
        super.init("MapTiler Circles")
        mapTilerStyle = 0
    }

    override func setup(_ map: MapboxKindaMap) {
        super.setup(map)
        map.styleSettings.markerScale = 1.0
    }

    override func getStyles() -> [(name: String, sheet: String)] {
        return [
            ("Custom", "maptiler_test_circles")
        ]
    }

}

