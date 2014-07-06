var $, d3, nv, window, document;
var mouseX, mouseY;


/**
 *  Credit to Jacob Kelly for context.js found at http://lab.jakiestfu.com/contextjs/ 
 */
var context = context || (function () {

  var options = {
    fadeSpeed: 100,
    filter: null,
    above: 'auto',
    preventDoubleContext: true,
    compress: false
  };

  function initialize(opts) {

    options = $.extend({}, options, opts);
    
    $(document).on('click', 'html', function () {
      $('.dropdown-context').fadeOut(options.fadeSpeed, function(){
        $('.dropdown-context').css({display:''}).find('.drop-left').removeClass('drop-left');
      });
    });
    if(options.preventDoubleContext){
      $(document).on('contextmenu', '.dropdown-context', function (e) {
        e.preventDefault();
      });
    }
    $(document).on('mouseenter', '.dropdown-submenu', function(){
      var $sub = $(this).find('.dropdown-context-sub:first'),
        subWidth = $sub.width(),
        subLeft = $sub.offset().left,
        collision = (subWidth+subLeft) > window.innerWidth;
      if(collision){
        $sub.addClass('drop-left');
      }
    });
    
  }

  function updateOptions(opts){
    options = $.extend({}, options, opts);
  }

  function buildMenu(data, id, subMenu) {
    var subClass = (subMenu) ? ' dropdown-context-sub' : '',
      compressed = options.compress ? ' compressed-context' : '',
      $menu = $('<ul class="dropdown-menu dropdown-context' + subClass + compressed+'" id="dropdown-' + id + '"></ul>');
        var i = 0, linkTarget = '';
        for(i; i<data.length; i++) {
      if (typeof data[i].divider !== 'undefined') {
        $menu.append('<li class="divider"></li>');
      } else if (typeof data[i].header !== 'undefined') {
        $menu.append('<li class="nav-header">' + data[i].header + '</li>');
      } else {
        if (typeof data[i].href == 'undefined') {
          data[i].href = '#';
        }
        if (typeof data[i].target !== 'undefined') {
          linkTarget = ' target="'+data[i].target+'"';
        }
        if (typeof data[i].subMenu !== 'undefined') {
          $sub = ('<li class="dropdown-submenu"><a tabindex="-1" href="' + data[i].href + '">' + data[i].text + '</a></li>');
        } else {
          $sub = $('<li><a tabindex="-1" href="' + data[i].href + '"'+linkTarget+'>' + data[i].text + '</a></li>');
        }
        if (typeof data[i].action !== 'undefined') {
          var actiond = new Date(),
            actionID = 'event-' + actiond.getTime() * Math.floor(Math.random()*100000),
            eventAction = data[i].action;
          $sub.find('a').attr('id', actionID);
          $('#' + actionID).addClass('context-event');
          $(document).on('click', '#' + actionID, eventAction);
        }
        $menu.append($sub);
        if (typeof data[i].subMenu != 'undefined') {
          var subMenuData = buildMenu(data[i].subMenu, id, true);
          $menu.find('li:last').append(subMenuData);
        }
      }
      if (typeof options.filter == 'function') {
        options.filter($menu.find('li:last'));
      }
    }
    return $menu;
  }

  function addContext(selector, data) {

    var d = new Date(),
      id = d.getTime(),
      $menu = buildMenu(data, id);
      
    $('body').append($menu);
    
    
    $(document).on('contextmenu', selector, function (e) {
      e.preventDefault();
      e.stopPropagation();
      
      $('.dropdown-context:not(.dropdown-context-sub)').hide();
      
      $dd = $('#dropdown-' + id);
      if (typeof options.above == 'boolean' && options.above) {
        $dd.addClass('dropdown-context-up').css({
          top: e.pageY - 20 - $('#dropdown-' + id).height(),
          left: e.pageX - 13
        }).fadeIn(options.fadeSpeed);
      } else if (typeof options.above == 'string' && options.above == 'auto') {
        $dd.removeClass('dropdown-context-up');
        var autoH = $dd.height() + 12;
        if ((e.pageY + autoH) > $('html').height()) {
          $dd.addClass('dropdown-context-up').css({
            top: e.pageY - 20 - autoH,
            left: e.pageX - 13
          }).fadeIn(options.fadeSpeed);
        } else {
          $dd.css({
            top: e.pageY + 10,
            left: e.pageX - 13
          }).fadeIn(options.fadeSpeed);
        }
      }
    });
  }
  
  function destroyContext(selector) {
    $(document).off('contextmenu', selector).off('click', '.context-event');
  }
  
  return {
    init: initialize,
    settings: updateOptions,
    attach: addContext,
    destroy: destroyContext
  };

})();

