function updateUILedRGB(xhttp) {
    var [r_value, g_and_b_value] = xhttp.responseText.slice(1).split('G');
    var [g_value, b_value] = g_and_b_value.split('B');
    var [red_element, green_element, blue_element] = getRGBnumberElements(STATIC_TAB_PREFIX)

    red_element.value = r_value;
    green_element.value = g_value;
    blue_element.value = b_value;

    handleColorElementInput(red_element);
    handleColorElementInput(green_element);
    handleColorElementInput(blue_element);
}

function updateUILedState(xhttp) {
    document.getElementById(LED_SWITCH_ID).checked = xhttp.responseText == "true" ? true : false;
}

function updateUITab(xhttp) {
    var mode = xhttp.responseText;
    handleTabSwitch(document.getElementById(mode + SEPERATOR + TAB_SUFFIX), mode);
}
getLedMode(updateUITab);
getLedState(updateUILedState);
getLedRGB(updateUILedRGB);


//TODO: Change function names, violates SRP