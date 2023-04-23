<?php

  // get the UID and MAC address from the GET parameters
  $uid = $_GET['uid'];
  $mac = $_GET['mac'];
  
  // connect to the MySQL database
  $servername = "ece508-project.cmnd3axd90tu.us-east-1.rds.amazonaws.com";
  $username = "admin";
  $password = "abc123456";
  $dbname = "DB1";

  $conn = new mysqli($servername, $username, $password, $dbname);

  // Check connection
  if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
  }

  // collect room based on MAC
  $sql = "SELECT MAC, Room FROM Door WHERE MAC= '$mac'";
  $result = $conn->query($sql);
  $room = $result->fetch_assoc()["Room"]; 

  // check if the UID exists in the User table
  $sql = "SELECT * FROM User WHERE UID = '$uid'";
  $result = $conn->query($sql);
  
  // check for errors in the query
  if (!$result) {
      die("Query failed: " . $conn->error);
  }

  if ($result->num_rows > 0) {
    
    // collect Name based on UID
    $name = $result->fetch_assoc()["Name_id"];
    
    // check if the UID and MAC address match any row in the Access_List table
    $sql = "SELECT * FROM Access_List WHERE UID = '$uid' AND MAC = '$mac'";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) { //case 1: scan correct door
      // grant access
      $status = "Access Granted";
      
      //echo "Access_Granted"; for testing
      
      // insert into Access_Status_Log table
      $sql = "INSERT INTO Access_Log (Name_id, Room, Status) VALUES ('$name', '$room', '$status')";
      if ($conn->query($sql) === TRUE) {
        echo "Access_Granted"; //respond back to arduino
      } else {
        echo "Error: " . $sql . "<br>" . $conn->error;
      }
      
    } else { //case 2: scan wrong door
      // deny access
      $status = "Access Denied";
      
      
      // insert into Access_Status_Log table
      $sql = "INSERT INTO Access_Log (Name_id, Room, Status) VALUES ('$name', '$room', '$status')";
      if ($conn->query($sql) === TRUE) {
        echo "Access_Denied"; //respond back to arduino
      } else {
        echo "Error: " . $sql . "<br>" . $conn->error;
      }
    }
  } else { //case 3: No UID found on DB
    // deny access
    $name = "Unknown";
    $status = "No Card Found";
      // insert into Access_Status_Log table
      $sql = "INSERT INTO Access_Log (Name_id, Room, Status) VALUES ('$name', '$room', '$status')";
      if ($conn->query($sql) === TRUE) {
        echo "No_Card_Found"; //respond back to arduino
      } else {
        echo "Error: " . $sql . "<br>" . $conn->error;
      }
    //echo "No_Card_Found";
  }

  $conn->close();
?>