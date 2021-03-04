function handleLEDSwitchInput(element) {
    setLEDState(element.checked);
}

function handleColorPickerInput(element) {
    var tab_prefix = element.getAttribute('id').split(SEPERATOR)[0]

    var [red_element, green_element, blue_element] = getRGBnumberElements(tab_prefix)

    var [r, g, b] = HEXtoRGB(element.value);

    red_element.value = r;
    green_element.value = g;
    blue_element.value = b;

    handleColorElementInput(red_element);
    handleColorElementInput(green_element);
    handleColorElementInput(blue_element);

}

function handleColorElementInput(element) {
    var [tab_prefix, color, input_type] = element.getAttribute('id').split(SEPERATOR);

    var other_input_type = element.getAttribute('type') == NUMBER_TYPE_SUFFIX ? RANGE_TYPE_SUFFIX : NUMBER_TYPE_SUFFIX;


    fixValue(element);
    var current_value = checkIfEmpty(element.value);

    var other_input_id = [tab_prefix, color, other_input_type].join(SEPERATOR);
    updateOtherColorElementInput(other_input_id, current_value);

    updatePicker(tab_prefix, current_value);

    var [red_element, green_element, blue_element] = getRGBnumberElements(tab_prefix);
    setLEDRGB(red_element.value, green_element.value, blue_element.value)


}

function fixValue(element) {
    if (element.getAttribute("type") == "number") {
        if (parseInt(element.max) < parseInt(element.value)) {
            element.value = element.max;
        }
    }
}

function checkIfEmpty(value) {
    return value == "" ? 0 : value;
}

function updateOtherColorElementInput(id, value) {
    document.getElementById(id).value = value;
}

function updatePicker(tab_prefix) {
    var [red_element, green_element, blue_element] = getRGBnumberElements(tab_prefix)
    var r = checkIfEmpty(red_element.value);
    var g = checkIfEmpty(green_element.value);
    var b = checkIfEmpty(blue_element.value);

    document.getElementById(tab_prefix + SEPERATOR + COLORPICKER_SUFFIX).value = RGBtoHEX(r, g, b);
}

function getRGBnumberElements(tab_prefix) {
    var red_element = document.getElementById([tab_prefix, COLOR_SUFFIXES['RED'], NUMBER_TYPE_SUFFIX].join(SEPERATOR))
    var green_element = document.getElementById([tab_prefix, COLOR_SUFFIXES['GREEN'], NUMBER_TYPE_SUFFIX].join(SEPERATOR))
    var blue_element = document.getElementById([tab_prefix, COLOR_SUFFIXES['BLUE'], NUMBER_TYPE_SUFFIX].join(SEPERATOR))
    return [red_element, green_element, blue_element]
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