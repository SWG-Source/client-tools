#!/usr/bin/perl
#This script looks at the output from Oracle to see if there were any errors
#If there were, it prints out the error message and deletes the file.

# parameters:
#   -f <filename>  Name of the file containing the output from Oracle
#   -n             On error, print the error message, but don't return 
#                  an error code to the caller.

require "getopts.pl";

&Getopts('nf:');

$filename = $opt_f;
#print "Checking $filename\n";
open (INFILE,$filename);

while (<INFILE>)
{
    if (/ERROR/ || /created with compilation errors/ )
    {
	print;
	while (<INFILE>)
	{
	    last if (/Disconnected from Oracle/);
	    print;
	}
	close INFILE;
	system("rm $filename");
	die unless ($opt_n ==1);
	last;
    }
}
