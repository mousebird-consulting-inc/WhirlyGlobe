//
//  StartupViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

class StartupViewController: UITableViewController, UIPopoverControllerDelegate {

	let tests = [
		GeographyClassTestCase(),
		StamenWatercolorRemote(),
		MapBoxSatelliteTestCase(),
		CesiumElevationTestCase(),
		AnimatedBasemapTestCase(),
		MapBoxVectorTestCase(),
		MapzenVectorTestCase(),
		VectorsTestCase(),
		ScreenLabelsTestCase(),
		ScreenMarkersTestCase(),
		ClusteredMarkersTestCase(),
		LabelsTestCase(),
		MarkersTestCase(),
		MegaMarkersTestCase(),
		ModelsTestCase(),
		WideVectorsTestCase(),
		StarsSunTestCase(),
		ShapesTestCase(),
		StickersTestCase(),
		LoftedPolysTestCase()
	]

	@IBOutlet weak var testsTable: UITableView!

	private var results = [String:MaplyTestResult]()

	private var testView: UIView?

	private var configViewC: ConfigViewController?
	private var popControl: UIPopoverController?

	override func viewWillAppear(animated: Bool) {
		let caches = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0] as NSString
		let dir = caches.stringByAppendingPathComponent("results")

		if NSFileManager.defaultManager().fileExistsAtPath(dir) {
			try! NSFileManager.defaultManager().removeItemAtPath(dir)
		}

		try! NSFileManager.defaultManager().createDirectoryAtPath(dir,
			withIntermediateDirectories: true,
			attributes: nil)

		results.removeAll(keepCapacity: true)
	}

	override func viewDidLoad() {
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Edit, target: self, action: "showConfig")
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.loadValues()
	}


	override func tableView(
		tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return tests.count
	}

	override func tableView(
		tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCellWithIdentifier("cell",
			forIndexPath: indexPath)

		cell.textLabel?.text = tests[indexPath.row].name
		cell.selectionStyle = .None

		if tests[indexPath.row].running {
			cell.accessoryType = .DisclosureIndicator
		}
		else {
			cell.accessoryType = tests[indexPath.row].selected
				? .Checkmark
				: .None
		}

		return cell
	}

	override func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {

		tests[indexPath.row].selected = !tests[indexPath.row].selected

		let cell = tableView.cellForRowAtIndexPath(indexPath)

		cell?.accessoryType = tests[indexPath.row].selected
			? .Checkmark
			: .None
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
		let destination = segue.destinationViewController as! ResultsViewController

		let sortedKeys = self.results.keys.sort { $0 < $1 }

		destination.titles = sortedKeys
		destination.results = [MaplyTestResult]()
		sortedKeys.forEach{
			destination.results.append(self.results[$0]!)
		}
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

		self.title = "Running tests..."

		// use same aspect ratio as results view
		self.testView = UIView(frame: CGRectMake(0, 0, 468, 672))
		self.testView!.hidden = true;
		self.view.addSubview(self.testView!)
		self.results.removeAll()
		startTests(tests)
	}

	private func startTests(tests: [MaplyTestCase]) {
		if let head = tests.first {
			let tail = Array(tests.dropFirst())

			if (head.selected) {
				head.options = .None

				if configViewC!.valueForSection(.Options, row: .RunGlobe) {
					head.options.insert(.Globe)
				}

				if configViewC!.valueForSection(.Options, row: .RunMap) {
					head.options.insert(.Map)
				}

				head.resultBlock = { test in
					if let mapResult = test.mapResult {
						self.results["\(test.name) - Map"] = mapResult
					}

					if let globeResult = test.globeResult {
						self.results["\(test.name) - Globe"] = globeResult
					}

					self.startTests(tail)
				}

				head.testView = self.testView;
				head.start()
				tableView.reloadData()
			}
			else {
				self.startTests(tail);
			}
		}
		else {
			self.finishTests()
		}
	}

	private func finishTests() {
		self.testView?.removeFromSuperview()
		tableView.reloadData()

		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")

		self.title = "Tests"

		self.performSegueWithIdentifier("results", sender: self)
	}

	private dynamic func stopTests() {
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: "runTests")
	}

	private dynamic func editDone() {
		self.navigationController?.popToViewController(self, animated: true)

		let select: Bool?

		if configViewC!.valueForSection(.Actions, row: .SelectAll) {
			select = true
		}
		else if configViewC!.valueForSection(.Actions, row: .SelectNone) {
			select = false
		}
		else {
			select = nil
		}

		if let select = select {
			tests.forEach { $0.selected = select }
			tableView.reloadData()

			configViewC!.selectAll(.Actions, select: false)
		}

	}

}
