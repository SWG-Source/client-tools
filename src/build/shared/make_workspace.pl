#!/usr/bin/perl

use File::Find;

# Process command line arguments.
while ($ARGV[0] =~ /^-/)
{
	$_ = shift;
	
	if ($_ eq "--debug")
	{
		$debug = 1;
	}
	else
	{
		die "unknown command line option";
	}
}

# Determine directory to run this on.
@findDirs = @ARGV ? @ARGV : ('.');

# Scan for appropriate files.
find(\&FindHandler, @findDirs);

# Done.
exit(0);

# ===================================================================

sub FindHandler 
{
	if (-d $_)
	{
		# Examining a directory entry.

		# Prune the directory if it's one I want to ignore.  This
		# stops the program from descending any further down this directory.
		if (m/^(compile|external|Debug|Optimized|Production|Release)$/i)
		{
			# Prune it.
			$File::Find::prune = 1;
			print STDERR "[Pruned Directory Entry: $File::Find::name]\n" if ($debug);
		}
	}
	elsif (-f $_)
	{
		# Handle files that should go in the workspace file.
		if (m/^.*(c|cpp|def|h|hpp|java|lnt|pl|pm|py|script(?:lib)?|sql|tab|txt)$/)
		{
			print "$_:$File::Find::dir/\n";
		}
	}
}

# ===================================================================
