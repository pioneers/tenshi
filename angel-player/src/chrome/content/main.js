function jsAlert() {
    alert('Hello world again!');
}

function showVersion() {
    alert('Control System Next IDE Version ' + appVersion.getVersion() +
        '\nRunning on ' + appVersion.getOS());
}

function onLoad() {
    appVersion.init();

    var versionDiv = document.getElementById('version-div');
    versionDiv.innerHTML = "Build " + appVersion.getBuildID();
}
