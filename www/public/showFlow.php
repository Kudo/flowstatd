<?php
if (!isset($_GET['ip']) || !preg_match('/^\d{1,3}\.\d{1,3}\.\d{1,3}.\d{1,3}$/', $_GET['ip']))
{
    header("Location: index.html");
    exit;
}
if (isset($_GET['date']) && preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['date']))
{
    $date = $_GET['date'];
}


?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<!--
Design by Free CSS Templates
http://www.freecsstemplates.org
Released for free under a Creative Commons Attribution 2.5 License

Name       : FronzenAge
Description: A two-column, fixed-width template suitable for business sites and blogs.
Version    : 1.0
Released   : 20071108

-->
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>國立中正大學宿網流量統計</title>
<meta name="keywords" content="ccudorm flow" />
<meta name="description" content="" />
<link href="css/style.css" rel="stylesheet" type="text/css" media="screen" />
<link rel="stylesheet" type="text/css" href="css/jquery-ui/jquery.ui.custom.css" />
<!--[if IE]>
<style type="text/css">
#sidebar #calendar {
	background-position: 0px 20px;
}
</style>
<![endif]-->
</head>
<body>
<div id="logo">
	<h1><a href="http://netflow.cna.ccu.edu.tw/new2/">國立中正大學宿網流量統計</a></h1>
	<h2>By <a href="http://www.cna.ccu.edu.tw/">CNA 校園網路策進會</a></h2>
</div>
<div id="menu">
	<ul>
		<li class="first"><a href="index.html" accesskey="1" title="Home">首頁</a></li>
		<li><a href="topFlow.html" accesskey="2" title="TopFlow">流量排行</a></li>
		<li><a href="#" accesskey="3" title="">Blog</a></li>
		<li><a href="#" accesskey="4" title="">About Us</a></li>
		<li><a href="#" accesskey="5" title="">Contacts</a></li>
		<li><a href="#" accesskey="6" title="">RSS</a></li>
	</ul>
	<div id="search">
		<form method="get" action="showFlow.php">
			<fieldset>
			<input id="s" type="text" name="ip" value="<?php echo $_GET['ip']; ?>" />
			<input id="x" type="image" name="imageField" src="images/img10.jpg" />
			</fieldset>
		</form>
	</div>
</div>
<hr />
<div id="banner"><img src="images/img04.jpg" alt="" width="960" height="147" /></div>
<!-- start page -->
<div id="page">
	<!-- start content -->
	<div id="content">
		<div id="loading"></div>
		<div class="post">
			<div id="flowchart">
			</div>
		</div>
	</div>
	<!-- end content -->
	<!-- start sidebar -->
	<div id="sidebar">
		<ul>
			<li id="calendar-bar">
			  <div id="calendar">
			  </div>
			</li>
		</ul>
	</div>
	<!-- end sidebar -->
</div>
<!-- end page -->
<div id="footer">
	<p class="legal">Copyright (c) 2011. All rights reserved.</p>
	<p class="credit">Web Designed by <a href="http://www.nodethirtythree.com/">NodeThirtyThree</a> + <a href="http://www.freecsstemplates.org/">Free CSS Templates</a></p>
</div>
<!-- JS -->
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery.ui.js"></script>
<script type="text/javascript">
	$(document).ready(function() {
<?php 
if (isset($date))
    echo "var g_selectedDate = '$date'\n";
else
    echo "var g_selectedDate = null\n";
?>

			function refreshFlowPage() {
				var chartObj = $("#flowchart");
				var loadingObj = $("#loading");

				chartObj.hide();
				loadingObj.show();

				if (g_selectedDate)
				    chartObj.load("ipFlowGraph.php?ip=<?php echo $_GET['ip']; ?>&date="+g_selectedDate, function(){ loadingObj.hide(); chartObj.show(); });
				else
				    chartObj.load("ipFlowGraph.php?ip=<?php echo $_GET['ip']; ?>", function(){ loadingObj.hide(); chartObj.show(); });
			};

			$("#calendar").datepicker({ 
maxDate: '+0',
dateFormat: 'yy-mm-dd',
onSelect: function(dateText, inst) {
	var selectedDate = $(this).datepicker('getDate');
	var currentDate = new Date();
	if (currentDate.toDateString() != selectedDate.toDateString()) {
		g_selectedDate = dateText;
	} else {
		g_selectedDate = null;
	}

		refreshFlowPage();
}
			});
			refreshFlowPage();
			setInterval(refreshFlowPage, 600000);
	});
</script>
</body>
</html>