 <?php
  // establish connection to database
  $servername = "ece508-project.cmnd3axd90tu.us-east-1.rds.amazonaws.com";
  $username = "admin";
  $password = "abc123456";
  $dbname = "DB1";

  $conn = new mysqli($servername, $username, $password, $dbname);

  // check connection
  if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
  }
  
  $table = "<table>";
  $table .= "<thead><tr><th>Name</th><th>Status</th><th>Door</th><th>Time</th></tr> </thead>";
  $table .= "<tbody>";
  
  
 // retrieve values from database
  $sql = "SELECT * FROM Access_Log ORDER BY Time DESC";
  $result = $conn->query($sql);

  if ($result->num_rows > 0) {
/*     echo "<table></th><th>Name</th><th>Status</th><th>Door</th><th>Time</th></tr>";
    while ($row = $result->fetch_assoc()) {
      echo "</td><td>" . $row["Name_id"] . "</td><td>" . $row["Status"] . "</td><td>" . $row["Room"] . "</td><td>" . $row["Time"] . "</td></tr>";
    }
    echo "</table>"; */
    while ($row = $result->fetch_assoc()) {
      $table .= "<tr>";
      $table .= "<td>" . $row['Name_id'] . "</td>";
      $table .= "<td>" . $row['Status'] . "</td>";
      $table .= "<td>" . $row['Room'] . "</td>";
      $table .= "<td>" . $row['Time'] . "</td>";
      $table .= "</tr>";
    }
    $table .= "</tbody></table>";
    echo $table;
  } else {
    echo "No records found";
  }

  $conn->close();
?>