//
//  TestCell.swift
//  AutoTester
//
//  Created by jmnavarro on 8/5/16.
//  Copyright © 2016-2017 mousebird consulting.
//

import UIKit

class TestCell: UITableViewCell {

	@IBOutlet weak var testName: UILabel!
	@IBOutlet weak var mapButton: UIButton!
	@IBOutlet weak var globeButton: UIButton!

	@IBOutlet weak var retryButton: UIButton!
	var globeTestExecution: (() -> ())?
	var mapTestExecution: (() -> ())?
	var rowPosition : Int?
	var implementations: MaplyTestCaseImplementations = [.globe, .map]

	required init?(coder aDecoder: NSCoder) {
		super.init(coder: aDecoder)
	}

	@IBAction func runGlobeInteractiveTest(_ sender: AnyObject) {
		globeTestExecution?()
	}
	
	@IBAction func runMapInteractiveTest(_ sender: AnyObject) {
		mapTestExecution?()
	}

}
