function handleLEDSwitchInput(element) {
    setLEDState(element.checked);
}

function handleColorElementInput(element) {
    var current_value = getCurrentColorValue(element);
    var [tab_prefix, color, input_type] = element.getAttribute('id').split(SEPERATOR);
    updateColorInput(element, current_value);
    var [red_element, green_element, blue_element] = getRGBnumberElements(tab_prefix);
    setLEDRGB(red_element.value, green_element.value, blue_element.value)
    return false;
}

function getCurrentColorValue(element) {
    return Math.min(element.value == "" ? 0 : parseInt(element.value), parseInt(element.max));
}

function handleColorPickerInput(element) {
    var tab_prefix = element.getAttribute('id').split(SEPERATOR)[0]

    var [red_element, green_element, blue_element] = getRGBnumberElements(tab_prefix)

    var [r, g, b] = HEXtoRGB(element.value);

    updateColorInput(red_element, r);
    updateColorInput(green_element, g);
    updateColorInput(blue_element, b);
    setLEDRGB(r,g,b);

}
function getRGBnumberElements(tab_prefix) {
    var red_element = document.getElementById([tab_prefix, COLOR_SUFFIXES['RED'], NUMBER_TYPE_SUFFIX].join(SEPERATOR))
    var green_element = document.getElementById([tab_prefix, COLOR_SUFFIXES['GREEN'], NUMBER_TYPE_SUFFIX].join(SEPERATOR))
    var blue_element = document.getElementById([tab_prefix, COLOR_SUFFIXES['BLUE'], NUMBER_TYPE_SUFFIX].join(SEPERATOR))
    return [red_element, green_element, blue_element]
}

function updateColorInput(element, value) {
    element.value = value;
    var [tab_prefix, color, input_type] = element.getAttribute('id').split(SEPERATOR);
    var input_type = element.getAttribute('type') == NUMBER_TYPE_SUFFIX ? RANGE_TYPE_SUFFIX : NUMBER_TYPE_SUFFIX;
    var other_input_id = [tab_prefix, color, input_type].join(SEPERATOR);
    updateOtherColorInput(other_input_id, value)
    updateColorPicker(tab_prefix);
}

function updateOtherColorInput(id, value) {
    document.getElementById(id).value = value;
}

function updateColorPicker(tab_prefix) {
    var [red_element, green_element, blue_element] = getRGBnumberElements(tab_prefix)
    var r = getCurrentColorValue(red_element)
    var g = getCurrentColorValue(green_element);
    var b = getCurrentColorValue(blue_element);

    document.getElementById(tab_prefix + SEPERATOR + COLORPICKER_SUFFIX).value = RGBtoHEX(r, g, b);
}

function RGBtoHEX() {
    var hex = "#";
    for (i in arguments) {
        hex += parseInt(arguments[i]).toString(16).padStart(2, '0');
    }
    return hex;

}

function HEXtoRGB(hex) {
    var r = parseInt(hex.substring(1, 3), 16);
    var g = parseInt(hex.substring(3, 5), 16);
    var b = parseInt(hex.substring(5, 7), 16);

    return [r.toString(), g.toString(), b.toString()];
}







