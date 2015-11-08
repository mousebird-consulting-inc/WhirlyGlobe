//
//  FullScreenViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 23/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit


class FullScreenViewController : UIViewController {

	@IBOutlet weak var actualImageView: UIImageView!
	@IBOutlet weak var baselineImageView: UIImageView!

	var actualImageResult: UIImage?
	var baselineImageResult: UIImage?

	override func viewDidLoad() {
		// disable swipe back gesture
		self.navigationController?.interactivePopGestureRecognizer?.enabled = false
	}

	override func viewWillAppear(animated: Bool) {
		actualImageView.image = actualImageResult
		baselineImageView.image = baselineImageResult
	}

	@IBAction func sliderChanged(sender: AnyObject) {
		let slider = sender as! UISlider

		actualImageView.alpha = CGFloat(slider.value)
	}
}
