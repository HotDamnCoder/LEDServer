function setLEDRGB(r, g, b) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", `/api/setColor?R=${r}&G=${g}&B=${b}`, true);
    xhttp.send();
}

function setLEDState(state) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", `/api/setState?state=${state}`, true);
    xhttp.send();
}

function setLEDMode(mode) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", `/api/setMode?mode=${mode}`, true);
    xhttp.send();
}

function getLedRGB(callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/api/static/getColor");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this);
        }
    };
    xhttp.send();
}

function getLedState(callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this);
        }
    };
    xhttp.open("GET", "/api/getState");
    xhttp.send();
}

function getLedMode(callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/api/getMode");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this);
        }
    };
    xhttp.send();
}