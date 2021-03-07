function handleTabSwitch(tab, target_content) {
    updateTab(tab, target_content)
    setLEDMode(target_content);
}
function updateTab(tab, target_content) {
    deactivateTabs();
    activateTab(tab, target_content);
}

function deactivateTabs() {
    deactivateTabLinks();
    deactivateTabContents();
}

function deactivateTabLinks() {
    tabs = document.getElementsByClassName('nav-tabs')[0].children;
    for (i = 0; i < tabs.length; i++) {
        tabs[i].firstElementChild.className = 'nav-link';
    }
}

function deactivateTabContents() {
    tab_content_element = document.getElementsByClassName("tab-content")[0];
    tab_panes = tab_content_element.children;
    for (i = 0; i < tab_panes.length; i++) {
        tab_panes[i].className = "tab-pane"
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
    document.getElementById(target_tab_content).className = "tab-pane active"
}