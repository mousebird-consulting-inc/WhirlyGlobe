//
//  StarsSunTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright 2015-2022 mousebird consulting.
//

import UIKit
import WhirlyGlobe

class StarsSunTestCase: MaplyTestCase {

	override init() {
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
    
	func addSun(_ globeVC: WhirlyGlobeViewController,
                _ sun: MaplySun,
                ambient: Float = 0.1,
                diffuse: Float = 0.8) -> MaplyComponentObject? {

        if let light = sun.makeLight(withAmbient: ambient, diffuse: diffuse) {
            globeVC.add(light)
        }

		let bill = MaplyBillboard()
		let centerGeo = sun.position
        let maxDistRad = 5.0 // 0.9 * globeVC.getMaxHeightAboveGlobe()    // need to compute from max view height...
        let distance = maxDistRad * 6371009     // actually about 23,000 Earth radii
		
		bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, Float(distance))
		bill.selectable = false
		bill.screenObj = MaplyScreenObject()
		
		let globeImage = UIImage(named: "SunImage")
		bill.screenObj?.addImage(globeImage, color: UIColor.white, size: CGSize(width: 0.9, height: 0.9))
		
		return globeVC.addBillboards([bill],
			desc: [
                kMaplyEnable: false,
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(value: kMaplySunDrawPriorityDefault as Int32)
			],
			mode: MaplyThreadMode.current)
    }

    func addMoon(_ globeVC: WhirlyGlobeViewController,
                 _ moon: MaplyMoon,
                 ambient: Float = 0.0,
                 diffuse: Float = 0.1) -> MaplyComponentObject? {
        
        let frac = Float(moon.illuminatedFraction)
        if let light = moon.makeLight(withAmbient: ambient * frac, diffuse: diffuse * frac) {
            globeVC.add(light)
        }

        let centerGeoMoon = moon.position
        let distance = 4.0 * 6371009  // actually about 58 Earth radii

        let billMoon = MaplyBillboard()
		billMoon.center = MaplyCoordinate3dMake(centerGeoMoon.x, centerGeoMoon.y, Float(distance))
		billMoon.selectable = false
		billMoon.screenObj = MaplyScreenObject()
		let moonImage = UIImage(named: "moon")
        let moonColor = CGFloat(moon.illuminatedFraction * 0.9 + 0.1)   // don't actually go to zero
        billMoon.screenObj?.addImage(moonImage,
                                     color: UIColor(white: moonColor, alpha: 1.0),
                                     size: CGSize(width: 0.5, height: 0.5))
		return globeVC.addBillboards([billMoon],
			desc: [
                kMaplyEnable: false,
				kMaplyBillboardOrient: kMaplyBillboardOrientEye,
				kMaplyDrawPriority: NSNumber(value: kMaplyMoonDrawPriorityDefault as Int32)
			],
			mode: MaplyThreadMode.current)
	}

    func addAtmosphere(_ globeVC: WhirlyGlobeViewController,
                       _ sun: MaplySun) -> MaplyAtmosphere? {
        // And some atmosphere, because the iDevice fill rate is just too fast
        guard let atmosObj = MaplyAtmosphere(viewC: globeVC) else { return nil }
        atmosObj.setWavelengthRed(0.650, green: 0.570, blue: 0.47)
        atmosObj.setSunPosition(sun.direction)
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

        let dateStr = "2021-11-08T00:00:00Z"
        let layer = "VIIRS_NOAA20_CorrectedReflectance_TrueColor"
        let baseUrl = "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/\(layer)/default/\(dateStr)/GoogleMapsCompatible_Level9/{z}/{y}/{x}.jpg"
		let tileSource1 = MaplyRemoteTileInfoNew(
            baseURL: baseUrl,
            minZoom: sampleParams.minZoom,
            maxZoom: sampleParams.maxZoom)
        tileSource1.cacheDir = "\(cacheDir)/daytexture-\(dateStr)/"

        // TODO: Get the loader to respect missing tiles at the lowest levels
        let nightDateStr = "2015-07-01";
        let nightUrl = "http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/\(nightDateStr)/GoogleMapsCompatible_Level8/{z}/{y}/{x}.jpg"
		let tileSource2 = MaplyRemoteTileInfoNew(baseURL: nightUrl, minZoom: 1, maxZoom: 8)
		tileSource2.cacheDir = "\(cacheDir)/nighttexture-\(nightDateStr)/"

        guard let imageLoader = MaplyQuadImageFrameLoader(params: sampleParams, tileInfos: [tileSource1, tileSource2], viewC: globeVC) else {
            return nil
        }
        imageLoader.setShader(globeVC.getShaderByName(kMaplyShaderDefaultTriNightDay))
        // This value isn't used directly, but a value between 0 and 1 is required to
        // make the loader pass the tiles from both sources as textures to the shader.
        imageLoader.setCurrentImage(0.5)

        return imageLoader
	}
	
    func setupSunAndMoon(_ globeVC: WhirlyGlobeViewController) -> MaplySun? {
        guard let sun = MaplySun(date: renderDate),
              let moon = MaplyMoon(date: renderDate) else {
              return nil;
          }

        let oldObjs = [sunObj, moonObj].compactMap { $0 }
        
        globeVC.clearLights()
        sunObj = addSun(globeVC, sun, ambient: 0.0, diffuse: 0.5)
        moonObj = addMoon(globeVC, moon, ambient: 0.0, diffuse: 0.2)

        let newObjs = [sunObj, moonObj].compactMap { $0 }
        globeVC.enable(newObjs, mode: .current)
        globeVC.remove(oldObjs, mode: .current)

        // Add some non-directional ambient light to make the night side visible
        let light = MaplyLight()
        light.ambient = UIColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1.0);
        light.viewDependent = false;
        globeVC.add(light)

        return sun;
    }

	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {

        globeVC.clearColor = UIColor.black

        guard let sun = setupSunAndMoon(globeVC) else { return }

        atmosObj = addAtmosphere(globeVC, sun)
        imageLoader = turnOnNightDay(globeVC, atmosObj: atmosObj)
        stars = addStars(globeVC)

        // Put the terminator in view
        globeVC.setPosition(MaplyCoordinateMake(sun.position.x + Float.pi * 0.3, 0.0), height: 2.0)

        // todo: use an active object to update more smoothly
        timer = Timer.scheduledTimer(withTimeInterval: 0.05, repeats: true) { [weak self, weak globeVC] (t) in
            guard let self = self, let vc = globeVC, t.isValid else { return }
            self.renderOffset = -self.startDate.timeIntervalSinceNow * self.timeFactor
            if let sun = self.setupSunAndMoon(vc) {
                self.atmosObj?.setSunPosition(sun.direction)
            }
        }
	}

    override func stop() {
        timer?.invalidate()
        timer = nil

        if let vc = baseViewController {
            vc.remove([sunObj, moonObj].compactMap { $0 }, mode: .current)
        }

        sunObj = nil
        moonObj = nil
        atmosObj?.removeFromViewC()
        atmosObj = nil
        stars?.removeFromViewC()
        stars = nil
        imageLoader?.shutdown()
        imageLoader = nil
        super.stop()
    }

    let startDate = Date()
    var renderDate: Date { get { Date().addingTimeInterval(renderOffset) } }
    var renderOffset: TimeInterval = 0.0
    let timeFactor = 2000.0
   
    var sunObj: MaplyComponentObject?
    var moonObj: MaplyComponentObject?
    var stars: MaplyStarsModel?
    var atmosObj: MaplyAtmosphere?
    var imageLoader: MaplyQuadImageFrameLoader?
    var timer: Timer?
}
