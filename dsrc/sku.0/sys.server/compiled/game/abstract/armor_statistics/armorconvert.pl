#!/usr/bin/perl

# Author: Brandon Reinhart
# Date:   04/10/03
#
# Converts armor_templates from version 0 to version 1.

use Cwd;

print "Converting armor statistics files to new format.\n";

# Test to make sure the file is a .tpf and convert it to the new format.
sub convertFile
{
#	print "  --> \[$_[0]\]\n";
	
	# Cull files that are not .tpfs.
	$_ = $_[0];
	if ( !/\.tpf$/ )
	{
		return;
	}
	
	# Open the file and our target file.
	open CURFILE, $_[0];
	open NEWFILE, ">$_[0].new";
	
	# Convert this file.
	$vullist = "0";
	$linetoprint = "";
	$printline = 0;
	while( <CURFILE> )
	{
		if ( /\n$/ )
		{
			chop;
		}
		if ( /\r$/ )
		{
			chop;
		}
		
		if ( $printline == 1 )
		{
			if ( $linetoprint =~ /type/ )
			{
				/effectiveness\s*=(\d*)/;
				$effnum = $1;
				if ( /type/ && $effnum > 1 )
				{
					chop($linetoprint);
					$linetoprint .= ",";
				}
				else
				{
					chop($linetoprint);
					$linetoprint .= "]";
				}
			}
			print NEWFILE $linetoprint . "\n";
			$linetoprint = "";
		}
		$printline = 1;
		
		# Convert class entry.
		s/class\s*armor_template\s*0/class armor_template 1/;
		
		# Convert specialProtection.
		if ( /type/ )
		{
			# Get endchar.
			/(.).$/;
			$endchar = $1;
		
			# Get type.
			/type\s*=\s*(.*),\s*elemental/;
			$pretype = $1;
			if ( $pretype =~ /elemental/ || $pretype =~ /environmental/ )
			{
				# Get elemental subtype.
				/elementalType\s*=\s*EDT(.*),\s*effect/;
				$pretype .= $1;
			}
			
			# Get effectiveness.
			/effectiveness\s*=(\d*)/;
			$effnum = $1;
			if ( $effnum == 0 || $effnum == 1 )
			{
				$vullist .= " \| $pretype";
				$printline = 0;
			}
			else
			{
				# Write the new line.
				$_ = "	[type = $pretype, effectiveness = $effnum]]";
			}
		}
		
		# Convert encumberance.
		if ( /encumberance/ )
		{
			# Get health.
			if ( /AT_strength/ )
			{
				/AT_strength]\s*=\s*(\d*)/;
				$_ = "encumbrance [0] = $1";
			}
			elsif ( /AT_quickness/ )
			{
				/AT_quickness]\s*=\s*(\d*)/;
				$_ = "encumbrance [1] = $1";
			}
			elsif ( /AT_focus/ )
			{
				/AT_focus]\s*=\s*(\d*)/;
				$_ = "encumbrance [2] = $1";
			}
			else
			{
				$printline = 0;
			}
		}
		
		# Put contents of line into new file.
		if ( $printline == 1 )
		{
			$linetoprint = $_;
		}
	}
	
	# Add final line.
	print NEWFILE $linetoprint;
	
	# Add the vulnerability line.
	print NEWFILE "vulnerability = $vullist\n";
	
	# Close the file.
	close CURFILE;
	close NEWFILE;

	# Rename the file.
	rename $_[0], "$_[0].old";
	rename "$_[0].new", $_[0];
	
	# Go through again.
	open CURFILE, $_[0];
	open NEWFILE, ">$_[0].new";
	$printline = 0;
	while ( <CURFILE> )
	{
		if ( /\n$/ )
		{
			chop;
		}
		if ( $printline == 1 )
		{
			# On the second pass, remove any empty sp blocks.
			if ( $linetoprint =~ /specialProtection/ )
			{
				if ( /type/ )
				{
					print NEWFILE $linetoprint . "\n";
				}
				else
				{
					print NEWFILE "specialProtection = []\n";
				}
			}
			else
			{
				print NEWFILE $linetoprint . "\n";
			}
		}
		
		$printline = 1;
		$linetoprint = $_;
	}
	
	# Add final line.
	print NEWFILE $linetoprint;

	# Close the files.
	close CURFILE;
	close NEWFILE;

	# Rename the file.
	rename "$_[0].new", $_[0];
}

sub fileRename
{
	rename "$_[0].old", $_[0];
}

# Open all files in a directory and convert them.  Called recursively.
sub convertDir
{
	print "OPENING \[$_[0]\]\n";
	opendir THISDIR, $_[0] or die "I could not open $_[0]: $!\n";
	my @allfiles = grep !/^\.\.?$/, readdir THISDIR;
	closedir THISDIR;
	chdir( $_[0] );
	
	foreach $file (@allfiles)
	{
		if ( -d $file )
		{
			convertDir($file);
			chdir( ".." ) or die "Can't change to .. $!\n";
		}
		else
		{
			convertFile($file);
			#fileRename($file);
		}
	}
}

convertDir ".";


