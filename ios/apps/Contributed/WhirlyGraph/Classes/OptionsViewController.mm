
/*
 File: OptionsViewController.m
 Abstract: View controller that sets up the table view and serves as the table view's data source and delegate.
 Version: 2.1
 
 */

#import "OptionsViewController.h"
#import "sqlhelpers.h"

@implementation DBWrapper

static const char * const kQueryDataSetNames = "SELECT variable_name FROM data_sets ORDER BY `variable_name`;";

static const NSString * const kQueryFindDataSets = @"SELECT variable_name FROM data_sets WHERE (`variable_name` LIKE '%##SEARCH_TERM##%') ORDER BY `variable_name`;";


static const char * const kQueryJoin = "SELECT * FROM `measurements` INNER JOIN `nations` ON (`nations`.`id` = `measurements`.`nation_id`) INNER JOIN `data_sets` ON (`data_sets`.`id` = `measurements`.`data_set_id`)";



static const NSString * const kQueryFilterDataSetName = @"SELECT ##SELECT## FROM `measurements` INNER JOIN `nations` ON (`nations`.`id` = `measurements`.`nation_id`) INNER JOIN `data_sets` ON (`data_sets`.`id` = `measurements`.`data_set_id`) WHERE (`data_sets`.`variable_name` = '##NAME##') and `measurement` not null;";

static const NSString * const kQueryFilterDataSetAndCountry = @"SELECT `measurement` FROM `measurements` INNER JOIN `nations` ON (`nations`.`id` = `measurements`.`nation_id`) INNER JOIN `data_sets` ON (`data_sets`.`id` = `measurements`.`data_set_id`) WHERE (`data_sets`.`variable_name` = '##DATASET##') AND (`nations`.`iso3` = '##ISO3##') and measurement not null;";



// SELECT * FROM `measurements` INNER JOIN `nations` ON (`nations`.`id` = `measurements`.`nation_id`) INNER JOIN `data_sets` ON (`data_sets`.`id` = `measurements`.`data_set_id`) WHERE (`variable_name` = 'Population - Aged 0 - 14')"

// WHERE (`variable_name` = 'Population - Aged 0 - 14')

- (NSString *)queryWithSelection:(NSString *)selection name:(NSString *)name
{
    NSMutableString *s = [kQueryFilterDataSetName mutableCopy];
    [s replaceOccurrencesOfString:@"##SELECT##" withString:selection options:NSLiteralSearch range:NSMakeRange(0, [s length])];
    [s replaceOccurrencesOfString:@"##NAME##" withString:name options:NSLiteralSearch range:NSMakeRange(0, [s length])];
    
//    NSLog(@"%@", s);
    
    return (NSString *)[s autorelease];
}

- (NSString *)queryWithDataSetName:(NSString *)dataSetName country:(NSString *)iso3
{
    NSMutableString *s = [kQueryFilterDataSetAndCountry mutableCopy];
    [s replaceOccurrencesOfString:@"##DATASET##" withString:dataSetName options:NSLiteralSearch range:NSMakeRange(0, [s length])];
    [s replaceOccurrencesOfString:@"##ISO3##" withString:iso3 options:NSLiteralSearch range:NSMakeRange(0, [s length])];
    return (NSString *)[s autorelease];
}

- (NSString *)queryForDataSetsContainingString:(NSString *)searchString
{
	NSMutableString *s = [kQueryFindDataSets mutableCopy];
    [s replaceOccurrencesOfString:@"##SEARCH_TERM##" withString:searchString options:NSLiteralSearch range:NSMakeRange(0, [s length])];
    return (NSString *)[s autorelease];
}

- (NSString *)dataSetDescription:(NSString *)dataSetName
{
    NSString *query = [NSString stringWithFormat:@"select units,une_field_name from data_sets where variable_name = '%@';",dataSetName];
//    NSLog(@"%@",query);
    sqlhelpers::StatementRead readStmt(_db, query);
    NSString *units = nil, *date = nil;
    if (readStmt.stepRow())
    {
        units = readStmt.getString();
        date = readStmt.getString();
    }
    
    if (units && date)
        return [NSString stringWithFormat:@"%@ (%@) - %@",dataSetName,units,date];
    
    return nil;
}

