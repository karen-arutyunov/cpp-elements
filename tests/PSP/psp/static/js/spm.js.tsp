//
// Expression token context
//

NG_ExpressionTokenContext.prototype = 
{ type : null, 
  pos : 0, 
  negated : false,
  nested : 0,

  dump : function()
  {
    var result;

    with(this)
    {
      result = "type " + type + ", pos " + pos + ", negated " + negated + 
               ", nested " + nested;
    }
  
    return result;
  }

};

function NG_ExpressionTokenContext(type, pos, negated, nested)
{
  if(typeof type == "object")
  {
    // Copy constructor

    this.type = type.type;
    this.pos = type.pos;
    this.negated = type.negated;
    this.nested = type.nested;
  }
  else
  {
    this.type = type !== undefined ? type : null;
    this.pos = pos !== undefined ? pos : 0;
    this.negated = negated !== undefined ? negated : false;
    this.nested = nested !== undefined ? nested : 0;
  }
}

//
// Expression token
//

NG_ExpressionToken.prototype = 
{
  text : null,
  type : null,
  range: { start : 0, end : 0},
  quoted : false,
  completed : true,
  context : null,

  dump : function()
  {
    var result;
  
    with(this)
    {
      result = text + ", type " + type + ", start " + range.start + ", end " + 
               range.end + ", quoted " + quoted + ", completed " + completed + 
               ", context: " + (context == null ? "null" : context.dump());
    }
    
    return result;
  },
  
  operators_can_follow : function(filter)
  {
    var operators = new Array();
  
    if(filter === undefined)
    {
      filter = "";
    }
  
    with(this)
    {
      var operator_exclusion = 
      [ NG_ExpressionToken.TT_NOT,
        NG_ExpressionToken.TT_ALL, 
        NG_ExpressionToken.TT_ANY,
        NG_ExpressionToken.TT_SITE,
        NG_ExpressionToken.TT_URL,
        NG_ExpressionToken.TT_OPEN 
      ];
  
      if(context.nested == 0)
      {
        operator_exclusion[operator_exclusion.length] = 
          NG_ExpressionToken.TT_CLOSE;
      }
  
      var operation_exclusion = 
      [ NG_ExpressionToken.TT_NOT,
        NG_ExpressionToken.TT_AND,
        NG_ExpressionToken.TT_COUNTRY,
        NG_ExpressionToken.TT_DOMAIN,
        NG_ExpressionToken.TT_EXCEPT,
        NG_ExpressionToken.TT_FEED,
        NG_ExpressionToken.TT_FRESH,
        NG_ExpressionToken.TT_LANG,
        NG_ExpressionToken.TT_OR,
        NG_ExpressionToken.TT_OUTDATED,
        NG_ExpressionToken.TT_CLOSE
      ];
  
      switch(context.type)
      { 
      case NG_ExpressionToken.TT_COUNTRY:
      case NG_ExpressionToken.TT_DOMAIN:
      case NG_ExpressionToken.TT_FEED:
      case NG_ExpressionToken.TT_LANG:
        {
          if(context.pos == 0)
          {
            if(NG_ExpressionToken.TT_NOT.substr(0, filter.length) == filter)
            {
              operators = [ NG_ExpressionToken.TT_NOT ];
            }
          }
          else if(context.pos > (context.negated == true ? 1 : 0))
          {
            operators = NG_ExpressionToken.special_words(filter, 
                                                         operator_exclusion);
          }
      
          break;
        }
      case NG_ExpressionToken.TT_ALL:
      case NG_ExpressionToken.TT_ANY:
      case NG_ExpressionToken.TT_SITE:
      case NG_ExpressionToken.TT_URL:
      case NG_ExpressionToken.TT_FRESH:
      case NG_ExpressionToken.TT_OUTDATED:
        {
          if(context.pos > 0)
          {
            operators = NG_ExpressionToken.special_words(filter, 
                                                         operator_exclusion);
          }
  
          break;
        }
      case NG_ExpressionToken.TT_CLOSE:
        {
          operators = NG_ExpressionToken.special_words(filter, 
                                                       operator_exclusion);
          break;
        }
      case NG_ExpressionToken.TT_OPEN:
      case NG_ExpressionToken.TT_AND:
      case NG_ExpressionToken.TT_OR:
      case NG_ExpressionToken.TT_EXCEPT:
        {
          operators = NG_ExpressionToken.special_words(filter, 
                                                       operation_exclusion);
          break;
        }
      }
    }
  
    return operators;
  }
};
  
