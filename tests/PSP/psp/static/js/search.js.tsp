function show_media(msg_id, visibility)
{
  var media_block = document.getElementById("md_" + msg_id);

  if(media_block !== undefined)
  {
    media_block.className = 
      visibility === 0 ? "with_images_0" : "with_images_1";

    var vis_link = document.getElementById("shm_" + msg_id);

    if(vis_link !== undefined)
    {
      vis_link.innerHTML = visibility === 0 ? "<? loc SHOW ?>" : "<? 
loc HIDE ?>";
      vis_link.href = "javascript: show_media('" + msg_id + "', " + 
                      (visibility === 0 ? 1 : 0) + ");";
    }
    
  }
}

function set_focus_next_link()
{
  var elems = document.getElementsByName("next_link");

  if(elems !== undefined && elems.length > 0)
  {
    elems[0].focus();
  }
  else
  {
    elems = document.getElementsByName("new_search");

    if(elems !== undefined && elems.length > 0)
    {
      elems[0].focus();
    }
  }
}

function change_language(ref)
{
  window.location = page_url + "&n=" + ref.options[ref.selectedIndex].id + "s";
}

function change_country(ref)
{
  window.location = page_url + "&y=" + ref.options[ref.selectedIndex].id + "s";
}
