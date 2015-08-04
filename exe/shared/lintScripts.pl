#!/usr/bin/perl

opendir(SCRIPTLINT, $ARGV[0]);
@lints = readdir(SCRIPTLINT);

foreach $lint (@lints)
{
    if($lint =~ /.pl/)
    {
	system("perl $ARGV[0]/$lint $ARGV[1]");
    }
}
