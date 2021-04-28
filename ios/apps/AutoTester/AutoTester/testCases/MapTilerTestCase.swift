//
//  MapTiler.swift
//  AutoTester
//
//  Created by Steve Gifford on 11/8/19.
//  Copyright © 2021 mousebird consulting. All rights reserved.
//

import UIKit

class MapTilerTestCase: MaplyTestCase {

    init(_ name: String, _ impl: MaplyTestCaseImplementations = [.map,.globe]) {
        super.init()
        self.name = name
        self.implementations = impl
        self.styles = getStyles()
    }

    override convenience init() {
        self.init("MapTiler Test Cases",[.map,.globe])
        
        let env = ProcessInfo.processInfo.environment
        mapTilerStyle = NumberFormatter().number(from: env["MAPTILER_STYLE"] ?? "")?.intValue ?? mapTilerStyle
    }

    func getStyles() -> [(name: String, sheet: String)] {
        return [
            ("Basic", "maptiler_basic"),
            ("Hybrid Satellite", "maptiler_hybrid_satellite"),
            ("Streets", "maptiler_streets"),
    //         ("Topo", "maptiler_topo"),   // ?
            ("Custom", "maptiler_expr_test")
        ]
    }

    var styles = [(name: String, sheet: String)]()
    var mapTilerStyle = 2
    var mapboxMap : MapboxKindaMap? = nil

    // Start fetching the required pieces for a Mapbox style map
    func startMap(_ style: (name: String, sheet: String), viewC: MaplyBaseViewController, round: Bool) {
        guard let fileName = Bundle.main.url(forResource: style.sheet, withExtension: "json") else {
            print("Style sheet missing from bundle: \(style.sheet)")
            return
        }
        
        print("Starting map with \(style.name) / \(style.sheet)")
        
        // Maptiler token
        // Go to maptiler.com, setup an account and get your own.
        // Go to Edit Scheme, select Run, Arguments, and add an "MAPTILER_TOKEN" entry to Environment Variables.
        let token = ProcessInfo.processInfo.environment["MAPTILER_TOKEN"] ?? "GetYerOwnToken"
        if token.count == 0 || token == "GetYerOwnToken" {
            let alertControl = UIAlertController(title: "Missing Token", message: "You need to add your own Maptiler token.\nYou can't use mine.", preferredStyle: .alert)
            alertControl.addAction(UIAlertAction(title: "Fine!", style: .cancel, handler: { _ in
                alertControl.dismiss(animated: true, completion: nil)
            }))
            viewC.present(alertControl, animated: true, completion: nil)
            return
        }

        // Parse it and then let it start itself
        let mapboxMap = MapboxKindaMap(fileName, viewC: viewC)
        mapboxMap.backgroundAllPolys = round     // Render all the polygons into an image for the globe
        mapboxMap.cacheDir = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask)[0].appendingPathComponent(name)
        // Replace the MapTilerKey in any URL with the actual token
        mapboxMap.fileOverride = {
            (url) in
            if url.isFileURL {
                return url
            }
            if url.absoluteString.contains("MapTilerKey") {
                return URL(string: url.absoluteString.replacingOccurrences(of: "MapTilerKey", with: token))!
            }
            // Tack a key on the end otherwise
            return URL(string: url.absoluteString.appending("?key=\(token)"))!
        }
        mapboxMap.postSetup = { (map) in
            let barItem = UIBarButtonItem(barButtonSystemItem: .edit, target: self, action: #selector(self.editAction))
            viewC.navigationItem.rightBarButtonItem = barItem
            // Display the legend
            if let legendVC = UIStoryboard(name: "LegendViewController", bundle: .main).instantiateInitialViewController() as? LegendViewController {
                legendVC.styleSheet = map.styleSheet
                legendVC.preferredContentSize = CGSize(width: 320.0, height: viewC.view.bounds.height)
                self.legendVC = legendVC
            }
        }
        setup(mapboxMap)
        mapboxMap.start()
        self.mapboxMap = mapboxMap
    }
    
    func setup(_ map: MapboxKindaMap) {
        map.styleSettings.textScale = 1.1
    }

    var legendVisibile = false
    var legendVC: LegendViewController? = nil
    
    @objc func editAction(_ sender: Any) {
        guard let legendVC = legendVC else {
            return
        }
        
        if legendVisibile {
            legendVC.dismiss(animated: true, completion: nil)
        } else {
            legendVC.modalPresentationStyle = .popover
            legendVC.popoverPresentationController?.sourceView = sender as? UIView
            legendVC.popoverPresentationController?.barButtonItem = sender as? UIBarButtonItem

            baseViewController?.present(legendVC, animated: true)
        }
    }
    
    override func stop() {
        mapboxMap?.stop()
        mapboxMap = nil
        legendVC = nil
        
        super.stop()
    }
    
    override func setUpWithMap(_ mapVC: MaplyViewController) {
        //mapVC.performanceOutput = true
        
        startMap(styles[mapTilerStyle], viewC: mapVC, round: false)

        mapVC.rotateGestureThreshold = 15;

        runProgram(mapVC)
        
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-100, 40.0), height: 1.5, time: 0.1)
    }
    
    override func setUpWithGlobe(_ mapVC: WhirlyGlobeViewController) {
        //mapVC.performanceOutput = true
        
        startMap(styles[mapTilerStyle], viewC: mapVC, round: true)
        
        runProgram(mapVC)
    }

    private func runProgram(_ viewC: MaplyBaseViewController) {
        let map = (viewC as? MaplyViewController)
        let globe = (viewC as? WhirlyGlobeViewController)
        
        // e.g., "35.66,139.835,0.025,0.0025,2,20"
        let env = ProcessInfo.processInfo.environment
        if let program = env["MAPTILER_PROGRAM"] {
            let components = program.components(separatedBy: ",")
                                    .map(NumberFormatter().number)
            if components.count == 6 {
                let lat = components[0]?.floatValue ?? 0
                let lon = components[1]?.floatValue ?? 0
                let center = MaplyCoordinate(x:lon*Float.pi/180, y:lat*Float.pi/180)
                let outHeight = components[2]?.floatValue ?? 0.01
                let inHeight = components[3]?.floatValue ?? 0.001
                let interval = TimeInterval(components[4]?.doubleValue ?? 1)
                let count = components[5]?.intValue ?? 1
                
                map?.setPosition(center, height: outHeight)
                globe?.setPosition(center, height: outHeight)

                for i in 0 ... 2 * count {
                    Timer.scheduledTimer(withTimeInterval: TimeInterval(i) * interval, repeats: false) { _ in
                        let height = (i % 2 == 0) ? inHeight : outHeight
                        map?.animate(toPosition: center, height: height, time: interval)
                        globe?.animate(toPosition: center, height: height, heading: 0, time: interval)
                    }
                }
            }
        }
    }

    override func maplyViewController(_ viewC: MaplyViewController, didTapAt coord: MaplyCoordinate) {
        mapboxMap?.stop()
        mapboxMap = nil

        mapTilerStyle = (mapTilerStyle + 1) % styles.count
        startMap(styles[mapTilerStyle], viewC: viewC, round: false)
    }
}

