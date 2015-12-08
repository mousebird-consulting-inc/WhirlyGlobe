//
//  StarsSunTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class StarsSunTestCase: MaplyTestCase {

	let renderDate: NSDate
	
	override init() {
		self.renderDate = NSDate()

		super.init()

		self.name = "Stars/Sun"
		self.captureDelay = 10
	}
	
	func addStars(globeVC: WhirlyGlobeViewController) {
		if let fileName = NSBundle.mainBundle().pathForResource("starcatalog_orig",
				ofType: "txt") {
			let stars = MaplyStarsModel(fileName: fileName)
			stars?.setImage(UIImage(named: "star_background")!)
			stars?.addToViewC(globeVC, date: renderDate, desc: nil, mode: MaplyThreadMode.Current)
		}
	}
	
	func addSun(globeVC: WhirlyGlobeViewController) {
		globeVC.clearColor = UIColor.blackColor()

		let sun = MaplySun(date: renderDate)
		let sunLight = sun.makeLight()
		
		globeVC.clearLights()
		globeVC.addLight(sunLight)
		
		let bill = MaplyBillboard()
		let centerGeo = sun.asPosition()
		
		bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, 5.4*6371000)
		bill.selectable = false
		bill.screenObj = MaplyScreenObject()
		
		let globeImage = UIImage(named: "SunImage")
		
		bill.screenObj?.addImage(globeImage, color: UIColor.whiteColor(), size: CGSizeMake(0.9, 0.9))
		
		globeVC.addBillboards([bill],
			desc: [
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(int: kMaplySunDrawPriorityDefault)
			],
			mode: MaplyThreadMode.Any)
		
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
			size: CGSizeMake(0.75, 0.75))
		globeVC.addBillboards([billMoon],
			desc: [
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(int: kMaplyMoonDrawPriorityDefault)
			],
			mode: MaplyThreadMode.Any)
		
		// And some atmosphere, because the iDevice fill rate is just too fast
		let atmosObj = MaplyAtmosphere(viewC: globeVC)

		atmosObj?.setWavelengthRed(0.650, green: 0.570, blue: 0.47)
		atmosObj?.setSunPosition(sun.getDirection())

		turnOnNightDay(globeVC, atmosObj: atmosObj!)
	}
	
	
	func turnOnNightDay (globeVC: WhirlyGlobeViewController, atmosObj : MaplyAtmosphere) {
		let cacheDir = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]
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
		layer?.currentImage = 0.5
		layer?.singleLevelLoading = false
		layer?.shaderProgramName = kMaplyShaderDefaultTriNightDay
		layer?.setTesselationValues([(-1) : 10, 0 : 20, 1 : 16])
		layer?.shaderProgramName = atmosObj.groundShader!.name
		globeVC.addLayer(layer!)
		layer?.drawPriority = kMaplyImageLayerDrawPriorityDefault
	}
	
	override func setUpWithGlobe(globeVC: WhirlyGlobeViewController) -> Bool {
		let baseLayer  = GeographyClassTestCase()
		baseLayer.setUpWithGlobe(globeVC)
		addStars(globeVC)
		addSun(globeVC)
		return true
	}

}
