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

		return cell
	}

	override func didReceiveMemoryWarning() {
		self.images.removeAll()
	}

}
