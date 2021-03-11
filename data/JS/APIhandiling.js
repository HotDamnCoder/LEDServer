apiSocket = new WebSocket(`ws://${window.location.hostname}:${window.location.port}/api`);
apiSocket.onmessage = handleAPIMessage;
apiSocket.onclose = handleConnectionClose;

function handleConnectionClose(event) {
    if (event.wasClean) {
        alert("API connection closed! Please refresh the website.");
    } else {
        alert('API connection died!' + event.data + ' Please refresh the website.');
    }
}
function handleAPIMessage(message) {
    var [action, value] = message.data.split("=")
    switch (action) {
        case "COLOR":
            var code_starts = value.indexOf('R');
            var red_ends = value.indexOf('G');
            var green_ends = value.indexOf('B');
            var code_ends = value.indexOf('E');

            var r = value.substring(code_starts + 1, red_ends)
            var g = value.substring(red_ends + 1, green_ends)
            var b = value.substring(green_ends + 1, code_ends)

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

function setLEDRGB(r, g, b) {
    apiSocket.send(`COLOR=R${r}G${g}B${b}E`)
}

function setLEDState(state) {
    apiSocket.send(`STATE=${state}`)
}

function setLEDMode(mode) {
    apiSocket.send(`MODE=${mode}`)
}

function setAudioSource(source) {
    apiSocket.send(`AUDIO_SOURCE=${source}`)
}
