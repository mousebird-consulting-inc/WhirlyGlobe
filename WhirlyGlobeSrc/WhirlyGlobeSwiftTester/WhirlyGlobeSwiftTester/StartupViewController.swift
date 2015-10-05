//
//  StartupViewController.swift
//  WhirlyGlobeSwiftTester
//
//  Created by jmnavarro on 14/09/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit

class StartupViewController: UITableViewController {

	private let supportedTypes: [Int:MapType] = [
		0: MapType.MaplyGlobe(elevation: false),
		1: MapType.MaplyGlobe(elevation: true),
		2: MapType.Maply3DMap,
		3: MapType.Maply2DMap]


	override func tableView(tableView: UITableView,
		numberOfRowsInSection section: Int) -> Int {

		return MapType.numTypes()
	}

	override func tableView(tableView: UITableView,
		cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {

		let cell = tableView.dequeueReusableCellWithIdentifier("cell", forIndexPath: indexPath) as! UITableViewCell

		cell.textLabel?.text = supportedTypes[indexPath.row]?.name

		return cell
	}

	override func tableView(tableView: UITableView,
		didSelectRowAtIndexPath indexPath: NSIndexPath) {

		self.performSegueWithIdentifier("tester", sender: indexPath.row)
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
		let destination = segue.destinationViewController as! TestViewController
		let row = sender as! Int
		destination.mapType = supportedTypes[row]!
	}

	



}
