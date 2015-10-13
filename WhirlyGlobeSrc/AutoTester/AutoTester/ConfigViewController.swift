//
//  ConfigViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

// Section in the configuration panel
class ConfigSection {

	// Section name (as dispalyed to user)
	var sectionName: String

	// Entries (name,boolean) within the section
	var rows: [String:Bool]

	// If set, user can only select one of the options
	var singleSelect: Bool

	init(sectionName: String, rows: [String:Bool], singleSelect: Bool) {
		self.sectionName = sectionName
		self.rows = rows
		self.singleSelect = singleSelect
	}

	func selectAll(select: Bool) {
		for (k,_) in rows {
			rows[k] = select
		}
	}

	class func firstSelected(section: [String:Bool]) -> String? {
		for (k,v) in section {
			if v {
				return k
			}
		}

		return nil
	}
}


// Configuration view lets the user decide what to turn on and off
class ConfigViewController: UIViewController, UITableViewDataSource, UITableViewDelegate {

	// Dictionary reflecting the current values from the table
	var values = [ConfigSection]()

	// Return the configuration value for a section/row
	func valueForSection(section: String, row: String) -> Bool {
		return values
			.filter{ $0.sectionName == section }
			.first?
			.rows[row] ?? false
	}


	func loadValues() {
		values.removeAll(keepCapacity: true)

		let baseLayersSection = ConfigSection(
			sectionName: kMaplyTestCategoryFeatures,
			rows: [
				kMaplyTestGeographyClass: false,
			],
			singleSelect: false)

		values.append(baseLayersSection)

	}

	func numberOfSectionsInTableView(tableView: UITableView) -> Int {
		return values.count
	}

	func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
		if section >= values.count {
			return nil
		}

		return values[section].sectionName
	}

	func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		if section >= values.count {
			return 0
		}

		return values[section].rows.count
	}

	func tableView(tableView: UITableView,
		willDisplayCell cell: UITableViewCell, forRowAtIndexPath
		indexPath: NSIndexPath) {

		if indexPath.section >= values.count {
			return
		}

		let section = values[indexPath.section]

		if indexPath.row >= section.rows.count {
			return
		}

		let items = Array(section.rows.keys)
		let key = items[indexPath.row]
		let selected = section.rows[key]

		cell.backgroundColor = (selected ?? false)
			? UIColor(red: 0.75, green: 0.75, blue: 1.0, alpha: 1.0)
			: UIColor.whiteColor()
	}

	func tableView(tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let section = values[indexPath.section]
		let items = Array(section.rows.keys)
		let key = items[indexPath.row]
		let cell = UITableViewCell(style: .Default, reuseIdentifier: "cell")
		cell.textLabel?.text = key

		return cell
	}

	func tableView(tableView: UITableView,
		didSelectRowAtIndexPath indexPath: NSIndexPath) {

		if indexPath.section >= values.count {
			return
		}

		let section = values[indexPath.section]

		if indexPath.row >= section.rows.count {
			return
		}

		let items = Array(section.rows.keys)
		let key = items[indexPath.row]
		let selected = section.rows[key] ?? false

		if section.singleSelect {
			// Turn everything else off and this one on
			section.selectAll(false)
			section.rows[key] = true
		}
		else {
			section.rows[key] = !selected
		}
		tableView.reloadData()
	}

}


public let kMaplyTestCategoryFeatures = "Features"

public let kMaplyTestBlank = "Blank"
public let kMaplyTestGeographyClass = "Geography Class - Local"
public let kMaplyTestBlueMarble = "NASA Blue Marble - Local"
public let kMaplyTestStamenWatercolor = "Stamen Watercolor - Remote"
public let kMaplyTestOSM = "OpenStreetMap (Mapquest) - Remote"
public let kMaplyTestMapBoxSat = "MapBox Satellite - Remote"
public let kMaplyTestMapBoxTerrain = "MapBox Terrain - Remote"
public let kMaplyTestMapBoxRegular = "MapBox Regular - Remote"
public let kMaplyTestNightAndDay = "Night/Day Images - Remote"
public let kMaplyTestQuadTest = "Quad Test Layer"
public let kMaplyTestQuadTestAnimate = "Quad Test Layer - Animated"
public let kMaplyTestQuadVectorTest = "Quad Vector Test Layer"
public let kMaplyTestElevation = "Cesium Elevation Test Layer"


// Overlay image layers
public let kMaplyTestCategoryOverlayLayers = "Overlay layers"

public let kMaplyTestUSGSOrtho = "USGS Ortho (WMS) - Remote"
public let kMaplyTestOWM = "OpenWeatherMap - Remote"
public let kMaplyTestForecastIO = "Forecast.IO Snapshot - Remote"
public let kMaplyTestMapboxStreets = "MapBox Streets Vectors - Remote"
public let kMaplyMapzenVectors = "Mapzen Vectors - Remote"

// Objects we can display
public let kMaplyTestCategoryObjects = "Maply Objects"

public let kMaplyTestLabel2D = "Labels - 2D"
public let kMaplyTestLabel3D = "Labels - 3D"
public let kMaplyTestMarker2D = "Markers - 2D"
public let kMaplyTestMarker3D = "Markers - 3D"
public let kMaplyTestSticker = "Stickers"
public let kMaplyTestShapeCylinder = "Shapes: Cylinders"
public let kMaplyTestShapeSphere = "Shapes: Spheres"
public let kMaplyTestShapeGreatCircle = "Shapes: Great Circles"
public let kMaplyTestShapeArrows = "Shapes: Arrows"
public let kMaplyTestModels = "Models"
public let kMaplyTestCountry = "Countries"
public let kMaplyTestLoftedPoly = "Lofted Polygons"
public let kMaplyTestQuadMarkers = "Quad Paging Markers"
public let kMaplyTestMegaMarkers = "Mega Markers"
public let kMaplyTestLatLon = "Lon/Lat lines"
public let kMaplyTestRoads = "SF Roads"
public let kMaplyTestArcGIS = "ArcGIS Vectors"
public let kMaplyTestStarsAndSun = "Stars and Sun"

// Animation
public let kMaplyTestCategoryAnimation = "Animation"

public let kMaplyTestAnimateSphere = "Animated Sphere"

// Gestures
public let kMaplyTestCategoryGestures = "Gestures"

public let kMaplyTestNorthUp = "Keep North Up"
public let kMaplyTestPinch = "Pinch Gesture"
public let kMaplyTestRotate = "Rotate Gesture"

// Low level
public let kMaplyTestCategoryInternal = "Internals"
public let kMaplyTestCulling = "Culling Optimization"
public let kMaplyTestPerf = "Performance Output"
public let kMaplyTestWaitLoad = "Image waitLoad"
