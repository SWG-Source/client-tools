#! /usr/bin/perl
# ======================================================================
# ======================================================================

use strict;
use warnings;

# ======================================================================
# Globals
# ======================================================================

my $scriptName = $0;
$scriptName =~ s/^(.*)[\\\/]//;

# ======================================================================
# Subroutines
# ======================================================================

sub usage()
{
	die "\nUsage:\n\t$scriptName [ debug | release ] (default is debug)\n";
}

sub perforceWhere
{
	local $_;

	# find out where a perforce file resides on the local machine
	my $result;
	{
		open(P4, "p4 where $_[0] |");
			$_ = <P4>;
			chomp;
			my @where = split;
			$result = $where[2];
		close(P4);
	}

	return $result;
}

# ======================================================================
# Main
# ======================================================================

usage() if(@ARGV && $ARGV[0] ne "debug" && $ARGV[0] ne "release");

my $serverType = (@ARGV) ? shift : "debug";

# Start up the server
system("$serverType/LoginServer -- \@loginServer.cfg &") == 0 || die "error starting LoginServer\n";
system("$serverType/TaskManager -- \@taskmanager.cfg &") == 0 || die "error starting TaskManager\n";
