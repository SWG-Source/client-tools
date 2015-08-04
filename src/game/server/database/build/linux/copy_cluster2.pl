#!/usr/bin/perl

use strict;
use Getopt::Long;

&main;

sub main
{
    my ($starttime, $endtime);

    &GetOptions("copycluster","exportcluster","importcluster","createtypes","wipeschema","recompilepackages","help","targetname:s","targetpassword:s","targetservice:s","sourcename:s","sourcepassword:s","sourceservice:s" );

    if ($::opt_help || !($::opt_copycluster || $::opt_exportcluster || $::opt_importcluster || $::opt_createtypes || $::opt_wipeschema || $::opt_recompilepackages ))
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
	print "  --targetname=NAME         Specify the target schema name. (Defaults to \$USER.)\n";
	print "  --targetpassword=PASSWORD Specify the target password.  (Defaults to \"changeme\".)\n";
	print "  --taregetservice=NAME     Specify the target database service name.  (Defaults to \"swodb\".)\n";
	print "  --sourcename=NAME         Specify the source schema name.\n";
	print "  --sourcepassword=PASSWORD Specify the source password.  (Defaults to \"changeme\".)\n";
	print "  --sourceservice=NAME      Specify the source database service name.  (Defaults to \"swodb\".)\n";

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
    my ($dumpfile);

    print "**** STARTING CLUSTER COPY ****\n\n";

       # check if another export has already been started from parallel execution
       # wait for up to 2 hours

            my $cnt = 1;
            my $total = 0;

            while (($cnt > 0 ) && ($total < 120))
            {
                open (INFILE,"ps -ef | grep exp | grep -v grep | wc -l |");
                $cnt = <INFILE>;
                $cnt =~ s/\s+//;
                close(INFILE);

                if ( $cnt > 0 )
                {
                        print "An export is already running thru another process ... waiting for it to complete!\n";
                        $total++;
                        sleep 60;
                }
            }
            if ($cnt > 0)
            {
                print "Export still running after two hours!\n Aborting copy cluster!\n";
                exit 1;
            }


    $dumpfile = &username($::opt_sourcename).".dmp";
    if (-e $dumpfile)
    {
	print "**** SKIPPING EXPORING CLUSTER BECAUSE EXPORT ALREADY EXISTS****\n\n";
    }
    else
    {
	print "**** EXPORING CLUSTER ****\n\n";
	&exportcluster;
    }
    print "**** DROPPING ALL OBJECTS IN USER SCHEMA ****\n\n";
    &wipeschema;
    print "**** RECREATING ORACLE USER TYPES ****\n\n";
    &createtypes;
    print "**** IMPORTING CLUSTER  ****\n\n";
    &importcluster;
    print "**** RECOMPILING USER PACKAGES ****\n\n";
    &recompilepackages;
    print "**** CLUSTER COPY COMPLETE ****\n\n";
}

#-----------------------------------------------------------------------

#export cluster to file using direct path
sub exportcluster
{
    if ($::opt_sourcename eq "")
    {
       print "You must select a valid source schema!\n\n";
       die;
    }
    system ("exp ".&sourcelogin." file=".&username($::opt_sourcename).".dmp owner=".&username($::opt_sourcename)." direct=y statistics=none > output.tmp");

}

#-----------------------------------------------------------------------

#import cluster from file using direct path
sub importcluster
{
    if ($::opt_sourcename eq "" || $::opt_targetname eq "")
    {
       print "You must select valid source and target schemas!\n\n";
       die;
    }

    $_ = $::opt_targetname;

    if (/gold/ || /GOLD/ || /publish/ || /PUBLISH/)
    {
       system ("imp ".&targetlogin." file=".&username($::opt_sourcename).".dmp fromuser=".&username($::opt_sourcename)." touser=".&username($::opt_targetname)." statistics=none ignore=y grants=n >> output.tmp");
    }
    else
    {
       print "You may only import to a gold schema!\n\n";
       die;
    }
}

#-----------------------------------------------------------------------

#wipe all objects in user schema
sub wipeschema
{
    $_ = $::opt_targetname;

    if (/gold/ || /GOLD/ || /publish/ || /PUBLISH/)
    {
       system (&sqlplus." @../../queries/drop_all_objects.sql >> output.tmp");
    }
    else
    {
       print "You may only drop objects in a gold schema!\n\n";
       die;
    }
}

#-----------------------------------------------------------------------

#recompile invalid packages
sub recompilepackages
{
    system ("./check_objects.sh ".&targetlogin." >> output.tmp");
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
	print "DEBUG: sqlplus command is:\n",&sqlplus," < ",$_,"\n\n";
	system (&sqlplus." < $_ > /tmp/database_update.tmp");
	&checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
}

#-----------------------------------------------------------------------

# Return oracle username string
sub username
{
    my ($user)=@_;
    $user =~ tr/A-Z/a-z/;

    return "$user";
}

#-----------------------------------------------------------------------

# Return oracle password string
sub password
{
    my ($pwd)=@_;
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");

    return "$pwd";
}

#-----------------------------------------------------------------------

# Return oracle service string
sub service
{
    my ($db)=@_;
    $db="swodb" if ($db eq "");

    return "$db";
}

#-----------------------------------------------------------------------

sub targetlogin
{
    return &username($::opt_targetname)."/".&password($::opt_targetpassword)."\@".&service($::opt_targetservice);
}

#-----------------------------------------------------------------------

sub sourcelogin
{
    return &username($::opt_sourcename)."/".&password($::opt_sourcepassword)."\@".&service($::opt_sourceservice);
}

#-----------------------------------------------------------------------

# Return the sqlplus command string
sub sqlplus
{
	print "DEBUG: targetlogin is:\n",&targetlogin,"\n\n";
    return "sqlplus ".&targetlogin;
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

