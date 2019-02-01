String GetDef_DR()
{
  return String( disp.Red( settings.defcolour_d ) );
}

String GetDef_DG()
{
  return String( disp.Green( settings.defcolour_d ));
}

String GetDef_DB()
{
  return String( disp.Blue( settings.defcolour_d ));
}

String GetDef_CR()
{
  return String( disp.Red( settings.defcolour_c ));
}

String GetDef_CG()
{
  return String( disp.Green( settings.defcolour_c ));
}

String GetDef_CB()
{
  return String( disp.Blue( settings.defcolour_c ) );
}


String IsGMT( float val )
{
  if ( settings.gmt == val )
    return " selected";
  else
    return "";  
}

String IsStartDay( int v )
{
  if ( settings.daylightstart_d == v )
    return " selected ";
  else
    return "";
}

String IsStartMonth( int v )
{
  if ( settings.daylightstart_m == v )
    return " selected ";
  else
    return "";
}

String IsStartTime( int v )
{
  if ( settings.daylightstart_t == v )
    return " selected ";
  else
    return "";
}

String IsEndDay( int v )
{
  if ( settings.daylightend_d == v )
    return " selected ";
  else
    return "";
}

String IsEndMonth( int v )
{
  if ( settings.daylightend_m == v )
    return " selected ";
  else
    return "";
}

String IsEndTime( int v )
{
  if ( settings.daylightend_t == v )
    return " selected ";
  else
    return "";
}

String SecsToHMS( const uint32_t seconds )
{
    uint32_t t = seconds;
    uint32_t s, m, h;

    s = t % 60;

    t = (t - s)/60;
    m = t % 60;

    t = (t - m)/60;
    h = t;

    return String( h ) + "h " + String( m ) + "m " + String( s ) +"s";
}


