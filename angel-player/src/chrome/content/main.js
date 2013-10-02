function setDebugFooterVisibility() {
    var debugFooter = document.getElementById('footer-debug-container');
    if (!debugModule.isDebugEnabled()) {
        debugFooter.style.display = "none";
    }
    else {
        debugFooter.style.display = "";
    }
}

function onLoad() {
    debugModule.init();

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
