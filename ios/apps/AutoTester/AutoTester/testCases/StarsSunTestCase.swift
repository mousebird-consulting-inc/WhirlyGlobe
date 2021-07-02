//
//  StarsSunTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015-2021 mousebird consulting.
//

import UIKit

class StarsSunTestCase: MaplyTestCase {

	override init() {
		self.renderDate = Date()
        self.sun = MaplySun(date: renderDate)
        self.moon = MaplyMoon(date: renderDate)
        super.init(name: "Stars/Sun (broken)", supporting: [.globe])
	}
	
	func addStars(_ globeVC: WhirlyGlobeViewController) -> MaplyStarsModel? {
		if let fileName = Bundle.main.path(forResource: "starcatalog_orig", ofType: "txt"),
           let stars = MaplyStarsModel(fileName: fileName),
           let starImage = UIImage(named: "star_background")
        {
			stars.setImage(starImage)

            let desc = [
                kMaplyEnable: true,
                kMaplyPointSize: 10,
            ] as [String : Any];
			stars.add(toViewC: globeVC, date: renderDate, desc: desc, mode: MaplyThreadMode.current)
            return stars
		}
        return nil
	}
    
	func addSun(_ globeVC: WhirlyGlobeViewController) -> MaplyComponentObject? {
		globeVC.clearLights()
		globeVC.add(sun.makeLight())
		
		let bill = MaplyBillboard()
		let centerGeo = sun.asPosition()
		
		bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, 5.4*6371000)
		bill.selectable = false
		bill.screenObj = MaplyScreenObject()
		
		let globeImage = UIImage(named: "SunImage")
		bill.screenObj?.addImage(globeImage, color: UIColor.white, size: CGSize(width: 0.9, height: 0.9))
		
		return globeVC.addBillboards([bill],
			desc: [
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(value: kMaplySunDrawPriorityDefault as Int32)
			],
			mode: MaplyThreadMode.any)
    }

    func addMoon(_ globeVC: WhirlyGlobeViewController) -> MaplyComponentObject? {
        let centerGeoMoon = moon.asPosition()

        let billMoon = MaplyBillboard()
		billMoon.center = MaplyCoordinate3dMake(centerGeoMoon.x, centerGeoMoon.y, 5.4*6371000)
		billMoon.selectable = false
		billMoon.screenObj = MaplyScreenObject()
		let moonImage = UIImage(named: "moon")
        let moonColor = CGFloat(moon.illuminatedFraction * 0.9 + 0.1)   // don't actually go to zero
        billMoon.screenObj?.addImage(moonImage,
                                     color: UIColor(white: moonColor, alpha: 1.0),
                                     size: CGSize(width: 0.75, height: 0.75))
		return globeVC.addBillboards([billMoon],
			desc: [
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(value: kMaplyMoonDrawPriorityDefault as Int32)
			],
			mode: MaplyThreadMode.any)
	}

    func addAtmosphere(_ globeVC: WhirlyGlobeViewController) -> MaplyAtmosphere? {
        // And some atmosphere, because the iDevice fill rate is just too fast
        guard let atmosObj = MaplyAtmosphere(viewC: globeVC) else { return nil }
        atmosObj.setWavelengthRed(0.650, green: 0.570, blue: 0.47)
        atmosObj.setSunPosition(sun.getDirection())
        return atmosObj
    }
	
	func turnOnNightDay (_ globeVC: WhirlyGlobeViewController, atmosObj : MaplyAtmosphere?) -> MaplyQuadImageFrameLoader? {
        // Parameters describing how we want a globe broken down
        let sampleParams = MaplySamplingParams()
        sampleParams.coordSys = MaplySphericalMercator(webStandard: ())
        sampleParams.coverPoles = true
        sampleParams.edgeMatching = true
        sampleParams.minZoom = 1
        sampleParams.maxZoom = 9
        sampleParams.singleLevel = true
        
        // Two tile sources, one night and one day
        let cacheDir = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]

        let dateFmt = DateFormatter()
        dateFmt.timeZone = TimeZone(secondsFromGMT: 0)
        //dateFmt.dateFormat = "yyyy-MM-dd'T'HH:00:00'Z'"
        dateFmt.dateFormat = "yyyy-MM-dd"
        let dateStr = dateFmt.string(from: renderDate)

        let layer = "VIIRS_NOAA20_CorrectedReflectance_TrueColor"
        let baseUrl = "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/\(layer)/default/\(dateStr)/GoogleMapsCompatible_Level9/{z}/{y}/{x}.jpg"
		let tileSource1 = MaplyRemoteTileInfoNew(
            baseURL: baseUrl,
            minZoom: sampleParams.minZoom,
            maxZoom: sampleParams.maxZoom)
        tileSource1.cacheDir = "\(cacheDir)/daytexture-\(dateStr)/"

        // TODO: Get the loader to respect missing tiles at the lowest levels
        let lightUrl = "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-05-07/GoogleMapsCompatible_Level8/{z}/{y}/{x}.jpg"
		let tileSource2 = MaplyRemoteTileInfoNew(
            baseURL: lightUrl,
            minZoom: 1,
            maxZoom: 8)
		tileSource2.cacheDir = "\(cacheDir)/nighttexture-2015-05-07-2/"

        guard let imageLoader = MaplyQuadImageFrameLoader(params: sampleParams, tileInfos: [tileSource1, tileSource2], viewC: globeVC) else {
            return nil
        }
        imageLoader.setShader(globeVC.getShaderByName(kMaplyShaderDefaultTriNightDay))
        imageLoader.setCurrentImage(0.5)
        
        return imageLoader
	}
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        globeVC.clearColor = UIColor.black
        atmosObj = addAtmosphere(globeVC)
        imageLoader = turnOnNightDay(globeVC, atmosObj: atmosObj)
        sunObj = addSun(globeVC)
        moonObj = addMoon(globeVC)
        stars = addStars(globeVC)
        let moonPos = moon.asPosition()
        let pos = MaplyCoordinateMake(moonPos.x + Float.pi * 0.8, -moonPos.y)
        globeVC.setPosition(pos, height: 3)
	}

    override func stop() {
        if let sunObj = sunObj { baseViewController?.remove(sunObj) }
        sunObj = nil
        if let moonObj = moonObj { baseViewController?.remove(moonObj) }
        moonObj = nil
        atmosObj?.removeFromViewC()
        atmosObj = nil
        stars?.removeFromViewC()
        stars = nil
        imageLoader?.shutdown()
        imageLoader = nil
        super.stop()
    }

    let renderDate: Date
    let sun: MaplySun
    var sunObj: MaplyComponentObject?
    let moon: MaplyMoon
    var moonObj: MaplyComponentObject?
    var stars: MaplyStarsModel?
    var atmosObj: MaplyAtmosphere?
    var imageLoader: MaplyQuadImageFrameLoader?
}
