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
	
	// Added to work around a loading problem:
	// http://cjwirth.com/2016/03/26/xcode-7-3-crashing/
	let bufix = ""

	enum Section : String {
		case Options = "Options"
		case ExecutionMode = "Execution Mode"
		case Actions = "Actions"
	}

	enum Row : String {
		case RunGlobe = "Run on globe"
		case RunMap = "Run on map"
		case ViewTest = "See test execution on View"
		case MultipleMode = "Multiple"
		case SingleMode = "Single"
		case InteractiveMode = "Interactive"
		case SelectAll = "Select all"
		case SelectNone = "Select none"

		var defaultState: Bool {
			switch self {
			case .RunGlobe, .RunMap, .InteractiveMode, .ViewTest:
				return true
			default:
				return false
			}
		}

		func save(_ on: Bool) {
			UserDefaults.standard.set(on ? "y" : "n", forKey: self.rawValue)
		}

		func load() -> Bool {
			let defaultStateChar = self.defaultState ? "y" : "n"
			return (UserDefaults.standard.string(forKey: self.rawValue) ?? defaultStateChar) == "y"
		}
	}

	// Section name (as dispalyed to user)
	var section: Section

	// Entries (name,boolean) within the section
	var rows: [Row:Bool]

	// If set, user can only select one of the options
	var singleSelect: Bool

	init(section: Section, rows: [Row:Bool], singleSelect: Bool) {
		self.section = section
		self.rows = rows
		self.singleSelect = singleSelect
	}

	func selectAll(_ select: Bool) {
		for (k,_) in rows {
			rows[k] = select
			k.save(select)
		}
	}

	class func firstSelected(_ section: [String:Bool]) -> String? {
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

	@IBOutlet weak var tableView: UITableView?

	// Dictionary reflecting the current values from the table
	var values = [ConfigSection]()

	// Return the configuration value for a section/row
	func valueForSection(_ section: ConfigSection.Section, row: ConfigSection.Row) -> Bool {
		return values
			.filter{ $0.section == section }
			.first?
			.rows[row] ?? false
	}

	func selectAll(_ section: ConfigSection.Section, select: Bool) {
		values
			.filter { $0.section == section }
			.first?
			.selectAll(select)

		tableView?.reloadData()
	}


	func loadValues() {
		values.removeAll(keepingCapacity: true)

		let modeSection = ConfigSection(
			section: .ExecutionMode,
			rows: [
				.MultipleMode : ConfigSection.Row.MultipleMode.load(),
				.SingleMode : ConfigSection.Row.SingleMode.load(),
				.InteractiveMode : ConfigSection.Row.InteractiveMode.load()
			], singleSelect: true)


		let optionsSection = ConfigSection(
			section: .Options,
			rows: [
				.RunGlobe: ConfigSection.Row.RunGlobe.load(),
				.RunMap: ConfigSection.Row.RunMap.load(),
				.ViewTest: ConfigSection.Row.ViewTest.load()
			],
			singleSelect: false)
		
		let actionsSection = ConfigSection(
			section: .Actions,
			rows: [
				.SelectAll: false,
				.SelectNone: false,
			],
			singleSelect: true)
		
		values.append(modeSection)
		if !ConfigSection.Row.InteractiveMode.load() {
			values.append(optionsSection)
			if !ConfigSection.Row.SingleMode.load() {
				values.append(actionsSection)
			}
		}
	}

	func numberOfSections(in tableView: UITableView) -> Int {
		return values.count
	}

	func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
		if section >= values.count {
			return nil
		}

		return self.values[section].section.rawValue
	}

	func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		if section >= values.count {
			return 0
		}

		return values[section].rows.count
	}

	func tableView(_ tableView: UITableView,
		willDisplay cell: UITableViewCell, forRowAt
		indexPath: IndexPath) {

		if (indexPath as NSIndexPath).section >= values.count {
			return
		}

		let section = values[(indexPath as NSIndexPath).section]

		if (indexPath as NSIndexPath).row >= section.rows.count {
			return
		}

		let items = Array(section.rows.keys).sorted {
			return $0.rawValue < $1.rawValue
		}
		let key = items[(indexPath as NSIndexPath).row]
		let selected = section.rows[key]

		cell.backgroundColor = (selected ?? false)
			? UIColor(red: 0.75, green: 0.75, blue: 1.0, alpha: 1.0)
			: UIColor.white
	}

	func tableView(_ tableView: UITableView,
		cellForRowAt indexPath: IndexPath) -> UITableViewCell {

		let section = values[(indexPath as NSIndexPath).section]
		let items = Array(section.rows.keys).sorted {
			return $0.rawValue < $1.rawValue
		}
		let key = items[(indexPath as NSIndexPath).row]
		let cell = UITableViewCell(style: .default, reuseIdentifier: "cell")
		cell.textLabel?.text = key.rawValue

		return cell
	}

	func tableView(_ tableView: UITableView,
		didSelectRowAt indexPath: IndexPath) {

		if (indexPath as NSIndexPath).section >= values.count {
			return
		}

		let section = values[(indexPath as NSIndexPath).section]

		if (indexPath as NSIndexPath).row >= section.rows.count {
			return
		}

		let items = Array(section.rows.keys).sorted {
			return $0.rawValue < $1.rawValue
		}
		let key = items[(indexPath as NSIndexPath).row]
		let selected = section.rows[key] ?? false

		if section.singleSelect {
			// Turn everything else off and this one on
			section.selectAll(false)
			section.rows[key] = true

			section.rows.forEach {
				let (row, state) = $0
				row.save(state)
			}
		}
		else {
			let newState = !selected
			section.rows[key] = newState
			key.save(newState)
		}
		loadValues()
		tableView.reloadData()
	}

}

