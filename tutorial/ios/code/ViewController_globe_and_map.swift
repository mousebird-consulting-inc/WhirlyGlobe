//
//  ViewController.swift
//  HelloEarth
//
//  Created by Steve Gifford on 12/18/19.
//  Copyright © 2019 Steve Gifford. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    let DoGlobe = true
    
    var theViewC : MaplyBaseViewController? = nil
        
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Start with either globe or map
        if DoGlobe {
            let globeViewC = WhirlyGlobeViewController()
            theViewC = globeViewC
        } else {
            let mapViewC = MaplyViewController()
            theViewC = mapViewC
        }

        // Wire this into the view hierarchy
        self.view.addSubview(theViewC!.view)
        theViewC!.view.frame = self.view.bounds
        addChild(theViewC!)
    }

}

