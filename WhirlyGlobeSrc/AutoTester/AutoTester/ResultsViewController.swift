//
//  ResultsViewController.swift
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit

class ResultsViewController: UITableViewController {

	var results = [MaplyTestResult]()

	override func tableView(tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return results.count
	}

	override func tableView(tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCellWithIdentifier("result", forIndexPath: indexPath) as! ResultCell

		cell.nameLabel?.text = results[indexPath.row].testName

		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)) {
			let baselineImage = UIImage(contentsOfFile: self.results[indexPath.row].baselineImageFile)
			let actualImage = UIImage(contentsOfFile: self.results[indexPath.row].actualImageFile)

			dispatch_async(dispatch_get_main_queue()) {
				cell.baselineImage?.image = baselineImage
				cell.actualImage?.image = actualImage
			}
		}

		return cell
	}


}
