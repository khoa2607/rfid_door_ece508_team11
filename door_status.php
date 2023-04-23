<?php

// Check if the door status is set in the HTTP request body
if (isset($_POST['door_status'])) {
  // Get the door status from the HTTP request body
  $door_locked = $_POST['door_status'];

  // Send the door status directly to the HTML page
  header('Content-Type: text/event-stream');
  header('Cache-Control: no-cache');
  echo "data: " . ($door_locked === 'true' ? 'locked' : 'unlocked') . "\n\n";
  flush();
} else {
  // Send an error message if the door status is not set
  header('HTTP/1.1 400 Bad Request');
  echo 'Error: door status is not set';
}
?>