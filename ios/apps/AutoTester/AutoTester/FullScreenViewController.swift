//
//  FullScreenViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 23/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit


class FullScreenViewController : UIViewController {

	@IBOutlet weak var actualImageView: UIImageView!
	@IBOutlet weak var baselineImageView: UIImageView!

	var actualImageResult: UIImage?
	var baselineImageResult: UIImage?

	override func viewDidLoad() {
		// disable swipe back gesture
		self.navigationController?.interactivePopGestureRecognizer?.isEnabled = false
	}

	override func viewWillAppear(_ animated: Bool) {
		actualImageView.image = actualImageResult
		baselineImageView.image = baselineImageResult
	}

	@IBAction func sliderChanged(_ sender: AnyObject) {
		let slider = sender as! UISlider

		actualImageView.alpha = CGFloat(slider.value)
	}
}
