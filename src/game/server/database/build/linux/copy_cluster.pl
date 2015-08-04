#!/usr/bin/perl

use strict;
use Getopt::Long;

&main;

sub main
{
    my ($starttime, $endtime);

    &GetOptions("copycluster","exportcluster","importcluster","createtypes","wipeschema","recompilepackages","help","username:s","password:s","service:s","source:s");

    if ($::opt_help || ($::opt_source eq "") || !($::opt_copycluster || $::opt_exportcluster || $::opt_importcluster || $::opt_createtypes || $::opt_wipeschema || $::opt_recompilepackages ))
    {
	print "Usage: copy_cluster.pl [COMMAND] [OPTION]... \n";
	print "Copy a cluster using Oracle export/import.\n\n";
	print "******************* WARNING *******************.\n";
	print "* This utility will delete all existing user  *\n";
	print "*            objects in your schema           *\n";
	print "***********************************************.\n";
	print "\n";
	print "Commands:\n";
	print "  --help                Display these options.\n";
	print "  --copycluster         Perform the complete copy to user schema.\n";
	print "\n";
	print "  --exportcluster       Export cluster to file using direct path.\n";
	print "  --importcluster       Import cluster from file using direct path.\n";
	print "  --createtypes         Recreate oracle typedefs.\n";
	print "  --wipeschema          Delete all objects from users schema.\n";
	print "  --recompilepackages   Recompile user database packages.\n";
	print "\n";
	print "Options:\n";
	print "  --username=NAME       Specify the database user name. (Defaults to \$USER.)\n";
	print "  --password=PASSWORD   Specify the database password.  (Defaults to \"changeme\".)\n";
	print "  --service=NAME        Specify the database service name.  (Defaults to \"swodb\".)\n";
	print "  --source=NAME         Specify the source schema name.\n";

	print "\n";
	
	exit;
    }
    
    $starttime = time();
    
    print "Start time:\t",scalar localtime($starttime),"\n\n";

  SWITCH: {

      if ($::opt_copycluster) { &copycluster; last SWITCH; }
      if ($::opt_exportcluster) { &exportcluster; last SWITCH; }
      if ($::opt_importcluster) { &importcluster; last SWITCH; }
      if ($::opt_createtypes) { &createtypes; last SWITCH; }
      if ($::opt_wipeschema) { &wipeschema; last SWITCH; }
      if ($::opt_recompilepackages) { &recompilepackages; last SWITCH; }
      print "Warning unknown option \n";
      system("echo Warning unknown command > sqloutput.tmp" );

  }
    $endtime = time();
    print "End time:\t",scalar localtime($endtime),"\n\n";
    print "Elasped time:\t",($endtime - $starttime)," second(s)\n";

}

#=======================================================================

#run all the steps at once involved in copying the  cluster
sub copycluster
{
    print "**** STARTING CLUSTER COPY ****\n\n";
    system ("rm cluster.dmp");
    print "**** EXPORING CLUSTER ****\n\n";
    &exportcluster;
    print "**** DROPPING ALL OBJECTS IN USER SCHEMA ****\n\n";
    &wipeschema;
    print "**** RECREATING ORACLE USER TYPES ****\n\n";
    &createtypes;
    print "**** IMPORTING CLUSTER  ****\n\n";
    &importcluster;
    print "**** RECOMPILING USER PACKAGES ****\n\n";
    &recompilepackages;
    print "**** CLUSTER COPY COMPLETE ****\n\n";
    print "Notes:\n\n";
    print "All packages may not recompile depending the version delta between the cluster packages\n";
    print " and your user schema packages\n\n";
    print "You need to sync your game code to a version that will work with the cluster schema\n";
    print " using perforce and then update the packages using database_update.pl\n\n";
    print "What version to sync to is up to you depending on what you'd like to test\n\n";
}

#-----------------------------------------------------------------------

#export cluster to file using direct path
sub exportcluster
{
    system ("exp ".&login." file=cluster.dmp owner=$::opt_source direct=y > output.tmp");
}

#-----------------------------------------------------------------------

#import cluster from file using direct path
sub importcluster
{
    system ("imp ".&login." file=cluster.dmp fromuser=$::opt_source touser=".&username." >> output.tmp");
}

#-----------------------------------------------------------------------

#wipe all objects in user schema
sub wipeschema
{
    system (&sqlplus." @../../queries/drop_all_objects.sql >> output.tmp");
}

#-----------------------------------------------------------------------

#recompile invalid packages
sub recompilepackages
{
    system ("./check_objects.sh ".&login." >> output.tmp");
    system (&sqlplus." \@check_objects.sql >> output.tmp");
}

#-----------------------------------------------------------------------

# recreate Oracle typedefs after wipe since typedef can't be imported
sub createtypes
{
    open (CREATELIST,"ls ../../schema/*.type|");
    while (<CREATELIST>)
    {
	print $_;
	chop;
	system (&sqlplus." < $_ > /tmp/database_update.tmp");
	&checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
}

#-----------------------------------------------------------------------

# Return oracle username string
sub username
{ 
    my ($user);
    $user=$::opt_username;
    $user=$ENV{"USER"} if ($user eq "");

    $user =~ tr/A-Z/a-z/;

    return "$user";
}

#-----------------------------------------------------------------------

# Return oracle password string
sub password
{
    my ($pwd);
    $pwd=$::opt_password;
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
    
    return "$pwd";
}

#-----------------------------------------------------------------------

# Return oracle service string
sub service
{
    my ($db);
    $db=$::opt_service;
    $db="swodb" if ($db eq "");

    return "$db";
}

#-----------------------------------------------------------------------

# Return oracle login command string
sub login
{
    return &username."/".&password."\@".&service;
}

#-----------------------------------------------------------------------

# Return the sqlplus command string
sub sqlplus
{
    return "sqlplus ".&login;
}

#-----------------------------------------------------------------------

sub checkError
{
    my ($filename,$dontdie)=@_;

    open (CHECKFILE,$filename);
    while (<CHECKFILE>)
    {
	if (/ERROR/ || /created with compilation errors/ )
	{
	    print;
	    while (<CHECKFILE>)
	    {
		last if (/Disconnected from Oracle/);
		print;
	    }
	    close CHECKFILE;
	    #system("rm $filename");
	    die unless $dontdie;
	    last;
	}
    }
    close (CHECKFILE);
    system("rm -f $filename");
}

