//
//  ResultsViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit


class ResultsViewController: UITableViewController, UIPopoverControllerDelegate {

	var titles = [String]()
	var results = [MaplyTestResult]()
	fileprivate var dropboxView : DropboxViewController?
	fileprivate var popControl: UIPopoverController?
    @IBOutlet weak var uploadButton: UIBarButtonItem!

	override func viewDidLoad() {
        self.navigationItem.rightBarButtonItem = nil;
	}

	@IBAction func uploadToDropbox(_ sender: AnyObject) {
		if UI_USER_INTERFACE_IDIOM() == .pad {
			popControl = UIPopoverController(contentViewController: dropboxView!)
			popControl?.delegate = self
			popControl?.setContentSize(CGSize(width: 400, height: 4.0/5.0*self.view.bounds.size.height), animated: true)
			popControl?.present(from: CGRect(x: 0, y: 0, width: 10, height: 10), in: self.view, permittedArrowDirections: .up, animated: true)
		}
		else {
			dropboxView!.navigationItem.hidesBackButton = false
			dropboxView!.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(ResultsViewController.editDone))
			self.navigationController?.pushViewController(dropboxView!, animated: true)
		}
	}

	let queue = { () -> OperationQueue in
		let queue = OperationQueue()

		queue.maxConcurrentOperationCount = 1
		queue.qualityOfService = .utility

		return queue
	}()

	override func tableView(_ tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return results.count
	}

	override func tableView(_ tableView: UITableView,
		cellForRowAt indexPath: IndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCell(withIdentifier: "result", for: indexPath) as! ResultCell

		let title = titles[(indexPath as NSIndexPath).row]
		cell.nameLabel?.text = title

		queue.addOperation {
			let baselineImage = UIImage(contentsOfFile: self.results[(indexPath as NSIndexPath).row].baselineImageFile)

			let actualImage: UIImage?

			if let actualImageFile = self.results[(indexPath as NSIndexPath).row].actualImageFile {
				actualImage = UIImage(contentsOfFile: actualImageFile)
			}
			else {
				actualImage = nil
			}

			DispatchQueue.main.async {
				cell.baselineImage?.image = baselineImage
				cell.actualImage?.image = actualImage
			}
		}

		return cell
	}

	override func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
		return (UI_USER_INTERFACE_IDIOM() == .pad) ? 500 : 344
	}

	override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {

		let cell = tableView.cellForRow(at: indexPath)

		self.performSegue(withIdentifier: "showFullScreen", sender: cell)
	}

	override func prepare(for segue: UIStoryboardSegue, sender: Any?) {

		if let cell = sender as? ResultCell {
			let destination = segue.destination as! FullScreenViewController
			destination.title = cell.nameLabel?.text
			destination.actualImageResult = cell.actualImage?.image
			destination.baselineImageResult = cell.baselineImage?.image
		}
	}

	fileprivate dynamic func editDone() {
		self.navigationController!.popToViewController(self, animated: true)
	}

}
