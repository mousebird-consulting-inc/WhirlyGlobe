//
//  LegendViewController.swift
//  AutoTester
//
//  Created by Steve Gifford on 7/29/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

import Foundation
import WhirlyGlobeMaplyComponent

class LegendCell: UITableViewCell {

    @IBOutlet weak var symbol: UIImageView!
    
    @IBOutlet weak var label: UILabel!
    
    @IBOutlet weak var checkMark: UIImageView!
    
}

// Single entry in the legends
class LegendEntry {
    let name: String
    let image: UIImage?
    var visible: Bool
    
    init(name: String, image: UIImage?, visible: Bool) {
        self.name = name
        self.image = image
        self.visible = visible
    }
}

/**
 Display the full Mapbox
 */
class LegendViewController: UITableViewController {
    
    var entries: [LegendEntry] = []
    
    var styleSheet: MapboxVectorStyleSet? {
        didSet {
            entries.removeAll()
            guard let sheet = styleSheet else {
                tableView.reloadData()
                return
            }
            let entries = sheet.layerLegend(CGSize(width: 64, height: 64), group: false)
            for entry in entries {
                if let image = entry.image {
                    self.entries.append(LegendEntry(name: entry.name, image: image, visible: true))
                }
            }
            tableView.reloadData()
        }
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return entries.count
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let entry = entries[indexPath.row]
        if let cell = tableView.dequeueReusableCell(withIdentifier: "Legend Cell") as? LegendCell {
            cell.checkMark.isHidden = !entry.visible
            cell.label.text = entry.name
            cell.symbol.image = entry.image

            return cell
        }
        
        return UITableViewCell()
    }
        
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
    }
}
