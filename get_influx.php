<?php

  require __DIR__ . '/vendor/autoload.php';

  
  use InfluxDB2\Client;
 
  // Set up the InfluxDB2 client options
  $url =  "http://localhost:8086";
  $organization = 'Home';
  $bucket = 'team11';
  $token = "WZ-CODMlBEqgSRmMh3euxTLi36W7B5BEkOqM6tYooryh6IiNop-wf-yXEpdBHjq-3CHSZ6eUbswm6Bk8BbYXHA==";

  $client = new Client([
      "url" => $url,
      "token" => $token,
      "bucket" => $bucket,
      "org" => $organization,
      "precision" => InfluxDB2\Model\WritePrecision::NS,
      "debug" => false
  ]);
  
   // Create a new QueryApi instance
  $queryApi = $client->createQueryApi();
    
  // Construct a Flux query to retrieve data from the "doorClosed" measurement
/*   $fluxQuery = 'from(bucket:"team11") '
      . '|> range(start: -1m)'
      . '|> filter(fn: (r) => r._field == "doorClosed")'; */

  $fluxQuery = 'from(bucket:"team11") '
        . '|> range(start: -1m)'
        . '|> filter(fn: (r) => r["_measurement"] == "mqtt_consumer")'
        . '|> filter(fn: (r) => r["_field"] == "DoorHold" or r["_field"] == "doorClosed")';

  // Use the QueryApi instance to execute the query
  $result = $queryApi->query($fluxQuery);
 
  $records = [];
  foreach ($result as $result) {
    foreach ($result->records as $record) {
        // because we will have multiple fields at the same second in time, we need to merge the data into a single array after we query it out
        $row = key_exists($record->getTime(), $records) ? $records[$record->getTime()] : [];
        $records[$record->getTime()] = array_merge($row, [$record->getField() => $record->getValue()]);
    }
  }
  //print_r($records);
  $lastValue = end($records);
  
  //print_r($lastValue);


     if( $lastValue["doorClosed"] == '1'){
      echo "Closed";
    }else{
      if ($lastValue["DoorHold"] == '1'){
        echo "Open -Door Hold Violation!!!!";
      }else{
        echo "Open";
      }
    } 
  
  
  // Close the client
  $client->close();

?>