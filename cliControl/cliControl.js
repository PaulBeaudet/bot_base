// cliControl.js ~ control your arduino with the command line
// Copyright 2015 ~ Paul Beaudet ~ MIT licence see LICENCE for detials

// dependencies
var sp = require("serialport"); // require serial com logic
var arduinoPort = "";                   // port that Arduino is on if connected
var arduino;                            // object instance of com port
var stdin = process.openStdin();        // create standard in object

function listPorts(err, ports){
	ports.forEach(function(port){
		console.log(port.comName);
		console.log(port.pnpId);
		console.log(port.manufacturer);
	  if(port.manufacturer.indexOf("Arduino") > -1){
		  arduinoPort = port.comName;
		  console.log("Arduino is: " + arduinoPort);
			arduino = new sp.SerialPort(arduinoPort, {
				baudrate: 115200,
				parser: sp.parsers.readline("\n")
			});
			arduino.on("open", onOpen);
	  }
	});
}

// ---- main function call -----
sp.list(listPorts);

// ---- Supporting functions ----
function onOpen(){
	console.log('port open. Data rate: ' + arduino.options.baudRate);
	arduino.on("data", recieveData);
	arduino.on("error", showError);
	arduino.on("close", showPortClose);
	stdin.on("data", sendData);
}

function recieveData(data){
	console.log("Arduino: " + data);
}

function sendData(data){
  var str = data.toString();
	str = str.trim(); // remove new line or cariage char
	if(str == "quit"){process.exit();}
  arduino.write(str);
	console.log("writing: " + str);
}

// other debugging messages
function showPortClose() {
   console.log('port closed.');
}

function showError(error) {
   console.log('Serial port error: ' + error);
}
