function handleTabSwitch(tab, target_content) {
  updateTab(tab, target_content);
  MODE = target_content;
  setLEDMode(target_content);
}

function updateTab(tab, target_content) {
  deactivateTabs(tab);
  activateTab(tab, target_content);
}

function deactivateTabs(tab) {
  deactivateTabLinks(tab);
  deactivateTabContents(tab);
}

function deactivateTabLinks(tab) {
  tab_links = tab.parentElement.parentElement.children;
  for (i = 0; i < tab_links.length; i++) {
    tab_links[i].firstElementChild.className = "nav-link";
  }
}

function deactivateTabContents(tab) {
  tab_panes = tab.parentElement.parentElement.nextElementSibling.children;
  for (i = 0; i < tab_panes.length; i++) {
    tab_panes[i].className = "tab-pane";
  }
}

function activateTab(tab, target_content) {
  activateTabLink(tab);
  activateTabContent(target_content);
}

function activateTabLink(tab) {
  tab.className = "nav-link active";
}

function activateTabContent(target_tab_content) {
  target_tab_content_element = document.getElementById(target_tab_content);
  if (target_tab_content_element != null) {
    target_tab_content_element.className = "tab-pane active";
  }
}
