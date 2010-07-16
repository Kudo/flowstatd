<?php

if (!isset($_GET['ip']))// || !preg_match('/\d{,3}\.\d{,3}\.\d{,3}.\d{,3}/', $_GET[ip]))
{
    echo "Wrong parameter!";
    exit;
}

include '../openFlashChart/php-ofc-library/open-flash-chart.php';

$data = getFlowData($_GET[ip]);

$title = new title("IP: " . $data['ip'] . "   Time: ". $data['time']);
$title->set_style( "{font-size: 15px; text-align: center;}" );

$bar_stack = new bar_stack();
$bar_stack->set_colours( array( '#3333ff', '#ffcc00', '#ff0033' ) );

$localtimeAssoc = localtime(time(), true);
$currHour = $localtimeAssoc['tm_hour'];
unset($localtimeAssoc);

for ($i = 0; $i < 24; ++$i)
{
    if ($data[$i][3] > 5300)
	$valList = array(5000, 300, $data[$i][3] - 5300);
    else if ($data[$i][3] > 5000)
	$valList = array(5000, $data[$i][3] - 5000, 0);
    else
	$valList = array($data[$i][3], 0, 0);

    if ($i > $currHour) 
    	$bar_stack->append_stack(array(0, 0));
    else
	$bar_stack->append_stack($valList);

    $xLabels[$i] = "$i";
}

$bar_stack->set_keys( 
    array(
	new bar_stack_key('#3333ff', 'Normal', 13),
	new bar_stack_key('#ffcc00', 'Warning', 13),
	new bar_stack_key('#ff0033', 'Caution', 13),
));

$bar_stack->set_tooltip('Hour #x_label#, Flow [#val#]<br />Total [#total#]');

$y = new y_axis();
$y->set_range(0, 8000, 1000);

$x = new x_axis();
$x->set_labels_from_array($xLabels);

$x_legend = new x_legend( 'Hour' );
$x_legend->set_style( '{font-size: 13px; color: #778877}' );

$y_legend = new y_legend( 'Flow (MB)' );
$y_legend->set_style( '{font-size: 13px; color: #778877}' );

$tooltip = new tooltip();
$tooltip->set_hover();

$chart = new open_flash_chart();
$chart->set_title( $title );
$chart->add_element( $bar_stack );
$chart->set_x_axis( $x );
$chart->set_x_legend( $x_legend );
$chart->add_y_axis( $y );
$chart->set_y_legend( $y_legend );
$chart->set_tooltip( $tooltip );

function getFlowData($ip)
{
    $fp = fsockopen("127.0.0.1", 9993, $errno, $errstr, 30);

    if (!$fp)
    {
	echo "$errstr ($errno)<br />\n";
	exit;
    }
    $buf = "GET $_GET[ip]\r\n";
    fwrite($fp, $buf);

    $data = '';
    while (($buf = fread($fp, 4096)))
	$data .= $buf;

    fclose($fp);

    $lines = preg_split('/\n/', $data);

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
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>

<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script type="text/javascript" src="static/js/json/json2.js"></script>
<script type="text/javascript" src="static/js/swfobject.js"></script>
<script type="text/javascript">
swfobject.embedSWF("open-flash-chart.swf", "flowChart", "600", "400", "9.0.0", {'loading': 'Loading...'});
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


</head>
<body>

<div id="flowChart"></div>

</body>
</html>