NG_ExpressionToken.TT_NONE = null;
NG_ExpressionToken.TT_REGULAR = "REGULAR";
NG_ExpressionToken.TT_ALL = "ALL";
NG_ExpressionToken.TT_ANY = "ANY";
NG_ExpressionToken.TT_SITE = "SITE";
NG_ExpressionToken.TT_URL = "URL";
NG_ExpressionToken.TT_AND = "AND";
NG_ExpressionToken.TT_EXCEPT = "EXCEPT";
NG_ExpressionToken.TT_OR = "OR";
NG_ExpressionToken.TT_OPEN = "(";
NG_ExpressionToken.TT_CLOSE = ")";
NG_ExpressionToken.TT_FRESH = "FRESH";
NG_ExpressionToken.TT_LANG = "LANGUAGE";
NG_ExpressionToken.TT_COUNTRY = "COUNTRY";
NG_ExpressionToken.TT_FEED = "FEED";
NG_ExpressionToken.TT_DOMAIN = "DOMAIN";
NG_ExpressionToken.TT_OUTDATED = "OUTDATED";
NG_ExpressionToken.TT_NOT = "NOT";

NG_ExpressionToken.SPECIAL_WORD_TYPE =
[ NG_ExpressionToken.TT_ALL,
  NG_ExpressionToken.TT_AND,
  NG_ExpressionToken.TT_ANY,
  NG_ExpressionToken.TT_COUNTRY,
  NG_ExpressionToken.TT_DOMAIN,
  NG_ExpressionToken.TT_EXCEPT,
  NG_ExpressionToken.TT_FEED,
  NG_ExpressionToken.TT_FRESH,
  NG_ExpressionToken.TT_LANG,
  NG_ExpressionToken.TT_NOT,
  NG_ExpressionToken.TT_OR,
  NG_ExpressionToken.TT_OUTDATED,
  NG_ExpressionToken.TT_SITE,
  NG_ExpressionToken.TT_URL,
  NG_ExpressionToken.TT_OPEN,
  NG_ExpressionToken.TT_CLOSE
]; 

NG_ExpressionToken.SPECIAL_WORD = 
[ "ALL",
  "AND",
  "ANY",
  "COUNTRY",
  "DOMAIN",
  "EXCEPT",
  "FEED",
  "FRESH",
  "LANGUAGE",
  "NOT",
  "OR",
  "OUTDATED",
  "SITE",
  "URL",
  "(",
  ")"
];

function NG_ExpressionToken(text, 
                            type, 
                            range, 
                            quoted, 
                            completed,
                            context)
{
  this.text = text;
  this.type = type;  
  this.range = range !== undefined ? range : { start : 0, end : 0};
  this.quoted = quoted !== undefined ? quoted : false;
  this.completed = completed !== undefined ? completed : true;
  this.context = context !== undefined ? context : null;
}

NG_ExpressionToken.special_words = function(prefix, exclusion)
{
  var result = new Array();

  with(this)
  {
    for(var i = 0; i < NG_ExpressionToken.SPECIAL_WORD.length; i++)
    {
      var word = SPECIAL_WORD[i];

      if(exclusion)
      {
        var j = 0;
        for(; j < exclusion.length && exclusion[j] != word; j++);

        if(j < exclusion.length)
        {
          continue;
        }
      }

      if(word.substr(0, prefix.length) == prefix)
      { 
        result[result.length] = word;
      }
    }
  }

  return result;
}

NG_ExpressionToken.begin_operators = function()
{
  var operators =
  [ NG_ExpressionToken.TT_ALL,
    NG_ExpressionToken.TT_ANY,
    NG_ExpressionToken.TT_SITE,
    NG_ExpressionToken.TT_URL,
    NG_ExpressionToken.TT_OPEN
  ];

  return operators;
}

