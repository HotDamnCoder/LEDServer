function handleTabSwitch(tab, target_content) {
    resetTabs()
    activateTab(tab);
    resetTabContents();
    activateTabContent(target_content);
    setLEDMode(target_content);

}

function resetTabs() {
    tabs = document.getElementsByClassName('nav-tabs')[0].children;
    for (i = 0; i < tabs.length; i++) {
        tabs[i].firstElementChild.className = 'nav-link';
    }
}

function activateTab(tab) {
    tab.className = "nav-link active";
}

function resetTabContents() {
    tab_content_element = document.getElementsByClassName("tab-content")[0];
    tab_panes = tab_content_element.children;
    for (i = 0; i < tab_panes.length; i++) {
        tab_panes[i].className = "tab-pane"
    }
}

function activateTabContent(target_tab_content) {
    document.getElementById(target_tab_content).className = "tab-pane active"
}