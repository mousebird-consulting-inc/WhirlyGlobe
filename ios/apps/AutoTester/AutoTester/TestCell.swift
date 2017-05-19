//
//  TestCell.swift
//  AutoTester
//
//  Created by jmnavarro on 8/5/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
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
	var implementations: MaplyTestCaseImplementations = [.globe, .map]

	var state: MaplyTestCaseState  = .error {
		didSet {
			switch state {
			case MaplyTestCaseState():
				downloadIndicator.isHidden = false;
				downloadIndicator.startAnimating()
				globeButton.isHidden = true
				mapButton.isHidden = true
				retryButton.isHidden = true;
				accessoryType = .none
				break
			case MaplyTestCaseState.error:
				downloadIndicator.isHidden = true;
				downloadIndicator.stopAnimating()
				globeButton.isHidden = true
				retryButton.isHidden = false;
				mapButton.isHidden = true
				accessoryType = .none
				backgroundColor = UIColor.red
				break
			default:
				downloadIndicator.stopAnimating()
				downloadIndicator.isHidden = true;
				retryButton.isHidden = true;
				backgroundColor = UIColor.white
			}
		}
	}

	var interactive: Bool = false {
		didSet {
			if interactive  {
				globeButton.isHidden = !implementations.contains(.globe)
				mapButton.isHidden = !implementations.contains(.map);
			}
			else{
				globeButton.isHidden = true
				mapButton.isHidden = true
			}
			accessoryType = .none

			switch state {
			case MaplyTestCaseState.running:
				accessoryType = .disclosureIndicator
			case MaplyTestCaseState.selected:
				accessoryType = .checkmark
			default:
				accessoryType = .none
			}
		}
	}

	required init?(coder aDecoder: NSCoder) {
		super.init(coder: aDecoder)
	}

	@IBAction func runGlobeInteractiveTest(_ sender: AnyObject) {
		globeTestExecution?()
	}
	
	@IBAction func runMapInteractiveTest(_ sender: AnyObject) {
		mapTestExecution?()
	}

	@IBAction func retryAction(_ sender: AnyObject) {
		retryDownloadResources?()
	}

}
