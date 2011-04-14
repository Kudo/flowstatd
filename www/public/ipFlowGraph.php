<?php

if (!isset($_GET['ip']) || !preg_match('/^\d{1,3}\.\d{1,3}\.\d{1,3}.\d{1,3}$/', $_GET['ip']))
{
    header("Status: 500 Invalid Parameter");
    exit;
}

if (isset($_GET['date']) && !preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['date']))
{
    header("Status: 500 Invalid Parameter");
    exit;
}


include '../OFChelper/open-flash-chart.php';

$date = isset($_GET['date']) ? $_GET['date'] : NULL;
$data = getFlowData($_GET['ip'], $date);

$title = new title("IP: " . $data['ip'] . "   Time: ". $data['time']);
$title->set_style( "{font-size: 15px; text-align: center;}" );

$barObj = new bar_glass();

$localtimeAssoc = localtime(time(), true);
$currHour = $localtimeAssoc['tm_hour'];
#$xAxisLength = $currHour > 12 ? $currHour : 24;
$xAxisLength = 24;
unset($localtimeAssoc);

for ($i = 0; $i < $xAxisLength; ++$i)
{
    if ($date)
	$flowValue = $data[$i][3];
    else if ($i <= $currHour)
	$flowValue = $data[$i][3];
    else
	$flowValue = 0;

    $valList[$i] = new bar_value($flowValue);
    if ($flowValue > 5300)
	$valList[$i]->set_colour('#ff0033');
    else if ($flowValue > 5000)
	$valList[$i]->set_colour('#ffcc00');

    $tooltip = sprintf("Upload [%f]\nDownload [%f]\nUpload+Download [%f]\nTotal [%f]", $data[$i][0], $data[$i][1], $data[$i][2], $data[$i][3]);
    $valList[$i]->set_tooltip($tooltip);

    $xLabels[$i] = "$i";
}

$barObj->set_values($valList);


$y = new y_axis();
$y->set_range(0, 8000, 1000);
$y->set_steps(2000);

$x = new x_axis();
$x->set_labels_from_array($xLabels);
$x->set_steps(4);

$x_legend = new x_legend( 'Hour' );
$x_legend->set_style( '{font-size: 13px; color: #778877}' );

$y_legend = new y_legend( 'Flow (MB)' );
$y_legend->set_style( '{font-size: 13px; color: #778877}' );

$tooltip = new tooltip();
$tooltip->set_hover();

$chart = new open_flash_chart();
$chart->set_title( $title );
$chart->add_element( $barObj );
$chart->set_x_axis( $x );
$chart->set_x_legend( $x_legend );
$chart->add_y_axis( $y );
$chart->set_y_legend( $y_legend );
$chart->set_tooltip( $tooltip );
$chart->set_bg_colour('#ffffff');

function getFlowData($ip, $date = NULL)
{
    $fp = fsockopen("127.0.0.1", 9993, $errno, $errstr, 30);

    if (!$fp)
    {
	echo "$errstr ($errno)<br />\n";
	exit;
    }
    if ($date) {
	$date = str_replace('-', ' ', $date);
    	$buf = "GETR $date $ip\r\n";
    } else {
    	$buf = "GET $ip\r\n";
    }
    fwrite($fp, $buf);

    $data = '';
    while (($buf = fread($fp, 4096)))
	$data .= $buf;

    fclose($fp);

    $lines = preg_split('/\n/', $data);
    if (count($lines) < 0 || $lines[0] == "No data") {
	$retData = array(
	    'ip' => $ip,
	    'time' => $_GET['date'] ? $_GET['date'] : 'Now',
	    'totalFlow' => 0,
	);

	for ($i = 0; $i < 24; ++$i)
	{
	    $retData[$i][0] = $retData[$i][1] = $retData[$i][2] = $retData[$i][3] = 0;
	}
	return $retData;
    }

    $totalFlow = preg_split('/\s/', substr($lines[2], 10));
    $totalFlow = $totalFlow[0];

    $retData = array(
	'ip' => $ip,
	'time' => substr($lines[1], 6),
	'totalFlow' => (float) $totalFlow,
    );

    for ($i = 0; $i < 24; ++$i)
    {
	$lineList = preg_split('/\t/', $lines[$i + 6]);
	$retData[$i][0] = (float) $lineList[1];
	$retData[$i][1] = (float) $lineList[2];
	$retData[$i][2] = (float) $lineList[3];
	$retData[$i][3] = (float) $lineList[4];
    }

    return $retData;
}

?>
<script type="text/javascript" src="js/json2.js"></script>
<script type="text/javascript" src="js/swfobject.js"></script>
<script type="text/javascript">
swfobject.embedSWF("flash/open-flash-chart.swf", "flowChart", "600", "400", "9.0.0", {'loading': 'Loading...'});
</script>

<script type="text/javascript">

function open_flash_chart_data()
{
    return JSON.stringify(data);
}

function findSWF(movieName) {
  if (navigator.appName.indexOf("Microsoft")!= -1) {
    return window[movieName];
  } else {
    return document[movieName];
  }
}
    
var data = <?php echo $chart->toPrettyString(); ?>;

</script>


<div id="flowChart"></div>
