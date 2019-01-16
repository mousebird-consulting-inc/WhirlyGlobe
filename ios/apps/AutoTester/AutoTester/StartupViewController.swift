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

		AnimatedBasemapTestCase(),
		ScreenLabelsTestCase(),
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

//        PagingLayerTestCase(),
//        VectorMBTilesTestCase(),
//        OpenMapTilesTestCase(),
        OpenMapTilesHybridTestCase(),

//        StarsSunTestCase(),
		ShapesTestCase(),
		LoftedPolysTestCase(),

//        CartoDBTestCase(),

//        BNGCustomMapTestCase(),
//        BNGTestCase(),
//        ElevationLocalDatabase(),
//        ParticleTestCase(),
        RunwayBuilderTestCase(),

//        AnimatedColorRampTestCase(),
		ExtrudedModelTestCase(),
		ModelsTestCase(),
		GreatCircleTestCase(),
		
		LabelAnimationTestCase(),
//        WMSTestCase(),
		FindHeightTestCase(),
		FullAnimationTest(),
		ActiveObjectTestCase(),
		AnimationDelegateTestCase(),
		LocationTrackingSimTestCase(),
		LocationTrackingRealTestCase(),

//        LIDARTestCase()
	]

	@IBOutlet weak var testsTable: UITableView!

	fileprivate var lastInteractiveTestSelected: MaplyTestCase?
    
    override func viewWillAppear(_ animated: Bool) {
        if let last = self.lastInteractiveTestSelected {
            self.navigationController?.popToViewController(self, animated: true)
            last.baseViewController?.teardown()
            lastInteractiveTestSelected = nil
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
        if test.implementations.contains(.globe) {
            cell.globeButton.isHidden = false
        }
        if test.implementations.contains(.map) {
            cell.mapButton.isHidden = false
        }
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
