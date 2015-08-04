use FindBin '$Bin';
use Sys::Hostname;
use Getopt::Long;
use Socket;

&GetOptions("clustername:s", "dbuser:s", "host:s", "dbservice:s", "help");
if($::opt_help)
{
    print "Usage: $0 [OPTION]...\n";
    print "Options:\n";
    print "  --clustername=<clustername>        create default.cfg using <clustername> as the cluster name\n";
    print "  --dbuser=<db user id>              use <db user id> to id the server\n";
    print "  --host:<hostname>                  create a default.cfg that uses <hostname> for server addresses\n";
    print "  --dbservice:<db server>            use <db server> (e.g. from tnsnames)\n";
    print "\nBy default, the script will try to generate a default.cfg using reasonable defaults.\n";
}

# get hostname
my $hostname = hostname();
if($::opt_host)
{
    $hostname=$::opt_host;
}

my $dbuser = getpwuid($<);

if($::opt_dbuser)
{
    $dbuser = $::opt_dbuser;
}
else
{
    my $branch = "";
    @dirs = split('/', $Bin);
    $x = 0;
    for $i(@dirs)
    {
	if($i eq "swg")
	{
	    $branch = @dirs[$x+1];
	    break;
	}
	$x++;
    }

    $dbuser = "$dbuser\_$branch";
}

my $dbservice="swodb";
if($::opt_dbservice)
{
    $dbservice = $::opt_dbservice;
}

my $clustername = getpwuid($<);
if($::opt_clustername)
{
    $clustername = $::opt_clustername;
}


my $hostip = inet_ntoa(inet_aton($hostname));

open (CONFIG, "> default.cfg") or die "Could not open default.cfg for writing\n";
print CONFIG "[TaskManager]\n";
print CONFIG "loginServerAddress=aus-lindevlogin-01.starwarsgalaxies.net\n";
print CONFIG "clusterName=$clustername\n";
print CONFIG "node0=$hostip\n\n";
print CONFIG "[dbProcess]\n";
print CONFIG "DSN=$dbservice\n";
print CONFIG "databaseUID=$dbuser\n\n";
print CONFIG "[LoginServer]\n";
print CONFIG "DSN=$dbservice\n";
print CONFIG "databaseUID=$dbuser\n";
print CONFIG "developmentMode=true\n";
print CONFIG "\n[CentralServer]\n";
print CONFIG "developmentMode=true\n";
print CONFIG "\n[GameServer]\n";
print CONFIG "adminGodToAll=true\n";

close(CONFIG);

if(! open(TMRC, "<taskmanager.rc"))
{
    open(TMRC, ">taskmanager.rc") or die "could not open taskmanager.rc for writing\n";
    print TMRC "CentralServer local debug/CentralServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "ConnectionServer local debug/ConnectionServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "SwgDatabaseServer local debug/SwgDatabaseServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "PlanetServer local debug/PlanetServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "SwgGameServer any debug/SwgGameServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "ChatServer local debug/ChatServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "LogServer local debug/LogServer -- \@serverNetwork.cfg \@logServerTargets.cfg\n";
    print TMRC "MetricsServer local debug/MetricsServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "CommoditiesServer local debug/CommoditiesServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    close(TMRC);
}

if(! open(TMRC, "<taskmanager.debug.rc"))
{
    open(TMRC, ">taskmanager.debug.rc") or die "could not open taskmanager.debug.rc for writing\n";
    print TMRC "CentralServer local debug/CentralServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "ConnectionServer local debug/ConnectionServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "SwgDatabaseServer local debug/SwgDatabaseServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "PlanetServer local debug/PlanetServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "SwgGameServer any debug/SwgGameServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "ChatServer local debug/ChatServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "LogServer local debug/LogServer -- \@serverNetwork.cfg \@logServerTargets.cfg\n";
    print TMRC "MetricsServer local debug/MetricsServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "CommoditiesServer local debug/CommoditiesServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    close(TMRC);
}

if(! open(TMRC, "<taskmanager.release.rc"))
{
    open(TMRC, ">taskmanager.release.rc") or die "could not open taskmanager.release.rc for writing\n";
    print TMRC "CentralServer local release/CentralServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "ConnectionServer local release/ConnectionServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "SwgDatabaseServer local release/SwgDatabaseServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "PlanetServer local release/PlanetServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "SwgGameServer any release/SwgGameServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "ChatServer local release/ChatServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "LogServer local release/LogServer -- \@serverNetwork.cfg \@logServerTargets.cfg\n";
    print TMRC "MetricsServer local release/MetricsServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    print TMRC "CommoditiesServer local release/CommoditiesServer -- \@serverNetwork.cfg \@servercommon.cfg\n";
    close(TMRC);
}

