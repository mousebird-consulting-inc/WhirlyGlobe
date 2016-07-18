//
//  SimpleAnnotationViewController.m
//  WhirlyGlobeMicelloDemo
//
//  Created by Ranen Ghosh on 2016-04-16.
//  Copyright 2011-2016 mousebird consulting
//

#import "SimpleAnnotationViewController.h"

@interface SimpleAnnotationViewController () {
    NSString *_name, *_desc, *_hours, *_location, *_phone, *_website;
}

@end

@implementation SimpleAnnotationViewController

- (instancetype) initWithName:(NSString *)name desc:(NSString *)desc hours:(NSObject *)hours location:(NSString *)location phone:(NSString *)phone website:(NSString *)website {
    
    self = [super initWithNibName:nil bundle:nil];
    
    if (self) {
        _name = name;
        _desc = desc;
        
        if ([hours isKindOfClass:[NSString class]])
            _hours = (NSString *)hours;
        else if ([hours isKindOfClass:[NSArray class]])
            _hours = ((NSArray *)hours)[0];
        else
            _hours = @"";
        _location = location;
        _phone = phone;
        _website = website;
    }
    
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.lblName.text = _name;
    self.lblDesc.text = _desc;
    self.lblHours.text = _hours;
    self.lblLocation.text = _location;
    self.txtPhone.text = _phone;
    self.txtPhone.textContainerInset = UIEdgeInsetsMake(3, 0, 0, 0);
    self.txtWebsite.text = _website;
    self.txtWebsite.textContainerInset = UIEdgeInsetsZero;
    self.txtWebsite.textContainerInset = UIEdgeInsetsMake(3, 0, 0, 0);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
