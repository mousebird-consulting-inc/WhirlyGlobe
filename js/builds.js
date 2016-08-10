var url = 'http://mousebird-home.asuscomm.com:8080/api/json?depth=2&pretty=true&tree=jobs[name,lastBuild[number,duration,timestamp,result,estimatedDuration]]'
var s3Url = "https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/";

function getAPIData() {
	$.getJSON(url,function(json) { 
		var items = [];
		$.each (json, function (x1, y1) {
			if (x1 == "jobs") {
				var table = "<div class='table-responsive builds text-center'><table class='table table-bordered'><thead><tr><th>Name</th><th>Status</th><th>Last Duration</th><th>Build Count</th><th>Last Result</th><th>Last Run Date</th><th>Last Binary</th><th>Others Binaries</th></tr></thead><tbody>";
				$.each (y1, function(x2, y2) {
					table = table + convertJobToHTMLRow(y2);
				});
				table = table +"</tbody></table></div>";
				document.getElementById("table").innerHTML = table;
			}
		});
	}).error(function() {
		document.getElementById("table").innerHTML = "<div class='alert alert-danger'><strong>Something bad happened...</strong> Mr. Jenkins seems to be too busy... try again later</div>";
	});
}

function timeConverter(UNIX_timestamp) {
  var a = new Date(UNIX_timestamp);

  var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
  var year = a.getFullYear();
  var month = months[a.getMonth()];
  var date = a.getDate();
  var hour = a.getHours();
  if (hour < 10) {
  	hour = "0"+hour;
  }
  var min = a.getMinutes();
  if (min < 10) {
  	min = "0"+min;
  }
  var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min;
  return time;
}

function getDuration(seconds) {
	var sec_num = parseInt(seconds, 10); // don't forget the second param
    var hours   = Math.floor(sec_num / 3600);
    var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
    var seconds = sec_num - (hours * 3600) - (minutes * 60);

    var result = "";
    if (hours > 0) {
    	if (minutes < 10) { minutes = "0"+minutes; }
    	result = hours + ":" + minutes + " hours";
    }
    else if (minutes > 0) {
    	if (seconds < 10) { seconds = "0"+seconds; }
    	result = minutes + ":" + seconds + " minutes";
    }
    else if (seconds > 0) {
    	result = seconds + " seconds";
    }
    return result;
}

function convertJobToHTMLRow(job) {
	var data = getArrayDataJob(job);
	var html = "<tr>";
	for (var i = 0; i < data.length; i++) {
		switch (i) {
			case 0:
			case 1:
			case 4:
			case 5:
				html = html + "<td><p>" + data[i] + "</p></td>";
				break;
			case 2:
				if (data[i] != "") {
					html = html + "<td><p>" + getDuration(data[i]/1000) + "</p></td>";
				}
				else {
					html = html +"<td></td>";
				}
				break;
			case 3:
				break;
			case 6:
				if (data[i] != "") {
					html = html + "<td><p>" + timeConverter(data[i]) + "</p></td>";
				}
				else {
					html = html +"<td></td>";
				}
				break;
			default:
				break;
		}
	}
	var fields = data[0].split('_');
	var binaries = getS3Data(s3Url, data[0], fields[1]);
	if (binaries.length > 0) {
		var lastBinary = binaries[binaries.length-1];
		var binaryName = lastBinary.split("/");
		html = html + "<td><a href="+s3Url+lastBinary+">"+binaryName[1]+"</a></td><td>"
		if (binaries.length > 1) {
			html = html + "<button type='button' class='btn btn-info btn-md' data-toggle='modal' data-target='#"+data[0]+"'>See More</button>";
			html = html + "<div id='"+data[0]+"' class='modal fade' role='dialog'><div class='modal-dialog'>";
			html = html + "<div class='modal-content'><div class='modal-header tutorial-main'><button type='button' class='close' data-dismiss='modal'>&times;</button><h2>Binaries</h2></div>";
			html = html + "<div class='modal-body tutorial tutorial-main'>";
			for (var i = binaries.length-2; i >= 0 ; i--){
				binaryName = binaries[i].split("/");
				html = html + "<p><a href="+s3Url+binaryName[1]+">"+binaryName[1]+"</a></p>"
			}
			html = html+"</div><div class='modal-footer'><button type='button' class='btn btn-default' data-dismiss='modal'>Close</button></div></div></div></div></td>";
		}
		else {
			html = html+"None</td>";
		}
	}
	else {
		html = html + "<td>None</td><td>None</td>";
	}

	html = html + "</tr>";
	return html;
}

function getArrayDataJob(job){

	/**
		Position:Description
		0: Name
		1: Status
		2: Last Duration
		3: Estimated Duration
		4: Execution Number
		5: Last Build Result
		6: Last Execution Date
	*/
	var data = [];

	for (var i = 0; i < 7; i++) {
		data.push("");
	}


	$.each(job, function(x1, y1) {
		switch (x1) {
			case "name":
				data[0] = y1;
				break;
			case "lastBuild":
				if (y1 != null && y1 != undefined) {
					if (y1['building'] == "true" || y1['building'] == true) {
						data[1] = "Running...";
					}
					else {
						data[1] = "Idle";
						data[2] = y1['duration'];
						data[5] = y1['result'];
					}
					
					data[3] = y1['estimatedDuration'];
					data[4] = y1['number'];
					data[6] = y1['timestamp'];
 				}
				break;
			default:
				break;
		}
	});
	return data;
}

function getS3Data(html, testName, platform)
{
	var binaries = [];
	jQuery.ajax({
		type:"GET",
		url: html,
		async: false,
		dataType: "xml",
		success: function(dataXML) {
			$(dataXML).find('Contents').each(function() {
				var key = $(this).find('Key').text();
				if (key.toLowerCase().includes(platform)) {
					if (key.toLowerCase().includes("nightly")) {
						if (testName.toLowerCase().includes("nightly")) {
							binaries.push(key);
						}
					}
					else {
						binaries.push(key);
					}
				}
			});
		},
		error : function(error) {
		}
	});
	binaries.sort();
	return binaries;
}

