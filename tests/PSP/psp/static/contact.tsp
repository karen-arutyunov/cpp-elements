<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<title><? loc PAGE_TITLE ?></title>
<link rel="stylesheet" type="text/css" href="/fixed/css/contact.css" media="all" />
</head>
<body>

<div id="topbar">

    <table>
    <tr><td id="page_name"><? loc PAGE_NAME ?> </td>
    <td id="right_menu"><a href="/"><? loc NEW_SEARCH ?></a></td>
    </tr>
    </table>

</div>

<div id="main_area" align="center">

<div id="email_info"><? loc EMAIL_INFO ?></div>
<div id="feed_back"><? loc FEED_BACK ?></div>

<form id="f" action="/feedback/<? lang ?>" method="post">
<textarea rows="10" cols="80" name="q" id="q"></textarea>
<input type="submit" value="<? loc BUTTON_TEXT ?>" name="b" id="b"/>
<span id="error"><? param e ?></span>
</form>

</div>

<div id="copyright" valign="bottom"><? loc COPYRIGHT ?></div>

<script>
  var e = document.getElementById("q");
  if(e !== undefined) e.focus();
</script>

</body>
</html>
