<?php
// Check if the door_status variable has been set by the POST request from the Arduino
if (isset($_POST["door_status"])) {
    // Get the door status value sent by the Arduino
    $door_status = $_POST["door_status"];

    // Debugging statement - print out the value of the door_status variable
    //echo "Received door status: $door_status<br>";

    // Save the door status value to a file for later retrieval (optional)
    file_put_contents("door_status.txt", $door_status);

    // Return the door status value to the Arduino
    echo $door_status;
} 
 else {
    // If the door_status variable has not been set, retrieve the most recent door status from the file
    $door_status = file_get_contents("door_status.txt");

    // Print the door status value to the web browser
    echo $door_status;
}  
/* else {
  echo "door_status is not set";
} */
?>