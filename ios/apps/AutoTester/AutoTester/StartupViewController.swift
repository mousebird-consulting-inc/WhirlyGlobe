//
//  StartupViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015-2021 mousebird consulting. All rights reserved.
//

import UIKit

class StartupViewController: UITableViewController, UIPopoverPresentationControllerDelegate {

	let tests = [
        StamenWatercolorRemote(),
		GeographyClassTestCase(),
        NASAGIBSTestCase(),
		AnimatedBasemapTestCase(),
        ImageReloadTestCase(),
        BNGCustomMapTestCase(),
        BNGTestCase(),
        // Note: Endpoint missing
//        WMSTestCase(),

        ScreenLabelsTestCase(),
		ScreenMarkersTestCase(),
		MarkersTestCase(),
		AnimatedMarkersTestCase(),
        ClusteredMarkersTestCase(),
        LabelAnimationTestCase(),
        MovingScreenMarkersTestCase(),

        VectorsTestCase(),
        ChangeVectorsTestCase(),
        GreatCircleTestCase(),
		VectorStyleTestCase(),
		VectorHoleTestCase(),
        ShapefileTestCase(),
		WideVectorsTestCase(),
		TextureVectorTestCase(),
        SimpleStyleTestCase(),
        GeoJSONStyleTestCase(),
        LoftedPolysTestCase(),
        RepresentationsTestCase(),
        AirwayTestCase(),

        // Note: 3D labels are currently broken
        LabelsTestCase(),
		StickersTestCase(),

        PagingLayerTestCase(),
        VectorMBTilesTestCase(),
        CartoDBTestCase(),
        MapTilerTestCase(),
        MapTilerCircleTestCase(),
        MapboxTestCase(),

		ShapesTestCase(),
        ExtrudedModelTestCase(),
        ModelsTestCase(),
        BillboardTestCase(),
        RunwayBuilderTestCase(),
        StarsSunTestCase(),

		FindHeightTestCase(),
		AnimationDelegateTestCase(),
		LocationTrackingSimTestCase(),
		LocationTrackingRealTestCase(),
        
//        OfflineRenderTestCase(),
        
        StartupShutdownTestCase(),
        LayerStartupShutdownTestCase()
	]

	@IBOutlet weak var testsTable: UITableView!

	fileprivate var lastInteractiveTestSelected: MaplyTestCase?
    
    override func viewWillAppear(_ animated: Bool) {
        if let last = self.lastInteractiveTestSelected {
            self.navigationController?.popToViewController(self, animated: true)
            last.stop()
            lastInteractiveTestSelected = nil
        }
    }
    
    private var firstView = true
    override func viewDidAppear(_ animated: Bool) {
        if firstView {
            firstView = false;
            let env = ProcessInfo.processInfo.environment
            let type = MaplyTestCaseImplementations.init(rawValue:UInt.init(env["START_TEST_TYPE"] ?? "") ?? MaplyTestCaseImplementations.globe.rawValue) // 2=globe, 4=map
            if let autoStartTest = env["START_TEST"], !autoStartTest.isEmpty {
                if let test = tests.filter({ test -> Bool in return test.name == autoStartTest }).first {
                    self.lastInteractiveTestSelected = test
                    if (type.contains(MaplyTestCaseImplementations.map) &&
                            test.implementations.contains(MaplyTestCaseImplementations.map)) {
                        test.startMap(self.navigationController!)
                    } else if (type.contains(MaplyTestCaseImplementations.globe) &&
                                test.implementations.contains(MaplyTestCaseImplementations.globe)) {
                        test.startGlobe(self.navigationController!)
                    }
                }
            }
        }
    }
    
	override func tableView(
		_ tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return tests.count
	}

	override func tableView(
		_ tableView: UITableView,
		cellForRowAt indexPath: IndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCell(withIdentifier: "cell",
			for: indexPath) as! TestCell

        let test = tests[(indexPath as NSIndexPath).row]
		cell.testName.text = test.name
		cell.selectionStyle = .none
		cell.implementations = test.implementations
        cell.globeButton.isHidden = !test.implementations.contains(.globe)
        cell.mapButton.isHidden = !test.implementations.contains(.map)
        cell.globeTestExecution = {
            self.lastInteractiveTestSelected = test
            test.startGlobe(self.navigationController!)
        }
        cell.mapTestExecution = {
            self.lastInteractiveTestSelected = test
            test.startMap(self.navigationController!)
        }

		return cell
	}
}
