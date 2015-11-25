//
//  AppDelegate.swift
//  HelloEarthSwift
//
//  Created by jmnavarro on 18/08/15.
//  Copyright (c) 2015 Mousebird. All rights reserved.
//

import UIKit
import SwiftyDropbox

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

	var window: UIWindow?


	func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject: AnyObject]?) -> Bool {

		let path = NSBundle.mainBundle().pathForResource("Info", ofType: "plist")
		if let path = path {
			let dictionary = NSDictionary.init(contentsOfFile: path)
			if let dictionary = dictionary {
				let key = dictionary.valueForKey("Dropbox Key")
				if let key = key {
					Dropbox.setupWithAppKey(key as! String)
				}
			}
		}

		return true
	}

	func application(app: UIApplication, openURL url: NSURL, options: [String : AnyObject]) -> Bool {

		if let authResult = Dropbox.handleRedirectURL(url) {
			switch authResult {
			case .Success(let token):
				print("Success! User is logged into Dropbox with token: \(token)")
			case .Error(let error, let description):
				print("Error \(error): \(description)")
			}
		}

		return false
	}

	func applicationWillResignActive(application: UIApplication) {
		// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
		// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
	}

	func applicationDidEnterBackground(application: UIApplication) {
		// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
		// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
	}

	func applicationWillEnterForeground(application: UIApplication) {
		// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
	}

	func applicationDidBecomeActive(application: UIApplication) {
		// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
	}

	func applicationWillTerminate(application: UIApplication) {
		// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
	}

}