- (BOOL)open
{
    NSString *dbString = [[NSBundle mainBundle] pathForResource:@"une" ofType:@"sqlite3"];
    BOOL success = (sqlite3_open([dbString cStringUsingEncoding:1],&_db) == SQLITE_OK);
    
    if ( !success )
    {
        NSLog(@"Warning: SQLITE problem");
    }
    
    return success;        
}

- (NSArray *)dataSetNames
{
    NSMutableArray *names = [[NSMutableArray alloc] initWithCapacity:400];
    
    sqlhelpers::StatementRead readStmt(_db, kQueryDataSetNames);
    while ( readStmt.stepRow() )
    {
        [names addObject:readStmt.getString()];
    }
    
    [names addObject:@"None"];
    
//    NSLog(@"names = %@", names);
    
    return (NSArray *)[names autorelease];
}

- (NSArray *)findDataSetsContainingString:(NSString *)queryString
{
	NSMutableArray *results = [[NSMutableArray alloc] initWithCapacity:400];
	
	NSString *query = [self queryForDataSetsContainingString:queryString];
    
    sqlhelpers::StatementRead readStmt(_db, query);
    while ( readStmt.stepRow() )
    {
        [results addObject:readStmt.getString()];
    }
    
    [results addObject:@"None"];
    
    return (NSArray *)[results autorelease];
}

- (NSNumber *)max:(NSString *)dataSetName
{
    
    NSString *query = [self queryWithSelection:@"max(`measurement`)" name:dataSetName];
    //NSString *query = MAX_DATASET_VALUE_QUERY(dataSetName);
    
    // query = @"SELECT max(measurement) FROM `measurements` INNER JOIN `nations` ON (`nations`.`id` = `measurements`.`nation_id`) INNER JOIN `data_sets` ON (`data_sets`.`id` = `measurements`.`data_set_id`) WHERE (`variable_name` = 'Population - Aged 0 - 14');";
    
//    NSLog(@"query:\n%@\n", query);
    
    sqlhelpers::StatementRead readStmt(_db, query);
    NSNumber *v = nil;
    if ( readStmt.stepRow() )
        v = [NSNumber numberWithFloat:(float)readStmt.getDouble()];
    
    return v;
}

- (NSNumber *)min:(NSString *)dataSetName
{
    NSString *query = [self queryWithSelection:@"min(`measurement`)" name:dataSetName];
//    NSLog(@"query:\n%@\n", query);

    sqlhelpers::StatementRead readStmt(_db, query);
    
    NSNumber *v = nil;
    if ( readStmt.stepRow() )
        v = [NSNumber numberWithFloat:(float)readStmt.getDouble()];

    return v;
}

- (NSNumber *)valueForDataSetName:(NSString *)dataSetName country:(NSString *)iso3Code
{
    NSString *query = [self queryWithDataSetName:dataSetName country:iso3Code];
 
//    NSLog(@"query:\n%@\n", query);
    
    sqlhelpers::StatementRead readStmt(_db, query);

    NSNumber *v = nil;
    if ( readStmt.stepRow() )
        v = [NSNumber numberWithFloat:(float)readStmt.getDouble()];

    return v;
}


@end

@implementation OptionsViewController

@synthesize dataSetName = _dataSetName;
@synthesize arrayOfStrings;
@synthesize delegate;

@synthesize tableView = _tableView;
@synthesize searchBar = _searchBar;
@synthesize searchController = _searchController;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if ( self )
	{
		if ( ! _db )
		{
			_db = [[DBWrapper alloc] init];
			[_db open];
		}
		
		// BRC: todo - add sort clause to query params
		NSMutableArray *strArray = [[NSMutableArray alloc] initWithArray:[_db dataSetNames]];
		[strArray sortUsingSelector:@selector(compare:)];
		arrayOfStrings = strArray;
		
		_searchResults = [[NSMutableArray alloc] initWithCapacity:400];
        
        firstAppear = true;
	}
	return self;
}

