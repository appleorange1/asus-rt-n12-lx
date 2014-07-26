<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
.Bar_container{
	width:85%;
	height:21px;
	border:2px inset #999;
	margin:0 auto;
	background-color:#FFFFFF;
	z-index:100;
}
#proceeding_img_text{
	position:absolute; z-index:101; font-size:11px; color:#000000; margin-left:110px; line-height:21px;
}
#proceeding_img{
	height:21px;
	background:#C0D1D3 url(/images/proceeding_img.gif);
}
</style>

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="util_gw.js"></script>
<script>
var MWJ_progBar = 0;
var time=0;
var delay_time=1000;
var loop_num=0;
function initial(){
	show_banner(1);
	show_menu(5,5,3);
	show_footer();
	disableCheckChangedStatus();
}

</script>
</head>

<body onload="initial();">

<div id="TopBanner"></div>

<div id="LoadingBar" class="popup_bar_bg" style="background-image: url(/images/popup_bg2.gif);">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
		<td height="80">
		<div class="Bar_container">
			<span id="proceeding_img_text" ></span>
			<div id="proceeding_img"></div>
		</div>
		<div style="margin:5px auto; width:85%;"><#FIRM_ok_desc#></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div><!--for uniform show, useless but have exist-->

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" action="upgrade.cgi" enctype="multipart/form-data" name="form" target="hidden_frame">
<input type="hidden" name="current_page" value="upload.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% getInfo("preferred_lang"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>

		<td valign="top" width="202">
		<div id="mainMenu"></div>
		<div id="subMenu"></div>
		</td>
 
 <td valign="top">
 <div id="tabMenu" class="submenuBlock"></div><br />
 <!--==================================Beginning of Main Content===========================================-->
 <table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
 <tr>
 <td align="left" valign="top" >
 
 <table width="500" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
 <thead>
 <tr>
 <td><#menu5_6#> - <#menu5_6_3#></td>
 </tr>
 </thead> 
 <tr>
 <td bgcolor="#FFFFFF"><#FW_desc1#>
 <ol>
 <li><#FW_desc2#></li>
 <li><#FW_desc3#></li>
 <li><#FW_desc4#></li>
 <li><#FW_desc5#></li>
 <li><#FW_desc6#></li>
 </ol> 
 </td>
 </tr>
 <tr>
 <td bgcolor="#FFFFFF">
 <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
<tr>
  <!--<th>Firmware Version:</th>-->
  <th><#FW_item2#></th>
  <td><% get_fw_version(); %></td>
</tr>

<tr>
  <!--<th>Select File:</th>-->
  <th><#FW_item5#></th>
  <td>
	<input type="file" name="file" class="input" size=20>
  </td>
</tr>

<tr>
  <td colspan="2" align="right">
	<input type="button" name="button" class="button" onclick="onSubmitCtrlOnly(this, 'Upload1');" value="<#CTL_upload#>" />
  </td>
</tr>

<tr>
  <td colspan="2">
  <strong><#FW_note#></strong>
  <ol>
  <li><#FW_n1#></li>
  <li><#FW_n2#></li>
  </ol>
  </td>
</tr>	

</table>

 </td>
 </tr>
 </table>
 </td>
 <td id="help_td" style="width:15px;" valign="top">
 
 <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
 <div id="hintofPM" style="display:none;">
 <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
 <thead>
 <tr>
 <td>
 <div id="helpname" class="AiHintTitle"></div>
 <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
 </td>
 </tr>
 </thead>
 
 <tr> 
 <td valign="top" >
 <div class="hint_body2" id="hint_body"></div>
 <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
 </td>
 </tr>
 </table>
 </div><!--nd of hintofPM-->
 </td>
 </tr>
 </table> 
 <!--==================================Ending of Main Content===========================================--> 
 </td>
 
 <td width="10" align="center" valign="top">&nbsp;</td>
 </tr>
</table>

<div id="footer"></div>
</form>
<form name="hint_form"></form>
</body>
</html>
