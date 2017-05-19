//
//  StickersTestCase.swift
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit

class StickersTestCase: MaplyTestCase {

	override init() {
		super.init()
		
		self.name = "Stickers"
		self.captureDelay = 4
		self.implementations = [.globe, .map]
	}

	func addStickers (_ arrayComp: NSArray , baseViewC: MaplyBaseViewController) {
		let startImage = UIImage(named: "Smiley_Face_Avatar_by_PixelTwist")
        let startTex = baseViewC.addTexture(startImage!, desc: [kMaplyTexMagFilter: kMaplyMinFilterLinear], mode: MaplyThreadMode.current)
		var stickers = [MaplySticker]()
		for object in arrayComp {
			let sticker = MaplySticker()
			let center = (object as! MaplyVectorObject).center()
			sticker.ll = center
			sticker.ur = MaplyCoordinate(x: center.x + 0.1, y: center.y+0.1)
			sticker.image = startTex
			stickers.append(sticker)
		}
		baseViewC.addStickers(stickers, desc: [kMaplyFade: (1.0)])
	}
    
    func addTestSticker (baseViewC: MaplyBaseViewController) {
        let startImage = UIImage(named: "greensquare.png")
        
        let sticker = MaplySticker()
        sticker.coordSys = MaplySphericalMercator.init(webStandard: ())
        sticker.ll = (sticker.coordSys?.geo(toLocal: MaplyCoordinateMakeWithDegrees(-223.9008, -63.4974) ))!
        sticker.ur = (sticker.coordSys?.geo(toLocal: MaplyCoordinateMakeWithDegrees(-105.3074, 36.1572) ))!
//        sticker.ll.x -= Float(2*M_PI)
//        sticker.ll = (sticker.coordSys?.geo(toLocal: MaplyCoordinateMakeWithDegrees(-76.4594, -19.9719) ))!
//        sticker.ur = (sticker.coordSys?.geo(toLocal: MaplyCoordinateMakeWithDegrees( 19.954, 61.5041) ))!

//        sticker.ll = MaplyCoordinateMakeWithDegrees(-223.9008, -63.4974)
//        sticker.ur = MaplyCoordinateMakeWithDegrees(-105.3074, 36.1572)
//        sticker.ll = MaplyCoordinateMakeWithDegrees(-76.4594, -19.9719)
//        sticker.ur = MaplyCoordinateMakeWithDegrees(19.954, 61.5041)
        sticker.image = startImage
//        baseViewC.addStickers([sticker], desc: [kMaplyColor: UIColor.init(white: 0.25, alpha: 0.25)] )
        baseViewC.addStickers([sticker], desc: [kMaplyColor: UIColor.init(white: 1.0, alpha: 1.0)] )
    }
	
	override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        let baseLayer = VectorsTestCase()
        baseLayer.setUpWithGlobe(globeVC)

        addStickers(baseLayer.compList, baseViewC: globeVC)
        
//        addTestSticker(baseViewC: globeVC)
	}
	
	override func setUpWithMap(_ mapVC: MaplyViewController) {
		let baseLayer = VectorsTestCase()
		baseLayer.setUpWithMap(mapVC)

//        addStickers(baseLayer.compList, baseViewC: mapVC)
        
        addTestSticker(baseViewC: mapVC)
	}
}