function bindMouse() {
  $(".container-fluid, .dropdown-menu").mousemove(function(event) {
    mouseX = event.pageX;
    mouseY = event.pageY;
  });
}

var chartRT = function (widget) {
    var _self = this;

    function s4() {
        return Math.floor((1 + Math.random()) * 0x10000)
             .toString(16)
             .substring(1);
    }

    function guid() {
        return s4() + s4() + '-' + s4() + '-' + s4() + '-' +
         s4() + '-' + s4() + s4() + s4();
    }

    _self.resizeChartX = function() {
	var ticks = window.prompt("How many ticks do you want? (number on x-axis)", "Currently "+String(_self.Ticks));
	if(ticks > 99) {
	// Shift data when resized
	if (ticks < _self.Ticks) {
	    for (i=0; i < _self.Ticks-ticks;i++) {
	        for (j in _self.DataSeries) {
            	    _self.DataHistory[j].Data.push(_self.DataSeries[j].Data.shift());
                }
	    }
	} else {
	    for (i=0; i < ticks-_self.Ticks;i++) {
	        for (j in _self.DataSeries) {
            	    _self.DataSeries[j].Data.unshift(_self.DataHistory[j].Data.pop());
                }
	    }
	};
        _self.Ticks = parseInt(ticks);
	};
        _self.w = _self.Ticks * 3;
        _self.width = _self.w - _self.margin.left - _self.margin.right;
        _self.Init();
    };

    _self.resizeChartY = function() {
	var newHeight = window.prompt("How tall do you want the chart to be?", "Currently "+String(_self.height));
	if (newHeight > 99) {
	_self.height = parseInt(newHeight);
	};
	_self.h = _self.height + _self.margin.top + _self.margin.bottom;
	_self.Init();
    };

    _self.guid = guid();
    _self.DataSeries = [];
    _self.DataHistory = [];
    _self.Ticks = 20;
    _self.TickDuration = 200; // 5 Hz
    _self.MaxValue = 10;
    _self.w = 300; // pixels
    _self.h = 300; // pixels
    _self.margin = { top: 50, right: 50, bottom: 50, left: 50 };     // pixels
    _self.width = _self.w - _self.margin.left - _self.margin.right;  // pixels
    _self.height = _self.h - _self.margin.top - _self.margin.bottom; // pixels
    _self.xText = '';
    _self.yText = '';
    _self.titleText = '';
    _self.chartSeries = {};
    _self.container = d3.selectAll(widget.toArray());

    _self.Init = function () {
      d3.select('#chart-' + _self.guid).remove();
        //  SVG Canvas
        _self.svg = _self.container.append("svg")
          .attr("id", 'chart-' + _self.guid)
          .attr("width", _self.w)
          .attr("height", _self.h)
          .append("g")
          .attr("transform", "translate(" + _self.margin.left + "," + _self.margin.top + ")");
        //  Use Clipping to hide chart mechanics
        _self.svg.append("defs").append("clipPath")
          .attr("id", "clip-" + _self.guid)
          .append("rect")
          .attr("width", _self.width)
          .attr("height", _self.height);
        // Generate colors from DataSeries Names
        _self.color = d3.scale.category10();
        _self.color.domain(_self.DataSeries.map(function (d) { return d.Name; }));
        //  X,Y Scale
        _self.xscale = d3.scale.linear().domain([0, _self.Ticks]).range([0, _self.width]);
        _self.yscale = d3.scale.linear().domain([0, _self.MaxValue]).range([_self.height, 0]);
        //  X,Y Axis
        _self.xAxis = d3.svg.axis()
          .scale(d3.scale.linear().domain([0, _self.Ticks]).range([_self.width, 0]))
          .orient("bottom");
        _self.yAxis = d3.svg.axis()
          .scale(_self.yscale)
          .orient("left");
        //  Line/Area Chart
        _self.line = d3.svg.line()
          .interpolate("basis")
          .x(function (d, i) { return _self.xscale(i-1); })
          .y(function (d) { return _self.yscale(d.Value); });
        //
        _self.area = d3.svg.area()
          .interpolate("basis")
          .x(function (d, i) { return _self.xscale(i-1); })
          .y0(_self.height)
          .y1(function (d) { return _self.yscale(d.Value); });
        //  Title 
        _self.Title = _self.svg.append("text")
          .attr("id", "title-" + _self.guid)
          .style("text-anchor", "middle")
          .text(_self.titleText)
          .attr("transform", function (d, i) { return "translate(" + _self.width / 2 + "," + -10 + ")"; });
        //  X axis text
        _self.svg.append("g")
          .attr("class", "x axis")
          .attr("transform", "translate(0," + _self.yscale(0) + ")")
          .call(_self.xAxis)
          .append("text")
          .attr("id", "xName-" + _self.guid)
          .attr("x", _self.width / 2)
          .attr("dy", "3em")
          .style("text-anchor", "middle")
          .text(_self.xText);
        // Y axis text
        _self.svg.append("g")
          .attr("class", "y axis")
          .call(_self.yAxis)
          .append("text")
          .attr("id", "yName-" + _self.guid)
          .attr("transform", "rotate(-90)")
          .attr("y", 0)
          .attr("x", -_self.height / 2)
          .attr("dy", "-3em")
          .style("text-anchor", "middle")
          .text(_self.yText);
        // Vertical grid lines
        _self.svg.selectAll(".vline").data(d3.range(_self.Ticks)).enter()
          .append("line")
          .attr("x1", function (d) { return d * (_self.width / _self.Ticks); })
          .attr("x2", function (d) { return d * (_self.width / _self.Ticks); })
          .attr("y1", function (d) { return 0; })
          .attr("y2", function (d) { return _self.height; })
          .style("stroke", "#eee")
          .style("opacity", 0.5)
          .attr("clip-path", "url(#clip-" + _self.guid + ")")
          .attr("transform", "translate(" + (_self.width / _self.Ticks) + "," + 0 + ")");
        // Horizontal grid lines
        _self.svg.selectAll(".hline").data(d3.range(_self.Ticks)).enter()
          .append("line")
          .attr("x1", function (d) { return 0; })
          .attr("x2", function (d) { return _self.width; })
          .attr("y1", function (d) { return d * (_self.height / (_self.MaxValue / 10)); })
          .attr("y2", function (d) { return d * (_self.height / (_self.MaxValue / 10)); })
          .style("stroke", "#eee")
          .style("opacity", 0.5)
          .attr("clip-path", "url(#clip-" + _self.guid + ")")
          .attr("transform", "translate(" + 0 + "," + 0 + ")");
        //  Bind DataSeries to chart
        _self.Series = _self.svg.selectAll(".Series")
          .data(_self.DataSeries)
          .enter().append("g")
          .attr("clip-path", "url(#clip-" + _self.guid + ")")
          .attr("class", "Series");
        //  Draw path from Series Data Points
        _self.path = _self.Series.append("path")
          .attr("class", "area")
          .attr("d", function (d) { return _self.area(d.Data); })
          .style("fill", function (d) { return _self.color(d.Name); })
          .style("fill-opacity", 0.25)
          .style("stroke", function (d) { return _self.color(d.Name); });
        //  Legend 
        // _self.Legend = _self.svg.selectAll(".Legend")
        //   .data(_self.DataSeries)
        //   .enter().append("g")
        //   .attr("class", "Legend");
        // _self.Legend.append("circle")
        //   .attr("r", 4)
        //   .style("fill", function (d) { return _self.color(d.Name); })
        //   .style("fill-opacity", .5)
        //   .style("stroke", function (d) { return _self.color(d.Name); })
        //   .attr("transform", function (d, i) { return "translate(" + (_self.width + 6) + "," + (10 + (i * 20)) + ")"; });
        // _self.Legend.append("text")
        //   .text(function (d) { return d.Name; })
        //   .attr("dx", "0.5em")
        //   .attr("dy", "0.25em")
        //   .style("text-anchor", "start")
        //   .attr("transform", function (d, i) { return "translate(" + (_self.width + 6) + "," + (10 + (i * 20)) + ")"; });

        _self.tick = function (id) {
            _self.thisTick = new Date();
            var elapsed = parseInt(_self.thisTick - _self.lastTick, 10);
            var elapsedTotal = parseInt(_self.lastTick - _self.firstTick, 10);
            if (elapsed < _self.TickDuration && elapsedTotal > 0) {
                _self.lastTick = _self.thisTick;
                return;
            }
            if (id < _self.DataSeries.length-1 && elapsedTotal > 0) {
                return;
            }
            _self.lastTick = _self.thisTick;
            //Add new values
            for (var i in _self.DataSeries) {
                _self.DataSeries[i].Data.push({ Value: _self.chartSeries[_self.DataSeries[i].Name] });
                //Backfill missing values
                while (_self.DataSeries[i].Data.length -1<_self.Ticks+3 ) {
                    _self.DataSeries[i].Data.unshift({ Value: 0 });
                }
		while (_self.DataHistory[i].Data.length -1< 400-_self.Ticks+3 ) {
		    _self.DataHistory[i].Data.unshift({ Value: 0 });
		}
            }

            d3.select("#yName-" + _self.guid).text(_self.yText);
            d3.select("#xName-" + _self.guid).text(_self.xText);
            d3.select("#title-" + _self.guid).text(_self.titleText);

            _self.path
                .attr("d", function (d) { return _self.area(d.Data); })
                .attr("transform", null)
                .transition()
                .duration(_self.TickDuration)
                .ease("linear")
                .attr("transform", "translate(" + _self.xscale(-1) + ",0)")
                .each("end", function (d, i) { _self.tick(i); });

            //Remove oldest values
            for (i in _self.DataSeries) {
            	_self.DataHistory[i].Data.push(_self.DataSeries[i].Data.shift());
		_self.DataHistory[i].Data.shift();
            }
        };

        _self.firstTick = new Date();
        _self.lastTick = new Date();
        _self.start = function () {
            _self.firstTick = new Date();
            _self.lastTick = new Date();
            _self.tick(0);

        };
        _self.start();
    };

    _self.addSeries = function (SeriesName) {
        _self.chartSeries[SeriesName] = 0;
        _self.DataSeries.push({ Name: SeriesName, Data: [{ Value: 0}] });
	_self.DataHistory.push({ Name: SeriesName, Data: [{ Value: 0}] });
        _self.Init();
    };

};