- (void)dealloc {
	delegate = nil;
	[arrayOfStrings release]; arrayOfStrings = nil;
	[_searchResults release]; _searchResults = nil;
	
	_tableView.delegate = nil;
	_tableView.dataSource = nil;
	[_tableView release]; _tableView = nil;
	
	_searchBar.delegate	= nil;
	[_searchBar release]; _searchBar = nil;
	
	_searchController.delegate = nil;
	_searchController.searchResultsDataSource = nil;
	_searchController.searchResultsDelegate = nil;
	[_searchController release]; _searchController = nil;
	[super dealloc];
}


#pragma mark view lifecycle

- (void)loadView
{
	CGRect frame = CGRectMake(0, 0, 320.f, 480.f);
	UIView *v = [[UIView alloc] initWithFrame:frame];
	v.autoresizingMask = UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleLeftMargin|UIViewAutoresizingFlexibleRightMargin|UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleBottomMargin;
	v.autoresizesSubviews = YES;
	
	_searchBar = [[UISearchBar alloc] initWithFrame:CGRectMake(0, 0, CGRectGetWidth(_tableView.bounds), 0)];
	_searchBar.autoresizingMask = UIViewAutoresizingFlexibleWidth;
	_searchBar.delegate = self;
	
	_tableView = [[UITableView alloc] initWithFrame:v.bounds style:UITableViewStylePlain];
	_tableView.autoresizingMask	= UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth;
	_tableView.delegate = self;
	_tableView.dataSource = self;
	
	// Add searchBar to tableHeaderView
	[_searchBar sizeToFit];
	_tableView.tableHeaderView = _searchBar;

	// Hide searchBar
	UIEdgeInsets contentInset = _tableView.contentInset;
	contentInset.top = -CGRectGetHeight(_searchBar.bounds);
	_tableView.contentInset = contentInset;

	[v addSubview:_tableView];
	self.view = v;
}

- (void)viewDidUnload
{
	_tableView.delegate = nil;
	_tableView.dataSource = nil;
	[_tableView release]; _tableView = nil;
	
	_searchBar.delegate	= nil;
	[_searchBar release]; _searchBar = nil;
	
	_searchController.delegate = nil;
	_searchController.searchResultsDataSource = nil;
	_searchController.searchResultsDelegate = nil;
	[_searchController release]; _searchController = nil;
}

- (void)viewDidLoad {
	// initialize search display controller
	_searchController = [[UISearchDisplayController alloc] initWithSearchBar:_searchBar contentsController:self];
	_searchController.delegate = self;
	_searchController.searchResultsDataSource = self;
	_searchController.searchResultsDelegate = self;	
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	
	// set initial scroll position such that searchBar is hidden.
    if (firstAppear)
    {
        self.tableView.contentOffset = CGPointMake(0, CGRectGetHeight(_searchBar.bounds));
        firstAppear = false;
    }
}

- (NSDictionary *)getResult
{
    return nil;
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	// There is only one section.
	return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
	
	NSUInteger count = 0;
	if ( tableView == _tableView )
	{
		// standard data table
		count = arrayOfStrings.count;
	}
	else if ( tableView == _searchController.searchResultsTableView )
	{
		count = _searchResults.count;
	}
	
	return count;
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
	if ( !_searchBarIsShowing && (_tableView.contentOffset.y < 0) )
	{
		// reveal searchBar
		UIEdgeInsets contentInset = _tableView.contentInset;
		contentInset.top = 0;
		_tableView.contentInset = contentInset;
		_searchBarIsShowing = YES;
	}
}

