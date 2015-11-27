//
//  DropboxViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 18/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit
import SwiftyDropbox
import MBProgressHUD

class DropboxSection {
	
	enum Section : String{
		case Login = "Login Actions"
		case Actions = "Dropbox Actions"
	}
	
	enum Row : String {
		case LoginUser = "Login"
		case LogoutUser = "Logout"
		
		case Upload = "Upload Tests"
		case Back = "Back"
	}
	
	var section : Section
	
	var rows : [Row]
	
	init (section: Section, isLogin:Bool){
		self.section = section
		if section == .Login {
			self.rows = [isLogin ? .LogoutUser : .LoginUser]
		}
		else {
			self.rows = isLogin ? [.Upload] : []
			self.rows.append(.Back)
		}
	}

}


class DropboxViewController: UIViewController, UITableViewDataSource, UITableViewDelegate {

	@IBOutlet weak var tableView: UITableView?
	
	var values = [DropboxSection]()
	private var processLogin = false
	
	var results = [MaplyTestResult]()
	var titles = [String]()
	var folder : String?
	
	
	func loadValues(){
		
		values.removeAll(keepCapacity: true)
		let isLogin = (Dropbox.authorizedClient != nil)

		let loginSection = DropboxSection(
			section: .Login,
			isLogin: isLogin)
		
		let actionSection =  DropboxSection (
			section: .Actions,
			isLogin: isLogin)
		values.append(loginSection)
		values.append(actionSection)
		
	}
	
	
	func numberOfSectionsInTableView(tableView: UITableView) -> Int {
		return values.count
	}
	
	func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
		if section >= values.count {
			return nil
		}
		
