function setDebugFooterVisibility() {
    var debugFooter = document.getElementById('footer-debug-container');
    if (!debugModule.isDebugEnabled()) {
        debugFooter.style.display = "none";
    }
    else {
        debugFooter.style.display = "";
    }
}

function setVersionInfoTag() {
    var infoTag = document.getElementById('version-data');
    infoTag.innerHTML = "Build ID " + appVersion.getBuildID() +
        ", Version " + appVersion.getVersion() +
        " (running on " + appVersion.getOS() + ")";
}

function onLoad() {
    debugModule.init();

    setVersionInfoTag();

    // Hide debug stuff if debugging is off.
    setDebugFooterVisibility();
}

function toggleDebug() {
    debugModule.toggleDebug();
    setDebugFooterVisibility();
}

function jsAlert() {
    alert('Hello world again!');
}

function openInspector() {
    window.open("chrome://inspector/content/inspector.xul", "", "chrome");
}
