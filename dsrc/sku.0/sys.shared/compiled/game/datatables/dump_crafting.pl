#!/usr/bin/perl


# Defines

$output_filename  = "crafting_results.tab";
$schematic_group  = "j:\\swg\\x1\\dsrc\\sku.0\\sys.shared\\compiled\\game\\datatables\\crafting\\schematic_group.tab";


# Subroutines

sub printHeader;
sub printGroupHeader;
sub processCraftingData;
sub processSchematic;


###
# main
###

# Accumulate crafting info and dump analysis to disk.
print "\ndump_crafting process started\n";
print "Output file: $output_filename\n";

# Print tab delimited header.
printHeader();

# Process the crafting data.
processCraftingData();


###
# sub printHeader
###

sub printHeader
{
	open OUTFILE, ">$output_filename";
	print OUTFILE "Schematic\tUpdate\tComplexity\tSlots\tOpt Slots\tRes #\tComp #\tXP Value\tXP Eff\t\tS1 #\tS1 Type\tS2 #\tS2 Type\tS3 #\tS3 Type\tS4 #\tS4 Type\tS5 #\tS5 Type\tS6 #\tS6 Type\tS7 #\tS7 Type\tS8 #\tS8 Type\tS9 #\tS9 Type\tS10 #\tS10 Type\tDecay Rate\tHit Points\tXN1\tXV1\tXN2\tXV2\tXN3\tXV3\tXN4\tXV4\tXN5\tXV5\tXN6\tXV6\tXN7\tXV7\tXN8\tXV8\tXN9\tXV9\tXN10\tXV10\tXN11\tXV11\tXN12\tXV12\tXN13\tXV13\tXN14\tXV14\n";
	close OUTFILE;
}


###
# sub printGroupHeader
###

sub printGroupHeader
{
	open OUTFILE, ">>$output_filename" or die "Couldn't open $output_filename: $!\n";
	print OUTFILE "\n$_[0]\n";
	close OUTFILE;
}


###
# sub processCraftingData
###

sub processCraftingData
{
	# Open the schematic_group.tab
	open SCHMGRP, $schematic_group or die "Couldn't open $schematic_group: $!\n";
	
	# For each entry in the file, output crafting data.
	my $i = 2;
	$current_group = "";
	while ( <SCHMGRP> )
	{
		# Skip the first two lines.
		if ( $i > 0 )
		{
			$i--;
			next;
		}
		
		# Parse the group and schematic name.
		/(.*)\t(.*)/;
		$group_name = $1;
		$schematic_name = $2;
		if ( $group_name eq "end" )
		{
			next;
		}
		if ( $group_name ne $current_group )
		{
			# This is a new group, print group header.
			printGroupHeader( $group_name );
			$current_group = $group_name;
		}
		
		# Process the schematic.
		processSchematic( $schematic_name );
	}
	
	# Close schematic_group.tab.
	close SCHMGRP;
}


###
# sub processSchematic
###

sub processSchematic
{	
	# Strip the .iff.
	$_[0] =~ /(.*)\.iff/;
	$schematic_name = "$1.tpf";
	$schematic_name =~ s/\//\\/g;

	###
	# SERVER
	###

	# Open the schematic.
	open SCHM, "j:\\swg\\x1\\dsrc\\sku.0\\sys.server\\compiled\\game\\$schematic_name" or print "!! Couldn't open $schematic_name: $!\n";
	
	# Parse out the data we need.
	my $complexity = 0;
	my $slots = 0;
	my $optional_slots = 0;
	my $raw_count = 0;
	my $comp_count = 0;
	my @res_types;
	my @res_counts;
	while ( <SCHM> )
	{
		# Complexity
		if ( /^complexity/ )
		{
			/^complexity\s*=\s*(\d*)/;
			$complexity = $1;
		}
		
		# Slots
		if ( /ingredientType/ )
		{			
			# Resource Type
			/ingredient=\"(.*)\",/;
			$res_types[$slots] = $1;
			
			# Resource Count
			/count\s*=\s*(\d*)/;
			$res_counts[$slots] = $1;
			if ( /IT_resourceClass/ )
			{
				$raw_count += $res_counts[$slots];
			}
			else
			{
				$comp_count++;
			}
			$slots++;
		}
		
		# Optional Slots
		if ( /optional\s*=\s*true/ )
		{
			$optional_slots++;
		}
	}
		
	# Close the schematic.
	close SCHM;
	
	###
	# SHARED
	###
	
	# Update the name.
	$schematic_name =~ s/\\(\w*)\.tpf/\\shared_$1\.tpf/;
	
	# Open the shared schematic.
	open SCHM, "j:\\swg\\x1\\dsrc\\sku.0\\sys.shared\\compiled\\game\\$schematic_name" or print "!! Couldn't open $schematic_name: $!\n";
	
	# Parse out the data we need.
	my $decay_rate = "";
	my $hitpoints = "";
	my $xp_value = "";
	my @exp_names;
	my @exp_values;
	my $count = 0;
	while ( <SCHM> )
	{
		# Decay rate.
		if ( /decay_rate/ || /decayRate/ )
		{
			/.*value\s*=\s*(\d+\.\.\d+).*/;
			$decay_rate = $1;
		}

		# Hit points.
		if ( /hit_points"/ || /hitPoints/)
		{
			/.*value\s*=\s*(\d+\.\.\d+).*/;
			$hitpoints = $1;
		}
		
		# XP.
		if ( /\"xp\"/ )
		{
			/.*value\s*=\s*(\d+)\.\.\d+.*/;
			$xp_value = $1;
		}
		
		
		# Experiment 
		
		
		
			# Get Name
		if ( /"crafting"/ )
		{
			if ( $count < 16 )
			{
				/name\s*=\s*"crafting"\s\"(\w+).*/;
				
				if (!/complexity/ && !/hit_points/ && !/hitPoints/ && !/\bxp\b/)
				{
					$exp_names[$count] = $1;
					/.*value\s*=\s*(\d+\.\.\d+).*/;
					$exp_values[$count] = $1;	
					$count++;
				}
			}	
			
		}	
			
		
		
	}
	
	# Close the schematic.
	close SCHM;

	# Calculate XP value & efficiency.
	$xp_eff = 0;
	if ( $raw_count > 0 )
	{
		$xp_eff = $xp_value / $raw_count;
		$xp_eff = sprintf("%.1f", $xp_eff);
	}

	###
	# REPORT
	###

	# Update the name.
	$schematic_name =~ s/(shared_)//;
	
	# Open output file.
	open OUTFILE, ">>$output_filename" or die "Couldn't open $output_filename: $!\n";
	
	# Report results.
	print OUTFILE "$schematic_name\t\t$complexity\t$slots\t$optional_slots\t$raw_count\t$comp_count\t$xp_value\t$xp_eff\t";
	for ( my $i=0; $i<10; $i++ )
	{
		print OUTFILE "\t$res_counts[$i]\t$res_types[$i]";
	}
	print OUTFILE "\t$decay_rate\t$hitpoints";
	
	for ( my $j=0; $j<14; $j++ )
	{
		print OUTFILE "\t$exp_names[$j]\t$exp_values[$j]"
	}
	
	print OUTFILE "\n";
	
	
	# Close output.
	close OUTFILE;
}