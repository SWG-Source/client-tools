#!/usr/bin/perl

$fileName = $ARGV[0];

open(SOURCE, $fileName) or die "Can't open $fileName";

# Open and buffer the script file
while(<SOURCE>) {

	$scriptFile[++$#scriptFile] = $_;
}

# Initialize variables
$lineNo = 0;
$scope = 0;
$comment = 0;

# List of reserved words
@reservedWords = ('true', 'false', 'self');

# Step through each line of the buffered script file
foreach $l (@scriptFile) {

	# Normal people start counting at 1
	$lineNo = $lineNo+1;

	# No objvar name has been found yet
	$varName = "";

	# Currently the variable has not been identified as a reserved word
	$reserved = 0;

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

	# If this line opens a curly brace, increase scope
	if($l =~ /(\{).*/) {

		if($1 eq "{") {

			$scope = $scope+1;

			#print "Increasing scope: scope is now $scope\n";
		}
	}

	# If this line closes a curly brace, decrease scope
	if($l =~ /(\}).*/) {

		if($1 eq "}") {

			$scope = $scope-1;

			#print "Decreasing scope: scope is now $scope\n";

			# This should not happen
			# if($scope < 0) {
			#
			#	print "WARNING: Scope mis-match error in line $lineNo\n";
			# }
		}
	}

	# If this line contains a call to the setObjVar() function
	if($l =~ /setObjVar/) {

		@parts = split /,/, $l;								# Split the line up by commas

		$varName = $parts[2];								# We only care about the part after the last comma
		$varName =~ s/^\s*//;								# Get rid of leading whitespace
		$varName =~ s/\s*\);.*\n*$//;						# Get rid of anything after and including the last ');'

		# These lines for debugging only
		# $l =~ s/^\s*//;
		# $l =~ s/\s*\n$//;

		# If the variable is only a number skip it, otherwise check deeper
		if($varName =~ /\D/) {

			# If the variable contains any numeric or boolean operators, it's probably not an array
			if($varName !~ /\||\+|\-|\&/) {
			
				# Cleaning up and fixing formatting for some weird cases
				if($varName =~ /^\(\w+\)\((.*)\)$/) {

					$varName = $1;

				} elsif ($varName =~ /^\((.*)\)$/) {

					$varName = $1;
				}

				# Go through the entire list of reserved words as described above
				foreach $word (@reservedWords) {

					# If our variable is a reserved word, we're fairly certain it's not an array
					if($varName eq $word) {

						$reserved = 1;
						last;
					}
				}

				# If the variable is not a reserved word, keep checking
				if(!$reserved) {

					# If the variable is surrounded by quotes, it is probably a literal string
					if($varName !~ /^".*"$/) {

						# If the variable is being called from another script it may be a constant, isolate variable name
						$varName =~ s/^.*\.//;

						# If the variable has only Uppercase letters, it's probably a constant, otherwise keep checking
						if($varName =~ /[a-z]+/) {

							# If the variable has a parenthesis, it is probably a function call, if not keep checking
							if($varName !~ /^\w+\s*\(/){

								#print "Starting check on $varName\n";

								# BEGIN RECURSIVE BUFFER CHECK FOR ARRAY DECLARATION
								$scopeCheck = $scope;
								$declaredLine = 0;
								$warning = 0;

								for($line = $lineNo-1; $line >= 0; $line--) {
									 
									if($scriptFile[$line] =~ /\w+\s*\[\]\s+\Q$varName\E|\w+\s+\Q$varName\E\s*\[\]/) {

											$warning = 1;
											$declaredLine = $line;
											last;
									}

									if($scopeCheck <= 0) {

										#print "Abandoning check due to scope!\n";
										last;
									}

									if($scriptFile[$line] =~ /\{/) {

										#print "Scope check! Scope is now $scopeCheck\n";
										$scopeCheck = $scopeCheck - 1;
									}

									if($scriptFile[$line] =~ /\}/) {

										#print "Scope check! Scope is now $scopeCheck\n";
										$scopeCheck = $scopeCheck + 1;
									}
								}

								if($declaredLine) {

									for($line = $lineNo-1; $line >= $declaredLine; $line--) {

										if($scriptFile[$line] =~ /$varName\.length/) {

											$warning = 0;
											last;
										}
									}
								}

								if($warning) {

									if($l =~ /[Ss][Uu][Pp][Pp][Rr][Ee][Ss][Ss]\W*$/) {

										print "$fileName:$lineNo: Cannot supress lint warning. Please provide reason for supression.\n";
										print "$fileName:$lineNo: WARNING: $varName could be an empty array set as an object variable\n";

									} elsif($l !~ /[Ss][Uu][Pp][Pp][Rr][Ee][Ss][Ss].*\w+.*$/) {

										print "$fileName:$lineNo: WARNING: $varName could be an empty array set as an object variable\n";
									}
								}

							# Else statement is for debugging only, we don't really need it
							# } else {
							#
							#	# Debugging statement
							#	print "Line $lineNo: $l Ignored because $varName looks like a function call\n";

							}

						# Else statement is for debugging only, we don't really need it
						# } else {
						#
						#	#Debugging statement
						#	print "Line $lineNo: $l Ignored because $varName looks like a constant\n";

						}

					# Else statement is for debugging only, we don't really need it
					# } else {
					#
					#	#Debugging statement
					#	print "Line $lineNo: $l Ignored because $varName looks like a literal string\n";

					}

				# Else statement is for debugging only, we don't really need it
				# } else {
				#
				#	# Debugging statement
				#	print "Line $lineNo: $l Ignored because $varName looks like a reserved word\n";

				}
			}

		# Else statement is for debugging only, we don't really need it
		# } else {
		#
		#	print "Line $lineNo: $l Ignored because $varName looks like a number\n";

		}
	}

	# If this line starts a comment block, note it and continue
	# All lines will be ignored until the comment block ends
	if($l =~ /\/\*/) {

		$comment = 1;
	}
}
