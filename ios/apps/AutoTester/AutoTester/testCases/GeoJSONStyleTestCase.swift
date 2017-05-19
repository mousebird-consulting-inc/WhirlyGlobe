//
//  GeoJSONStyleTestCase.swift
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-12-23.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

import UIKit

class GeoJSONStyleTestCase: MaplyTestCase {
    
    fileprivate var roadsSource: GeoJSONSource?
    fileprivate var waterAreaSource: GeoJSONSource?
    fileprivate var waterLineSource: GeoJSONSource?
    fileprivate var buildingsSource: GeoJSONSource?
    fileprivate var landUseSource: GeoJSONSource?
    fileprivate var amenitiesSource: GeoJSONSource?
    
    fileprivate var switchRoads: UISwitch?
    fileprivate var switchWater: UISwitch?
    fileprivate var switchBuildings: UISwitch?
    fileprivate var switchLandUse: UISwitch?
    fileprivate var switchAmenities: UISwitch?
    
    override init() {
        super.init()
        
        self.name = "GeoJSON SLD Style Test Case"
        self.captureDelay = 4
        self.implementations = [.globe, .map]
    }
    
    func setupCommon(baseVC: MaplyBaseViewController) {
        let frameView = UIView.init(frame: CGRect(x: 10, y: 75, width: 180, height: 215))
        frameView.backgroundColor = UIColor.lightGray;
        frameView.alpha = 0.8
        baseVC.view.addSubview(frameView)
        
        self.setupMenuWithFrameView(frameView: frameView)
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            
            self.landUseSource = GeoJSONSource.init(viewC: self.baseViewController!, geoJSONURL: Bundle.main.url(forResource: "belfast_ireland_landusages", withExtension: "geojson")!, sldURL: Bundle.main.url(forResource: "osm_landuse", withExtension: "sld")!, relativeDrawPriority:100)
            self.landUseSource?.startParse(completion: {
                
                self.switchLandUse?.isOn = true
                self.switchLandUse?.isEnabled = true
                
                self.waterAreaSource = GeoJSONSource.init(viewC: self.baseViewController!, geoJSONURL: Bundle.main.url(forResource: "belfast_ireland_waterareas", withExtension: "geojson")!, sldURL: Bundle.main.url(forResource: "osm_water", withExtension: "sld")!, relativeDrawPriority:200)
                self.waterAreaSource?.startParse(completion: {
                    
                    self.waterLineSource = GeoJSONSource.init(viewC: self.baseViewController!, geoJSONURL: Bundle.main.url(forResource: "belfast_ireland_waterways", withExtension: "geojson")!, sldURL: Bundle.main.url(forResource: "water_lines", withExtension: "sld")!, relativeDrawPriority:300)
                    self.waterLineSource?.startParse(completion: {
                        
                        self.switchWater?.isOn = true
                        self.switchWater?.isEnabled = true
                        
                        self.buildingsSource = GeoJSONSource.init(viewC: self.baseViewController!, geoJSONURL: Bundle.main.url(forResource: "belfast_ireland_buildings", withExtension: "geojson")!, sldURL: Bundle.main.url(forResource: "osm_buildings", withExtension: "sld")!, relativeDrawPriority:400)
                        self.buildingsSource?.startParse(completion: {
                            
                            self.switchBuildings?.isOn = true
                            self.switchBuildings?.isEnabled = true
                            
                            self.roadsSource = GeoJSONSource.init(viewC: self.baseViewController!, geoJSONURL: Bundle.main.url(forResource: "belfast_ireland_roads", withExtension: "geojson")!, sldURL: Bundle.main.url(forResource: "osm_roads", withExtension: "sld")!, relativeDrawPriority:500)
                            self.roadsSource?.startParse(completion: {
                                
                                self.switchRoads?.isOn = true
                                self.switchRoads?.isEnabled = true
                                
                                self.amenitiesSource = GeoJSONSource.init(viewC: self.baseViewController!, geoJSONURL: Bundle.main.url(forResource: "belfast_ireland_amenities", withExtension: "geojson")!, sldURL: Bundle.main.url(forResource: "amenities", withExtension: "sld")!, relativeDrawPriority:600)
                                self.amenitiesSource?.startParse(completion: {
                                    
                                    self.switchAmenities?.isOn = true
                                    self.switchAmenities?.isEnabled = true
                                    
                                })

                                
                            })
                        })
                    })
                })
            })
        }
    }
    
    override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = CartoDBTestCase()
        baseLayer.setUpWithGlobe(globeVC)
        
        setupCommon(baseVC: globeVC)
        
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(-5.93, 54.597), height: 0.005, heading: 0.0, time: 0.5)
    }

    override func setUpWithMap(_ mapVC: MaplyViewController) {
        let baseLayer = CartoDBTestCase()
        baseLayer.setUpWithMap(mapVC)
        
        setupCommon(baseVC: mapVC)
        
        mapVC.animate(toPosition:MaplyCoordinateMakeWithDegrees(-5.93, 54.597), height: 0.005, time: 1.0)
        mapVC.setZoomLimitsMin(0.0002, max: 4.0)

    }
    
    func onSwitchLandUse() {
        self.landUseSource!.enabled = !(self.landUseSource!.enabled)
    }
    
    func onSwitchWater() {
        self.waterAreaSource!.enabled = !(self.waterLineSource!.enabled)
        self.waterLineSource!.enabled = !(self.waterLineSource!.enabled)
    }
    
    func onSwitchBuildings() {
        self.buildingsSource!.enabled = !(self.buildingsSource!.enabled)
    }
    
    func onSwitchRoads() {
        self.roadsSource!.enabled = !(self.roadsSource!.enabled)
    }
    
    func onSwitchAmenities() {
        self.amenitiesSource!.enabled = !(self.amenitiesSource!.enabled)
    }

    
    func teardownCommon(baseVC: MaplyBaseViewController) {
        
    }
    
    override func tearDown(withMap mapVC: MaplyViewController) {
        teardownCommon(baseVC: mapVC)
    }
    
    override func tearDown(withGlobe globeVC: WhirlyGlobeViewController) {
        teardownCommon(baseVC: globeVC)
    }
    
    func setupMenuWithFrameView(frameView: UIView) {
        
        var label = UILabel.init(frame: CGRect(x: 10, y: 20, width: 100, height: 30))
        label.text = "Land Use"
        frameView.addSubview(label)
        
        switchLandUse = UISwitch.init(frame: CGRect(x: 120, y: 20, width: 60, height: 30))
        switchLandUse?.isOn = false
        switchLandUse?.isEnabled = false
        switchLandUse?.addTarget(self, action: #selector(onSwitchLandUse), for: .valueChanged)
        frameView.addSubview(switchLandUse!)
        
        
        label = UILabel.init(frame: CGRect(x: 10, y: 55, width: 100, height: 30))
        label.text = "Water"
        frameView.addSubview(label)
        
        switchWater = UISwitch.init(frame: CGRect(x: 120, y: 55, width: 60, height: 30))
        switchWater?.isOn = false
        switchWater?.isEnabled = false
        switchWater?.addTarget(self, action: #selector(onSwitchWater), for: .valueChanged)
        frameView.addSubview(switchWater!)
        
        
        label = UILabel.init(frame: CGRect(x: 10, y: 90, width: 100, height: 30))
        label.text = "Buildings"
        frameView.addSubview(label)
        
        switchBuildings = UISwitch.init(frame: CGRect(x: 120, y: 90, width: 60, height: 30))
        switchBuildings?.isOn = false
        switchBuildings?.isEnabled = false
        switchBuildings?.addTarget(self, action: #selector(onSwitchBuildings), for: .valueChanged)
        frameView.addSubview(switchBuildings!)
        
        
        label = UILabel.init(frame: CGRect(x: 10, y: 125, width: 100, height: 30))
        label.text = "Roads"
        frameView.addSubview(label)
        
        switchRoads = UISwitch.init(frame: CGRect(x: 120, y: 125, width: 60, height: 30))
        switchRoads?.isOn = false
        switchRoads?.isEnabled = false
        switchRoads?.addTarget(self, action: #selector(onSwitchRoads), for: .valueChanged)
        frameView.addSubview(switchRoads!)
        

        label = UILabel.init(frame: CGRect(x: 10, y: 160, width: 100, height: 30))
        label.text = "Amenities"
        frameView.addSubview(label)
        
        switchAmenities = UISwitch.init(frame: CGRect(x: 120, y: 160, width: 60, height: 30))
        switchAmenities?.isOn = false
        switchAmenities?.isEnabled = false
        switchAmenities?.addTarget(self, action: #selector(onSwitchAmenities), for: .valueChanged)
        frameView.addSubview(switchAmenities!)

        
    }
}
