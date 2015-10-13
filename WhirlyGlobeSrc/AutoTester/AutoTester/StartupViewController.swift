//
//  StartupViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

class StartupViewController: UITableViewController, UIPopoverControllerDelegate {

	private var log = [String]()

	@IBOutlet weak var logTable: UITableView!

	private var configViewC: ConfigViewController?
	private var popControl: UIPopoverController?


	override func viewDidLoad() {
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Edit, target: self, action: "showConfig")
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.loadValues()
	}


	override func tableView(tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return log.count
	}

	override func tableView(tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCellWithIdentifier("cell", forIndexPath: indexPath) 

		cell.textLabel?.text = log[indexPath.row]

		return cell
	}


	private dynamic func showConfig() {
		if UI_USER_INTERFACE_IDIOM() == .Pad {
			popControl = UIPopoverController(contentViewController: configViewC!)
			popControl?.delegate = self
			popControl?.setPopoverContentSize(CGSizeMake(400, 4.0/5.0*self.view.bounds.size.height), animated: true)
			popControl?.presentPopoverFromRect(CGRectMake(0, 0, 10, 10), inView: self.view, permittedArrowDirections: .Up, animated: true)
		}
		else {
			configViewC!.navigationItem.hidesBackButton = true
			configViewC!.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Done, target: self, action: "editDone")
			self.navigationController?.pushViewController(configViewC!, animated: true)
		}
	}

	private dynamic func runTests() {
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Stop, target: self, action: "stopTests")
		showLog("Start run...")

		let test = GeographyClassTestCase()
		showLog("Testing \(test.name)...")
		test.resultBlock = {
			self.showLog("\(test.name) \($0 ? "failed!" : "passed.")")

			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")
		}
		test.runTest()

	}

	private dynamic func stopTests() {
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")
	}

	private dynamic func editDone() {
		self.navigationController?.popToViewController(self, animated: true)
	}

	private func showLog(value: String) {
		log.append(value);

		self.logTable.reloadData()
	}


}
