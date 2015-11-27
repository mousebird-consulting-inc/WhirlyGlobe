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
		else{
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
			if Dropbox.authorizedClient != nil {
				self.loadValues()
				tableView?.reloadData()
			}
			processLogin = false
		}
	}
	
	func uploadToDropbox () {
		if let client = Dropbox.authorizedClient {
			let loadingNotification = MBProgressHUD.showHUDAddedTo(self.view, animated:true)
			loadingNotification.mode = MBProgressHUDMode.Indeterminate
			loadingNotification.labelText = "Starting"
			let date = NSDate()
			let dateFormatter = NSDateFormatter()
			dateFormatter.dateFormat = "dd-MM-yyy HH:mm"
			self.folder = dateFormatter.stringFromDate(date) as String
			self.createFolderAndUploadImages(client,  loadingNotification: loadingNotification)
		}
	}
	
	func createFolderAndUploadImages (client: DropboxClient, loadingNotification: MBProgressHUD?) -> (Void){
		loadingNotification?.labelText = "Creating folder"
		client.filesCreateFolder(path: "/\(self.folder!)").response { (response, error) -> Void in
			if response != nil {
				self.uploadImages(client, loadingNotification: loadingNotification)
			}
			else{
				print(error!)
				loadingNotification?.hide(true)
			}
		}
	}
	
	func uploadImages(client: DropboxClient, loadingNotification: MBProgressHUD?){
		loadingNotification?.labelText = "Uploading images"
		var uploads = self.results.count
		for var i = 0; i < self.results.count; ++i {
			let result = self.results[i]
			let title = titles[i]
			if  let image = result.actualImageFile {
				let data = NSData.init(contentsOfFile: image)
				let imageName = title.stringByReplacingOccurrencesOfString(" ", withString: "")
				client.filesUpload(path: "/\(self.folder!)/images/\(imageName).png", body: data!).response({ (response, error) -> Void in
					if let file = response {
						print("Uploaded file name:\(file.name)")
					}
					else{
						print(error!)
					}
					uploads--
					if uploads == 0 {
						self.createJson(client, loadingNotification: loadingNotification)
					}
				})
			}
		}
	}

	func createJson(client: DropboxClient, loadingNotification: MBProgressHUD?) {
		loadingNotification?.labelText = "Generating json"
		let device = UIDevice.currentDevice()
		let jsonObject: [String: AnyObject] = [
			"Test Name" : self.folder!,
			"Name" : device.name,
			"System Name": device.systemName,
			"System Version": device.systemVersion,
			"Model": device.model,
			"Localized Model": device.localizedModel,
			"tests count" : self.results.count
		]
		let valid = NSJSONSerialization.isValidJSONObject(jsonObject)
		if valid {
			do {
				let data :NSData? = try NSJSONSerialization.dataWithJSONObject(jsonObject, options: .PrettyPrinted)
				client.filesUpload(path:"/\(self.folder!)/data.json", body: data!).response({ (response, error) -> Void in
					if  let file = response {
						print("Uploaded json file name:\(file.name)")
					}
					else{
						print("Error creating json file")
					}
					self.createHTMLTestResult(client, loadingNotification: loadingNotification)
				})
			}
			catch let myJSONError  {
				print(myJSONError)
				loadingNotification?.hide(true)
			}
		}
		else {
			loadingNotification?.hide(true)
			print("JSON not valid")
		}
		
	}
	
	func createHTMLTestResult (client: DropboxClient, loadingNotification: MBProgressHUD?){
		
		var htmlString : String?
		loadingNotification?.labelText = "Creating html file"
		htmlString = "<!DOCTYPE html><html<head><meta charset=\"UTF-8\"><meta name=\"description\" content=\"Execution results of tests\"><link type=\"text/css\" rel=\"stylesheet\" href=\"css/styles.css\"><Title>\(self.folder!)</Title></head><body><h1>Tests Results - \(self.folder!)</h1>"
		for var i = 0; i < self.results.count; ++i {
			let result = self.results[i]
			let compImgName = result.baselineImageFile.componentsSeparatedByString("/")
			let baselineImage = compImgName[compImgName.count-1]
			htmlString = htmlString! + "<div id=\"tests\"><h2>TEST \(result.testName)</h2><div id=\"expected\"><h3>Expected</h3><img src=\"images/\(baselineImage)\"></div><div id=\"actual\"><h3>Actual</h3><img src=\"images/\(self.titles[i].stringByReplacingOccurrencesOfString(" ", withString: "")).png\"></div></div>"
		}
		htmlString = htmlString! + "</body></html>"
		let data = htmlString?.dataUsingEncoding(NSUTF8StringEncoding)
		client.filesUpload(path: "/\(self.folder!)/index.html", body: data!).response { (response, error) -> Void in
			if let file = response{
				print("Uploaded html file name:\(file.name)")
				self.uploadCSSFile(client, loadingNotification: loadingNotification)
			}
			else {
				print("Error creating html file")
				loadingNotification?.hide(true)
			}
		}
	}
	
	func uploadCSSFile(client: DropboxClient, loadingNotification: MBProgressHUD?) {
		loadingNotification?.labelText = "Uploading CSS File"
		let path = NSBundle.mainBundle().pathForResource("styles", ofType: "css")
		if let path = path {
			let data = NSData(contentsOfFile: path)
			client.filesUpload(path: "/\(self.folder!)/css/styles.css", body: data!).response({ (response, error) -> Void in
				if let file = response{
					print("Uploaded css file name:\(file.name)")
				}
				else{
					print("Error uploadling css file")
				}
				loadingNotification?.hide(true)
			})
		}
		else{
			print("Error uploading css file")
			loadingNotification?.hide(true)
		}
	}

}
