//
//  FullScreenViewController.swift
//  AutoTester
//
//  Created by jmnavarro on 23/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

import UIKit


class FullScreenViewController : UIViewController {
    
    @IBOutlet weak var imageResult: UIImageView!
    
    var image = UIImage()
    
    override func viewDidLoad() {
        dispatch_async(dispatch_get_main_queue()){
            self.imageResult?.image = self.image
        }
    }
    
}