//
// Expression parser
//

NG_ExpressionParser.prototype = 
{ 
  istr : null,
  last_token : null,
  last_token_type : null,
  last_token_begins : 0,
  last_token_quoted : false,
  last_token_completed : true,

  read_token : function() 
  { 
    with(this)
    {
      last_token = "";
      last_token_type = NG_ExpressionToken.TT_NONE;
      last_token_begins = 0;
      last_token_quoted = false;
      last_token_completed = true;
   
      var chr;
      while((chr = istr.get_char()) != null)
      {
        if(!el_isspace(chr))
        {
          istr.putback();
          break;
        }
      }      
  
      var reading_token = !istr.fail;
  
      if(reading_token)
      {
        last_token_begins = istr.position;
  
        if(chr == '"')
        {
          istr.get_char(); // skip quote
  
          last_token_quoted = true;
            
          var prev_chr = null;
  
          for(chr = istr.get_char(); chr != null; chr = istr.get_char())
          {
            if(prev_chr == '"' && el_isspace(chr))
            {
              istr.putback();
              break;
            }
                  
            last_token = last_token.concat(chr);
            prev_chr = chr;
          }
  
          if(prev_chr != '"')
          {
            // Incompleted quoted token
  
            last_token_type = NG_ExpressionToken.TT_REGULAR;
            last_token_completed = false;
  
            return last_token_type;
          }
  
          last_token = last_token.substr(0, last_token.length - 1);
        }
        else
        {
          for(chr = istr.get_char(); chr != null; chr = istr.get_char())
          {
            if(el_isspace(chr))
            {
              istr.putback();
              break;
            }
                
            last_token = last_token.concat(chr);
          }
        }
      }
  
      if(!istr.fail || istr.eof && reading_token)
      {
        if(!last_token_quoted)
        {
          for(var i = 0; i < NG_ExpressionToken.SPECIAL_WORD.length; i++)
          {
            if(last_token == NG_ExpressionToken.SPECIAL_WORD[i])
            {
              last_token_type = NG_ExpressionToken.SPECIAL_WORD_TYPE[i];
              return last_token_type;
            }
          }
        }
          
        last_token_type = NG_ExpressionToken.TT_REGULAR;
        return last_token_type;
      }
  
      if(!istr.eof)
      {
        // Reading stream failed somewhy
        throw new Error("Reading stream failed");
      }
  
      last_token_begins = istr.position;
      return last_token_type;
    }
  
  }, // end of NG_ExpressionParser.prototype.read_token function

  parse : function(pos)
  { 
    var tokens = new Array();

    with(this)
    {
      var text_length = istr.string.length;

      var token_type;
      var last_operator = NG_ExpressionToken.TT_NONE;
      var last_token_end = null;
  
      var token_context = 
        new NG_ExpressionTokenContext(NG_ExpressionToken.TT_ALL, 1);
  
      while(true)
      {
        var chr;
        while(istr.position < pos && (chr = istr.get_char()) != null)
        {
          if(!el_isspace(chr))
          {
            istr.putback();
            break;
          }
        }
  
        if(istr.position >= pos || (token_type = read_token()) == null)
        {
          break;
        }
  
        last_token_end = istr.position;
  
        switch(token_type)
        {
        case NG_ExpressionToken.TT_NOT: 
          {
            token_context.negated = true;
            break;
          }
        case NG_ExpressionToken.TT_REGULAR: 
          {
            // Leave context type as is
            break;
          }
        default: 
          {
            token_context.type = token_type; 
            token_context.pos = 0;
            token_context.negated = false;
  
            break;
          }
        }
  
        if(token_type == NG_ExpressionToken.TT_OPEN)
        {
          token_context.nested++;
        }
  
        if(token_type == NG_ExpressionToken.TT_CLOSE && 
           token_context.nested > 0)
        {
          token_context.nested--;
        }
  
        var context = new NG_ExpressionTokenContext(token_context);
  
        var token = 
          new NG_ExpressionToken(
            last_token, 
            token_type, 
            { start : last_token_begins, end : last_token_end },
            last_token_quoted,
            last_token_completed,
            context);
  
        tokens[tokens.length] = token;
  
        switch(token_type)
        {
        case NG_ExpressionToken.TT_AND: 
        case NG_ExpressionToken.TT_OR: 
        case NG_ExpressionToken.TT_EXCEPT: 
        case NG_ExpressionToken.TT_OPEN:
        case NG_ExpressionToken.TT_CLOSE:
          {
            token_context.type = NG_ExpressionToken.TT_ALL; 
            token_context.pos = 1;
            token_context.negated = false;
            break;
          }
        default: 
          {
            token_context.type = context.type; 
            token_context.pos++; 
            break;
          }
        }
  
      }
    }
  
    return tokens;
  }
} // end of NG_ExpressionParser.prototype.parse function

