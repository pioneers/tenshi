function onLoad() {
    var globalLoader = parent.document.tenshiGlobals.loader;
    var module = loader.Module('tenshi/blockeditor/blockeditor',
        'chrome://angel-player/content/blockeditor/blockeditor.js');
    loader.load(globalLoader, module);
    module.exports.onLoad(document);
}
