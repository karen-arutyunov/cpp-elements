//
// Extensions
//

function el_isspace(chr)
{
  return chr == " " || chr == "\t" || chr == "\n" || chr == "\r";
}

function el_isdigit(chr)
{
  return chr.charCodeAt(0) >= String("0").charCodeAt(0) &&
         chr.charCodeAt(0) <= String("9").charCodeAt(0);
}

function el_int_error(str, minval, maxval)
{
  var len = str.length;
  var i = 0;
 
  for(; i < len && el_isspace(str.charAt(i)); i++);

  if(i == len)
  {
    return 0;
  }

  var begin = i;
  var chr;

  for(; i < len && (el_isdigit(chr = str.charAt(i)) || 
      i == begin && (chr == "+" || chr == "-")); i++);

  var end = i;
  for(; i < len && el_isspace(str.charAt(i)); i++);

  if(i != len)
  {
    return i;
  }

  if(minval !== undefined)
  {
    var intval = parseInt(str.substring(begin, end));

    if(intval < minval)
    {
      return begin;
    }

    if(maxval !== undefined && intval > maxval)
    {
      return begin;
    }
  }

  return -1;
}

//
// Simple stream
//

El_InputStringStream.prototype = 
{ 
  string : "",
  position : 0,
  eof : false,
  fail : false,

  dump : function() 
  { 
    with(this)
    {
      return "El_InputStringStream: " + position + " " + eof + " " + fail + 
             " '" + string + "'";
    }
  },

  get_char : function() 
  { 
    with(this)
    {
      if(position >= string.length)
      {
        eof = fail = true;
      }

      return eof ? null : string.charAt(position++); 
    }
  },

  putback : function() 
  { 
    with(this)
    {
      if(!eof && position > 0)
      {
        position--;
      }
    }
  }

};

function El_InputStringStream(string)
{
  this.string = string;
  this.position = 0;
  this.eof = false;
  this.fail = false;
}

//
// Range
//

El_Range.prototype = 
{
  start : 0,
  end : 0,

  dump : function()
  {
    return this.start + ":" + this.end;
  }
};

function El_Range(start, end)
{
  this.start = start === undefined ? 0 : start;
  this.end = end === undefined ? start : end;
}

//
// Selection
//

function el_set_selection_(range)
{
  if(this.selectionStart !== undefined)
  {
    this.selectionStart = range.start;
    this.selectionEnd = range.end;
  }
  else
  {
    var text_range = this.createTextRange();
    text_range.collapse(true);
    text_range.moveStart("character", range.start);
    text_range.moveEnd("character", range.end - range.start);
    text_range.select();     
  }
}

function el_get_selection_()
{    
  var result = new El_Range();
    
  with(this)
  {
    if(this.selectionStart !== undefined)
    {
      result.start = this.selectionStart;
      result.end = this.selectionEnd;
    }
    else
    {
      result.begin = -1;
      result.end = -1;
  
      var caretPos = document.selection.createRange();
  
      if(caretPos)
      {
        var bookmark = "~~@NG!~~";
        var orig = this.value.replace(/\r/g, "");
 
        caretPos.text = bookmark;
        var str = this.value.replace(/\r/g, "");
  
        result.start = str.search(bookmark);

        result.end = 
          result.start + orig.length - str.length + bookmark.length;
  
        this.value = orig;
  
        var range = this.createTextRange();
        range.collapse(true);
        range.moveStart("character", result.start);
        range.moveEnd("character", result.end - result.start);
        range.select();
      }
    }
  }
  
  return result;
}
  
function el_insert_(text, range)
{
  this.focus();

  if(range !== undefined)
  {
    this.el_set_selection(range);
  }

  var sel = this.el_get_selection();

  if(this.selectionStart !== undefined)
  {
    var val = this.value;      
    this.value = val.substr(0, sel.start) + text + val.substr(sel.end);
  }
  else
  {
    var caretPos = document.selection.createRange();
 
    if(caretPos)
    {
      caretPos.text = text;
    }
  }

  var new_sel = new El_Range(sel.start + text.length);
  this.el_set_selection(new_sel);
}

//
// Enrich objects
//

function el_enrich(object)
{
  if(object !== null)
  {
    if(object.nodeName == "TEXTAREA" || 
       object.nodeName == "INPUT" && object.type == "text")
    {
      el_enrich_edit(object);
    }
  }
  
  return object;
}

function el_enrich_edit(object)
{
  object.el_set_selection = el_set_selection_;
  object.el_get_selection = el_get_selection_;
  object.el_insert = el_insert_;
}
