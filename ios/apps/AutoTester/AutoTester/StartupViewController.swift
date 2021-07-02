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
        ActiveObjectTestCase(),
        AirwayTestCase(),
        AnimatedBasemapTestCase(),
        AnimatedMarkersTestCase(),
        AnimationDelegateTestCase(),
        BNGCustomMapTestCase(),
        BNGTestCase(),
        BillboardTestCase(),
        CartoDBLightTestCase(),
        CartoDBTestCase(),
        ChangeVectorsTestCase(),
        ClusteredMarkersTestCase(),
        ESRIRemoteTestCase(),
        ExtrudedModelTestCase(),
        FindHeightTestCase(),
        GeoJSONStyleTestCase(),
        GeographyClassTestCase(),
        GlobeSamplerTestCase(),
        GlyphProblemTestCase(),
        GreatCircleTestCase(),
        ImageReloadTestCase(),
        Issue721TestCase(),
        LIDARTestCase(),
        LabelAnimationTestCase(),
        LabelsTestCase(),       // Note: 3D labels are currently broken
        LayerStartupShutdownTestCase(),
        LocationTrackingRealTestCase(),
        LocationTrackingSimTestCase(),
        LoftedPolysTestCase(),
        MapTilerCircleTestCase(),
        MapTilerTestCase(),
        MapboxTestCase(),
        MarkersTestCase(),
        MegaMarkersTestCase(),
        ModelsTestCase(),
        MovingScreenLabelsTestCase(),
        MovingScreenMarkersTestCase(),
        NASAGIBSTestCase(),
        OfflineRenderTestCase(),
        PagingLayerTestCase(),
        ParticleTestCase(),
        RepresentationsTestCase(),
        RunwayBuilderTestCase(),
        ScreenLabelsTestCase(),
        ScreenMarkersTestCase(),
        ShapefileTestCase(),
        ShapesTestCase(),
        SimpleStyleTestCase(),
        StamenWatercolorRemote(),
        StarsSunTestCase(),
        StartupShutdownTestCase(),
        StickersTestCase(),
        TextureVectorTestCase(),
        VectorHoleTestCase(),
        VectorMBTilesTestCase(),
        VectorStyleTestCase(),
        VectorsTestCase(),
        WideVectorsTestCase(),
        WMSTestCase(),        // Note: Endpoint missing
    ].sorted(by: { (a,b) in a.name.caseInsensitiveCompare(b.name) == ComparisonResult.orderedAscending } )

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
