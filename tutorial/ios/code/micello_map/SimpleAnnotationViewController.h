//
//  SimpleAnnotationViewController.h
//  WhirlyGlobeMicelloDemo
//
//  Created by Ranen Ghosh on 2016-04-16.
//  Copyright 2011-2016 mousebird consulting
//

#import <UIKit/UIKit.h>

@interface SimpleAnnotationViewController : UIViewController

- (instancetype) initWithName:(NSString *)name desc:(NSString *)desc hours:(NSObject *)hours location:(NSString *)location phone:(NSString *)phone website:(NSString *)website;

@property (nonatomic, strong) IBOutlet UILabel *lblName;
@property (nonatomic, strong) IBOutlet UILabel *lblDesc;
@property (nonatomic, strong) IBOutlet UILabel *lblHours;
@property (nonatomic, strong) IBOutlet UILabel *lblLocation;
@property (nonatomic, strong) IBOutlet UITextView *txtPhone;
@property (nonatomic, strong) IBOutlet UITextView *txtWebsite;

@end
