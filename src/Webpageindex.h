//
// PROGMEM stores data in flash memory, default is storing in SRAM
//   --> usually, only worth to be used for large block of data
// R infront of string indicates a RAW string literal
//   --> no need to escape linebreaks, quotationmarks etc.
//   --> allows to put full html-website into variable
//   --> beginning and end of RAW string literal indicated by
//       =====( ... )=====
//
const char MAIN_page[] PROGMEM = R"=====(
  <!doctype html>
  <html>
  <head>
    <title>Capteur de CO2</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!-- Disable favico -->
    <link rel="icon" href="data:,">
    <!--script src = "https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.5.0/chart.js"></script-->
    <script src = "/chart.js"></script>
    <style>
      canvas{ -moz-user-select: none; -webkit-user-select: none; -ms-user-select: none; }
      html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center; }
      #dataTable { border-collapse : collapse; width: 100%; }
      #dataTable td, #dataTable th { border: 1px solid #ddd; padding: 8px; }
      #dataTable tr:nth-child(even){ background-color: #f2f2f2; }
      #dataTable tr:hover { background-color: #ddd; }
      #dataTable th { padding-top: 12px; padding-bottom: 12px; text-align: left; background-color: #4CAF50; color: white; }
      .modal { display: none; position: fixed; z-index: 1; padding-top: 100px; left: 0; top: 0; width: 100%; height: 100%; overflow: auto;}
      .modal-content { margin: auto; padding: 20px; border: 1px solid #888; width: 60%;}
    </style>
  </head>
  <body>
    <div style="text-align:center;">
      <p style="font-size:20px">
        <b>Capteur CO<sub>2</sub></b>
        <button type="button" onclick="downloadData()">Télécharger les données</button>
        <button type="button" id="config-button">Configuration du capteur</button>
      </p>
    </div>
    <div id="config-dialog" class="modal">
      <div class="modal-content">
        <form onsubmit="return storeConfig(this)">
          <label for="finterval">Intervalle de rafraichissement (secondes)</label><br>
          <input type="text" id="finterval" name="finterval"><br>
          <label for="faltitude">Altitude (mètres)</label><br>
          <input type="text" id="faltitude" name="faltitude"><br>
          <label for="fledon">LED</label><br>
          <input type="text" id="fledon" name="fledon"><br>
          <button type="button" id="config-cancel">Annuler</button>
          <input type="submit" value="Valider">
        </form>
      </div>
    </div>
    <script>
      var configDialog = document.getElementById("config-dialog");
      document.getElementById("config-button").onclick = function() {
        configDialog.style.display = "block";
      }
      document.getElementById("config-cancel").onclick = function() {
        configDialog.style.display = "none";
      }
      window.onclick = function(event) {
        if (event.target == configDialog) {
          configDialog.style.display = "none";
        }
      }
    </script>
    <br>
    <div class="chart-container" position: relative; height:350px; width:100%">
        <canvas id="Chart1" width="400" height="400"></canvas>
    </div>
    <br>
    <div class="chart-container" position: relative; height:350px; width:100%">
        <canvas id="Chart2" width="400" height="400"></canvas>
    </div>
    <br>
    <div>
      <table id="dataTable">
        <tr>
          <th><i class="far fa-clock"></i> Temps</th>
          <th><i class="fas fa-head-side-cough" style="color:#ffffff;"></i>Concentration en CO2 (ppm)</th>
          <th><i class="fas fa-thermometer-half" style="color:#ffffff;"></i>Température en &deg;C</th>
          <th><i class="fas fa-tint" style="color:#ffffff;"></i>Taux d'humidité en %</th>
        </tr>
      </table>
    </div>
  <br>
  <br>
  <script>

    // arrays for data values, will be dynamically filled
    // if length exceeds threshold, first (oldest) element is deleted
    var CO2values       = [];
    var Tvalues         = [];
    var Hvalues         = [];
    var timeStamp       = [];
    var maxArrayLength  = 1000;

    // update intervall for getting new data in milliseconds
    updateIntervall = 20;
    document.getElementById('finterval').value = updateIntervall;
    // fetch config value from the sensor and store value in updateIntervall
    getConfig();

    // Graphs visit: https://www.chartjs.org
    // graph for CO2 concentration
    var ctx = document.getElementById("Chart1").getContext('2d');
    var Chart1 = new Chart(ctx, {
      type: 'line',
      data: {
        labels: timeStamp,  //Bottom Labeling
        datasets: [{
          label : "Concentration CO2",
          fill : 'origin',                   // 'origin': fill area to x-axis
          backgroundColor : 'rgba( 243, 18, 156 , .5)', // point fill color
          borderColor : 'rgba( 243, 18, 156 , 1)',  // point stroke color
          data : CO2values,
        }],
      },
      options: {
        title: { display : false, text : "Concentration CO2 (ppm)"},
        maintainAspectRatio: false,
        elements: { line: { tension : 0.1 //Smoothening (Curved) of data lines
          }},
        scales: {
          yAxes: [{
            display : true,
            position : 'left',
            ticks: { beginAtZero :false, precision : 0, fontSize :16},
            scaleLabel: { display : true, labelString : 'CO\u2082 (ppm)', fontSize : 20 },
          }]
        }
      }
    });
    // temperature and humidity graph
    var ctx2 = document.getElementById("Chart2").getContext('2d');
     var Chart2 = new Chart(ctx2, {
      type: 'line',
      data: {
        labels: timeStamp,  //Bottom Labeling
        datasets: [{
          label : "Température",
          fill : false,                      // fill area to xAxis
          backgroundColor : 'rgba( 243, 156, 18 , 1)',  // marker color
          borderColor : 'rgba( 243, 156, 18 , 1)',  // line Color
          yAxisID : 'left',
          data : Tvalues,
        }, {
          label : "Humidity",
          fill : false,                      // fill area to xAxis
          backgroundColor : 'rgba(104, 145, 195, 1)',   // marker color
          borderColor : 'rgba(104, 145, 195, 1)',   // line Color
          data : Hvalues,
          yAxisID : 'right',
        }],
      },
      options: {
        title: { display : false, text : "CO2 Monitor" },
        maintainAspectRatio: false,
        elements: { line: { tension : 0.1 }},
        scales: {
          yAxes: [{
            id : 'left',
            position : 'left',
            scaleLabel: { display : true, labelString : 'Température en \u00B0C', fontSize: 20 },
            ticks: { suggestedMin: 18, suggestedMax: 30, fontSize : 16 }
          }, {
            id : 'right',
            position : 'right',
            scaleLabel: { display : true, labelString : 'Humidity in %', fontSize : 20 },
            ticks: { suggestedMin: 40, suggestedMax: 70, fontSize : 16 }
          }]
        }
      }
    });
    // function to dynamically updating graphs
    // much more efficient than replotting every time
    function updateCharts() {
      // update datasets to be plotted
      Chart1.data.datasets[0].data = CO2values;
      Chart2.data.datasets[0].data = Tvalues;
      Chart2.data.datasets[1].data = Hvalues;
      // update the charts
      Chart1.update();
      Chart2.update();
    };

    // function to download data arrays into csv-file
    function downloadData() {
      // build array of strings with data to be saved
      var data = [];
      for ( var ii=0 ; ii < CO2values.length ; ii++ ){
        data.push( [ timeStamp[ii],
                     Math.round(CO2values[ii]).toString(),
                     Tvalues[ii].toString(),
                     Hvalues[ii].toString()
                   ]);
      }

      // build String containing all data to be saved (csv-formatted)
      var csv = 'Time,CO2 in ppm,Temperature in Celsius,Humidity in percent\n';
      data.forEach(function(row) {
        csv += row.join(',');
        csv += "\n";
      });

      // save csv-string into file
      // create a hyperlink element (defined by <a> tag)
      var hiddenElement     = document.createElement('a');
      // similar functions: encodeURI(), encodeURIComponent() (escape() not recommended)
      hiddenElement.href    = 'data:text/csv;charset=utf-8,'+encodeURI(csv);
      hiddenElement.target  = '_blank';
      hiddenElement.download= 'CO2monitor.csv';
      hiddenElement.click();
    };

    // script ajax pour télécharger régulièrement les données
    function getData() {
      console.log("getData");
      console.log("getData updateIntervall = ",updateIntervall);
      var xhttp = new XMLHttpRequest();
      // onreadystatechange property defines a function to be executed when the readyState changes
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var time = new Date().toLocaleTimeString();
          var txt = this.responseText;
          var obj = JSON.parse(txt);
          CO2values.push(obj.COO);
          Tvalues.push(obj.Temperature);
          Hvalues.push(obj.Humidity);
          timeStamp.push(time);
          // if array becomes too long, delete oldest element to not overload graph
          // also delete first row of data table
          if (CO2values.length > maxArrayLength) {
            // shift() method to delete first element
            CO2values.shift();
            Tvalues.shift();
            Hvalues.shift();
            timeStamp.shift();
            // HTMLTableElement.deleteRow(index), index=-1 for last element
            document.getElementById("dataTable").deleteRow(-1);
          }
          // update graphs
          updateCharts();
          // update data table
          var table       = document.getElementById("dataTable");
          var row         = table.insertRow(1); //Add after headings
          var cell1       = row.insertCell(0);
          var cell2       = row.insertCell(1);
          var cell3       = row.insertCell(2);
          var cell4       = row.insertCell(3);
          cell1.innerHTML = time;
          cell2.innerHTML = Math.round(obj.COO);
          cell3.innerHTML = obj.Temperature;
          cell4.innerHTML = obj.Humidity;
        }
      };
      xhttp.open("GET", "data", true);
      xhttp.send();
      window.setTimeout(getData, updateIntervall*1000);
    }
    getData();

    // ajax script to get configuration
    function getConfig() {
      var xhttp = new XMLHttpRequest();
      // onreadystatechange property defines a function to be executed when the readyState changes
      xhttp.onreadystatechange = function() {
        if (xhttp.readyState == 4 && xhttp.status == 200) {
          var txt = xhttp.responseText;
          var obj = JSON.parse(txt);
          updateIntervall = obj.updateIntervall;
          document.getElementById('finterval').value = updateIntervall;
          document.getElementById('faltitude').value = obj.altitude;
          document.getElementById('fledon').value = obj.ledON;
        }
      };
      xhttp.open("GET", "config", true);
      xhttp.send();
    }

    // Update config and store configuration with ajax script
    function storeConfig(form) {
      console.log("storeConfig fledon = ", parseInt(form["fledon"].value));
      var http = new XMLHttpRequest();
      var url = 'config';
      var params = 'updateIntervall='+form["finterval"].value
        +',altitude='+form["faltitude"].value+',ledON='+form["fledon"].value+'/';
      http.open('POST', url, true);
      // onreadystatechange property defines a function to be executed when the readyState changes
      http.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          console.log(http.responseText);
        }
      };
      http.send(params);
      configDialog.style.display = "none";
      return false;
    }

  </script>
  </body>
  </html>
)=====";
