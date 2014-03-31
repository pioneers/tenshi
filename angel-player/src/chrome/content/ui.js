var window,
    $;

// Subpages for the left and right panes
var links_left = {
  "welcome" : {
    text : "Welcome",
    url : "welcome/welcome.html"
  },
  "texteditor" : {
    text: "Text Editor",
    url : "texteditor/editor.html"
  },
  "blockeditor" : {
    text : "Block Editor (TODO)",
    url : "about:blank"
  },
  "maker" : {
    text : "Robot Maker",
    url : "simulator/maker.html",
    float : true
  }
};

var links_right = {
  "simulator" : {
    text: "Simulator",
    url: "simulator/main.html"
  },
  "console" : {
    text: "Console",
    url: "console/console.html"
  },
  "api_help" : {
    text: "API Help (TODO)",
    url: "about:blank"
  },
  "language_help" : {
    text: "Language Reference (TODO)",
    url: "about:blank"
  },
  "library" : {
    text: "Library (TODO)",
    url: "about:blank"
  }
};

var links_footer = {
  "controls": {
    text: "",
    url: "controls/main.html"
  }
};

function load_subpage(container, links, link) {
  // Loads the subpage in an iframe

  var subpages = $(container).data("subpages");

  if (links[link] === undefined) {
    throw "ERROR";
  }
  $(container).find("iframe").css("display", "none");

  if (subpages === undefined) {
    subpages = {};
  }

  if (subpages[link] === undefined) {
    var height = "550px";
    if (arguments.length > 3) {
      height = arguments[4];
    }
    subpages[link] = $("<iframe/>");
    subpages[link].attr({
      "src": links[link].url,
      "width": "100%",
      "height": height
    });

    if (links[link].hasOwnProperty("float") && links[link].float) {
      var div = $("<div class='floater'/>").appendTo($(container));
      subpages[link].appendTo(div);
    } else {
      subpages[link].appendTo($(container));
    }
  } else {
    // Call onResume() when resuming that page
    var onResume = subpages[link][0].contentWindow.onResume;
    if (onResume !== undefined) {
      onResume();
    }
  }
  subpages[link].css("display", "inherit");
  $(container).data("subpages", subpages);
}

function populate_left(key) {
  // Populate the left pane tab bar with links
  $("<a href='#'>" + links_left[key].text + "</a>").click(function(){
    load_subpage('#pages-left', links_left, key);
  }).appendTo('#tabs-left');
}

function populate_right(key) {
  // Populate the right pane selection menu with links
  $("<option>" + links_right[key].text + "</option>").click(function(){
    $("#tabs-right").children(".delete-on-deselect").remove();
    load_subpage('#pages-right', links_right, key);
  }).appendTo('#tabs-right');
}

exports.init = function(_window) {
  window = _window;
  $ = require("jquery")(window);

  // Initialize the left pane to an empty page.
  // Add and pre-select an empty option for the drop-down, which will be
  // removed once a different page is chosen
  $("<option class='delete-on-deselect' selected></option>").appendTo("#tabs-right");

  // Auto-populate subpage selection
  $.each(links_left, populate_left);
  $.each(links_right, populate_right);

  // Initialize the right pane to a welcome page
  load_subpage("#pages-left", links_left, "welcome");

  // Initialize the bottom page to the controls.
  // There are no other tabs for the bottom page
  load_subpage("footer", links_footer, "controls", "auto");
};

