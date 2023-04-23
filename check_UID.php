<?php
  $servername = "ece508-project.cmnd3axd90tu.us-east-1.rds.amazonaws.com";
  $username = "admin";
  $password = "abc123456";
  $dbname = "DB1";

  // Create connection
  $conn = new mysqli($servername, $username, $password, $dbname);
  // Check connection
  if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
  }

  // get the UID from the GET request
  $UID = $_GET["UID"];
  echo $UID;
  
  $sql = "SELECT UID FROM User WHERE UID = '$UID'";
  $result = $conn->query($sql);
  
  // check for errors in the query
  if (!$result) {
      die("Query failed: " . $conn->error);
  }
  
  if ($result->num_rows == 0){
    echo "false";
  }else{
    echo "true";
  }
   $conn->close();

?>