function NG_ExpressionParser(string)
{
  with(this)
  {
    istr = new El_InputStringStream(string);
    last_token = null;
    last_token_type = null;
    last_token_begins = 0;
    last_token_quoted = false;
    last_token_completed = true;
  }
}

//
// Item
//

Item.prototype = 
{
  type : null,
  id : null,
  value : null,
  suffix : null,

  dump : function()
  {
    var result;

    with(this)
    {
      result = "type " + type + ", id " + id + ", value " + value + 
               ", suffix " + suffix;
    }
  
    return result;    
  }
};

function Item(type, id, value, suffix)
{
  this.type = type !== undefined ? type : null;  
  this.id = id !== undefined ? id : null;
  this.value = value !== undefined ? value : null;
  this.suffix = suffix !== undefined ? suffix : null;
}

//
// Item Manager
//

ItemManager.prototype = 
{
  url : undefined,
  items : null,
  request : null,
  handler : null,

  dump : function()
  {
    if(items === null)
    {
      return "Not loaded";
    }

    var result = "";

    with(this)
    {
      for(var i = 0; i < items.length; i++)
      {
        result += items[i].dump() + "\n";
      }
    } 

    return result;
  },

  ready : function()
  {
    return this.items !== null;
  },

  init : function()
  {
    with(this)
    {
      if(initialized())
      {
        return;
      }

      try
      {
        request = new ActiveXObject("Msxml2.XMLHTTP");
      }
      catch(e)
      {
        request = new XMLHttpRequest();
      }

      request.open("GET", url, true);
      request.onreadystatechange = onreadystatechange;
      request.send("");
    }
  },

  onload : function()
  {
    with(this)
    {
      items = new Array();

      var item_xml = request.responseXML.getElementsByTagName("item");

      for(var i = 0; i < item_xml.length; i++)
      {
        var node = item_xml[i];

        items[items.length] = 
          new Item(node.getAttribute("type"), 
                   node.getAttribute("id"), 
                   node.getAttribute("value"), 
                   node.getAttribute("suffix"));
      }

      request = null;

      if(handler !== null)
      {
        eval(handler);
        handler = null;
      }
    }
  },

  initialized : function()
  {
    return this.ready() || this.request !== null;
  },

  select_items : function(exclude_items, prefix, all_items)
  {
    var result = new Array();

    if(prefix === undefined)
    {
      prefix = "";
    }

    if(all_items !== undefined && all_items !== null)
    {
      all_items.length = 0;
    }

    with(this)
    {
      for(var i = 0; i < items.length; i++)
      {
        var type = items[i].type;

        if(type !== undefined && type !== null || type == "item")
        { 
          if(all_items !== undefined && all_items !== null)
          {
            all_items[all_items.length] = items[i];
          }

          continue;
        }

        var id = items[i].id;

        if(id === undefined || id === null)
        { 
          result[result.length] = items[i];
          continue;
        }

        if(id.substr(0, prefix.length) == prefix)
        { 
          var j = 0;
          for(; j < exclude_items.length && exclude_items[j] != id; j++);

          if(j < exclude_items.length)
          {
            continue;
          }

          result[result.length] = items[i];

          if(all_items !== undefined && all_items !== null)
          {
            all_items[all_items.length] = items[i];
          }
        }
      }
    }

    return result;
  },

  onload_handler : function(handler)
  {
    this.handler = handler;
  }
};

