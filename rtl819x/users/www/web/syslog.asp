<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_7_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="util_gw.js"> </script>
<script>
function initial(){
	show_banner(1);
	show_menu(5,6,1);
	show_footer();
	showclock();
	showbootTime();
}

function showclock(){
	JS_timeObj.setTime(systime_millsec);
	systime_millsec += 1000;
	
	JS_timeObj2 = JS_timeObj.toString();	
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  checkTime(JS_timeObj.getHours()) + ":" +
				  checkTime(JS_timeObj.getMinutes()) + ":" +
				  checkTime(JS_timeObj.getSeconds()) + "  " +
				  JS_timeObj.getFullYear() + " GMT" +
				  timezone;
	$("system_time").value = JS_timeObj2;
	setTimeout("showclock()", 1000);
	if(navigator.appName.indexOf("Microsoft") >= 0)
		document.getElementById("textarea").style.width = "650";
/*
	$("system_time").value=showtime;
	setTimeout("showclock()", 1000);
	if(navigator.appName.indexOf("Microsoft") >= 0)
		document.getElementById("textarea").style.width = "650";
*/
}

function showbootTime(){

	Days = Math.floor(boottime / (60*60*24));	
	Hours = Math.floor((boottime / 3600) % 24);
	Minutes = Math.floor(boottime % 3600 / 60);
	Seconds = Math.floor(boottime % 60);
	
	$("boot_days").innerHTML = Days;
	$("boot_hours").innerHTML = Hours;
	$("boot_minutes").innerHTML = Minutes;
	$("boot_seconds").innerHTML = Seconds;
	boottime += 1;
	setTimeout("showbootTime()", 1000);
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="apply.cgi" >
<input type="hidden" name="current_page" value="syslog.asp">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% getInfo("preferred_lang"); %>">
</form>

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>
    	<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div><br/>
			
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top" >
					<table width="95%" border="0" align="center" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle">
						<thead>
						<tr>
							<td><#menu5_7#> - <#menu5_7_2#></td>
						</tr>
						</thead>
<tbody>
						<tr>
							<td bgcolor="#FFFFFF">
								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
									<tr>
										<th width="20%"><#General_x_SystemTime_itemname#></th>
										<td>
											<input type="text" id="system_time" name="system_time" size="40" class="devicepin" value="" readonly="1" style="font-size:12px;">											
										</td>										
									</tr>
									<tr>
										<th><!--a class="hintstyle" href="javascript:void(0);" onClick="openHint(12, 1);"--><#General_x_SystemUpTime_itemname#></a></th>
										<td><span id="boot_days"></span> <#Day#> <span id="boot_hours"></span> <#Hour#> <span id="boot_minutes"></span> <#Minute#> <span id="boot_seconds"></span> <#Second#></td>
									</tr>
								</table>
							</td>
						</tr>
<tr>
  <td align="center" bgcolor="#FFFFFF">
	<textarea cols="63" rows="14" wrap="off" readonly="readonly" id="textarea" style="width:98%; font-family:'Courier New', Courier, mono; font-size:11px;" ><% sysLogList(); %></textarea>
  </td>
</tr>
						</tbody>
</table>

<table width="300" border="0" align="right" cellpadding="2" cellspacing="0">

				<tr align="center">
					<td>
<form method="post" name="form1" action="apply.cgi">
<input type="hidden" name="current_page" value="/syslog.asp">
<input type="hidden" name="action_mode" value=" Clear ">
<input type="hidden" name="next_host" value="">
<input type="submit" onClick="document.form1.next_host.value = location.host; onSubmitCtrl(this, ' Clear ')" value="<#CTL_clear#>" class="button">
</form>
					</td>
					
					<td>
<form method="post" name="form2" action="syslog.cgi">
<input type="hidden" name="next_host" value="">
<input type="submit" onClick="document.form2.next_host.value = location.host; onSubmitCtrl(this, ' Save ')" value="<#CTL_onlysave#>" class="button">
</form>
					</td>
					
					<td>
<form method="post" name="form3" action="apply.cgi">
<input type="hidden" name="current_page" value="syslog.asp">
<input type="hidden" name="action_mode" value=" Refresh ">
<input type="hidden" name="next_host" value="">
<input type="button" onClick="location.href=location.href" value="<#CTL_refresh#>" class="button">

</form>
					</td>
				</tr>


</table>

		</td>

        </tr>
      </table>		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

<form name="hint_form"></form>

</body>
</html>


