#!/usr/bin/perl


# Defines

$source_filename  = "crafting_results.tab";
$complexity_field = 2;
$xp_field = 7;
$slot_field = 10;
$decay_field = 30;
$hp_field = 31;


# Subroutines

sub updateCraftingData;
sub updateSchematic;


###
# main
###

# Accumulate crafting info and dump analysis to disk.
print "\nupdate_crafting process started\n";
print "Source file: $source_filename\n";

# Process the crafting data.
updateCraftingData();

# Finish up.
print "\nupdate_crafting process ended\n";

###
# sub updateCraftingData
###

sub updateCraftingData
{
	# Open crafting data.
	open SOURCE, $source_filename or die $!;
	
	my $i = 3;
	while ( <SOURCE> )
	{
		# Skip header lines.
		while ( $i > 0 )
		{
			$i--;
			next;
		}
		
		if ( /\n$/ || /\r$/ )
		{
			chop;
		}
		if ( /\n$/ || /\r$/ )
		{
			chop;
		}
		
		# Check and see if this line needs updating.
		if ( /^(\S+)\tx\t/ )
		{
			# Update this line.
			updateSchematic( $1, $_ );
		}
	}
	
	# Close crafting data.
	close SOURCE;
}


###
# sub updateSchematic
###

sub updateSchematic
{
	# Indicate we are updating this schematic.
	print "Updating: $_[0]\n";
	
	# Parse out the things we are interested in updating.
	@fields = split /\t/, $_[1];
	
	###
	# SERVER
	###
	
	# Open the server file & output file.
	open SCHM_SRV, "j:\\swg\\x1\\dsrc\\sku.0\\sys.server\\compiled\\game\\$_[0]";
	open OUTPUT, ">j:\\swg\\x1\\dsrc\\sku.0\\sys.server\\compiled\\game\\$_[0].new";
	
	# Update the file.
	my $slot = 0;
	while ( <SCHM_SRV> )
	{
		if ( /\n$/ || /\r$/ )
		{
			chop;
		}
		if ( /\n$/ || /\r$/ )
		{
			chop;
		}
	
		# Update complexity.
		if ( /^complexity/ )
		{
			$_ = "complexity = $fields[$complexity_field]";
		}
		
		# Update slots.
		if ( /ingredientType/ )
		{
			# Ingredient.
			$type_idx = $slot_field + 2*$slot + 1;
			s/(.*)ingredient\s*=\s*".*",(.*)/$1ingredient="$fields[$type_idx]",$2/;
			
			# Count.
			$count_idx = $slot_field + 2*$slot;
			s/(.*)count\s*=\s*\d+(.*)/$1count=$fields[$count_idx]$2/;
			
			$slot++;
		}
						
		# Print the line into the output file.
		print OUTPUT $_ . "\n";
	}
	
	# Close the server side file & output file.
	close SCHM_SRV;
	close OUTPUT;
	
	# Rename the file.
	rename "j:\\swg\\x1\\dsrc\\sku.0\\sys.server\\compiled\\game\\$_[0].new", "j:\\swg\\x1\\dsrc\\sku.0\\sys.server\\compiled\\game\\$_[0]";
	
	###
	# SHARED
	###
	
	# Get the shared filename.
	$filename = $_[0];
	$filename =~ s/\\(\w*)\.tpf/\\shared_$1\.tpf/;
	print "Updating: $filename\n";
	
	# Open the shared file & output file.
	open SCHM_SHD, "j:\\swg\\x1\\dsrc\\sku.0\\sys.shared\\compiled\\game\\$filename";
	open OUTPUT, ">j:\\swg\\x1\\dsrc\\sku.0\\sys.shared\\compiled\\game\\$filename.new";
	
	while ( <SCHM_SHD> )
	{
		if ( /\n$/ || /\r$/ )
		{
			chop;
		}
		if ( /\n$/ || /\r$/ )
		{
			chop;
		}

		# Update complexity.
		if ( /complexity/ )
		{
			$cmp = $fields[$complexity_field];
			s/(.*)value\s*=\s*\d+\.\.\d+(.*)/$1value = $cmp\.\.$cmp$2/;
		}

		# Update xp.
		if ( /\"xp\"/ )
		{
			$xp = $fields[$xp_field];
			s/(.*)value\s*=\s*\d+\.\.\d+(.*)/$1value = $xp\.\.$xp$2/;
		}

		# Update decay rate.
		if ( /\"decay_rate\"/ || /\"decayRate\"/ )
		{
			$decay = $fields[$decay_field];
			s/(.*)value\s*=\s*\d+\.\.\d+(.*)/$1value = $decay$2/;
		}

		# Update hitpoints.
		if ( /\"hit_points\"/ || /\"hitPoints\"/ || /\"hitPointsMax\"/)
		{
			$hp = $fields[$hp_field];
			s/(.*)value\s*=\s*\d+\.\.\d+(.*)/$1value = $hp$2/;
		}
		# Update Experimental Values
		$count = 32;
		while ($count < 60)
		{	
			if ( /\"$fields[$count]\"/ && ($fields[$count] ne "") )
			{
				$count++;
				$exp_1 = $fields[$count];
				s/(.*)value\s*=\s*\d+\.\.\d+(.*)/$1value = $exp_1$2/;
			}
			else
			{
				$count++;
			}
			$count++;
		}
		
		# Print the line into the output file.
		print OUTPUT $_ . "\n";
	}
	
	# Close the shared file & output file.
	close SCHM_SHD;
	close OUTPUT;
	
	# Rename the file.
	rename "j:\\swg\\x1\\dsrc\\sku.0\\sys.shared\\compiled\\game\\$filename.new", "j:\\swg\\x1\\dsrc\\sku.0\\sys.shared\\compiled\\game\\$filename" or die "Couldn't rename file: $!";
}