//
//  StartupViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

class StartupViewController: UITableViewController, UIPopoverPresentationControllerDelegate {

	let tests = [
        GlobeSamplerTestCase(),
        
		GeographyClassTestCase(),
//        StamenWatercolorRemote(),
//        CartoDBLightTestCase(),
//        ImageSingleLevelTestCase(),

		AnimatedBasemapTestCase(),
		ScreenLabelsTestCase(),
        GlyphProblemTestCase(),
		ScreenMarkersTestCase(),
		MarkersTestCase(),
		AnimatedMarkersTestCase(),
		VectorsTestCase(),
		VectorStyleTestCase(),
		VectorHoleTestCase(),
//        ShapefileTestCase(),
		WideVectorsTestCase(),
		WideVectorGlobeTestCase(),
		TextureVectorTestCase(),
		GeoJSONStyleTestCase(),
		
		ClusteredMarkersTestCase(),
//        LabelsTestCase(),
		StickersTestCase(),

		PagingLayerTestCase(),
		VectorMBTilesTestCase(),
//        OpenMapTilesTestCase(),
//        OpenMapTilesHybridTestCase(),
        MapTilerTestCase(),
        MapboxTestCase(),

		StarsSunTestCase(),
		ShapesTestCase(),
		LoftedPolysTestCase(),

		CartoDBTestCase(),

		BNGCustomMapTestCase(),
		BNGTestCase(),
		ElevationLocalDatabase(),
		ParticleTestCase(),
		RunwayBuilderTestCase(),

//        AnimatedColorRampTestCase(),
		ExtrudedModelTestCase(),
		ModelsTestCase(),
		GreatCircleTestCase(),
		
		LabelAnimationTestCase(),
        // Note: Endpoint is missing
//		WMSTestCase(),
		FindHeightTestCase(),
		FullAnimationTest(),
		ActiveObjectTestCase(),
		AnimationDelegateTestCase(),
		LocationTrackingSimTestCase(),
		LocationTrackingRealTestCase(),

		LIDARTestCase(),
        
        StartupShutdownTestCase(),
        LayerStartupShutdownTestCase()
	]

	@IBOutlet weak var testsTable: UITableView!

	fileprivate var results = [String:MaplyTestResult]()
	fileprivate var queue = OperationQueue()

	fileprivate var testView: UIView?
	fileprivate var testViewBlack: UIView?

	fileprivate var configViewC: ConfigViewController?

	fileprivate var timer = Timer()
	fileprivate var seconds = 0
	fileprivate var cancelled = false

	fileprivate var lastInteractiveTestSelected: MaplyTestCase?

	override func viewWillAppear(_ animated: Bool) {
		let caches = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0] as NSString
		let dir = caches.appendingPathComponent("results")

		if FileManager.default.fileExists(atPath: dir) {
			try! FileManager.default.removeItem(atPath: dir)
		}

		try! FileManager.default.createDirectory(atPath: dir,
			withIntermediateDirectories: true,
			attributes: nil)

