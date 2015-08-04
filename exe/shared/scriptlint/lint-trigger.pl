#!/usr/bin/perl

use Cwd;

$fileName = $ARGV[0];

$controlFile = getcwd();

if($controlFile =~ /(.*\/script)/) {

	$controlFile = $1 . "/base_script.script";
	
	open(CONTROL, $controlFile) or die "Can't open $controlFile\n";
	
	while(<CONTROL>) {
	
		if($_ =~ /trigger\s+(\w+)\s*\(/) {
		
			$triggers[++$#triggers] = $1;
		}
	}
	
	close CONTROL;
}

open(SOURCE, $fileName) or die "Can't open $fileName\n";

# Open and buffer the script file
while(<SOURCE>) {

	$scriptFile[++$#scriptFile] = $_;
}

# Initialize variables
$lineNo = 0;
$comment = 0;

# Step through each line of the buffered script file
foreach $l (@scriptFile) {

	# Normal people start counting at 1
	$lineNo = $lineNo+1;

	# If this section of code is within a comment block...
	if($comment) {

		# If the line contains the end of the comment block, note it and begin searching for objvars again
		if($l =~ /\*\//) {

			$comment = 0;

		# Otherwise skip to the next line
		} else {

			next;
		}
	}

	# If this line has been commented out, skip to the next line
	if($l =~ /^\s*\/\//) {

		next;
	}

	if($l =~ /trigger\s+(\w+)\s*\(/) {

		$warning = 1;

		foreach $t (@triggers) {

			if($t eq $1) {

				$warning = 0;
				last;
			}
		}
		
		if($warning) {

			if($l =~ /[Ss][Uu][Pp][Pp][Rr][Ee][Ss][Ss]\W*$/) {

				print "$fileName:$lineNo: Cannot supress lint warning. Please provide reason for supression.\n";
				print "$fileName:$lineNo: WARNING: Undefined trigger function: $1\n";

			} elsif($l !~ /[Ss][Uu][Pp][Pp][Rr][Ee][Ss][Ss].*\w+.*$/) {

				print "$fileName:$lineNo: WARNING: Undefined trigger function: $1\n";
			}
		}
	}

	# If this line starts a comment block, note it and continue
	# All lines will be ignored until the comment block ends
	if($l =~ /\/\*/) {

		$comment = 1;
	}
}
