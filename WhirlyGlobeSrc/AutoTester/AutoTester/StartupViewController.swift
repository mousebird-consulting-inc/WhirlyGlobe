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
	private var results = [String:MaplyTestResult]()

	@IBOutlet weak var logTable: UITableView!
	var testView: UIView?

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

	override func tableView(tableView: UITableView,
		didSelectRowAtIndexPath indexPath: NSIndexPath) {

		self.performSegueWithIdentifier("results", sender: self)
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
		let destination = segue.destinationViewController as! ResultsViewController

		destination.results = [MaplyTestResult](self.results.values)
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

		let tests = [
			GeographyClassTestCase(),
			StamenWatercolorRemote(),
		]

		let testView = UIView(frame: self.view.bounds)
		testView.hidden = true;
		self.view.addSubview(testView)

		startTests(tests, inView: testView, passed: 0, failed: 0)
	}

	private func startTests(tests: [MaplyTestCase], inView testView: UIView, passed: Int, failed: Int) {
		if let head = tests.first {
			let tail = Array(tests.dropFirst())

			head.resultBlock = { test in
				self.showLog("\(test.name) \(test.result.passed ? "passed." : "failed!")")
				print("\(test.result.actualImageFile)")
				self.results[test.name] = test.result

				self.startTests(tail,
					inView: testView,
					passed: passed + (test.result.passed ? 1 : 0),
					failed: failed + (test.result.passed ? 0 : 1))
			}

			showLog("Testing \(head.name)...")
			head.testView = testView;
			head.start()
		}
		else {
			self.finishTestsInView(testView, passed: passed, failed: failed)
		}
	}

	private func finishTestsInView(testView: UIView, passed: Int, failed: Int) {
		testView.removeFromSuperview()

		showLog("Completed. \(passed) passed. \(failed) failed.")
		showLog("See results")

		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")
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
