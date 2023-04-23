Instruction for project - door RFID + door contact + lock

software requires: 

- XAMPP server  (Apache server)
- Composer  (Mange extension/package for php)

Setup:
1) Install XAMPP server  (default locaiton at: C:\xampp) - Download online
2) Include composer 
3) Copy all the PHP files and HTML to C:\xampp\htdocs
4) At C:\xampp\htdocs. Install InfluxDB2\Client package (src: https://github.com/influxdata/influxdb-client-php)
  - Open Terminal and go to directory C:\xampp\htdocs
  - composer require influxdata/influxdb-client-php guzzlehttp/guzzle
  (if there is error about missing ZIP file or GIT - isntall those software like 7zip or git
  then add extension=zip to php.ini file). To open php.ini file, open xampp control panel and click on 
  config->php.ini
  
Purpose: 
Arduino -> APACHE server -> Database (instead of Arduino -> Dababase directly which makes the code
convoluted and hard to manage, also reduce bottleneck while Arduino establish connection) 

Run:

- Download final_project_v0_1_updated_database.ino to Arduino 
- Open Serial
- Open Telegraf (modify the output directory accordingly)
- Open InfluxDB (maybe I have to run it on my end) 


Wiring:
Door contact: PIN 2
Lock: pin 3
Card Reader: SS 10, RST 9 (MOSI,MISO,CLK as default) 

  
