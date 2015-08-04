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

$cmd = "ulimit -c unlimited; killall TaskManager_d; cd /swg/swg/test/exe/linux ; ./TaskManager_d -- \@remote_taskmanager.cfg";

$cluster = $ARGV[0];
if ( ! $node{"$cluster"}{"01"} ) 
{
    print "cluster: $cluster not defined\n";
    exit;
} 
else 
{
    system("killall LoginServer_d");
    system("killall CentralServer_d");
    foreach $n ( sort { $a <=> $b } keys % {$node{"$cluster"}} ) 
    {
	$hostname = "$node{\"$cluster\"}{\"$n\"}.$domain";
	system("/usr/local/bin/ssh -f $user\@$hostname \"$cmd 2>&1 \" > logs/taskmanager-$hostname.log");
    }
#    system("./LoginServer_d -- \@loginServer.cfg &");
#    system("./TaskManager_d -- \@taskmanager.cfg > logs/taskmanager-swo-dev17.log");
}

