<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<title><? loc PAGE_TITLE ?></title>
<link rel="stylesheet" type="text/css" href="/fixed/css/create-rss.css" media="all" />
<script src="/static/js/elements.js" type="text/javascript"></script>
<script src="/static/js/<? lang ?>/crss.js" type="text/javascript"></script>
</head>

<body onload="pm.onload()">

<form id="f">
<table id="skeleton">

  <tr><td id="topbar">

    <table>
    <tr><td id="page_name"><? loc PAGE_NAME ?> </td>
    <td id="right_menu"><a href="/<? lang ?>?q=<? param q 
?>&amp;p=<? param p ?>&amp;c=<? param c ?>&amp;i=<? param i 
?>&amp;l=<? param l ?>"><? loc NEW_SEARCH ?></a></td>
    </tr>
  </table>

  </td></tr>

  <tr><td id="main_area" align="center" valign="top">
    <table id="msg_count">
    <tr><td>
    <? loc ITEMS_COUNT ?> 
    <INPUT type="text" value="20" name="mc" id="mc" maxlength="3"
           onkeypress="return pm.on_key_press(event)" />
    <INPUT type="hidden" value="http://<? conf endpoint ?>/search/<? lang 
?>?q=<? param q ?>&t=r&n=<? param n ?>&y=<? param y ?>&i=<? param i ?>&e=<? 
param e ?>&u=<? param u ?>&a=<? param a ?>" 
           name="rq" id="rq"/>
    </td></tr>
<!--    <tr><td id="d"></td></tr> -->
    <tr><td id="o"></td></tr>
    </table>
  </td></tr>

  <tr><td id="copyright" valign="bottom"><? loc COPYRIGHT 
?> <a href="/static/<?lang?>/contact"><? loc CONTACT ?></a></td></tr>

</table>
</form>

</body>
</html>
