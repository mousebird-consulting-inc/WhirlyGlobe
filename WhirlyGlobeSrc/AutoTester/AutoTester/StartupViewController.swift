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
		NASAGIBSTestCase(),
		MapquestSatelliteTestCase(),

		AnimatedBasemapTestCase(),
		ScreenLabelsTestCase(),
		ScreenMarkersTestCase(),
		VectorsTestCase(),
		WideVectorsTestCase(),
		ClusteredMarkersTestCase(),
		MegaMarkersTestCase(),
		LabelsTestCase(),
		MarkersTestCase(),
		StickersTestCase(),

		MapzenVectorTestCase(),
		MapjamVectorTestCase(),
		VectorMBTilesTestCase(),

		StarsSunTestCase(),
		ShapesTestCase(),
		LoftedPolysTestCase(),

		CartoDBTestCase(),

		BNGCustomMapTestCase(),
		BNGTestCase(),
		ElevationLocalDatabase(),
		ParticleTestCase(),
		CesiumElevationTestCase(),
		RunwayBuilderTestCase(),

		AnimatedColorRampTestCase(),
		ExtrudedModelTestCase(),
		ModelsTestCase(),
		GreatCircleTestCase(),

		AerisWeatherTestCase(),
		
		LabelAnimationTestCase(),
		WMSTestCase(),
		FindHeightTestCase(),
		FullAnimationTest()
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
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Edit, target: self, action: #selector(StartupViewController.showConfig))

		if ConfigSection.Row.MultipleMode.load() {
			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: #selector(StartupViewController.runTests))
		}

		let rect = UIScreen.mainScreen().applicationFrame
		testViewBlack = UIView(frame: CGRectMake(0, 0, rect.width, rect.height))
		testViewBlack?.backgroundColor = UIColor.blackColor()
		testViewBlack?.hidden = true

		self.view.addSubview(testViewBlack!)

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.loadValues()

		self.scrollToLastPosition()
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
			forIndexPath: indexPath) as! TestCell

		cell.testName.text = tests[indexPath.row].name
		cell.selectionStyle = .None

		if ConfigSection.Row.InteractiveMode.load() {
			cell.globeButton.hidden = false
			cell.mapButton.hidden = false;
			cell.rowPosition = indexPath.row
			cell.accessoryType = .None
			cell.globeTestExecution = {
				NSUserDefaults.standardUserDefaults().setInteger(indexPath.row, forKey: "scrollPos")
				self.runInteractiveTest(self.tests[indexPath.row], type: .RunGlobe)
			}
			
			cell.mapTestExecution = {
				NSUserDefaults.standardUserDefaults().setInteger(indexPath.row, forKey: "scrollPos")
				self.runInteractiveTest(self.tests[indexPath.row], type: .RunMap)
			}
		}
		else{
			cell.globeButton.hidden = true
			cell.mapButton.hidden = true
			cell.accessoryType = .None
			if tests[indexPath.row].running {
				cell.accessoryType = .DisclosureIndicator
			}
			else {
				cell.accessoryType = tests[indexPath.row].selected
					? .Checkmark
					: .None
			}
		}
		return cell
	}

	override func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {

		NSUserDefaults.standardUserDefaults().setInteger(indexPath.row, forKey: "scrollPos")

		let interactiveMode = ConfigSection.Row.InteractiveMode.load()
		let singleMode = ConfigSection.Row.SingleMode.load()
		let normalMode = ConfigSection.Row.MultipleMode.load()

		if !interactiveMode {
			if singleMode {
				tableView.setContentOffset(CGPointZero, animated: false)
				runTest(self.tests[indexPath.row])
			}
			if normalMode {
				tests[indexPath.row].selected = !tests[indexPath.row].selected
				let cell = tableView.cellForRowAtIndexPath(indexPath)
				cell?.accessoryType = tests[indexPath.row].selected
					? .Checkmark
					: .None
			}
		}
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
			popControl?.presentPopoverFromRect(CGRectMake(0, 0, 10, 10), inView: self.navigationController!.view, permittedArrowDirections: .Up, animated: true)
		}
		else {
			configViewC!.navigationItem.hidesBackButton = true
			configViewC!.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Done, target: self, action: #selector(StartupViewController.editDone))
			self.navigationController?.pushViewController(configViewC!, animated: true)
		}
	}
	
	
	private func prepareTestView (){
		
		let rect = UIScreen.mainScreen().applicationFrame
		self.testViewBlack?.frame = CGRectMake(0, 0, rect.width, rect.height)
		self.testViewBlack?.hidden = false
		
		let testView = UIView(frame: CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height))
		testView.backgroundColor = UIColor.redColor()
		
		testView.center = CGPointMake(self.testViewBlack!.frame.size.width  / 2,
									  self.testViewBlack!.frame.size.height / 2)
		testView.hidden = false;
		self.testView = testView
		self.testViewBlack?.addSubview(self.testView!)
		tableView.scrollEnabled = false;

	}
	
	private func scrollToLastPosition(){
		let pos = NSUserDefaults.standardUserDefaults().integerForKey("scrollPos")
		testsTable.scrollToRowAtIndexPath(NSIndexPath(forRow: pos, inSection: 0), atScrollPosition: UITableViewScrollPosition.Top, animated: false)
	}
	
	private func runInteractiveTest( test: MaplyTestCase, type : ConfigSection.Row) {
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Done, target: self, action: #selector(StartupViewController.stopInteractiveTest))
		
		// use same aspect ratio as results view
		tableView.setContentOffset(CGPointZero, animated: false)
		self.prepareTestView()
		test.interactive = true
		
		if (type == .RunMap) {
			test.options.insert(.Map)
		}
		else  if (type == .RunGlobe) {
			test.options.insert(.Globe)
		}
		test.testView = self.testView
		test.start()
	}

	func stopInteractiveTest() {
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Edit, target: self, action: #selector(StartupViewController.showConfig))
		self.testViewBlack?.hidden = true
		tableView.scrollEnabled = true
		self.scrollToLastPosition()
	}
	
	func runTests() {
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Stop, target: self, action: #selector(StartupViewController.stopTests))
		
		self.title = "Running tests..."
		
		self.prepareTestView()
		self.results.removeAll()
		
		cancelled = false
		
		startTests(tests)
	}
	
	func stopTests() {
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
					if self.cancelled {
						self.finishTests()
					}
					else {
						if let mapResult = test.mapResult {
							self.results["\(test.name) - Map"] = mapResult
						}
						
						if let globeResult = test.globeResult {
							self.results["\(test.name) - Globe"] = globeResult
						}
						
						self.startTests(tail)
					}
					
				}
				if configViewC!.valueForSection(.Options, row: .ViewTest){
					self.seconds = head.captureDelay
					self.title = "\(head.name) (\(self.seconds))"
					self.timer = NSTimer.scheduledTimerWithTimeInterval(1,
																		target: self,
																		selector: #selector(StartupViewController.updateTitle(_:)),
																		userInfo: head.name,
																		repeats: true)
				}
				else {
					tableView.reloadData()
				}
				head.testView = self.testView;
				head.start()
			}
			else {
				self.startTests(tail);
			}
		}
		else {
			self.finishTests()
		}
	}
		
	private func runTest(test: MaplyTestCase) {
		self.title = "Running test..."

		self.prepareTestView()
		
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

		if configViewC!.valueForSection(.Options, row: .ViewTest) {
			self.seconds = test.captureDelay
			self.title = "\(test.name) (\(self.seconds))"
			self.timer = NSTimer.scheduledTimerWithTimeInterval(1,
				target: self,
				selector: #selector(StartupViewController.updateTitle(_:)),
				userInfo: test.name,
				repeats: true)
		}
		
		test.testView = self.testView
		test.start()
	}

	func updateTitle(timer: NSTimer) {
		self.seconds -= 1
		self.title = "\(timer.userInfo!) (\(self.seconds))"
		if self.seconds == 0 {
			self.timer.invalidate()
		}
	}

	private func finishTests() {
		self.testViewBlack?.hidden = true
		tableView.reloadData()

		if ConfigSection.Row.MultipleMode.load() {
			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: #selector(StartupViewController.runTests))
		}

		self.title = "Tests"
		tableView.scrollEnabled = true
		scrollToLastPosition()
		if !cancelled {
			self.performSegueWithIdentifier("results", sender: self)
		}
	}

	private dynamic func editDone() {
		self.navigationController?.popToViewController(self, animated: true)
		changeSettings()

		if ConfigSection.Row.MultipleMode.load() {
			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .Play, target: self, action: #selector(StartupViewController.runTests))
		}
		else{
			self.navigationItem.rightBarButtonItem = nil
		}
		testsTable.reloadData()
	}

	func popoverControllerDidDismissPopover(popoverController: UIPopoverController) {
		changeSettings()
	}

	func changeSettings() {
		let select: Bool?

		if configViewC!.valueForSection(.Actions, row: .SelectAll) {
			select = true
		}
		else if configViewC!.valueForSection(.Actions, row: .SelectNone) {
			select = false
		}
		else {
			select = false
		}

		if let select = select {
			tests.forEach {
				$0.selected = select
			}
			tableView.reloadData()

			configViewC!.selectAll(.Actions, select: false)
		}
	}
}
