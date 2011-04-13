<?php

if (!isset($_GET['topN']) || !preg_match('/^\d+$/', $_GET['topN']))
{
    header("Status: 500 Invalid Parameter");
    exit;
}

if (isset($_GET['date']) && !preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['date']))
{
    header("Status: 500 Invalid Parameter");
    exit;
}


function getFlowData($topN, $date = NULL)
{
    $fp = fsockopen("127.0.0.1", 9993, $errno, $errstr, 30);

    if (!$fp)
    {
	echo "$errstr ($errno)<br />\n";
	exit;
    }
    if ($date) {
	$date = str_replace('-', ' ', $date);
    	$buf = "TOPR $date $topN\r\n";
    } else {
    	$buf = "TOP $topN\r\n";
    }
    fwrite($fp, $buf);

    $data = '';
    while (($buf = fread($fp, 4096)))
	$data .= $buf;

    fclose($fp);

    $retData = array();
    $lines = preg_split('/\n/', $data);
    if (count($lines) < 2) {
	return $retData;
    }

    $nLines = count($lines) - 4;
    for ($i = 0; $i < $nLines; ++$i)
    {
	$lineList = preg_split('/\t/', $lines[$i + 3]);
	$retData[$i][0] = trim($lineList[1]);
	$retData[$i][1] = (float) $lineList[2];
	$retData[$i][2] = (float) $lineList[3];
	$retData[$i][3] = (float) $lineList[4];
    }

    return $retData;
}

?>

<table id="ipFlowTable">
  <thead>
		<tr>
			<th>No.</th>
			<th>IP</th>
			<th>總流量 (MB)</th>
		</tr>
  </thead>

  <tbody>
<?php
$topN = isset($_GET['topN']) ? $_GET['topN'] : 5;
$date = isset($_GET['date']) ? $_GET['date'] : NULL;
$data = getFlowData($topN, $date);
$nData = count($data);
for ($i = 0; $i < $nData; ++$i) {
    if ($data[$i][3] > 5300)
	$classStr = ' class="ui-state-error"';
    else if ($data[$i][3] > 5000)
	$classStr = ' class="ui-state-highlight"';
    else
	$classStr = '';

    if ($date)
	$dateStr = "&date=$date";
    else
	$dateStr = '';

    printf('<tr><td%s>%d</td><td%s><a href="showFlow.php?ip=%s%s">%s</a></td><td%s>%f</td></tr>'."\n",
	$classStr, ($i + 1), $classStr, $data[$i][0], $dateStr, $data[$i][0], $classStr, $data[$i][3]);
}
?>
  </tbody>
</table>
