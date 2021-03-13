apiSocket = new WebSocket(`ws://${window.location.hostname}:${window.location.port}/api`);
apiSocket.onmessage = handleAPIMessage;
apiSocket.onclose = handleAPIConnectionClose;
apiSocket.onerror = handleAPIError;

function handleAPIError(error) {
    alert(`An error occured in the api!\n${error.message}`);
}

function handleAPIConnectionClose(close) {
    if (close.wasClean) {
        alert(`API connection closed! ${close.message} Please refresh the website.`);
    } else {
        alert('API connection died! Please refresh the website.');
    }
}

function handleAPIMessage(message) {
    var [action, value] = message.data.split("=")
    switch (action) {
        case "COLOR":
            var [r, g, b] = getColorValuesFromMessage(value)
            var [red_element, green_element, blue_element] = getAllColorElements()

            updateColorForm(red_element, r);
            updateColorForm(green_element, g);
            updateColorForm(blue_element, b);
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

function getColorValuesFromMessage(message) {
    var code_starts = message.indexOf('R');
    var red_ends = message.indexOf('G');
    var green_ends = message.indexOf('B');
    var code_ends = message.indexOf('E');

    var r = message.substring(code_starts + 1, red_ends);
    var g = message.substring(red_ends + 1, green_ends);
    var b = message.substring(green_ends + 1, code_ends);

    return [r, g, b];
}

function APIisOpen() {
    return apiSocket.readyState == 1
}

function setLEDRGB(r, g, b) {
    if (APIisOpen()) {
        apiSocket.send(`COLOR=R${r}G${g}B${b}E`)
    }
}

function setLEDState(state) {
    if (APIisOpen()) {
        apiSocket.send(`STATE=${state}`)
    }
}

function setLEDMode(mode) {
    if (APIisOpen()) {
        apiSocket.send(`MODE=${mode}`)
    }
}

function setAudioSource(source) {
    if (APIisOpen()) {
        apiSocket.send(`AUDIO_SOURCE=${source}`)
    }
}