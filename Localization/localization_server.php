<?php
function get_geo_location($wifi_data){
$allData=explode('_', $wifi_data);
array_pop($allData);

foreach ($allData as $key) 
  {

  list($MAC,$Channel,$RSSI) = explode(",",$key);
  
  $intermediate = array(
   "macAddress" => $MAC,
   "signalStrength" => intval($RSSI),
   "channel" => intval($Channel) ,
  );

  $data[]=$intermediate;
 
  }
  
  $data = array("wifiAccessPoints" => $data);
  //print_r($data);

  $str_data = json_encode($data);
  $url ="https://www.googleapis.com/geolocation/v1/geolocate?key=your_key";

  $ch = curl_init($url);
  curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");  
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_POSTFIELDS,$str_data);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json')); 
  $result = curl_exec($ch);
  curl_close($ch);  // Seems like good practice
  return $result;
}


date_default_timezone_set("Asia/Riyadh");
$TAG_ID = $_GET["ID"];
$handle = fopen($TAG_ID."_History.txt", "a");

fwrite($handle,"New Session Established");
fwrite($handle, "\n");

fwrite($handle,"Recieved at Server Time: ");
fwrite($handle, date("m/d/y g.i:a", time()));
fwrite($handle, "\n");

$f = fopen($TAG_ID."_Last_Location_Time.txt","w");
fwrite($f, time());
fclose($f);

fwrite($handle,"TAG ID: ");
fwrite($handle, $TAG_ID);
fwrite($handle, "\n");

fwrite($handle,"WiFi Data Recieved: ");
fwrite($handle, $_GET["W"]);
fwrite($handle, "\n");
fwrite($handle, "\n");
fclose($handle);


$location = get_geo_location($_GET["W"]);


if (!(strpos($location,'error') !== false))
{
  

  $location = json_decode($location);
  

  $location = (array) $location;
  

  $accuracy = $location["accuracy"];
  $location = (array) $location["location"];

  $handle_2 = fopen($TAG_ID."_Lat.txt", "w");
  fwrite($handle_2, $location["lng"]);
  fclose($handle_2);

  $handle_2 = fopen($TAG_ID."_Lng.txt", "w");
  fwrite($handle_2, $location["lat"]);
  fclose($handle_2);
  
  $handle_2 = fopen($TAG_ID."_Provider.txt", "w");
  fwrite($handle_2, "GSM");
  fclose($handle_2);
  
  $handle_2 = fopen($TAG_ID."_Accuracy.txt", "w");
  fwrite($handle_2,$accuracy);
  fclose($handle_2);
}
else
{
  echo "Error";
  echo "<br />";
  echo $location;
}
?>