function ItemManager(url)
{
  this.url = url;
  this.items = null;
  this.request = null,
  this.handler = null;
}

//
// Country manager
//

CountryManager.prototype =
   new ItemManager("/static/xml/<? lang ?>/countries.xml");

CountryManager.prototype.onreadystatechange = function()
{
  if(country_manager.request.readyState == 4)
  {
    country_manager.onload();
  }  
}

function CountryManager()
{  
}

//
// Language manager
//

LanguageManager.prototype =
   new ItemManager("/static/xml/<? lang ?>/languages.xml");

LanguageManager.prototype.onreadystatechange = function()
{
  if(language_manager.request.readyState == 4)
  {
    language_manager.onload();
  }  
}

function LanguageManager()
{  
}

//
// FeedKind manager
//

FeedKindManager.prototype =
   new ItemManager("/static/xml/<? lang ?>/feed_kinds.xml");

FeedKindManager.prototype.onreadystatechange = function()
{
  if(feed_kind_manager.request.readyState == 4)
  {
    feed_kind_manager.onload();
  }
}

function FeedKindManager()
{
}

//
// Domain manager
//

DomainManager.prototype =
   new ItemManager("/static/xml/<? lang ?>/domains.xml");

DomainManager.prototype.onreadystatechange = function()
{
  if(domain_manager.request.readyState == 4)
  {
    domain_manager.onload();
  }  
}

function DomainManager()
{  
}

//
// Site manager
//

SiteManager.prototype =
   new ItemManager("/static/xml/<? lang ?>/sites.xml");

SiteManager.prototype.onreadystatechange = function()
{
  if(site_manager.request.readyState == 4)
  {
    site_manager.onload();
  }  
}

function SiteManager()
{  
}

//
// Url manager
//

UrlManager.prototype =
   new ItemManager("/static/xml/<? lang ?>/urls.xml");

UrlManager.prototype.onreadystatechange = function()
{
  if(url_manager.request.readyState == 4)
  {
    url_manager.onload();
  }  
}

function UrlManager()
{  
}

//
// Creating singletons
//

var country_manager = new CountryManager(); 
var language_manager = new LanguageManager(); 
var feed_kind_manager = new FeedKindManager(); 
var domain_manager = new DomainManager();
var site_manager = new SiteManager();
var url_manager = new UrlManager();

//
// Page Manager
//

