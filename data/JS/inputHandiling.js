SEPARATOR = "_";
LED_SWITCH_ID = "LEDSwitch";
AUDIO_SOURCE_TEXT_ID = "AudioSource";
AUDIO_SOURCE_INPUT_ID = "AudioSourceInput";
TAB_SUFFIX = "tab";
NUMBER_TYPE_SUFFIX = "number";
RANGE_TYPE_SUFFIX = "range";
COLORPICKER_SUFFIX = "colorPicker";
COLOR_SUFFIXES = {
  RED: "R",
  GREEN: "G",
  BLUE: "B",
  WHITE: "W",
};

MODE = "";

function handleLEDSwitchInput(element) {
  setLEDState(element.checked);
}

function handleAudioButtonConnect() {
  var new_source = document.getElementById(AUDIO_SOURCE_INPUT_ID).value;
  updateAudioSource(new_source);
  setAudioSource(new_source);
}

function handleAudioButtonDisconnect() {
  document.getElementById(AUDIO_SOURCE_INPUT_ID).value = "";
  updateAudioSource("none");
  setAudioSource("");
}

function handleColorElementInput(element) {
  var current_value = getElementValue(element);

  updateColorForm(element, current_value);
  updateColorPicker();

  var [r, g, b, w] = getCurrentColorValue();
  setLEDRGB(r, g, b, w);
}

function handleColorPickerInput(element) {
  var [
    red_element,
    green_element,
    blue_element,
    white_element,
  ] = getAllColorElements();

  var [r, g, b] = HEXtoRGB(element.value);
  var w = white_element.value;

  updateColorForm(red_element, r);
  updateColorForm(green_element, g);
  updateColorForm(blue_element, b);

  setLEDRGB(r, g, b, w);
}

function updateLEDSwitchState(state) {
  document.getElementById(LED_SWITCH_ID).checked =
    state == "true" ? true : false;
}

function updateAudioSource(ip) {
  var inner_text = document.getElementById(AUDIO_SOURCE_TEXT_ID).innerText;
  var prev_text = inner_text.split(" :")[0];
  document.getElementById(
    AUDIO_SOURCE_TEXT_ID
  ).innerText = `${prev_text} : ${ip}`;
}

function updateColorPicker() {
  var [red_element, green_element, blue_element] = getAllColorElements().slice(
    0,
    3
  );
  var r = Math.max(0, getElementValue(red_element));
  var g = Math.max(0, getElementValue(green_element));
  var b = Math.max(0, getElementValue(blue_element));

  document.getElementById(
    `${MODE}${SEPARATOR}${COLORPICKER_SUFFIX}`
  ).value = RGBtoHEX(r, g, b);
}

function updateColorForm(element, value) {
  element.value = value;
  updateOtherColorInput(element, value);
}

function updateOtherColorInput(element, value) {
  var [tab_prefix, color, input_type] = element
    .getAttribute("id")
    .split(SEPARATOR);
  var input_type =
    element.getAttribute("type") == "number"
      ? RANGE_TYPE_SUFFIX
      : NUMBER_TYPE_SUFFIX;
  var other_input_id = `${tab_prefix}${SEPARATOR}${color}${SEPARATOR}${input_type}`;
  document.getElementById(other_input_id).value = value;
}

function getCurrentColorValue() {
  var color = [];
  var color_elements = getAllColorElements();
  for (elementIndex in color_elements) {
    color.push(getElementValue(color_elements[elementIndex]));
  }
  return color;
}

function getElementValue(element) {
  return Math.max(
    element.min,
    Math.min(
      element.value == "" ? 0 : parseInt(element.value),
      parseInt(element.max)
    )
  );
}

function getAllColorElements() {
  var red_element = document.getElementById(
    `${MODE}${SEPARATOR}${COLOR_SUFFIXES["RED"]}${SEPARATOR}${NUMBER_TYPE_SUFFIX}`
  );
  var green_element = document.getElementById(
    `${MODE}${SEPARATOR}${COLOR_SUFFIXES["GREEN"]}${SEPARATOR}${NUMBER_TYPE_SUFFIX}`
  );
  var blue_element = document.getElementById(
    `${MODE}${SEPARATOR}${COLOR_SUFFIXES["BLUE"]}${SEPARATOR}${NUMBER_TYPE_SUFFIX}`
  );
  var white_element = document.getElementById(
    `${MODE}${SEPARATOR}${COLOR_SUFFIXES["WHITE"]}${SEPARATOR}${NUMBER_TYPE_SUFFIX}`
  );

  return [red_element, green_element, blue_element, white_element];
}

function RGBtoHEX() {
  var hex = "#";
  for (i in arguments) {
    hex += parseInt(arguments[i]).toString(16).padStart(2, "0");
  }
  return hex;
}

function HEXtoRGB(hex) {
  var r = parseInt(hex.substring(1, 3), 16);
  var g = parseInt(hex.substring(3, 5), 16);
  var b = parseInt(hex.substring(5, 7), 16);

  return [r.toString(), g.toString(), b.toString()];
}
