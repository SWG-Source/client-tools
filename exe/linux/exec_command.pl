#!/usr/bin/perl

use Getopt::Std;
require "/m1/homes/swgsrv/nodes.pl";

# verify cmd line arguments and abort if incorrect
$Getopt::Std::opt_v = "";
Getopt::Std::getopts("v");

if ( ! $ARGV[0] ) 
{
    print "usage: startcluster.pl <CLUSTER>\n";
    print "  e.g: startcluster.pl dev\n";
    exit;
}

$cmd = $ARGV[1];
$cluster = $ARGV[0];
if ( ! $node{"$cluster"}{"01"} ) 
{
    print "cluster: $cluster not defined\n";
    exit;
} 
else 
{
    foreach $n ( sort { $a <=> $b } keys % {$node{"$cluster"}} ) 
    {
	$hostname = "$node{\"$cluster\"}{\"$n\"}.$domain";
	system("/usr/local/bin/ssh -f $user\@$hostname \"$cmd \"");
    }
}

