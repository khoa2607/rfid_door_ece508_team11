function updateDoorStatus() {
  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("door-status").innerHTML = this.responseText;
    }
/*     else{
        document.getElementById("door-status").innerHTML = "Offline";
    } */
  };
  //xhttp.open("GET", "get_door_status.php", true);
  xhttp.open("GET", "get_influx.php", true);
  xhttp.send();
}
setInterval(updateDoorStatus, 1000);

function refreshIframe(){
  var iframe = document.getElementById("access_log_iframe");
  iframe.src = iframe.src;
}
window.setInterval(refreshIframe, 1000); 


setInterval(function() {
    $.ajax({
        url: "query_access_log.php",
        success: function(data) {
            $("#latest-data").html(data);
        }
    });
}, 1000);