PageManager.prototype = 
{
  query_box : null,
  output : null,
  debug : null,
  current_condition : 0,
  browser : null,
  os : null,

  initialized : function()
  {
    return this.query_box !== null;
  },

  onload : function(pos)
  {
    with(this)
    {
      browser="<? browser ?>";
      os="<? os ?>";

      if(browser == "opera" || os != "windows")
      {
        var hot_key = document.getElementById("hk");

        if(hot_key != undefined)
        {
          hot_key.innerHTML = "(Ctrl+x)";
        }
      }

      query_box = el_enrich(document.getElementById("q"));
      output = document.getElementById("o");
      debug = document.getElementById("d");

      set_focus_query_box();

      if(pos !== undefined)
      {
        if(pos < 0)
        {
          query_box.el_set_selection({ start: 0, 
                                       end: query_box.value.length });
        }
        else
        {
          query_box.el_set_selection({ start: pos - 1, end: pos - 1});
        }
      }

    }
  },

  set_focus_query_box : function()
  {
    with(this)
    {
      if(initialized())
      {
        query_box.focus();
      }
    }
  },

  insert : function(text, reprompt, range)
  {
    with(this)
    {
      query_box.el_insert(text, range);  

      if(reprompt === 1)
      {
        window.setTimeout("pm.show_prompt(" + this.current_condition + ")", 0);
      }
      else
      {
        output.innerHTML = "";
      }

    }
  },

  insert_link : function(operator, op_suffix, range, desc)
  {
    var str = " <a href=\"javascript: pm.insert('" + operator + " ', 1";

    if(range !== undefined)
    {
      str += ", { start: " + range.start + ", end: " + range.end + "}";
    }
          
    str += ")\">" + operator + "</a>" + op_suffix;

    if(desc !== undefined && desc !== null)
    {
      str += " " + desc;
    }

    return str;
  },
  
  produce_compound_prompt : function(parameters, ins_range)
  {
    var prompt = "";
    var id_prefix = "";
    var id_suffix = "";
    var value_suffix = "";
    var show_other = false;

    with(this)
    {
      for(var i = 0; i < parameters.length; i++)
      {
        var param = parameters[i];

        if(param.type == "output")
        {
          prompt += param.value;
        }
        else if(param.type == "id-pefix")
        {
          id_prefix = param.value;
        }
        else if(param.type == "id-suffix")
        {
          id_suffix = param.value;
        }
        else if(param.type == "value-suffix")
        {
          value_suffix = param.value;
        }
        else if(param.type == "other")
        {
          if(show_other)
          {
            prompt += param.value;
            show_other = false;
          }
        }
        else
        {
          prompt += id_prefix;
          prompt += insert_link(param.id, 
                                id_suffix,
                                ins_range,
                                param.value);

          if(param.suffix !== undefined && param.suffix !== null)
          { 
            prompt += param.suffix;
          }
          else
          {
            prompt += value_suffix;
          }

          show_other = true;
        }
      }
    }

    return prompt;
  },

  on_click : function(event)
  {
    with(this)
    {
      if(initialized())
      {
        output.innerHTML = "";
        current_condition++;
      }
    }
  },
  
  on_key_press : function(event)
  {
    with(this)
    {
      if(!initialized())
      {
        return true;
      }

      if(event.keyCode == 13)
      {
        document.getElementById("f").submit();
        return false;
      }

      return true;
    }
  },
  
  on_key_down : function(event)
  {
    with(this)
    {
      switch(event.keyCode)
      {
        case 16: // Shift
        case 17: // Ctrl
        case 18: return true; // Alt
      }

      if(!initialized())
      {
        return true;
      }

      output.innerHTML = "";    
      current_condition++;

      if(browser == "opera" || os != "windows")
      {
        if(event.ctrlKey == false || event.keyCode != 88)
        {
          return true;
        }
      }
      else
      {
        if(event.altKey == false || event.keyCode != 49)
        {
         return true;
        }
      }

      show_prompt();
      return false;
    }
  },

  show_help : function()
  {
    with(this)
    {
      if(!initialized())
      {
        return true;
      }
  
      output.innerHTML = "";
      current_condition++;

      set_focus_query_box();

      show_prompt();
    }
  },

  select_items : function(item_manager, exclude_items, prefix, all_items)
  {
    if(!item_manager.initialized())
    {
      item_manager.init();
    }

    if(prefix === undefined)
    {
      prefix = "";
    }

    if(item_manager.ready())
    {
      parameters = item_manager.select_items(exclude_items, prefix, all_items);
    }
    else
    {
      parameters = undefined;
      item_manager.onload_handler("pm.show_prompt(" + 
                                  this.current_condition + ")");
    }

    return parameters;
  },
    
  get_parameters : function(tokens, cursor_pos, prefix, all_items)
  {
    if(tokens.length == 0)
    {
      throw new Error("Token array should not be empty");
    }

    var last_token = tokens[tokens.length - 1];
    var exclude_items = new Array();

    for(var i = tokens.length - (cursor_pos == last_token.range.end ? 2 : 1); 
        i >= 0 && tokens[i].context.pos > 0; i--)
    {
      exclude_items[exclude_items.length] = tokens[i].text;
    }

    with(this)
    {
      switch(last_token.context.type)
      {
        case NG_ExpressionToken.TT_COUNTRY:
        {
          return select_items(country_manager, exclude_items, prefix, all_items);
        }
        case NG_ExpressionToken.TT_LANG:
        {
          return select_items(language_manager, exclude_items, prefix, all_items);
        }
        case NG_ExpressionToken.TT_FEED:
        {
          return select_items(feed_kind_manager, exclude_items, prefix, all_items);
        }
        case NG_ExpressionToken.TT_DOMAIN:
        {
          return select_items(domain_manager, exclude_items, prefix, all_items);
        }
        case NG_ExpressionToken.TT_SITE:
        {
          return select_items(site_manager, exclude_items, prefix, all_items);
        }
        case NG_ExpressionToken.TT_URL:
        {
          return select_items(url_manager, exclude_items, prefix, all_items);
        }
      }
    }

    return null;
  },

  show_prompt : function(condition_id)
  {
    with(this)
    {
      if(condition_id !== undefined && current_condition != condition_id)
      {
        return;
      }

//      output.innerHTML = "";

      if(debug)
      {
        debug.innerHTML = "";
      }
    
      var sel = query_box.el_get_selection();
      var text = query_box.value.replace(/\r/g, "");
    
      if(debug)
      {
        debug.innerHTML = "Browser: " + browser + "->" + 
                          navigator.userAgent + "<br>" +
                          "Text: " + text + "<br>" +
                          "Selection: " + sel.start + ":" + sel.end + 
                          "(" + text.charAt(sel.start) + ":" +
                          text.charAt(sel.end) + ")<br>";
      }
        
      var last_token = null;
      var prev_token = null;
      var token_auto_completion = false;
    
      if(sel.start == sel.end)
      {
        if(sel.start > 0 && sel.start < text.length && 
           !el_isspace(text.charAt(sel.start - 1)) && 
           !el_isspace(text.charAt(sel.start)))
        {
          insert(" ", 0);
          query_box.el_set_selection(sel);
          text = query_box.value.replace(/\r/g, "");
        }

        var parser = new NG_ExpressionParser(text);
        var tokens = parser.parse(sel.end);
    
        if(tokens.length > 0)
        {
          if(debug)
          {
            for(var i = 0; i < tokens.length; i++)
            {
              var token = tokens[i];
              debug.innerHTML += token.dump() + "<br>";
            }
          }
    
          last_token = tokens[tokens.length - 1];

          token_auto_completion = sel.end == last_token.range.end;
    
          if(tokens.length > 1)
          {
            prev_token = tokens[tokens.length - 2];
          }
        }
      }
      
      if(debug)
      {
        debug.innerHTML += 
          "token_auto_completion " + token_auto_completion + "<br>";
      }
    
      var operators = new Array();
      var parameters = null;
      var all_items = new Array();

      if(token_auto_completion)
      {
        if(prev_token == null)
        {
          var exclusion = 
          [ NG_ExpressionToken.TT_NOT,
            NG_ExpressionToken.TT_AND,
            NG_ExpressionToken.TT_COUNTRY,
            NG_ExpressionToken.TT_DOMAIN,
            NG_ExpressionToken.TT_EXCEPT,
            NG_ExpressionToken.TT_FEED,
            NG_ExpressionToken.TT_FRESH,
            NG_ExpressionToken.TT_LANG,
            NG_ExpressionToken.TT_OR,
            NG_ExpressionToken.TT_OUTDATED,
            NG_ExpressionToken.TT_CLOSE
          ];
          
          operators = 
            NG_ExpressionToken.special_words(last_token.text, exclusion);
        }
        else
        {
          operators = prev_token.operators_can_follow(last_token.text);
        }

        parameters = 
          get_parameters(tokens, sel.end, last_token.text, all_items);

        if(parameters !== undefined && operators.length + 
           (parameters !== null ? parameters.length : 0) == 1)
        {      
          //
          // Parameters loaded and only one completion variant exist
          //

          var full_token = 
            operators.length ? operators[0] : parameters[0].id;
    
          if(full_token != last_token.text)
          {
            insert(full_token + " ", 0, last_token.range);
    
            last_token.range.end = last_token.range.start + full_token.length;
          }
          else
          {
            insert(" ", 0);
          }
    
          // Re-evaluating expression after auto-completion
    
          sel = query_box.el_get_selection();
          text = query_box.value.replace(/\r/g, "");
          parser = new NG_ExpressionParser(text);
          tokens = parser.parse(sel.end);
          last_token = tokens[tokens.length - 1];
    
          operators = new Array();
          parameters = null;

          token_auto_completion = false;
    
          if(debug)
          {
            debug.innerHTML += "Re-evaluating expression<br>";
    
            for(var i = 0; i < tokens.length; i++)
            {
              var token = tokens[i];
              debug.innerHTML += token.dump() + "<br>";
            }
          }
        }
      }

      var operator_specific_prompt = null;
    
      if(!token_auto_completion)
      {
        //
        // Prompting for operator
        //    

        if(last_token == null)
        {
          operators = NG_ExpressionToken.begin_operators();
        }
        else
        {    
          operators = last_token.operators_can_follow();
        }

        if(last_token != null)
        {
          parameters = get_parameters(tokens, sel.end, "", all_items);
        }

      }
    
      //
      // Prompting for operator-specific parameter
      //
    
      var context_type = last_token == null ? 
        NG_ExpressionToken.TT_ALL : last_token.context.type;
    
      switch(context_type)
      { 
      case NG_ExpressionToken.TT_COUNTRY:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_SRC_COUNTRY_CODES ?>" : 
            "<? loc PLS_MSG_SRC_COUNTRY_CODES ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_DOMAIN:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_SRC_DOMAIN_NAMES ?>" : 
            "<? loc PLS_MSG_SRC_DOMAIN_NAMES ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_FEED:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_SRC_KINDS ?>" :
            "<? loc PLS_MSG_SRC_KINDS ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_LANG:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_SRC_LANG_CODES ?>" :
            "<? loc PLS_MSG_SRC_LANG_CODES ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_OPEN:
      case NG_ExpressionToken.TT_EXCEPT:
      case NG_ExpressionToken.TT_AND:
      case NG_ExpressionToken.TT_OR:
      case NG_ExpressionToken.TT_ALL:
      case NG_ExpressionToken.TT_ANY:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_WORDS ?>" : "<? loc PLS_MSG_WORDS ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_SITE:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_SRC_SITES ?>" :
            "<? loc PLS_MSG_SRC_SITES ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_URL:
        {
          operator_specific_prompt = operators.length ? 
            "<? loc MSG_SRC_URLS ?>" : "<? loc PLS_MSG_SRC_URLS ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_FRESH:
        {
          if(last_token.context.pos)
          {
            break;
          }
    
          operator_specific_prompt = operators.length ? 
            "<? loc MAX_MSG_AGE ?>" :
            "<? loc PLS_MAX_MSG_AGE ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_OUTDATED:
        {
          if(last_token.context.pos)
          {
            break;
          }
    
          operator_specific_prompt = operators.length ?
            "<? loc MIN_MSG_OUTDATE ?>" :
            "<? loc PLS_MIN_MSG_OUTDATE ?>";
    
          break;
        }
      case NG_ExpressionToken.TT_CLOSE:
        { 
          // No operator specific prompt
          break;
        }
      }
    
//      output.innerHTML = "";

      var buffer = "";

      if(operators.length)
      {
         buffer += operators.length > 1 ? 
          "<? loc PLS_TYPE_SPEC_WORDS ?>" : "<? loc PLS_TYPE_ONE_SPEC_WORD ?>";
    
        for(var i = 0; i < operators.length; i++)
        {
           buffer +=
            insert_link(operators[i], 
                        "",
                        token_auto_completion ? last_token.range : sel);
                        // : undefined
        }

        buffer += "<br>";
      }

      if(operator_specific_prompt != null)
      {
        buffer += operator_specific_prompt;

        if(parameters !== null)
        {
          if(last_token !== null && token_auto_completion)
          {
            var text = new String(" <? loc APPROP_VALS_BEGIN_WITH ?>");
            buffer += text.replace(/%%PREFIX%%/, last_token.text);
          }
          else
          { 
            buffer += " <? loc APPROP_VALS ?>";
          }
  
          if(parameters === undefined)
          {
            buffer += " <? loc LOADING ?>";
          }
          else
          {
            buffer += "<br>" + 
              produce_compound_prompt(
                all_items, 
                token_auto_completion ? last_token.range : sel);
                // : undefined
          }
        }

      }

      output.innerHTML = buffer;
    
      return false;
    }
  }  
};
  
function PageManager()
{
  with(this)
  {
    query_box = null;
    output = null;
    debug = null;
    current_condition = 0;
    browser = null;
    os = null;
  }
}
  
//
// Creating singleton
//
var pm = new PageManager();