		return self.values[section].section.rawValue
	}
	
	func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		if section >= values.count {
			return 0
		}
		
		return values[section].rows.count
	}
	
	func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
		
		let section = values [indexPath.section]
		let key = section.rows[indexPath.row]
		let cell = UITableViewCell(style: .Default, reuseIdentifier: "cell")
		cell.textLabel?.text = key.rawValue
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
			
			let key = section.rows[indexPath.row]
			switch(key){
				case .LoginUser:
					processLogin = true
					Dropbox.authorizeFromController(self)
					break;
				case .LogoutUser:
					Dropbox.unlinkClient()
					self.loadValues()
					tableView.reloadData()
					break;
				case .Upload:
					uploadToDropbox()
					break;
				case .Back:
					self.navigationController?.popViewControllerAnimated(true)
					break;
			}
			tableView.deselectRowAtIndexPath(indexPath, animated: false)
			
	}

	override func viewDidLoad() {
		super.viewDidLoad()

		tableView!.tableFooterView = UIView()
		self.loadValues()
	}
	
	override func viewWillAppear(animated: Bool) {
		if processLogin {
			// Quick & dirty hack. Instead of add delay, we should
			// wait for dropbox notification
			let delay = 2 * Double(NSEC_PER_SEC)
			let time = dispatch_time(DISPATCH_TIME_NOW, Int64(delay))
			dispatch_after(time, dispatch_get_main_queue()) {
				if Dropbox.authorizedClient != nil {
					self.loadValues()
				}
				self.tableView?.reloadData()
				self.processLogin = false
			}
		}
	}
	
	func resizeImage(image: UIImage) -> (NSData?) {
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

		let rect = CGRectMake(0.0, 0.0, actualWidth, actualHeight)
		UIGraphicsBeginImageContext(rect.size)
		image.drawInRect(rect)
		let img = UIGraphicsGetImageFromCurrentImageContext()
		let imageData = UIImageJPEGRepresentation(img, compressionQuality)
		UIGraphicsEndImageContext()

		return imageData
	}

	func uploadToDropbox () {
		if let client = Dropbox.authorizedClient {
			let hud = MBProgressHUD.showHUDAddedTo(self.view, animated:true)
			hud.mode = MBProgressHUDMode.Indeterminate
			hud.labelText = "Starting..."

			let date = NSDate()
			let dateFormatter = NSDateFormatter()
			dateFormatter.dateFormat = "yyy-MM-dd HH-mm"
			self.folder = dateFormatter.stringFromDate(date) as String

			self.createFolder(client, hud: hud) {
				self.uploadImages(client, hud: hud) {
					self.uploadMetadataJSON(client, hud: hud) {
						self.createHTMLTestResult(client, hud: hud) {
							self.uploadCSSFile(client, hud: hud)
						}
					}
				}
			}
		}
	}
	
	func createFolder(client: DropboxClient,
			hud: MBProgressHUD,
			continuation: () -> ()) {

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
	
	func uploadImages(client: DropboxClient,
			hud: MBProgressHUD,
			continuation: () -> ()) {

		hud.labelText = "Uploading images"
		var uploads = self.results.count

		for (i,result) in self.results.enumerate() {
			let title = titles[i]
			if let actualImage = result.actualImageFile,
					data = NSData.init(contentsOfFile: actualImage),
					image = UIImage(data: data),
					imageToUpload = self.resizeImage(image) {

				let imageName = title.stringByReplacingOccurrencesOfString(" ", withString: "")

				client.files.upload(
					path: "/\(self.folder!)/images/\(imageName)-actual.jpg",
					body: imageToUpload).response({ (response, error) -> () in
						if let error = error {
							print("Error uploading \(imageName): \(error)")
						}
						else {
							print("Uploaded file name: \(response?.name)")
						}

						uploads--
						if uploads == 0 {
							continuation()
						}
				})
			}
		}
	}

	func uploadMetadataJSON(client: DropboxClient,
			hud: MBProgressHUD,
			continuation: () -> ()) {

		func createMetadata() -> [String : AnyObject] {
			let device = UIDevice.currentDevice()

			return [
				"Test Name" : self.folder!,
				"Name" : device.name,
				"System Name": device.systemName,
				"System Version": device.systemVersion,
				"Model": device.model,
				"Localized Model": device.localizedModel,
				"tests count" : self.results.count
			]
		}

		func createJSONDataFromDict(dict: [String: AnyObject]) throws -> NSData {
			if NSJSONSerialization.isValidJSONObject(dict) {
				//			do {
				if let data = try? NSJSONSerialization.dataWithJSONObject(dict,
					options: .PrettyPrinted) {
						return data
				}
				else {
					throw NSError(domain: "AutoTester", code: 1, userInfo: nil)
				}
			}
			else {
				throw NSError(domain: "AutoTester", code: 2, userInfo: nil)
			}
			
		}

		hud.labelText = "Generating metadata..."

		do {
			let data = try createJSONDataFromDict(createMetadata())
			client.files.upload(
				path:"/\(self.folder!)/data.json",
				body: data).response({ (response, error) in
					if let error = error {
						print("Error uploading json file: \(error)")
					}
					else {
						print("Uploaded json file name: \(response?.name)")
						continuation()
					}
				})

		}
		catch let error {
			print("Error creating json file: \(error)")
			hud.hide(true)
		}
	}


	func createHTMLTestResult(client: DropboxClient,
			hud: MBProgressHUD,
			continuation: () -> ()) {

		hud.labelText = "Creating html file"

		var htmlString = "<!DOCTYPE html><html<head><meta charset=\"UTF-8\"><meta name=\"description\" content=\"Execution results of tests\"><link type=\"text/css\" rel=\"stylesheet\" href=\"css/styles.css\"><Title>\(self.folder!)</Title></head><body><h1>Tests Results - \(self.folder!)</h1>"

		for (i,result) in self.results.enumerate() {
			let title = self.titles[i].stringByReplacingOccurrencesOfString(" ", withString: "")
			let compImgName = result.baselineImageFile.componentsSeparatedByString("/")
			let baselineImage = compImgName[compImgName.count-1]

			htmlString += "<div id=\"tests\"><h2>TEST \(result.testName)</h2><div id=\"expected\"><h3>Expected</h3><img src=\"images/\(baselineImage)\"></div><div id=\"actual\"><h3>Actual</h3><img src=\"images/\(title)-actual.jpg\"></div></div>"
		}

		htmlString += "</body></html>"
		let data = htmlString.dataUsingEncoding(NSUTF8StringEncoding)

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
	
	func uploadCSSFile(client: DropboxClient, hud: MBProgressHUD) {
		hud.labelText = "Uploading CSS File"

		if let path = NSBundle.mainBundle().pathForResource("styles", ofType: "css"),
				data = NSData(contentsOfFile: path) {
			client.files.upload(
				path: "/\(self.folder!)/css/styles.css",
				body: data).response({ response, error in
					if let error = error {
						print("Error uploadling css file: \(error)")
					}
					else {
						print("Uploaded css file name:\(response?.name)")
						hud.hide(true)
					}
			})
		}
		else {
			print("Error getting css file")
			hud.hide(true)
		}
	}

}
