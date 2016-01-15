//
//  ResultsViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit
import SwiftyDropbox


class ResultsViewController: UITableViewController, UIPopoverControllerDelegate {

	var titles = [String]()
	var results = [MaplyTestResult]()
	private var dropboxView : DropboxViewController?
	private var popControl: UIPopoverController?

	override func viewDidLoad() {
		dropboxView = DropboxViewController(nibName: "DropboxViewController", bundle: nil)
	}

	@IBAction func uploadToDropbox(sender: AnyObject) {
	
		if UI_USER_INTERFACE_IDIOM() == .Pad {
			popControl = UIPopoverController(contentViewController: dropboxView!)
			popControl?.delegate = self
			popControl?.setPopoverContentSize(CGSizeMake(400, 4.0/5.0*self.view.bounds.size.height), animated: true)
			popControl?.presentPopoverFromRect(CGRectMake(0, 0, 10, 10), inView: self.view, permittedArrowDirections: .Up, animated: true)
		}
		else {
			dropboxView!.navigationItem.hidesBackButton = false
			dropboxView!.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Done, target: self, action: "editDone")
			self.navigationController?.pushViewController(dropboxView!, animated: true)
		}
	}

	let queue = { () -> NSOperationQueue in
		let queue = NSOperationQueue()

		queue.maxConcurrentOperationCount = 1
		queue.qualityOfService = .Utility

		return queue
	}()

	override func tableView(tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return results.count
	}

	override func tableView(tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCellWithIdentifier("result", forIndexPath: indexPath) as! ResultCell

		let title = titles[indexPath.row]
		cell.nameLabel?.text = title

		queue.addOperationWithBlock {
			let baselineImage = UIImage(contentsOfFile: self.results[indexPath.row].baselineImageFile)

			let actualImage: UIImage?

			if let actualImageFile = self.results[indexPath.row].actualImageFile {
				actualImage = UIImage(contentsOfFile: actualImageFile)
			}
			else {
				actualImage = nil
			}

			dispatch_async(dispatch_get_main_queue()) {
				cell.baselineImage?.image = baselineImage
				cell.actualImage?.image = actualImage
			}
		}

		return cell
	}

	override func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
		return (UI_USER_INTERFACE_IDIOM() == .Pad) ? 500 : 344
	}

	override func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {

		let cell = tableView.cellForRowAtIndexPath(indexPath)

		self.performSegueWithIdentifier("showFullScreen", sender: cell)
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {

		if let cell = sender as? ResultCell {
			let destination = segue.destinationViewController as! FullScreenViewController
			destination.title = cell.nameLabel?.text
			destination.actualImageResult = cell.actualImage?.image
			destination.baselineImageResult = cell.baselineImage?.image
		}
	}

	private dynamic func editDone() {
		self.navigationController?.popToViewController(self, animated: true)
	}

}