String getPage()
{
  String page = "<html lang='en'><head>";

  page += "<link rel='stylesheet' href='http://3sprockets.com.au/um/wificlock.css'></script>";
   
  page += "<title>Laser Wifi Clock - unexpectedmaker.com</title></head>";
  page += "<body onload=\"document.getElementById('loading').style.display='none'; colorD(); colorC(); \">";
  page += "<div id=\"loading\">";
  page += "<img id=\"loading-image\" src=\"http://3sprockets.com.au/um/logo.png\" alt=\"Loading...\" />";
  page += "</div>";
  page += "<img src=\"http://3sprockets.com.au/um/logo.png\" class='logo'>";
  page += "<h1>Laser Wifi Clock</h1>";
  page += "<h3>by Unexpected Maker</h3>";
  page += "<h2>Settings</h2>";

  bool firstSetup = ( settings.gmt < -999 );

  // Clock & Wifi Stats
  page += "<div class='div-sec'>";
  page +=   "<div class='div-head'>Wifi & Clock Status</div>";
  page +=   "<div class='div-con'><span class='pad'>IP</span><b>"+WiFi.localIP().toString()+"</b></div>";
  page +=   "<div class='div-con'><span class='pad'>IP Reset</span><b>"+String( ipResets )+"</b></div>";
  page +=   "<div class='div-con'><span class='pad'>Wifi SSID</span><b>"+  WiFi.SSID()+"</b></div>";
  page +=   "<div class='div-con'><span class='pad'>Wifi Strength</span><b>"+String(getWifiQuality())+"%</b></div>";
  page +=   "<div class='div-con'><span class='pad'>Clock Up Time</span><b>"+SecsToHMS( millis() / 1000 )+"</b></div>";
  page +=   "<div class='div-con'><span class='pad'>NTP Updates</span><b>"+String( ntpSuccess )+"</b></div>";  
  page +=   "<div class='div-con'><span class='pad'>NTP Failures</span><b>"+String( ntpFailed )+"</b></div>";
  page +=   "<div class='div-con'><span class='pad'>Date</span><b>"+String( day() )+"/"+ String( month() )+"/"+ String( year() )+"</b></div>";
  page += "</div>";

  if ( !firstSetup )
  {

  // NTP Server
  page += "<div class='div-sec'>";
  page +=   "<div class='div-head'>NTP Settings</div>";
  page +=   "<div class='div-con'><span class='pad'>NTP Server</span>";
  page +=     "<input type=\"text\" value=\""+String(GetNTPServer())+"\" onchange=\"javascript:location.href = 'ntp_'+this.value;\" >";
  page +=   "</div>";  
  page +=   "<div class='div-con'><span class='pad'>Update Freq</span>";
  page +=     "<input type=\"range\" style=\"width:150px;\" value=\""+String(settings.updateSchedule)+"\" min=\"1\" max=\"12\" onchange=\"javascript:location.href = 'U'+this.value;\" >&nbsp"+String(settings.updateSchedule)+"&nbsp;hour(s)";
  page +=   "</div>"; 
  page += "</div>";


  // Brightness settings
  page += "<div class='div-sec'>";
  page +=   "<div class='div-head'>Brightness Settings</div>";
  page +=   "<div class='div-con'><span class='pad'>Use LDR</span>";
    page +=     "<select onchange=\"javascript:location.href = this.value;\">";
  if ( settings.useLDR )
  {
     page +=       "<option value=\"ldr_on\" selected>Yes</option>";
     page +=       "<option value=\"ldr_off\">No</option>";
  }
  else
  {
     page +=       "<option value=\"ldr_on\">Yes</option>";
     page +=       "<option value=\"ldr_off\" selected>No</option>";
  }
  page +=     "</select>";
  page +=     "</div>";
  page +=   "<div class='div-con'><span class='pad'>Max Brigtness</span>";
  page +=    "<input type=\"range\" style=\"width:150px;\" value=\""+String(settings.brightness)+"\" min=\"33\" max=\"100\" onchange=\"javascript:location.href = 'B'+this.value;\" >&nbsp;"+String(settings.brightness)+"%";
  page +=   "</div>";  
  page += "</div>";

  // Colour Picker
  page += "<div class='div-sec'>";
  page +=   "<div class='div-head'>Default Clock Colours</div>";
  page +=   "<div class='div-con'>";
  page +=     "<span class='pad'>Digits</span>R&nbsp;<input id='d_r' type=\"range\" style=\"width:100px;\" value='"+GetDef_DR()+"' min=\"0\" max=\"255\" onchange=\"colorD(); javascript:location.href = 'def_DR'+this.value;\" >&nbsp;<div id='swatch_d' class='swatch_d'></div><br>";
  page +=     "<span class='pad'>&nbsp;</span>G&nbsp;<input id='d_g' type=\"range\" style=\"width:100px;\" value='"+GetDef_DG()+"' min=\"0\" max=\"255\" onchange=\"colorD(); javascript:location.href = 'def_DG'+this.value;\" ><br>";
  page +=     "<span class='pad'>&nbsp;</span>B&nbsp;<input id='d_b' type=\"range\" style=\"width:100px;\" value='"+GetDef_DB()+"' min=\"0\" max=\"255\" onchange=\"colorD(); javascript:location.href = 'def_DB'+this.value;\" ><br>";
  page +=   "</div>";
  page +=   "<div class='div-con'>";
  page +=     "<span class='pad'>Colon</span>R&nbsp;<input id='c_r' type=\"range\" style=\"width:100px;\" value='"+GetDef_CR()+"' min=\"0\" max=\"255\" onchange=\"colorC(); javascript:location.href = 'def_CR'+this.value;\" >&nbsp;<div id='swatch_c' class='swatch_c'></div><br>";
  page +=     "<span class='pad'>&nbsp;</span>G&nbsp;<input id='c_g' type=\"range\" style=\"width:100px;\" value='"+GetDef_CG()+"' min=\"0\" max=\"255\" onchange=\"colorC(); javascript:location.href = 'def_CG'+this.value;\" ><br>";
  page +=     "<span class='pad'>&nbsp;</span>B&nbsp;<input id='c_b' type=\"range\" style=\"width:100px;\" value='"+GetDef_CB()+"' min=\"0\" max=\"255\" onchange=\"colorC(); javascript:location.href = 'def_CB'+this.value;\" ><br>";
  page +=   "</div>";  
  page += "</div>";

  }
  
  // Time format
  page += "<div class='div-sec'>";
  if ( firstSetup )
    page += "<div class='div-head-err'>Time Settings - Set to continue</div>";
  else
    page += "<div class='div-head'>Time Settings</div>";

  if ( !firstSetup )
  {
    
    page +=   "<div class='div-con'><span class='pad'>Format</span>";
    page +=     "<select onchange=\"javascript:location.href = this.value;\">";
    if ( Is24Hour() )
    {
       page +=       "<option value=\"\\f12\">12 hour</option>";
       page +=       "<option value=\"\\f24\" selected>24 hour</option>";
    }
    else
    {
      page +=       "<option value=\"\\f12\" selected>12 hour</option>";
      page +=       "<option value=\"\\f24\">24 hour</option>";
    }
    page +=     "</select>";
    page +=   "</div>";  

  }
  
  // GMT zone
  page +=   "<div class='div-con'><span class='pad'>Zone</span>";
  page +=     "<select onchange=\"javascript:location.href = this.value;\">";
  page +=       "<option value=\"G-12.0\""+IsGMT(-1000)+">Please Set GMT!!!</option>";
  page +=       "<option value=\"G-12.0\""+IsGMT(-12)+">(GMT -12:00) Eniwetok, Kwajalein</option>";
  page +=       "<option value=\"G-11.0\""+IsGMT(-11)+">(GMT -11:00) Midway Island, Samoa</option>";
  page +=       "<option value=\"G-10.0\""+IsGMT(-10)+">(GMT -10:00) Hawaii</option>";
  page +=       "<option value=\"G-9.0\""+IsGMT(-9)+">(GMT -9:00) Alaska</option>";
  page +=       "<option value=\"G-8.0\""+IsGMT(-8)+">(GMT -8:00) Pacific Time (US &amp; Canada)</option>";
  page +=       "<option value=\"G-7.0\""+IsGMT(-7)+">(GMT -7:00) Mountain Time (US &amp; Canada)</option>";
  page +=       "<option value=\"G-6.0\""+IsGMT(-6)+">(GMT -6:00) Central Time (US &amp; Canada), Mexico City</option>";
  page +=       "<option value=\"G-5.0\""+IsGMT(-5)+">(GMT -5:00) Eastern Time (US &amp; Canada), Bogota, Lima</option>";
  page +=       "<option value=\"G-4.0\""+IsGMT(-4)+">(GMT -4:00) Atlantic Time (Canada), Caracas, La Paz</option>";
  page +=       "<option value=\"G-3.5\""+IsGMT(-3.5)+">(GMT -3:30) Newfoundland</option>";
  page +=       "<option value=\"G-3.0\""+IsGMT(-3)+">(GMT -3:00) Brazil, Buenos Aires, Georgetown</option>";
  page +=       "<option value=\"G-2.0\""+IsGMT(-2)+">(GMT -2:00) Mid-Atlantic</option>";
  page +=       "<option value=\"G-1.0\""+IsGMT(-1)+">(GMT -1:00 hour) Azores, Cape Verde Islands</option>";
  page +=       "<option value=\"G0.0\""+IsGMT(0)+">(GMT) Western Europe Time, London, Lisbon, Casablanca</option>";
  page +=       "<option value=\"G1.0\""+IsGMT(1)+">(GMT +1:00 hour) Brussels, Copenhagen, Madrid, Paris</option>";
  page +=       "<option value=\"G2.0\""+IsGMT(2)+">(GMT +2:00) Kaliningrad, South Africa</option>";
  page +=       "<option value=\"G3.0\""+IsGMT(3)+">(GMT +3:00) Baghdad, Riyadh, Moscow, St. Petersburg</option>";
  page +=       "<option value=\"G3.5\""+IsGMT(3.5)+">(GMT +3:30) Tehran</option>";
  page +=       "<option value=\"G4.0\""+IsGMT(4)+">(GMT +4:00) Abu Dhabi, Muscat, Baku, Tbilisi</option>";
  page +=       "<option value=\"G4.5\""+IsGMT(4.5)+">(GMT +4:30) Kabul</option>";
  page +=       "<option value=\"G5.0\""+IsGMT(5)+">(GMT +5:00) Ekaterinburg, Islamabad, Karachi, Tashkent</option>";
  page +=       "<option value=\"G5.5\""+IsGMT(5.5)+">(GMT +5:30) Bombay, Calcutta, Madras, New Delhi</option>";
  page +=       "<option value=\"G5.75\""+IsGMT(5.75)+">(GMT +5:45) Kathmandu</option>";
  page +=       "<option value=\"G6.0\""+IsGMT(6)+">(GMT +6:00) Almaty, Dhaka, Colombo</option>";
  page +=       "<option value=\"G7.0\""+IsGMT(7)+">(GMT +7:00) Bangkok, Hanoi, Jakarta</option>";
  page +=       "<option value=\"G8.0\""+IsGMT(8)+">(GMT +8:00) Beijing, Perth, Singapore, Hong Kong</option>";
  page +=       "<option value=\"G9.0\""+IsGMT(9)+">(GMT +9:00) Tokyo, Seoul, Osaka, Sapporo, Yakutsk</option>";
  page +=       "<option value=\"G9.5\""+IsGMT(9.5)+">(GMT +9:30) Adelaide, Darwin</option>";
  page +=       "<option value=\"G10.0\""+IsGMT(10)+">(GMT +10:00) Eastern Australia, Guam, Vladivostok</option>";
  page +=       "<option value=\"G11.0\""+IsGMT(11)+">(GMT +11:00) Magadan, Solomon Islands, New Caledonia</option>";
  page +=       "<option value=\"G12.0\""+IsGMT(12)+">(GMT +12:00) Auckland, Wellington, Fiji, Kamchatka</option>";
  page +=     "</select>";
  page +=   "</div>";  
  page += "</div>";

  if ( !firstSetup )
  {
  
  // Daylight savings state
  page += "<div class='div-sec'>";
  page +=   "<div class='div-head'>Daylight Saving ( DAY, MONTH, HOUR )</div>";
  page +=   "<div class='div-con'><span class='pad'>Starts</span>";
  page +=     "<select onchange=\"javascript:location.href = 'ds_d'+this.value;\">";

 for ( int i = 0; i < 32; i++ )
 {
    page +=       "<option value=\""+String(i)+"\" "+ IsStartDay(i)+" >"+String(i)+"</option>\n";
 }

  page +=     "</select>";

  page +=     "<select onchange=\"javascript:location.href = 'ds_m'+this.value;\">";

 for ( int i = 0; i < 13; i++ )
    page +=       "<option value='"+String(i)+"' "+ IsStartMonth(i)+" >"+String(i)+"</option>\n";

  page +=     "</select>";


    page +=     "<select onchange=\"javascript:location.href = 'ds_t'+this.value;\">";

 for ( int i = 0; i < 24; i++ )
    page +=       "<option value=\""+String(i)+"\" "+ IsStartTime(i)+" >"+String(i)+"</option>\n";

  page +=     "</select>&nbsp;&nbsp;Set to 0 to ignore";
  page +=   "</div>"; 

  page +=   "<div class='div-con'><span class='pad'>Ends</span>";
  page +=     "<select onchange=\"javascript:location.href = 'de_d'+this.value;\">";

 for ( int i = 0; i < 32; i++ )
    page +=       "<option value=\""+String(i)+"\" "+ IsEndDay(i)+" >"+String(i)+"</option>\n";

  page +=     "</select>";

  page +=     "<select onchange=\"javascript:location.href = 'de_m'+this.value;\">";

 for ( int i = 0; i < 13; i++ )
    page +=       "<option value=\""+String(i)+"\" "+ IsEndMonth(i)+" >"+String(i)+"</option>\n";

  page +=     "</select>";


    page +=     "<select onchange=\"javascript:location.href = 'de_t'+this.value;\">";

 for ( int i = 0; i < 24; i++ )
    page +=       "<option value=\""+String(i)+"\" "+ IsEndTime(i)+" >"+String(i)+"</option>\n";

  page +=     "</select>\n";
  page +=   "</div>\n"; 
  
  page += "</div>";

  }
  
  // footer
  page += "<p class='footer'>To find out more about Unexpected Maker, visit<br><a href='http://unexpectedmaker.com'>unexpectedmaker.com</p>";
  
   page +="<script>";
   page +="function colorD() {";
    page +=" var valR = document.getElementById('d_r').value;";
    page +=" var valG = document.getElementById('d_g').value;";
    page +=" var valB = document.getElementById('d_b').value;";
    page +=" var div = document.getElementById('swatch_d');";
   page +="  div.style.backgroundColor = 'rgb(' + valR + ',' + valG + ',' + valB + ')';";
    page +=" }";
    page +="function colorC() {";
    page +=" var valR = document.getElementById('c_r').value;";
    page +=" var valG = document.getElementById('c_g').value;";
    page +=" var valB = document.getElementById('c_b').value;";
    page +=" var div = document.getElementById('swatch_c');";
   page +="  div.style.backgroundColor = 'rgb(' + valR + ',' + valG + ',' + valB + ')';";
    page +=" }";
   page +="</script>";

  
  
  page +="</body></html>";

  return page;
}
