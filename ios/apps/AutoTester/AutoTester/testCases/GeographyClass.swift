//
//  GeographyClass.swift
//  AutoTester
//
//  Created by Steve Gifford on 2/12/20.
//  Copyright © 2015-2020 mousebird consulting.
//

import UIKit

public class GeographyClassTestCase: MaplyTestCase {

    override init() {
        super.init()

        self.name = "Geography Class"
        self.implementations = [.globe, .map]
    }
    
    typealias ImageLayer = (loader: MaplyQuadImageLoader, fetcher: MaplyMBTileFetcher)
    var layers : [ImageLayer] = []
    var varTarget : MaplyVariableTarget? = nil
    
    func setupMBTiles(_ name: String, offscreen: Bool, transparent: Bool, drawPriority: Int32, viewC: MaplyBaseViewController) -> ImageLayer? {
        guard let fetcher = MaplyMBTileFetcher(mbTiles: name) else {
            return nil
        }
        
        let params = MaplySamplingParams()
        if (transparent) {
            params.forceMinLevel = false
        }
        params.minZoom = 0
        params.maxZoom = fetcher.maxZoom()
        params.coordSys = MaplySphericalMercator(webStandard: ())
        params.singleLevel = true
        
        guard let loader = MaplyQuadImageLoader(params: params, tileInfo: fetcher.tileInfo(), viewC: viewC) else {
            return nil
        }
        loader.setTileFetcher(fetcher)
        loader.baseDrawPriority = drawPriority
        
        let layer = ImageLayer(loader: loader, fetcher: fetcher)

        // For offscreen rendering, we need a target
        if offscreen {
            if varTarget == nil {
                let target = MaplyVariableTarget(type: .imageIntRGBA, viewC: viewC)
                target.setScale(1.0)
                target.drawPriority = drawPriority
                target.buildRectangle = true
                varTarget = target
            }
            loader.setRenderTarget(varTarget!.renderTarget)
        }
        
        return layer
    }
    
    func setupLayers(_ viewC: MaplyBaseViewController) {
        if let layer = setupMBTiles("geography-class_medres",
                                    offscreen: false,
                                    transparent: false,
                                    drawPriority: kMaplyImageLayerDrawPriorityDefault,
                                    viewC: viewC) {
            layers.append(layer)
        }
    }
    
    public override func stop() {
        for layer in layers {
            layer.loader.shutdown()
        }
        layers = []
        if let target = varTarget {
            target.shutdown()
        }
        varTarget = nil
    }
    
    public override func setUpWithGlobe(_ globeVC: WhirlyGlobeViewController) {
        setupLayers(globeVC)
        globeVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }

    public override func setUpWithMap(_ mapVC: MaplyViewController) {
        setupLayers(mapVC)
        mapVC.animate(toPosition: MaplyCoordinateMakeWithDegrees(151.211111, -33.859972), time: 1.0)
    }

}
