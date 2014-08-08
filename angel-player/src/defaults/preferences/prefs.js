pref("toolkit.defaultChromeURI", "chrome://angel-player/content/main.xul");

pref("tenshi.enableDebug", false);

// This prevents the middle mouse button from messing up things (we do our own
// scroll handling).
// TODO(rqou): Which of these are strictly necessary?
pref("middlemouse.contentLoadURL", false);
pref("middlemouse.openNewWindow", false);
pref("middlemouse.paste", false);
pref("middlemouse.scrollbarPosition", false);
pref("general.autoScroll", false);

pref("dom.gamepad.enabled", true);
// This allows us to use events to handle buttons/axes, rather than polling
// for them. This is Mozilla-only!
pref("dom.gamepad.non_standard_events.enabled", true);

// Data telemetry. To be changed later
pref("tenshi.telemetryUrl", "https://rqou.com/bigbrother.sh");
