API_SOCKET = new WebSocket(`ws://${window.location.hostname}:${window.location.port}/api`);
API_SOCKET.onmessage = handleAPIMessage;
API_SOCKET.onclose = handleAPIConnectionClose;
API_SOCKET.onerror = handleAPIError;

API_SIDED_CLOSE_MESSAGE = ""
function handleAPIError(error) {
    if (API_SIDED_CLOSE_MESSAGE == ""){
        alert(`An error occured in the api!\n${error.message}`);
    }
}

function handleAPIConnectionClose(close) {

    if (API_SIDED_CLOSE_MESSAGE != "") {
        alert(`API connection closed with the message:\n"${API_SIDED_CLOSE_MESSAGE}"`);
        API_SIDED_CLOSE_MESSAGE = ""
    } else {
        alert('API connection died! Please refresh the website.');
    }
}

function handleAPIMessage(message) {
    if ("=" in message){
        var [action, value] = message.data.split("=")
        switch (action) {
            case "COLOR":
                var [r, g, b, w] = getColorValuesFromMessage(value)
                var [red_element, green_element, blue_element, white_element] = getAllColorElements()
    
                updateColorForm(red_element, r);
                updateColorForm(green_element, g);
                updateColorForm(blue_element, b);
                updateColorForm(white_element, w);
                updateColorPicker();
                break;
            case "MODE":
                updateTab(document.getElementById(value + SEPERATOR + TAB_SUFFIX), value);
                MODE = value;
                break;
            case "STATE":
                updateLEDSwitchState(value)
                break;
            case "AUDIO_SOURCE":
                updateAudioSource(value);
            default:
                break;
        }
    }
    else{
        API_SIDED_CLOSE_MESSAGE = message.data
    }
}

function getColorValuesFromMessage(message) {
    var code_starts = message.indexOf('R');
    var red_ends = message.indexOf('G');
    var green_ends = message.indexOf('B');
    var blue_ends = message.indexOf('W');
    var code_ends = message.indexOf('E');

    var r = message.substring(code_starts + 1, red_ends);
    var g = message.substring(red_ends + 1, green_ends);
    var b = message.substring(green_ends + 1, blue_ends);
    var w = message.substring(blue_ends + 1, code_ends);

    return [r, g, b, w];
}

function APIisOpen() {
    return API_SOCKET.readyState == 1
}

function setLEDRGB(r, g, b, w) {
    if (APIisOpen()) {
        API_SOCKET.send(`COLOR=R${r}G${g}B${b}W${w}E`)
    }
}

function setLEDState(state) {
    if (APIisOpen()) {
        API_SOCKET.send(`STATE=${state}`)
    }
}

function setLEDMode(mode) {
    if (APIisOpen()) {
        API_SOCKET.send(`MODE=${mode}`)
    }
}

function setAudioSource(source) {
    if (APIisOpen()) {
        API_SOCKET.send(`AUDIO_SOURCE=${source}`)
    }
}