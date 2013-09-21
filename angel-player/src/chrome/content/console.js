$(document).ready(function(){
     /* Main Console */
     var console = $('<div class="console">');
     $('body').append(console);
      var controller = console.console({
      promptLabel: '>>> ',
      commandValidate:function(line){
          if (line == "") return false;
          else return true;
          },
      commandHandle:function(line){
          return [{msg:"=> [12,42]",
                   className:"jquery-console-message-value"},
                   ]
       },
       autofocus:true,
       animateScroll:true,
       promptHistory:true,
       charInsertTrigger:function(keycode,line){
          return true;
       }
       });
    });
