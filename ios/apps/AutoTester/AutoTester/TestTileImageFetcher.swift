//
//  TestTileImageFetcher.swift
//  AutoTester
//
//  Created by Tim Sylvester on 4/20/21.
//  Copyright © 2021 mousebird consulting. All rights reserved.
//

import Foundation

class TestTileImageFetcher : NSObject, MaplyTileFetcher {

    public var tileSize = 256
    public var alpha = 255

    init(control: MaplyBaseViewController, name: String, minZoom: Int = 0, maxZoom: Int = 20) {
        self.control = control
        self.fetcherName = name
        self.minZoom = minZoom
        self.maxZoom = maxZoom
        super.init()
        queue.name = name
        queue.maxConcurrentOperationCount = 5
    }

    func name() -> String { return fetcherName }

    public class TestTileInfo : NSObject, MaplyTileInfoNew {
        init(minZoom: Int32, maxZoom: Int32) {
            self._minZoom = minZoom
            self._maxZoom = maxZoom
        }
        func fetchInfo(forTile tileID: MaplyTileID, flipY: Bool) -> Any? {
            return TestTileFetchInfo(tileID: tileID)
        }
        func minZoom() -> Int32 { return _minZoom }
        func maxZoom() -> Int32 { return _maxZoom }
        private let _minZoom: Int32
        private let _maxZoom: Int32
    }
    
    private struct TestTileFetchInfo {
        let tileID: MaplyTileID
    }

    func startTileFetches(_ requests: [MaplyTileFetchRequest]) {
        for request in requests {
            queue.addOperation { self.startTileFetch(request) }
        }
    }
    
    func startTileFetch(_ request: MaplyTileFetchRequest) {
        if let result = tryTileFetch(request) {
            request.success?(request, result)
        } else {
            request.failure?(request, NSError(domain:"", code:1, userInfo:nil))
        }
    }

    func tryTileFetch(_ request: MaplyTileFetchRequest) -> Data? {
        guard let info = request.fetchInfo as? TestTileFetchInfo else { return nil; }
        let tileID = info.tileID
        return UIGraphicsImageRenderer(size: CGSize(width: tileSize, height: tileSize)).image { ctx in
            let color = colors[Int(info.tileID.level) % colors.count]
            ctx.cgContext.setFillColor(color.cgColor)
            ctx.cgContext.setStrokeColor(UIColor.white.cgColor)
            ctx.cgContext.setAlpha(CGFloat(Float(alpha) / 255))
            ctx.cgContext.setLineWidth(2)

            let rectangle = CGRect(x: 0, y: 0,
                                   width: tileSize,
                                   height: tileSize)
            ctx.cgContext.addRect(rectangle)
            ctx.cgContext.drawPath(using: .fillStroke)

            if let font = UIFont(name: "Helvetica", size: 24) {
                let textStyle = NSMutableParagraphStyle.default.mutableCopy() as? NSMutableParagraphStyle ?? NSMutableParagraphStyle()
                textStyle.alignment = .center

                let attrs = [NSAttributedString.Key.font: font,
                             NSAttributedString.Key.foregroundColor: color,
                             NSAttributedString.Key.paragraphStyle: textStyle]

                let string = "\(tileID.level): (\(tileID.x),\(tileID.y))"
                let strCtx = NSStringDrawingContext()
                let textBounds = string.boundingRect(with: rectangle.size, context: strCtx)
                let vOffset = rectangle.height / 2 - textBounds.height / 2
                let textRect = CGRect(x: rectangle.minX,
                                      y: rectangle.minY + vOffset,
                                      width: rectangle.width,
                                      height: rectangle.height - vOffset)
                string.draw(with: textRect, attributes: attrs, context: strCtx)
            }
        }.pngData()
    }

    func updateTileFetch(_ fetchID: Any, priority: Int32, importance: Double) -> Any? { return nil }
    func cancelTileFetches(_ requestRets: [Any]) { }
    
    func shutdown() {
        queue.cancelAllOperations()
    }

    let minZoom: Int
    let maxZoom: Int
    let fetcherName: String

    let colors = [
        UIColor.init(red: 0x86/255.0, green: 0x81/255.0, blue: 0x2D/255.0, alpha: 1),
        UIColor.init(red: 0x5E/255.0, green: 0xB9/255.0, blue: 0xC9/255.0, alpha: 1),
        UIColor.init(red: 0x2A/255.0, green: 0x7E/255.0, blue: 0x3E/255.0, alpha: 1),
        UIColor.init(red: 0x4F/255.0, green: 0x25/255.0, blue: 0x6F/255.0, alpha: 1),
        UIColor.init(red: 0xD8/255.0, green: 0x9C/255.0, blue: 0xDE/255.0, alpha: 1),
        UIColor.init(red: 0x77/255.0, green: 0x3B/255.0, blue: 0x28/255.0, alpha: 1),
        UIColor.init(red: 0x33/255.0, green: 0x3D/255.0, blue: 0x99/255.0, alpha: 1),
        UIColor.init(red: 0x86/255.0, green: 0x2D/255.0, blue: 0x52/255.0, alpha: 1),
        UIColor.init(red: 0xC2/255.0, green: 0xC6/255.0, blue: 0x53/255.0, alpha: 1),
        UIColor.init(red: 0xB8/255.0, green: 0x58/255.0, blue: 0x3D/255.0, alpha: 1),
    ]
    
    weak var control: MaplyBaseViewController?
    let queue = OperationQueue()
}
