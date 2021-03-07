
/* function colorChangeEventHandler(event) {
  var color_code = event.data;
  console.log(event.data);
  color_code = color_code.slice(1)
  var [r_value, g_and_b_value] = color_code.split('G');
  var [g_value, b_value] = g_and_b_value.split('B');

  var [red_element, green_element, blue_element] = getRGBnumberElements(STATIC_TAB_PREFIX); //FIX THIS. NO STATIC_TAB PREFIX BY DEFAULT

  updateColorInput(red_element, r_value);
  updateColorInput(green_element, g_value);
  updateColorInput(blue_element, b_value);
}

function stateChangeEventHandler(event) {
  console.log(event.data);
  document.getElementById(LED_SWITCH_ID).checked = event.data == "true" ? true : false;
}

function modeChangeEventHandler(event) {
  var mode = event.data;
  console.log(event.data);
  updateTab(document.getElementById(mode + SEPERATOR + TAB_SUFFIX), mode);
} */
