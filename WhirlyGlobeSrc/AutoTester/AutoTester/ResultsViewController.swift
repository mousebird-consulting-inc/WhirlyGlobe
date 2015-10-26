//
//  ResultsViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class ResultsViewController: UITableViewController {

	var titles = [String]()
	var results = [MaplyTestResult]()

	var images = [String:(UIImage?,UIImage?)]()

	enum ImageType: Int {
		case Expected = 1
		case Actual = 2

		var message: String {
			switch self {
			case .Expected:
				return "Expected"
			case .Actual:
				return "Actual"
			}
		}
	}

	override func tableView(tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return results.count
	}

	override func tableView(tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCellWithIdentifier("result", forIndexPath: indexPath) as! ResultCell

		let title = titles[indexPath.row]
		cell.nameLabel?.text = title

		if let images = images[title] {
			cell.baselineImage?.image = images.0
			cell.actualImage?.image = images.1
		}
		else {
			dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
				let baselineImage = UIImage(contentsOfFile: self.results[indexPath.row].baselineImageFile)

				let actualImage: UIImage?

				if let actualImageFile = self.results[indexPath.row].actualImageFile {
					actualImage = UIImage(contentsOfFile: actualImageFile)
				}
				else {
					actualImage = nil
				}

				self.images[title] = (baselineImage, actualImage)

				dispatch_async(dispatch_get_main_queue()) {
					cell.baselineImage?.image = baselineImage
					cell.actualImage?.image = actualImage
				}
			}
		}

		for img in [cell.actualImage!, cell.baselineImage!] {
			let tap = UITapGestureRecognizer(target: self, action: "tappedImage:")
			tap.numberOfTapsRequired = 1
			tap.numberOfTouchesRequired = 1

			img.addGestureRecognizer(tap)
			img.restorationIdentifier = cell.nameLabel?.text
		}

		cell.baselineImage?.tag = ImageType.Expected.rawValue
		cell.actualImage?.tag = ImageType.Actual.rawValue

		return cell
	}

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
		let destination = segue.destinationViewController as! FullScreenViewController
		if let imageView = sender as? UIImageView,
				image = imageView.image,
				imageType = ImageType(rawValue: imageView.tag) {
			destination.title = "\(imageView.restorationIdentifier!) - \(imageType.message)"
			destination.image = image
		}
    }

	func tappedImage(sender: UITapGestureRecognizer) {
		self.performSegueWithIdentifier("showFullScreen", sender: sender.view)
    }

	override func didReceiveMemoryWarning() {
		self.images.removeAll()
	}

}
