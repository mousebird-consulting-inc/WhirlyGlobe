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

	@IBOutlet weak var retryButton: UIButton!
	@IBOutlet weak var downloadIndicator: UIActivityIndicatorView!
	var globeTestExecution: (() -> ())?
	var mapTestExecution: (() -> ())?
	var retryDownloadResources: (() -> ())?
	var rowPosition : Int?

	var state: MaplyTestCaseState  = .Error {
		didSet {
			switch state {
			case MaplyTestCaseState.Downloading:
				downloadIndicator.hidden = false;
				downloadIndicator.startAnimating()
				globeButton.hidden = true
				mapButton.hidden = true
				retryButton.hidden = true;
				accessoryType = .None
				break
			case MaplyTestCaseState.Error:
				downloadIndicator.hidden = true;
				downloadIndicator.stopAnimating()
				globeButton.hidden = true
				retryButton.hidden = false;
				mapButton.hidden = true
				accessoryType = .None
				backgroundColor = UIColor.redColor()
				break
			default:
				downloadIndicator.stopAnimating()
				downloadIndicator.hidden = true;
				retryButton.hidden = true;
				backgroundColor = UIColor.whiteColor()
			}
		}
	}

	var interactive: Bool = false {
		didSet {
			globeButton.hidden = !interactive
			mapButton.hidden = !interactive;
			accessoryType = .None

			switch state {
			case MaplyTestCaseState.Running:
				accessoryType = .DisclosureIndicator
			case MaplyTestCaseState.Selected:
				accessoryType = .Checkmark
			default:
				accessoryType = .None
			}
		}
	}

	required init?(coder aDecoder: NSCoder) {
		super.init(coder: aDecoder)
	}

	@IBAction func runGlobeInteractiveTest(sender: AnyObject) {
		globeTestExecution?()
	}
	
	@IBAction func runMapInteractiveTest(sender: AnyObject) {
		mapTestExecution?()
	}

	@IBAction func retryAction(sender: AnyObject) {
		retryDownloadResources?()
	}

}
