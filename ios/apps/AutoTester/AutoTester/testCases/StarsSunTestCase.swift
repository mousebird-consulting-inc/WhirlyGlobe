//
//  StarsSunTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class StarsSunTestCase: MaplyTestCase {

	let renderDate: Date
	
	override init() {
		self.renderDate = Date()

		super.init()

		self.name = "Stars/Sun"
		self.captureDelay = 10
		self.implementations = [.globe]
	}
	
	func addStars(_ globeVC: WhirlyGlobeViewController) {
		if let fileName = Bundle.main.path(forResource: "starcatalog_orig",
				ofType: "txt") {
			let stars = MaplyStarsModel(fileName: fileName)
			stars?.setImage(UIImage(named: "star_background")!)
			stars?.add(toViewC: globeVC, date: renderDate, desc: nil, mode: MaplyThreadMode.current)
		}
	}
    
    var atmosObj = MaplyAtmosphere()
	
	func addSun(_ globeVC: WhirlyGlobeViewController) {
		globeVC.clearColor = UIColor.black

		let sun = MaplySun(date: renderDate)
		let sunLight = sun.makeLight()
		
		globeVC.clearLights()
		globeVC.add(sunLight)
		
		let bill = MaplyBillboard()
		let centerGeo = sun.asPosition()
		
		bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, 5.4*6371000)
		bill.selectable = false
		bill.screenObj = MaplyScreenObject()
		
		let globeImage = UIImage(named: "SunImage")
		
		bill.screenObj?.addImage(globeImage, color: UIColor.white, size: CGSize(width: 0.9, height: 0.9))
		
		globeVC.addBillboards([bill],
			desc: [
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(value: kMaplySunDrawPriorityDefault as Int32)
			],
			mode: MaplyThreadMode.any)
		
		//Position for the moon
		
		let moon = MaplyMoon(date: renderDate)
		
		let billMoon = MaplyBillboard()
		
		let centerGeoMoon = moon.asPosition()
		
		billMoon.center = MaplyCoordinate3dMake(centerGeoMoon.x, centerGeoMoon.y, 5.4*6371000)
		billMoon.selectable = false
		billMoon.screenObj = MaplyScreenObject()
		let moonImage = UIImage(named: "moon")
		billMoon.screenObj?.addImage(moonImage,
			color: UIColor(white: CGFloat(moon.illuminatedFraction), alpha: 1.0),
			size: CGSize(width: 0.75, height: 0.75))
		globeVC.addBillboards([billMoon],
			desc: [
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(value: kMaplyMoonDrawPriorityDefault as Int32)
			],
			mode: MaplyThreadMode.any)
		
		// And some atmosphere, because the iDevice fill rate is just too fast
		atmosObj = MaplyAtmosphere.init(viewC: globeVC)!

		atmosObj.setWavelengthRed(0.650, green: 0.570, blue: 0.47)
		atmosObj.setSunPosition(sun.getDirection())

//		turnOnNightDay(globeVC, atmosObj: atmosObj!)
	}
	
	
	func turnOnNightDay (_ globeVC: WhirlyGlobeViewController, atmosObj : MaplyAtmosphere?) {
		let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
		let tileSource1 = MaplyRemoteTileInfo(baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/MODIS_Terra_CorrectedReflectance_TrueColor/default/2015-05-07/GoogleMapsCompatible_Level9/{z}/{y}/{x}", ext: "jpg", minZoom: Int32(1), maxZoom: Int32(8))
		let tileSource2 = MaplyRemoteTileInfo(baseURL: "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-05-07/GoogleMapsCompatible_Level8/{z}/{y}/{x}", ext: "jpg", minZoom: Int32(1), maxZoom: Int32(8))
		tileSource1.cacheDir = "\(cacheDir)/daytexture-2015-05-07/"
		tileSource2.cacheDir = "\(cacheDir)/nighttexture-2015-05-07/"
		
		let tileSource = MaplyMultiplexTileSource(sources: [tileSource1, tileSource2])
		
		let layer = MaplyQuadImageTilesLayer(coordSystem: tileSource1.coordSys!, tileSource: tileSource!)
		
		layer?.drawPriority = kMaplyImageLayerDrawPriorityDefault
		layer?.handleEdges = true
		layer?.requireElev = false
		layer?.maxTiles = 256
		layer?.imageDepth = 2
		layer?.allowFrameLoading = false
//		layer?.currentImage = 0.5
		layer?.singleLevelLoading = false
		layer?.shaderProgramName = kMaplyShaderDefaultTriNightDay
		layer?.setTesselationValues([(-1) : 10, 0 : 20, 1 : 16])
        // The ground shader is overkill here
//		layer?.shaderProgramName = atmosObj?.groundShader!.name
		globeVC.add(layer!)
		layer?.drawPriority = kMaplyImageLayerDrawPriorityDefault
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
//		let baseLayer  = GeographyClassTestCase()
//		baseLayer.setUpWithGlobe(globeVC)
		addStars(globeVC)
		addSun(globeVC)
		turnOnNightDay(globeVC, atmosObj: nil)
	}

}
