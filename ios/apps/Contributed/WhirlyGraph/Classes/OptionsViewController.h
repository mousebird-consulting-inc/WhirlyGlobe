
/*
     File: OptionsViewController.h
 Abstract: View controller that sets up the table view and serves as the table view's data source and delegate.
  Version: 2.1
 
 
 */

#import <UIKit/UIKit.h>
#import "sqlite3.h"

@interface DBWrapper : NSObject {
@private
    sqlite3* _db;
}
- (NSString *)queryWithSelection:(NSString *)selection name:(NSString *)name;
- (NSString *)queryWithSelection:(NSString *)selection name:(NSString *)name;
- (BOOL)open;
- (NSArray *)dataSetNames;
- (NSArray *)findDataSetsContainingString:(NSString *)queryString;
- (NSNumber *)max:(NSString *)dataSetName;
- (NSNumber *)min:(NSString *)dataSetName;
- (NSNumber *)valueForDataSetName:(NSString *)dataSetName country:(NSString *)iso3Code;
- (NSString *)dataSetDescription:(NSString *)dataSetName;
@end

@protocol OptionsViewControllerDelegate <NSObject>

-(void)didTap:(NSString *)queryString desc:(NSString *)dataSetDesc;

@end

@class DBWrapper;

@interface OptionsViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, UISearchBarDelegate, UISearchDisplayDelegate>
{    
    DBWrapper* _db;
    NSString* _dataSetName;
	
	UITableView* _tableView;
	UISearchBar* _searchBar;
	BOOL _searchBarIsShowing;
	
	UISearchDisplayController* _searchController;
	NSMutableArray* _searchResults;
    
    NSArray *arrayOfStrings;
    id delegate;
    bool firstAppear;
}

@property (nonatomic, copy) NSString * dataSetName;

@property (nonatomic, retain) UITableView *tableView;
@property (nonatomic, retain) UISearchBar *searchBar;
@property (nonatomic, retain) UISearchDisplayController *searchController;

@property (nonatomic, retain) NSArray *arrayOfStrings;
@property (nonatomic, assign) id<OptionsViewControllerDelegate> delegate;

@end