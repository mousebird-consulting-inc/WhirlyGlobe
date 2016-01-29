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
		LoftedPolysTestCase(),
		NASAGIBSTestCase(),
		CartoDBTestCase(),
		BNGCustomMapTestCase(),
		BNGTestCase(),
		ElevationLocalDatabase(),
        RunwayBuilderTestCase()
	]

	@IBOutlet weak var testsTable: UITableView!

	private var results = [String:MaplyTestResult]()

	private var testView: UIView?
	private var testViewBlack: UIView?

	private var configViewC: ConfigViewController?
	private var popControl: UIPopoverController?

	private var timer = NSTimer()
	private var seconds = 0
	private var cancelled = false

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

		let rect = UIScreen.mainScreen().applicationFrame
		testViewBlack = UIView(frame: CGRectMake(0, 0, rect.width, rect.height))
		testViewBlack?.backgroundColor = UIColor.blackColor()
		testViewBlack?.hidden = true

		self.view.addSubview(testViewBlack!)

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.loadValues()

		let pos = NSUserDefaults.standardUserDefaults().integerForKey("scrollPos")
		testsTable.scrollToRowAtIndexPath(NSIndexPath(forRow: pos, inSection: 0), atScrollPosition: UITableViewScrollPosition.Top, animated: false)
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

		NSUserDefaults.standardUserDefaults().setInteger(indexPath.row, forKey: "scrollPos")

		runTest(self.tests[indexPath.row])
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
		let destination = segue.destinationViewController as! ResultsViewController

		let sortedKeys = self.results.keys.sort { $0 < $1 }

		destination.titles = sortedKeys
		destination.results = [MaplyTestResult]()
		sortedKeys.forEach {
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

	private func runTest(test: MaplyTestCase) {
		self.title = "Running test..."

		// use same aspect ratio as results view
		let rect = UIScreen.mainScreen().applicationFrame
		self.testViewBlack?.frame = CGRectMake(0, 0, rect.width, rect.height)
		self.testViewBlack?.hidden = !configViewC!.valueForSection(.Options, row: .ViewTest)
		self.testView = UIView(frame: CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height))
		self.testView?.center = CGPointMake(self.testViewBlack!.frame.size.width  / 2,
			self.testViewBlack!.frame.size.height / 2)
		self.testView?.hidden = !configViewC!.valueForSection(.Options, row: .ViewTest)
		self.testView?.backgroundColor = UIColor.blackColor()
		self.testViewBlack?.addSubview(self.testView!)
		self.results.removeAll()

		test.options = .None
		if configViewC!.valueForSection(.Options, row: .RunGlobe) {
			test.options.insert(.Globe)
		}

		if configViewC!.valueForSection(.Options, row: .RunMap) {
			test.options.insert(.Map)
		}

		test.resultBlock = { test in
			if let mapResult = test.mapResult {
				self.results["\(test.name) - Map"] = mapResult
			}

			if let globeResult = test.globeResult {
				self.results["\(test.name) - Globe"] = globeResult
			}
			self.finishTests()
		}

		if configViewC!.valueForSection(.Options, row: .ViewTest){
			self.seconds = test.captureDelay
			self.title = "\(test.name) (\(self.seconds))"
			self.timer = NSTimer.scheduledTimerWithTimeInterval(1,
				target: self,
				selector: "updateTitle:",
				userInfo: test.name,
				repeats: true)
		}
		test.testView = self.testView
		test.start()
	}

	func updateTitle(timer: NSTimer){
		self.seconds--
		self.title = "\(timer.userInfo!) (\(self.seconds))"
		if self.seconds == 0 {
			self.timer.invalidate()
		}
	}

	private func finishTests() {
		self.testViewBlack?.hidden = true
		tableView.reloadData()

		self.title = "Tests"

		if !cancelled {
			self.performSegueWithIdentifier("results", sender: self)
		}
	}

	private dynamic func editDone() {
		self.navigationController?.popToViewController(self, animated: true)
	}
}
