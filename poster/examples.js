

function compile_on_godbolt(example_id) {

  let src = $('#' + example_id).attr("rawsrc");

  let url = `https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,source:'${src}'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:50,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g112,filters:(b:'0',binary:'1',commentOnly:'0',demangle:'0',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:hdf5,ver:'1121'),(name:highfive,ver:trunk)),options:'',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+11.2+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),k:50,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4`;
  window.open(url);

}

function setup_examples() {
  $(".example").each(function () {
    let cblock = this;
    let file = this.id + '.' + this.dataset.lang;
    $.ajax( {url: file,
      dataType: 'text',
      success: function( code ) {
        res = hljs.highlight(code, {language: cblock.dataset.lang, ignoreIllegals: true });
        cblock.innerHTML += res.value;
        let encoded = encodeURIComponent(code);
        cblock.setAttribute("rawsrc",encoded);
      }});

  });

  $(".godbolt").each(function(idx, el) {
    let example_id = el.id.substring(3);
    el.addEventListener("click", function () {
      compile_on_godbolt(example_id);
    });
  });

}


setup_examples();
