<?php
// CODE TO LOAD XML-FILE
class TList {
  var $SIZE;
  var $ID;
  var $DESCRIPTION;
  
  function TList ($aa) {
    foreach ($aa as $k=>$v)
      $this->$k = $aa[$k];
  }
}

class TSong {
  var $DESCRIPTION;
  var $SIZE;

  function TSong ($aa) {
    foreach ($aa as $k=>$v)
      $this->$k = $aa[$k];
  }
}

function readDatabase($filename) {
  // read the xml
  $data = implode("", file($filename));
  $parser = xml_parser_create();
  xml_parser_set_option($parser,XML_OPTION_CASE_FOLDING,1);
  xml_parser_set_option($parser,XML_OPTION_SKIP_WHITE,1);
  xml_parse_into_struct($parser,$data,$values,$tags);
  xml_parser_free($parser);

  // loop through the structures
  foreach ($tags as $key=>$val) {
    if ($key == "LIST" || $key == "CURRENTSONG" || $key == "SONG") {
      $ranges = $val;
      // each contiguous pair of array entries are the
      // lower and upper range for each definition
      for ($i = 0; $i < count($ranges); $i += 2) {
	$offset = $ranges[$i] + 1;
	$len = $ranges[$i + 1] - $offset;
	$tdb[$key][] = parse(array_slice($values, $offset, $len), $key);
      }
    } else {
      continue;
    }
  }
  return $tdb;
}

function parse($mvalues, $type) {
  for ($i = 0; $i < count($mvalues); $i++)
    $entry[$mvalues[$i]["tag"]] = $mvalues[$i]["value"];
  if ($type == "LIST") {
    return new TList($entry);
  } else if ($type == "CURRENTSONG" || $type == "SONG") {
    return new TSong($entry);
  } else {
    return "";
  }
}
?>
<?
$host = "http://localhost.localdomain:7676";
$db = readDatabase("$host/index.xml");
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML>
  <HEAD>
    <META HTTP-EQUIV="Pragma" CONTENT="no-cache">
    <META HTTP-EQUIV="Refresh" CONTENT="10">
    <STYLE TYPE="text/css">
      body.dummy {}
      body {
        font-size: 10pt;
      }
      table {
        font-size: 10pt;
      }
      form {
        font-size: 10pt;
      }
    </STYLE>
    <TITLE>XML NetDJ</TITLE>
  </HEAD>
  <BODY>
    <SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
      function addNetscapePanel() {
        if ((typeof window.sidebar == "object")
            && (typeof window.sidebar.addPanel == "function"))
          {
            window.sidebar.addPanel ("NetDJ", window.location,"");
          }
      }
    </SCRIPT>
    <A HREF="http://netdj.sourceforge.net">NetDJ</A> is currently playing:
    <P>
    <FORM TARGET="_CONTENT" ACTION="http://www.google.com/search">
      <INPUT maxLength=256 size=55 name=q value="<? print $db["CURRENTSONG"][0]->DESCRIPTION; ?>">
      <INPUT TYPE=SUBMIT VALUE="Google Search" NAME=btnG>
    </FORM>
    <P>
    [<A HREF="/">Refresh</A>]
    - [<A HREF="<? print "$host/cgi-bin/next?id=".$db["CURRENTSONG"][0]->UNID; ?>">Skip song</A>]
    - [<A HREF="<? print $host; ?>/cgi-bin/stop">Stop player</A>]
    - [<A HREF="<? print $host; ?>/cgi-bin/start">Start player</A>]
    <P>
    <STRONG>Next songs in cache:</STRONG><BR>
<?
    if ($db["SONG"]) {
      foreach ($db["SONG"] as $l) {
	print "$l->DESCRIPTION<BR>";
      }
    } else {
      print "Sorry, no info available<BR>";
    }
?>
    <P>
    <STRONG>Playlists:</STRONG><BR>
    <TABLE BORDER=1>
      <TR><TD><B>Size</B></TD><TD><B>Name</B></TD><TD><B>Description</B></TD></TR>
<?
    if ($db["LIST"]) {
      foreach ($db["LIST"] as $l) {
	print "<TR><TD>$l->SIZE</TD><TD>$l->ID</TD><TD>$l->DESCRIPTION</TD></TR>\n";
      }
    }
?>
    </TABLE>
    <P>
    Add to <A HREF="javascript:addNetscapePanel();">Netscape sidebar</A> - <A HREF="/" REL="sidebar" TITLE="NetDJ">Opera panel</A>
<!--
<PRE>
    <? print_r($db); ?>
</PRE>
-->
  </BODY>
</HTML>