function buildRTGraph(widget) {
  var chart = new chartRT(widget);
  chart.xText = "";
  chart.yText = "";
  chart.titleText = "";
  chart.Ticks = 100;
  chart.Init();
  chart.addSeries("");
  chart.addSeries("series2");

  $(widget).on("click", function() {
    for (var Name in chart.chartSeries) {
      chart.chartSeries[Name] = Math.random() * 10;
    }
  });

  $(widget).on("mousedown", function(e) {
    if (e.button == 2) {
      $('.focus').removeClass('focus');
      $(widget).addClass('focus');

      context.attach('.focus', [
        {text: 'Resize Chart X',
          action: chart.resizeChartX},
        {text: 'Resize Chart Y',
          action: chart.resizeChartY},
        {text: 'Delete Chart',
          action: removeWidget}
        ]);
    };
  });

  function removeWidget() {
    $('.focus').remove();
  };
};

exports.init = function(_window) {

  $ = _window.$;
  d3 = _window.d3;
  nv = _window.nv;
  window = _window;
  document = _window.document;

  context.init({compress: true});
  context.attach('.container-fluid', [
    {
      text: 'Add Widget...',
      action: addWidget
    },
    {divider: true},
    {header: 'Add Sensor'},
    {text: 'Graphical Sensor'},
    {text: 'Text Sensor'},
    {header: 'Add Control Feedback'}
  ]);


  
  bindMouse();
  
  function addWidget() {
    var newWidget = $("<div></div>");
    $(".container-fluid").append(newWidget);
    $(newWidget).addClass("widget");
    halfWidth = $(newWidget).width()/2;
    halfHeight = $(newWidget).height()/2;
    $(newWidget).css("left",mouseX - halfWidth);
    $(newWidget).css("top",mouseY - halfHeight);
    $(newWidget).draggable({containment: "parent"});

    $(".container-fluid").bind("mousemove", function(event) {
      mouseX = event.pageX;
      mouseY = event.pageY;
      if (mouseX - halfWidth <= 0) {
        $(newWidget).css("left", 0);
      } else if (mouseX + halfWidth >= $(".container-fluid").width()) {
        $(newWidget).css("left", $(".container-fluid").width() - $(newWidget).width());
      } else {        
        $(newWidget).css("left", mouseX - halfWidth);
      }
      if (mouseY - halfHeight <= 0) {
        $(newWidget).css("top", 0);
      } else if (mouseY + halfHeight >= $(".container-fluid").height()) {
        $(newWidget).css("top", $(".container-fluid").height() - $(newWidget).height());
      } else {        
        $(newWidget).css("top", mouseY - halfHeight);
      }
    });

    $(".container-fluid").bind("mousedown", function(event) {
      if (event.which == 3) {
        $(newWidget).remove();
      }
      $(".container-fluid").unbind("mousemove");
      $(".container-fluid").unbind("mousedown");
      bindMouse();

      buildRTGraph(newWidget);
    });
  }

};
