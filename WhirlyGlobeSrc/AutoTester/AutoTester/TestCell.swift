//
//  TestCell.swift
//  AutoTester
//
//  Created by jmnavarro on 8/5/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

import UIKit

class TestCell: UITableViewCell {

	@IBOutlet weak var testName: UILabel!
	@IBOutlet weak var mapButton: UIButton!
	@IBOutlet weak var globeButton: UIButton!

	var globeTestExecution: (() -> ())?
	var mapTestExecution: (() -> ())?
	var rowPosition : Int?

	required init?(coder aDecoder: NSCoder) {
		super.init(coder: aDecoder)
	}

	@IBAction func runGlobeInteractiveTest(sender: AnyObject) {
		globeTestExecution?()
	}
	
	@IBAction func runMapInteractiveTest(sender: AnyObject) {
		mapTestExecution?()
	}

}
