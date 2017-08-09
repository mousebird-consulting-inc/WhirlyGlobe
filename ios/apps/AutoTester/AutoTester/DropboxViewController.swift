//
//  DropboxViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 18/11/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

import UIKit
//import SwiftyDropbox

class DropboxSection {
	
	enum Section : String{
		case Login = "Login Actions"
		case Actions = "Dropbox Actions"
	}
	
	enum Row : String {
		case LoginUser = "Login"
		case LogoutUser = "Logout"
		case Upload = "Upload Tests"
	}
	
	var section : Section
	
	var rows : [Row]
	
	init (section: Section, isLogin:Bool) {
		self.section = section
		if section == .Login {
			self.rows = [isLogin ? .LogoutUser : .LoginUser]
		}
		else {
			self.rows = isLogin ? [.Upload] : []
		}
	}

}


class DropboxViewController: UIViewController, UITableViewDataSource, UITableViewDelegate {

	@IBOutlet weak var tableView: UITableView?
	
	var values = [DropboxSection]()
	fileprivate var processLogin = false
	
	var results = [MaplyTestResult]()
	var titles = [String]()
	var folder : String?
	
	func loadValues() {
		values.removeAll(keepingCapacity: true)
		let isLogin = false //(Dropbox.authorizedClient != nil)

		let loginSection = DropboxSection(
			section: .Login,
			isLogin: isLogin)
		
		let actionSection = DropboxSection(
			section: .Actions,
			isLogin: isLogin)

		values.append(loginSection)
		values.append(actionSection)
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
	
	func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
		
		let section = values [(indexPath as NSIndexPath).section]
		let key = section.rows[(indexPath as NSIndexPath).row]
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

		switch section.rows[(indexPath as NSIndexPath).row] {
			case .LoginUser:
				processLogin = true
//				Dropbox.authorizeFromController(self)
			case .LogoutUser:
//				Dropbox.unlinkClient()
				self.loadValues()
				tableView.reloadData()
			case .Upload: ()
//				uploadToDropbox()
		}

		tableView.deselectRow(at: indexPath, animated: false)
	}
/*
	override func viewDidLoad() {
		super.viewDidLoad()

		tableView!.tableFooterView = UIView()
		self.loadValues()
	}
	
	override func viewWillAppear(_ animated: Bool) {
		if processLogin {
			// Quick & dirty hack. Instead of add delay, we should
			// wait for dropbox notification
			let delay = 2 * Double(NSEC_PER_SEC)
			let time = DispatchTime.now() + Double(Int64(delay)) / Double(NSEC_PER_SEC)
			DispatchQueue.main.asyncAfter(deadline: time) {
				if Dropbox.authorizedClient != nil {
					self.loadValues()
				}
				self.tableView?.reloadData()
				self.processLogin = false
			}
		}
	}
	
	func resizeImage(_ image: UIImage) -> (Data?) {
		var actualHeight = image.size.height
		var actualWidth = image.size.width

		// Magic number: this is the size of baselines images.
		let maxHeight = 1008.0
		let maxWidth = 702.0

		var imgRatio = actualWidth / actualHeight
		let maxRatio = maxWidth / maxHeight
		let compressionQuality = 0.5 as CGFloat

		if actualHeight > CGFloat(maxHeight) || actualWidth > CGFloat(maxWidth) {
			if imgRatio < CGFloat(maxRatio) {
				imgRatio = CGFloat(maxHeight) / actualHeight
				actualWidth = imgRatio * actualWidth
				actualHeight = CGFloat(maxHeight)
			}
			else if imgRatio > CGFloat(maxRatio) {
				imgRatio = CGFloat(maxWidth) / actualWidth
				actualHeight = imgRatio * actualHeight
				actualWidth = CGFloat(maxWidth)
			}
			else {
				actualHeight = CGFloat(maxHeight)
				actualWidth = CGFloat (maxWidth)
			}
		}

		let rect = CGRect(x: 0.0, y: 0.0, width: actualWidth, height: actualHeight)
		UIGraphicsBeginImageContext(rect.size)
		image.draw(in: rect)
		let img = UIGraphicsGetImageFromCurrentImageContext()
		let imageData = UIImageJPEGRepresentation(img!, compressionQuality)
		UIGraphicsEndImageContext()

		return imageData
	}

	func uploadToDropbox () {
        if let client = Dropbox.authorizedClient {
			let hud = MBProgressHUD.showAdded(to: self.view, animated:true)
			hud?.mode = MBProgressHUDMode.indeterminate
			hud?.labelText = "Starting..."

			let date = Date()
			let dateFormatter = DateFormatter()
			dateFormatter.dateFormat = "yyy-MM-dd HH-mm"
			self.folder = dateFormatter.string(from: date) as String

			self.createFolder(client, hud: hud) {
				self.uploadImages(client, hud: hud) {
					self.checkBaselines(client, hud: hud) {
						self.uploadBaselines(client, hud: hud, missingBaselines: $0) {
							self.createHTMLTestResult(client, hud: hud) {
								self.uploadHTMLTestResult(client, hud: hud, html: $0) {
									self.uploadCSSFile(client, hud: hud) {
										self.updateHTMLTestList(client, hud: hud) {
											if let row = $0 {
												self.uploadHTMLIndexList(client, tableRow: row) {
													hud.hide(true)
												}
											}
											else {
												hud.hide(true)
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	func createFolder(_ client: DropboxClient,
			hud: MBProgressHUD,
			continuation: @escaping () -> ()) {

		hud.labelText = "Creating folders..."

		client.files.createFolder(
			path: "/\(self.folder!)").response { response, error in
				if let error = error {
					print("Error creating folder structure: \(error)")
					hud.hide(true)
				}
				else {
					continuation()
				}
		}
	}
	
	func uploadImages(_ client: DropboxClient,
			hud: MBProgressHUD,
			continuation: @escaping () -> ()) {

		func checkFinished(_ uploads: inout Int) {
			uploads -= 1
			if uploads == 0 {
				continuation()
			}
		}

		hud.labelText = "Uploading images"
		var uploads = self.results.count

		for (i,result) in self.results.enumerated() {
			let title = titles[i]
			if let actualImage = result.actualImageFile,
					let data = try? Data(contentsOf: URL(fileURLWithPath: actualImage)),
					let image = UIImage(data: data),
					let imageToUpload = self.resizeImage(image) {

				let imageName = title.replacingOccurrences(of: " ", with: "")

				client.files.upload(
					path: "/\(self.folder!)/images/\(imageName)-actual.jpg",
					body: imageToUpload).response({ (response, error) -> () in
						if let error = error {
							print("Error uploading \(imageName): \(error)")
						}
						else {
							print("Uploaded file name: \(response?.name)")
						}

						checkFinished(&uploads)
				})
			}
			else {
				checkFinished(&uploads)
			}
		}
	}

	func checkBaselines(_ client: DropboxClient,
			hud: MBProgressHUD,
			continuation: @escaping ([MaplyTestResult]) -> ()) {

		hud.labelText = "Checking baselines..."
		client.files.listFolder(
			path: "/baselines").response { response, error in
				if let response = response {
					// baselines exists
					var testResults = self.results
					for file in response.entries {
						for (i,result) in testResults.enumerated() {
							if file.name == self.baselineFilename(result) {
								testResults.remove(at: i)
								break
							}
						}
					}

					continuation(testResults)
				}
				else {
					client.files.createFolder(
						path: "/baselines").response { response, error in
							if let error = error {
								print("Error creating baselines folder: \(error)")
								continuation([])
							}
							else {
								continuation(self.results)
							}
					}
				}
		}
	}

	func uploadBaselines(_ client: DropboxClient,
			hud: MBProgressHUD,
			missingBaselines: [MaplyTestResult],
			continuation: @escaping () -> ()) {

		func checkFinished(_ uploads: inout Int) {
			uploads -= 1
			if uploads == 0 {
				continuation()
			}
		}

		if missingBaselines.isEmpty {
			continuation()
			return
		}

		hud.labelText = "Uploading baselines..."
		var uploads = missingBaselines.count

		for result in missingBaselines {
			if let data = try? Data(contentsOf: URL(fileURLWithPath: result.baselineImageFile)),
				let image = UIImage(data: data),
				let imageToUpload = self.resizeImage(image) {
			
				let imageName = baselineFilename(result)

				client.files.upload(
					path: "/baselines/\(imageName)",
					body: imageToUpload).response { response, error in
						if let response = response {
							print("Uploaded file: \(response.name)")
						}
						else {
							print("Error uploading \(imageName): \(error)")
						}

						checkFinished(&uploads)
					}
			}
			else {
				checkFinished(&uploads)
			}
		}
	}

	func createHTMLTestResult(_ client: DropboxClient,
			hud: MBProgressHUD,
			continuation: (String) -> ()) {

		hud.labelText = "Creating html file"

		let device = UIDevice.current

		var html = "<!DOCTYPE html><html<head><meta charset=\"UTF-8\"><meta name=\"description\" content=\"Execution results of tests\"><link type=\"text/css\" rel=\"stylesheet\" href=\"css/styles.css\"><Title>\(self.folder!)</Title></head><body><h1>Tests Results - \(self.folder!)</h1><div id=\"info\"><h2>Device Info</h2><h3>System Name: \(device.systemName)</h3><h3>System Version: \(device.systemVersion)</h3><h3>Model: \(device.model)</h3><h3>Localized Model: \(device.localizedModel)</h3>"

		for (i,result) in self.results.enumerated() {
			let title = self.titles[i].replacingOccurrences(of: " ", with: "")

			html += "<div id=\"tests\"><h2>\(result.testName) - \(result.testName.contains("-Globe-") ? "Globe" : "Map")</h2><div id=\"expected\"><h3>Expected</h3><img src=\"../baselines/\(baselineFilename(result))\"></div><div id=\"actual\"><h3>Actual</h3><img src=\"images/\(title)-actual.jpg\"></div></div>"
		}

		html += "</body></html>"

		continuation(html)
	}

	func uploadHTMLTestResult(_ client: DropboxClient,
			hud: MBProgressHUD,
			html: String,
			continuation: @escaping () -> ()) {

		let data = html.data(using: String.Encoding.utf8)

		client.files.upload(
			path: "/\(self.folder!)/index.html",
			body: data!).response { (response, error) in
				if let file = response{
					print("Uploaded html file name: \(file.name)")
					continuation()
				}
				else {
					print("Error creating html file")
					hud.hide(true)
				}
		}
	}

	func uploadCSSFile(_ client: DropboxClient,
			hud: MBProgressHUD,
			continuation: @escaping () -> ()) {
		hud.labelText = "Uploading CSS File"

		if let path = Bundle.main.path(forResource: "styles", ofType: "css"),
				let data = try? Data(contentsOf: URL(fileURLWithPath: path)) {
			client.files.upload(
				path: "/\(self.folder!)/css/styles.css",
				body: data).response { response, error in
					if let error = error {
						print("Error uploadling css file: \(error)")
					}
					else {
						print("Uploaded css file name:\(response?.name)")
						continuation()
					}
			}
		}
		else {
			print("Error getting css file")
			hud.hide(true)
		}
	}

	func updateHTMLTestList(_ client: DropboxClient,
			hud: MBProgressHUD,
			continuation: @escaping (String?) -> ()) {

		hud.labelText = "Updating HTML List"

		client.files.download(path: "/index.html", destination: {
			temporaryURL, response in
			let fileManager = FileManager.default
			let directoryURL = fileManager.urls(for: .documentDirectory, in: .userDomainMask)[0]
			// generate a unique name for this file in case we've seen it before
			let pathComponent = "\(UUID().uuidString)-\(response.suggestedFilename!)"
			return directoryURL.appendingPathComponent(pathComponent)
		}
		).response { response, error in
			var tableHTML = ""
			if let (_, url) = response {
				// index.html exists
				do {
					let text = try String(contentsOf: url, encoding: String.Encoding.utf8)
					let compHTML = text.components(separatedBy: "<table>")
					let compTable = compHTML.last?.components(separatedBy: "</table>")
					let compTests = compTable?.first?.replacingOccurrences(of: "<tr><th><strong>Test Name</strong></th><th><strong>Test Count</strong></th></tr>", with: "")
					let name = self.folder!
					tableHTML = "<tr><td><a href=\"\(name.replacingOccurrences(of: " ", with: "%20"))/index.html\">\(name)</a></td><td>\(self.results.count)</td>\(compTests!)"
				}
				catch let error {
					print("Error updating HTML TEST LIST file: \(error)")
					continuation(nil)
				}
			}
			else {
				let name = self.folder!
				tableHTML = "<tr><td><a href=\"\(name.replacingOccurrences(of: " ", with: "%20"))/index.html\">\(name)</a></td><td>\(self.results.count)</td>"
			}

			continuation(tableHTML)
		}
	}

	func uploadHTMLIndexList(_ client: DropboxClient,
			tableRow: String,
			continuation: @escaping () -> ()) {

		let htmlString = "<!DOCTYPE html><html><head><title>Execution list result</title> <style type=\"text/css\"> body{background: #ffffff; background: -moz-linear-gradient(left, #ffffff 0%, #f6f6f6 47%, #ededed 100%); background: -webkit-gradient(left top, right top, color-stop(0%, #ffffff), color-stop(47%, #f6f6f6), color-stop(100%, #ededed)); background: -webkit-linear-gradient(left, #ffffff 0%, #f6f6f6 47%, #ededed 100%); background: -o-linear-gradient(left, #ffffff 0%, #f6f6f6 47%, #ededed 100%); background: -ms-linear-gradient(left, #ffffff 0%, #f6f6f6 47%, #ededed 100%); background: linear-gradient(to right, #ffffff 0%, #f6f6f6 47%, #ededed 100%); filter: progid:DXImageTransform.Microsoft.gradient( startColorstr='#ffffff', endColorstr='#ededed',GradientType=1 );} h1{ color: #0C90E2; text-align: center; } strong{ color: #DEE20C; text-align: center; } table{ margin: auto; width: 90%; border:1px solid #73AD21; padding: 10px; overflow: hidden; margin-bottom: 10px;}</style></head><body><h1>Test List</h1><table><tr><th><strong>Test Name</strong></th><th><strong>Test Count</strong></th></tr>\(tableRow)</table></body></html>"

		if let data = htmlString.data(using: String.Encoding.utf8) {
			client.files.upload(path: "/index.html",
				mode: .overwrite,
				autorename: false,
				clientModified: Date(),
				mute: true,
				body: data).response { response, error in
					if let error = error {
						print("Error creating html file: \(error)")
					}
					else {
						print("Uploaded html file: \(response?.name)")
					}
					continuation()
				}
		}
		else {
			print("Error encoding html file")
			continuation()
		}
	}

	func baselineFilename(_ testResult: MaplyTestResult) -> String {
		return testResult.baselineImageFile
			.characters
			.split {
				$0 == "/"
			}
			.map(String.init)
			.last!
	}
*/
}