#pragma mark -
#pragma mark Table view selection

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath { 
	
	static UIColor * evenColor = nil;
	static UIColor * oddColor = nil;
	
	if ( !evenColor )
	{
		// [UIColor colorWithRed:0.7 green:0.7 blue:0.7 alpha:0.9]
		// premultiplied alpha:
		// 0.63, 0.63, 0.63, 1.0
		evenColor = [[UIColor colorWithRed:0.63 green:0.63 blue:0.63 alpha:1.0] retain];
	}
	
	if ( !oddColor )
	{
		// [UIColor colorWithRed:0.7 green:0.8 blue:0.7 alpha:0.9]
		// premultiplied alpha:
		// 0.63, 0.72, 0.63, 1.0
		oddColor = [[UIColor colorWithRed:0.63 green:0.72 blue:0.63 alpha:1.0] retain];
	}
	
    if((indexPath.row + (indexPath.section % 2))% 2 == 0){  
        cell.backgroundColor = evenColor;
		cell.textLabel.backgroundColor = evenColor;
		cell.detailTextLabel.backgroundColor = evenColor;
    } else {
        cell.backgroundColor = oddColor;
		cell.textLabel.backgroundColor = oddColor;
		cell.detailTextLabel.backgroundColor = oddColor;
    }
}  


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
    static NSString *MyIdentifier = @"MyIdentifier";
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:MyIdentifier];
	if (cell == nil) {
		// Use the default cell style.
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:MyIdentifier] autorelease];
		cell.textLabel.opaque = YES;
		cell.detailTextLabel.opaque = YES;
		cell.detailTextLabel.textColor = [UIColor colorWithWhite:0.3f alpha:1.0f];
		cell.backgroundView.opaque = YES;
	}
	
	NSString *text;
	
	if ( tableView == _tableView )
	{
		// standard data table
		text = [arrayOfStrings objectAtIndex:indexPath.row];
	}
	else if ( tableView == _searchController.searchResultsTableView )
	{
		text = [_searchResults objectAtIndex:indexPath.row];
	}
	
	NSRange detail = [text rangeOfCharacterFromSet:[NSCharacterSet punctuationCharacterSet]];
	
    cell.detailTextLabel.text = nil;
	if ( detail.location != NSNotFound )
	{
		cell.textLabel.text = [text substringToIndex:detail.location];
		
		NSUInteger detailStart = detail.location;
		NSRange detailTextRange = NSMakeRange(detailStart, text.length - detailStart);
		cell.detailTextLabel.text = [text substringWithRange:detailTextRange];
	}
	else
	{
		cell.textLabel.text = text;
	}
	
	return cell;
}

/*
 To conform to Human Interface Guildelines, since selecting a row would have no effect (such as navigation), make sure that rows cannot be selected.
 */
- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
	NSString *s = nil;
	
	if ( tableView == _tableView )
	{
		// standard data table
		s = [arrayOfStrings objectAtIndex:indexPath.row];
	}
	else if ( tableView == _searchController.searchResultsTableView )
	{
		s = [_searchResults objectAtIndex:indexPath.row];
	}
    
    self.dataSetName = s;
    
//    NSLog(@"Selected:  %@", self.dataSetName);
//    NSLog(@"  max: %f", [_db max:self.dataSetName]);
//    
//    NSLog(@"  v: %f", [_db valueForDataSetName:self.dataSetName country:@"USA"]);
    
    [delegate didTap:self.dataSetName desc:[_db dataSetDescription:self.dataSetName]];
        
	return nil;
}

#pragma mark searchBar delegate

- (void)searchBarTextDidBeginEditing:(UISearchBar *)searchBar
{
	searchBar.showsCancelButton = YES;
}

- (void)searchBarTextDidEndEditing:(UISearchBar *)searchBar
{
	searchBar.showsCancelButton = NO;
}

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText
{	
	if ( searchText.length == 0 )
	{
		[_searchResults removeAllObjects];
	}
	else
	{
		NSArray *results = [_db findDataSetsContainingString:searchText];
		[_searchResults setArray:results];
	}
	[self.tableView reloadData];
	
}

- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar
{
	[_searchResults removeAllObjects];
	[searchBar resignFirstResponder];
}

@end
