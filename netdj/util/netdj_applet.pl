#!/usr/bin/perl -w
# $Id$

use strict;
use LWP::UserAgent;
use XML::Simple;
use Gtk;
use Gnome;
use Gnome::Applet;
use Data::Dumper;

# I can't get Gtk::GladeXML to work with Gnome::Applet...
# use Gtk::GladeXML;

########################################
# CONFIG
my %CONFIG;
my $config_file = "$ENV{HOME}/.netdj_applet";
open(CONFFILE, $config_file) || die "Can't open $config_file: $!\n";
while (<CONFFILE>) {
  chomp;
  if ((!m/^#.*/) && m/([^ ]+) *= *([^ ]+)/) {
    $CONFIG{$1} = $2;
  }
};
close(CONFFILE);


########################################
# GLOBALS
my $NOINFO_TEXT = '- No Info - ';
my $NOTPLAYING_TEXT = '- Not Playing -';
my $last_status = "";
my $current = "";

########################################
# INITIALIZATION
# Gnome
init Gnome::AppletWidget 'netdj_applet.pl';

# UserAgent
my $ua = LWP::UserAgent->new(timeout => 1,
			     keep_alive => 0,
			     env_proxy => 1);
$ua->credentials($CONFIG{HOST}, 'NetDJ', $CONFIG{USER}, $CONFIG{PASS});

# Glade
# Well, doesn't work right now...
# my $glade = new Gtk::GladeXML('netdj_applet.glade');
# $glade->signal_autoconnect_from_package('main');


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
$applet->register_callback("Next", "Next", \&cmd_generic, "next");
$applet->register_callback("Start", "Start", \&cmd_generic, "start");
$applet->register_callback("Stop", "Stop", \&cmd_generic, "stop");
$applet->register_callback("GoogleSearch", "Search google...", sub
			   {
			     `gnome-moz-remote "http://www.google.com/search?q=$current"`;
			   });
$applet->register_stock_callback("Preferences", "Preferences", , "Preferences...", \&menu_pref);
$applet->register_stock_callback("About", "About", , "About...", \&menu_about);

# Box
my $box = new Gtk::HBox(0, 0);
$applet->add($box);

# Status button
my $button = new Gtk::Button;
my $label = new Gtk::Label $NOINFO_TEXT;
$button->set_name("button");
$button->signal_connect("clicked", \&cmd_update);
$button->add($label);
$button->set_usize($CONFIG{MAX_SIZE}, $applet->get_panel_pixel_size);
$box->pack_start($button, 0, 0, 1);

# Cache status label
my $cachestatus = new Gtk::Label "-";
$box->pack_start($cachestatus, 0, 0, 1);

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
sub keysort {
    $a <=> $b
}

sub cmd_update {
    my $response = $ua->get("http://$CONFIG{HOST}/index.xml");
    if ($response->is_error) {
	print "Information retrieval returned: ".$response->status_line."\n";
	$label->set_text($NOINFO_TEXT);
	$cachestatus->set_text("?");
	$applet->set_tooltip("");
    } else {
	$_ = $response->content;
	if (!($_ eq $last_status)) {
	    my $status = XMLin($_);
	    
	    # Current song
	    $current = $status->{currentsong}->{description};
	    if (ref $current) {
		$current = $NOTPLAYING_TEXT;
	    }
	    $label->set_text($current);
    

	    # Songlist
	    my $tool = "";
	    my $songs = $status->{song};

	    # BUG: Crashes when only one song in list ....
	    # grrrr, I hate XML-Simple..
	    foreach my $s (sort {$a <=> $b} keys %$songs) {
		$tool .= "$s) ".$songs->{$s}->{description}."\n";
	    }
	    $applet->set_tooltip($tool);

	    # Cache status
	    $cachestatus->set_text($status->{list}->{cache}->{size});
	}
    }
    
    return 1; # 1 == keep function on timeout-trigger.
}

sub cmd_generic {
    my $cmd = shift;
    $cmd = shift;

    my $response = $ua->get("http://$CONFIG{HOST}/cgi-bin/$cmd");

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

#  sub menu_pref {
#      my $pref_dialog = $glade->get_widget('pref_dialog');
#      $pref_dialog->show;
#      return 1;
#  }

#  sub pref_but_ok {
#      print "Button clicked: OK\n";

#      return 1;
#  }

#  sub pref_but_apply {
#      print "Button clicked: Apply\n";

#      return 1;
#  }

#  sub pref_but_cancel {
#      print "Button clicked: Cancel\n";

#      return 1;
#  }

