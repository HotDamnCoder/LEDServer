var websocket = new WebSocket('ws://192.168.1.139/ws');
var xhttp = new XMLHttpRequest();
websocket.onmessage = changeRGB;

function changeRGB(message) {
    var colorcode = message.data;
    console.log(colorcode);
    var code_starts = colorcode.indexOf('R');
    var red_ends = colorcode.indexOf('G');
    var green_ends = colorcode.indexOf('B');
    var code_ends = colorcode.indexOf('E');

    var r = colorcode.substring(code_starts + 1, red_ends)
    var g = colorcode.substring(red_ends + 1, green_ends)
    var b = colorcode.substring(green_ends + 1, code_ends)

    var [red_element, green_element, blue_element] = getRGBnumberElements(STATIC_TAB_PREFIX)


    updateColorInput(red_element, r);
    updateColorInput(green_element, g);
    updateColorInput(blue_element, b);
}
function setLEDRGB(r, g, b) {
    websocket.send(`R${r}G${g}B${b}E`)
}

function setLEDState(state) {

    xhttp.open("GET", `/api/setState?state=${state}`, true);
    xhttp.send();
}

function setLEDMode(mode) {
    xhttp.open("GET", `/api/setMode?mode=${mode}`, true);
    xhttp.send();
}

function getLedRGB(callback) {
    xhttp.open("GET", "/api/getColor");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this);
        }
    };
    xhttp.send();
}

function getLedState(callback) {
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this);
        }
    };
    xhttp.open("GET", "/api/getState");
    xhttp.send();
}

function getLedMode(callback) {
    xhttp.open("GET", "/api/getMode");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this);
        }
    };
    xhttp.send();
}