		results.removeAll(keepingCapacity: true)
	}

	override func viewDidLoad() {
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .edit, target: self, action: #selector(StartupViewController.showConfig))

		if ConfigSection.Row.MultipleMode.load() {
			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .play, target: self, action: #selector(StartupViewController.runTests))
		}

		let rect = UIScreen.main.bounds
		testViewBlack = UIView(frame: CGRect(x: 0, y: 0, width: rect.width, height: rect.height))
		testViewBlack?.backgroundColor = UIColor.black
		testViewBlack?.isHidden = true

		self.view.addSubview(testViewBlack!)

		configViewC = ConfigViewController(nibName: "ConfigViewController", bundle: nil)
		configViewC!.loadValues()
		queue.maxConcurrentOperationCount = 1
		tests.forEach(self.downloadTestResources)
	}

	func downloadTestResources(_ test: MaplyTestCase) {
		if let remoteResources = test.remoteResources() as? [String] {
			test.pendingDownload = remoteResources.count
			remoteResources.forEach {
				test.state = MaplyTestCaseState()
				let newOp = DownloadResourceOperation(
					url: URL(string: $0),
					test: test)
				queue.addOperation(newOp!)
			}
		}
	}

	override func tableView(
		_ tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return tests.count
	}

	func changeTestCellState (_ cell: TestCell, row : IndexPath) {
		cell.state = tests[(row as NSIndexPath).row].state

		if cell.state != MaplyTestCaseState() && cell.state != .error {
			cell.interactive = ConfigSection.Row.InteractiveMode.load()

			if cell.interactive {
				cell.globeTestExecution = {
					UserDefaults.standard.set((row as NSIndexPath).row, forKey: "scrollPos")
					self.runInteractiveTest(self.tests[(row as NSIndexPath).row], type: .RunGlobe)
				}

				cell.mapTestExecution = {
					UserDefaults.standard.set((row as NSIndexPath).row, forKey: "scrollPos")
					self.runInteractiveTest(self.tests[(row as NSIndexPath).row], type: .RunMap)
				}
			}
		}
	}

	override func tableView(
		_ tableView: UITableView,
		cellForRowAt indexPath: IndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCell(withIdentifier: "cell",
			for: indexPath) as! TestCell

		cell.testName.text = tests[(indexPath as NSIndexPath).row].name
		cell.selectionStyle = .none
		cell.implementations = tests[(indexPath as NSIndexPath).row].implementations

		tests[(indexPath as NSIndexPath).row].updateProgress = { enableIndicator in
			DispatchQueue.main.async(execute: {
				if enableIndicator {
//					cell.globeButton.hidden = true;
//					cell.mapButton.hidden = true;
					cell.accessoryType = .none;
				}
				else {
					self.changeTestCellState(cell, row: indexPath)
				}
				tableView.reloadData()
			})
		}
		cell.retryDownloadResources = {
			cell.downloadIndicator.isHidden = false;
			cell.downloadIndicator.startAnimating()
//			cell.globeButton.hidden = true
//			cell.mapButton.hidden = true
			cell.retryButton.isHidden = true;
			cell.accessoryType = .none
			self.downloadTestResources(self.tests[(indexPath as NSIndexPath).row])
		}
		changeTestCellState(cell, row: indexPath)
		return cell
	}

	override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {

		let interactiveMode = ConfigSection.Row.InteractiveMode.load()
		let singleMode = ConfigSection.Row.SingleMode.load()
		let normalMode = ConfigSection.Row.MultipleMode.load()

		if !interactiveMode {
			if singleMode {
				if self.tests[(indexPath as NSIndexPath).row].state != .error
					&& self.tests[(indexPath as NSIndexPath).row].state != MaplyTestCaseState()
					&& self.tests[(indexPath as NSIndexPath).row].state != .running {

					runTest(self.tests[(indexPath as NSIndexPath).row])
				}
			}
			else if normalMode {
				switch tests[(indexPath as NSIndexPath).row].state {
				case MaplyTestCaseState.selected:
					tests[(indexPath as NSIndexPath).row].state = .ready
					break
				case MaplyTestCaseState.ready:
					tests[(indexPath as NSIndexPath).row].state = .selected
					break;
				default:
					break;
				}
				let cell = tableView.cellForRow(at: indexPath)
				cell?.accessoryType = tests[(indexPath as NSIndexPath).row].state == .selected
					? .checkmark
					: .none
			}
		}
	}

	override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
		let destination = segue.destination as! ResultsViewController

		let sortedKeys = self.results.keys.sorted { $0 < $1 }

		destination.titles = sortedKeys
		destination.results = [MaplyTestResult]()
		sortedKeys.forEach {
			destination.results.append(self.results[$0]!)
		}
	}

	@objc fileprivate dynamic func showConfig() {
		if UI_USER_INTERFACE_IDIOM() == .pad {
            if let configViewC = configViewC {
                configViewC.modalPresentationStyle = UIModalPresentationStyle.popover
                configViewC.preferredContentSize = CGSize(width: 400, height: 4.0/5.0*self.view.bounds.size.height)
                present(configViewC, animated: true, completion: nil)
                if let popControl = configViewC.popoverPresentationController {
                    popControl.sourceView = self.view
                    popControl.delegate = self
                }
            }
		}
		else {
			configViewC!.navigationItem.hidesBackButton = true
			configViewC!.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(StartupViewController.editDone))
			self.navigationController?.pushViewController(configViewC!, animated: true)
		}
	}

	fileprivate func prepareTestView () {
		testViewBlack?.frame = CGRect(
			origin: CGPoint.zero,
			size: UIScreen.main.bounds.size)
		let visibleRect = self.view.convert(tableView.bounds, to: self.testViewBlack)
		self.testViewBlack?.frame = visibleRect


		let testView = UIView(frame: CGRect(x: 0, y: 0, width: self.view.frame.size.width, height: self.view.frame.size.height))
		testView.backgroundColor = UIColor.red
		
		testView.center = CGPoint(x: self.testViewBlack!.frame.size.width  / 2,
									  y: self.testViewBlack!.frame.size.height / 2)
		self.testView = testView
		self.testViewBlack?.addSubview(testView)
		tableView.isScrollEnabled = false;
		testView.isHidden = false;
		self.testViewBlack?.isHidden = false
	}
		
	fileprivate func runInteractiveTest( _ test: MaplyTestCase, type : ConfigSection.Row) {
		if test.state != MaplyTestCaseState()
			&& test.state != MaplyTestCaseState.error
			&& test.state != MaplyTestCaseState.running {

			lastInteractiveTestSelected = test
			self.navigationItem.leftBarButtonItem = UIBarButtonItem(
				barButtonSystemItem: .done,
				target: self,
				action: #selector(StartupViewController.stopInteractiveTest))
			
			self.prepareTestView()
			test.interactive = true

			test.options = []
			
			if (type == .RunMap) {
				test.options.insert(.map)
			}
			else  if (type == .RunGlobe) {
				test.options.insert(.globe)
			}
			test.testView = self.testView
			test.start()
		}
	}

	@objc func stopInteractiveTest() {
		self.navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .edit, target: self, action: #selector(StartupViewController.showConfig))
		self.testViewBlack?.isHidden = true
		tableView.isScrollEnabled = true

		if let last = self.lastInteractiveTestSelected {
			last.state = .ready
			last.interactive = false
			if let globe = last.globeViewController {
				last.tearDown(withGlobe: globe)
				last.removeGlobeController()
			}
			if let map = last.mapViewController {
				last.tearDown(withMap: map)
				last.removeMapController()
			}
		}
	}
	
	@objc func runTests() {
		self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .stop, target: self, action: #selector(StartupViewController.stopTests))
		
		self.title = "Running tests..."
		
		self.prepareTestView()
		self.results.removeAll()
		
		cancelled = false
		
		startTests(tests)
	}
	
	@objc func stopTests() {
	}

	fileprivate func startTests(_ tests: [MaplyTestCase]) {
		if let head = tests.first {
			let tail = Array(tests.dropFirst())
			
			if (head.state == .selected) {
				head.options = []
				if configViewC!.valueForSection(.Options, row: .RunGlobe) {
					head.options.insert(.globe)
				}
				
				if configViewC!.valueForSection(.Options, row: .RunMap) {
					head.options.insert(.map)
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
					self.timer = Timer.scheduledTimer(timeInterval: 1,
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
		
	fileprivate func runTest(_ test: MaplyTestCase) {
		self.title = "Running test..."

		self.prepareTestView()
		
		self.results.removeAll()

		test.options = []
		if configViewC!.valueForSection(.Options, row: .RunGlobe) {
			test.options.insert(.globe)
		}

		if configViewC!.valueForSection(.Options, row: .RunMap) {
			test.options.insert(.map)
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
			self.timer = Timer.scheduledTimer(timeInterval: 1,
				target: self,
				selector: #selector(StartupViewController.updateTitle(_:)),
				userInfo: test.name,
				repeats: true)
		}
		
		test.testView = self.testView
		test.start()
	}

	@objc func updateTitle(_ timer: Timer) {
		self.seconds -= 1
		self.title = "\(timer.userInfo!) (\(self.seconds))"
		if self.seconds == 0 {
			self.timer.invalidate()
		}
	}

	fileprivate func finishTests() {
		self.testViewBlack?.isHidden = true
		tableView.reloadData()

		if ConfigSection.Row.MultipleMode.load() {
			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .play, target: self, action: #selector(StartupViewController.runTests))
		}

		self.title = "Tests"
		tableView.isScrollEnabled = true
		if !cancelled {
			self.performSegue(withIdentifier: "results", sender: self)
		}
	}

	@objc fileprivate dynamic func editDone() {
		self.navigationController!.popToViewController(self, animated: true)
		changeSettings()

		if ConfigSection.Row.MultipleMode.load() {
			self.navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .play, target: self, action: #selector(StartupViewController.runTests))
		}
		else {
			self.navigationItem.rightBarButtonItem = nil
		}
		testsTable.reloadData()
	}

	func popoverControllerDidDismissPopover(_ popoverController: UIPopoverPresentationController) {
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
			tests.filter {
				$0.state != .error && $0.state != MaplyTestCaseState()
			}.forEach {
				$0.state = select ? .selected : .ready
			}
			tableView.reloadData()
			configViewC!.selectAll(.Actions, select: false)
		}
	}
}
