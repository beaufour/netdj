#!/usr/bin/perl -w
# $Id$

use strict;
use LWP::UserAgent;
use XML::Simple;
use Gtk;
use Gnome;
use Gnome::Applet;
use Gtk::GladeXML;


########################################
# CONFIG
my $USER = 'allan';
my $PASS = 'slartibarfast';
my $HOST = 'localhost:7676';
my $MAX_SIZE = 350;


########################################
# GLOBALS
my $NOINFO_TEXT = '- No Info - ';
my $last_status = "";


########################################
# INITIALIZATION
# Gnome
init Gnome::AppletWidget 'netdj_applet.pl';

# UserAgent
my $ua = LWP::UserAgent->new(timeout => 1,
			     keep_alive => 0,
			     env_proxy => 1);
$ua->credentials($HOST, 'NetDJ', $USER, $PASS);

# Glade
my $glade = new Gtk::GladeXML('netdj_applet.glade');
$glade->signal_autoconnect_from_package('main');


########################################
# UI
Gtk::Rc->parse_string(<<"EOF");
style "panel-font"
{
        font = "-adobe-helvetica-medium-r-normal--*-80-*-*-*-*-*-*"
        fg[NORMAL] = {0., 0., 0.}
}
widget "*AppletWidget.*GtkLabel" style "panel-font"
EOF

my $applet = new Gnome::AppletWidget 'netdj_applet.pl';

# Commands
$applet->register_callback("Next", "Næste", \&cmd_generic, "next");
$applet->register_callback("Start", "Start", \&cmd_generic, "start");
$applet->register_callback("Stop", "Stop", \&cmd_generic, "stop");
$applet->register_stock_callback("Preferences", "Preferences", , "Egenskaber...", \&menu_pref);
$applet->register_stock_callback("About", "About", , "Om...", \&menu_about);

# Box
my $box = new Gtk::HBox(0, 1);
$applet->add($box);

# Status button
my $button = new Gtk::Button;
my $label = new Gtk::Label $NOINFO_TEXT;
$button->set_name("button");
$button->set_usize($MAX_SIZE, $applet->get_panel_pixel_size);
$button->signal_connect("clicked", \&cmd_update);
$button->add($label);
$box->pack_start($button, 0, 0, 1);

# Next button
my $nextbutton = new Gtk::Button;
my $nextlabel = new Gtk::Label ">>";
$nextbutton->signal_connect("clicked", \&cmd_generic, "next");
$nextbutton->add($nextlabel);
$box->pack_start($nextbutton, 0, 0, 1);

# Install 10s timeout
Gtk->timeout_add(10000, \&cmd_update);

# Update label
cmd_update();

# Start showing off :)
$applet->show_all;
#gtk_main Gnome::AppletWidget;
main Gtk;


########################################
# SUBROUTINES
sub cmd_update {
    my $response = $ua->get("http://$HOST/index.xml");
    if ($response->is_error) {
	print "Information retrieval returned: ".$response->status_line."\n";
	$label->set_text($NOINFO_TEXT);
    } else {
	$_ = $response->content;
	if (!($_ eq $last_status)) {
	    # Hack to delete any '&' (should be fixed in netdj-bin...)
	    while (s/\&/and/) {};
	    my $status = XMLin($_);
	    
	    my $tool = "";
	    for (my $i = 1; $i < 11; ++$i) {
		$tool .= "$status->{song}[$i]{description}\n";
	    }
	    
	    my $current = $status->{song}[0]{description};
	    $label->set_text($current);
	    $applet->set_tooltip($tool);
	}
    }
    
    return 1; # 1 == keep function on timeout-trigger.
}

sub cmd_generic {
    my $cmd = shift;
    $cmd = shift;

    my $response = $ua->get("http://$HOST/cgi-bin/$cmd");

    if ($response->is_error) {
	print "Command '$cmd' returned: ".$response->status_line."\n";
    }
    return 1;
}

sub menu_about {
    my $about = new Gnome::About "NetDJ applet",
	"0.01",
	"(c) 2002 by Allan Beaufour Larsen",
	"Allan Beaufour Larsen <allan\@beaufour.dk>",
	'CVSinfo: $Id$';
    $about->show();
    return 1;
}

sub menu_pref {
    my $pref_dialog = $glade->get_widget('pref_dialog');
    $pref_dialog->show;
    return 1;
}

sub pref_but_ok {
    print "Button clicked: OK\n";

    return 1;
}

sub pref_but_apply {
    print "Button clicked: Apply\n";

    return 1;
}

sub pref_but_cancel {
    print "Button clicked: Cancel\n";

    return 1;
}
