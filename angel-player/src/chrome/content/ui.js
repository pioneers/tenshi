// Subpages for the left and right panes
var links_left = {
  "welcome" : {
    text : "Welcome",
    url : "welcome/welcome.html"
  },
  "texteditor" : {
    text: "Text Editor",
    url : "texteditor/editor.html"
  }
}

var links_right = {
  "simulator" : {
    text: "Simulator",
    url: "simulator/main.html"
  }
};

function load_subpage(container, links, link) {
  // Loads the subpage in an iframe

  var subpages = $(container).data("subpages");

  if (links[link] === undefined) {
    throw "ERROR";
  }
  $(container).children("iframe").css("display", "none");

  if (subpages === undefined) {
    subpages = {};
  }

  if (subpages[link] === undefined) {
    subpages[link] = $("<iframe/>");
    subpages[link].attr({
      "src": links[link].url,
      "width": "100%",
      "height": "550px"
    }).appendTo($(container));
  }
  subpages[link].css("display", "inherit");
  $(container).data("subpages", subpages);
}

$(function(){
  // Auto-populate subpage selection on load
  for (var key in links_left) {
    if (links_left.hasOwnProperty(key)) {
      (function(key) {
        $("<a href='#'>" + links_left[key].text + "</a>").click(function(){
            load_subpage('#pages-left', links_left, key);
        }).appendTo('#tabs-left');
      })(key);
    }
  }

  // Initialize the right pane to a welcome page
  load_subpage("#pages-left", links_left, "welcome");

  // Initialize the left pane to an empty page.
  // Add and pre-select an empty option for the drop-down, which will be
  // removed once a different page is chosen
  $("<option class='original' selected></option>").appendTo("#tabs-right");

  for (var key in links_right) {
    if (links_right.hasOwnProperty(key)) {
      (function(key) {
        $("<option>" + links_right[key].text + "</option>").click(function(){
          $("#tabs-right").children(".original").remove();
          load_subpage('#pages-right', links_right, key);
        }).appendTo('#tabs-right');
      })(key);
    }
  }
});


