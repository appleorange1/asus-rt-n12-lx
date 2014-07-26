<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<script>
<% get_changed_status(); %>
<% get_printer_info(); %>
<% wanlink(); %>

var flag = '<% get_parameter("flag"); %>';

var manually_stop_wan = '';
var ifWANConnect = "<% detect_if_wan(); %>";
var wan_status_log = "<% get_wan_status_log(); %>";
var detect_dhcp_pppoe = ""; 
var detect_wan_conn = "";
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var wan_ipaddr_t = '';

var qos_global_enable = '';
var qos_userdef_enable = '';
var qos_ubw = '';
var qos_manual_ubw = '';
var qos_ready = 1;
if((qos_global_enable == '1' || qos_userdef_enable == '1')
		&& qos_ubw == '0'
		&& (qos_manual_ubw == '0' || qos_manual_ubw == ''))
	qos_ready = 0;

function initial(){
	if(flag == "initial")
		parent.initial_change_status(manually_stop_wan,
                                 ifWANConnect,
                                 wanlink_statusstr(),
                                 detect_dhcp_pppoe,
                                 wan_status_log,
                                 get_disk_status_changed(),
                                 get_mount_status_changed(),																 
                                 printer_models()[0],
                                 wireless,
                                 qos_ready,
																 detect_wan_conn,
																 wan_ipaddr_t	
																 );
	else
		parent.set_changed_status(manually_stop_wan,
                              ifWANConnect,
                              wanlink_statusstr(),
                              detect_dhcp_pppoe,
                              wan_status_log,
                              get_disk_status_changed(),
                              get_mount_status_changed(),															
                              printer_models()[0],
                              wireless,
														  detect_wan_conn,
														  wan_ipaddr_t
															);
	
	parent.check_changed_status(flag);
}
</script>
</head>

<body onload="initial();">
</body>
</html>
