#!/usr/bin/perl

# Constants

$balance_sheet = "ship_balance.txt";
$comp_sheet = "crafted_component_balance.txt";

#$template_path = "C:\Projects\swg\current\exe\win32\TemplateCompiler.exe
$server_prefix = "\\swg\\current\\dsrc\\sku.0\\sys.server\\compiled\\game\\object\\draft_schematic\\space";
$shared_prefix = "\\swg\\current\\dsrc\\sku.0\\sys.shared\\compiled\\game\\object\\draft_schematic\\space";
$shared_bin_prefix = "\\swg\\current\\data\\sku.0\\sys.shared\\compiled\\game\\object\\draft_schematic\\space";

###
# main
###

# Read our balance sheet.
print "\nbalance_space_crafted process started\n";

if ( $ARGV[0] ne undef )
{
	$argtype = $ARGV[0];
}

# Read balance data.
readCoreBalance();
calculateCoreBalance();
readCraftedChunks();

###
# sub readCoreBalance
# Overall function for reading the balance sheet.
###

sub readCoreBalance
{
	open BALANCE, "<$balance_sheet";
	
	while ( <BALANCE> )
	{
		chop;
		@fields = split /\t/;
		if ( /Core Balance Data/ )
		{
			@header = @fields;
		}
		elsif ( /Player Baseline/ )
		{
			@baseline = @fields;
		}
		elsif ( /Player Factor/ )
		{
			@player_factor = @fields;
		}
	}
	
	close BALANCE;
}

###
# sub calculateCoreBalance
###

sub calculateCoreBalance
{
	for ( my $i=0; $i<@header; $i++ )
	{
		if ( $i == 0 )
		{
			next;
		}
		
		for ( my $tier=1; $tier<6; $tier++ )
		{
			$key = $header[$i];
			my $name = $key . "_tier" . $tier;
			
			# Modify by the player tier factor.
			$f = $baseline[$i];
			if ( $tier > 1 )
			{
				@factor = split /!/, $player_factor[$i];
				$pref = $factor[0];
				$postf = $factor[1];
				
				for ( my $x=1; $x<$tier; $x++ )
				{
					$f *= $pref;
					$pref *= $postf;
				}
			}
			$f = sprintf( "%.2f", $f );
			$balancedata{$name} = $f;
		}
	}
}

###
# sub readCraftedChunks
###

sub readCraftedChunks
{
	# Read balance data from the loot sheet.
	open CRAFTED_COMPONENTS, "<$comp_sheet";
	while ( <CRAFTED_COMPONENTS> )
	{
		$line = $_;
		for ( $chunk )
		{
			# Baselines
			/Mass Distribution/ and do { $massb = readBaseline( $line, $massb, \$massindex ); next; };
			/Reactor Drain Distribution/ and do { $reactorb = readBaseline( $line, $reactorb, \$reactorindex ); next; };
		
			# Chassis mass distribution.
			/Chassis/ and do { processChassis( $line ); next; };
			
			# Component chunks.
			if ( $chunk ne "" )
			{
				processComponentChunk( $line );
				next;
			}
			
			@cfields = split /\t/, $line;
			$chunk = $cfields[0];
			$directory = $cfields[1];
			$varmod = $cfields[2];
		}	
	}
	close CRAFTED_COMPONENTS;
}

###
# sub readBaseline
###

sub readBaseline
{
	my ($line, $var, $index) = @_;

	# Check for end of chunk.
	if ( $line eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return $var;
	}
	
	# Read some data.
	chop($line);
	@fields = split /\t/, $line;
	if ( $fields[0] ne "BASELINE" )
	{
		return $line;
	}
	else
	{
		$$index = $line;
	}
}

###
# sub processChassis
###

sub processChassis
{
	$line = $_[0];

	# Check for end of chunk.
	if ( $line eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return;
	}
	return;
	# Write chassis mass values to the chassis draft schematics.
	chop($line);
	@fields = split /\t/, $line;
	if ( $fields[0] eq "Name" )
	{
		return;
	}
	else
	{
		my $filename = $shared_prefix . "\\chassis\\shared_" . $fields[0] . ".tpf";
		$filename =~ s/\\/\//g;
		my $p4name = $filename;
		my $filename = $shared_prefix . "\\chassis\\shared_" . $fields[0] . ".tpf";
		
		# Calculate the mass range for this chassis.
		@mass = split /\t/, $massb;
		$medmass = $fields[1] * $mass[0];
		$massvar = $fields[2] * $mass[0];
		$mass_min = $medmass - ($massvar/2);
		$mass_max = $medmass + ($massvar/2);
		
		# Check out the file to be edited.
		system( "p4 edit //depot" . $p4name );
		
		# Read the schematic.
		open CHASSIS, "<$filename";
		my $i = 0;
		while ( my $z = <CHASSIS> )
		{
			# Read the file into a buffer.
			$lines[$i++] = $z;
		}
		close CHASSIS;

		# Dump the modified schematic.
		open CHASSIS, ">$filename";
		for ( my $i=0; $i<@lines; $i++ )
		{
			if ( $lines[$i] =~ m/\"massMax\"/ )
			{
				$lines[$i] =~ s/(\d*)\.\.(\d*)/$mass_min\.\.$mass_max/;
			}
			print CHASSIS $lines[$i];
		}
		close CHASSIS;
		@lines = ();
		
		# Check out the .iff.
		my $sfilename = $shared_bin_prefix . "\\chassis\\shared_" . $fields[0] . ".iff";
		$sfilename =~ s/\\/\//g;
		my $sp4name = $sfilename;
		system( "p4 edit //depot" . $sp4name );
		
		# Compile the freaking file.
		system( "templatecompiler -compile $filename" );
	}
}

###
# sub processComponentChunk
###

sub processComponentChunk
{
	$line = $_[0];

	# Check for end of chunk.
	if ( $line =~ m/^\s*\n$/  )
	{
		# End of this chunk.
		$header = 0;
		$chunk = "";
		return;
	}
	chop($line);
	@fields = split /\t/, $line;
	
	if ( ($argtype ne undef) && ($directory ne $argtype) )
	{
		return;
	}
	
	# Read the chunk header.
	if ( $header == 0 )
	{
		@categories = @fields;
		$header++;
		return;
	}
	elsif ( $header == 1 )
	{
		@bal_cat_names = @fields;
		$header++;
		return;
	}
	elsif ( $header == 2 )
	{
		@var_names = @fields;
		$header++;
		return;
	}
	elsif ( $header == 3 )
	{
		@exp_names = @fields;
		$header++;
		return;
	}
	elsif ( $header == 4 )
	{
		@baseline = @fields;
		$header++;
		return;
	}
	
	# Read a component chunk entry.

	my $filename = "$shared_prefix\\$directory\\shared_$fields[0].tpf";
	$filename =~ s/\\/\//g;
	my $p4name = $filename;
	my $filename = "$shared_prefix\\$directory\\shared_$fields[0].tpf";
	
	print $filename . "\n";

	# Check out the file to be edited.
	system( "p4 edit //depot" . $p4name );
		
	# Read the schematic.
	open SCHEMATIC, "<$filename";
	my $i = 0;
	while ( my $z = <SCHEMATIC> )
	{
		# Read the file into a buffer.
		$lines[$i++] = $z;
	}
	close SCHEMATIC;
	
	# Write out our changes.
 	open SCHEMATIC, ">$filename";
	for ( my $i=0; $i<@lines; $i++ )
	{
		$lines[$i] =~ m/\"crafting\" \"(\w*)\", exp/;
		$var = $1;
		if ( $var eq undef )
		{
			print SCHEMATIC $lines[$i];
			next;
		}
		my $x = 0;
		foreach $str (@var_names)
		{
			if ( $var eq $str )
			{
#				print "var: $var\n";
				
				# Find our cert level.
				if ( $categories[1] eq "Cert" )
				{
					$cert = $fields[1];
				}
				else
				{
					$cert = 1;
				}

				# Find our tier.				
				$tier = sprintf( "%.0f", $cert/2 );
				$step = ($fields[1] % 2) == 0;
				$mod = 1;
				if ( $step )
				{
					$mod = 1.25;
				}
				
				# We want to calculate the correct range for this crafting variable.
				# >>>>>> THIS IS THE MEAT OF THIS CODE
#				print "  exp: $exp_names[$x]\n";

				$balcat = $bal_cat_names[$x];
				if ( $balcat =~ m/None/ )
				{
					# Just use this range as stated in the balance line.
					$range_min = $fields[$x];
					$range_max = $fields[$x];
				}
				elsif ( $balcat =~ m/Var/ )
				{
					# Just use this range as stated in the balance line, with variance.
					$variance = $fields[$x] * 0.15 * $varmod;
					$range_min = $fields[$x] - ($variance/2);
					$range_max = $fields[$x] + ($variance/2);
				}
				elsif ( $balcat =~ m/Mass/ )
				{
					# Mass is derived from the mass baseline for this object type.
					@massindexf = split /\t/, $massindex;
					@massf = split /\t/, $massb;
					for ( my $y=0; $y<@massindexf; $y++ )
					{
						if ( $massindexf[$y] eq $directory )
						{
							$basemass = $massf[$y] * $massf[0] * $fields[$x];
							$variance = $basemass * 0.15 * $varmod;
							$range_min = $basemass - ($variance/2);
							$range_max = $basemass + ($variance/2);
							last;
						}
					}
				}
				elsif ( $balcat =~ m/ReactorDrain/ )
				{
					# Reactor drain is derived from the reactor baseline for this object type.
					@drainindexf = split /\t/, $reactorindex;
					@drainf = split /\t/, $reactorb;
					for ( my $y=0; $y<@drainindexf; $y++ )
					{
						if ( $drainindexf[$y] eq $directory )
						{
							$basedrain = $drainf[$y] * $drainf[0] * $fields[$x];
							$variance = $basedrain * 0.15 * $varmod;
							$range_min = $basedrain - ($variance/2);
							$range_max = $basedrain + ($variance/2);
							last;
						}
					}
				}
				elsif ( $balcat =~ m/ReactorGen/ )
				{
					# Reactor gen is specific to the crafted object balance sheet.
					$data = $baseline[$x] * $fields[$x];
					$variance = $data * 0.15 * $varmod;
					$range_min = $data - ($variance/2);
					$range_max = $data + ($variance/2);
				}
				elsif ( $balcat =~ m/CompArmor/ )
				{
					# Armor on components is half normal component value.
					$data = $balancedata{"Component_tier".$tier} * $fields[$x] * $mod;
					$data = $data / 2;
					$variance = $data * 0.15 * $varmod;
					$range_min = $data - ($variance/2);
					$range_max = $data + ($variance/2);
				}
				elsif( ($balcat =~ m/WeaponRate/) || ($balcat =~ m/WeaponEffShield/) || ($balcat =~ m/WeaponEffArmor/) || ($balcat =~ m/ShieldRegen/) )
				{
					# These entries must be scaled up.
					$data = $balancedata{$bal_cat_names[$x]."_tier".$tier} * $fields[$x] * $mod * 1000;
					$variance = $data * 0.15 * $varmod;
					$range_min = $data - ($variance/2);
					$range_max = $data + ($variance/2);
				}
				else
				{
					# A normal entry.
					$data = $balancedata{$bal_cat_names[$x]."_tier".$tier} * $fields[$x] * $mod;
					$variance = $data * 0.15 * $varmod;
					$range_min = $data - ($variance/2);
					$range_max = $data + ($variance/2);
				}
				
				# Maintain left-to-right relationship.
				if ( $range_max < $range_min )
				{
					my $temp = $range_max;
					$range_max = $range_min;
					$range_min = $temp;
				}
				
				# Format into integers.
				$range_min = sprintf( "%.0f", $range_min );
				$range_max = sprintf( "%.0f", $range_max );
				
#				print "  range: $range_min..$range_max\n";

				$lines[$i] =~ m/^(.*=\s*)-?\d+\.\.-?\d+(].*)$/;
#				print $1 . "\n";
#				print $2 . "\n";
				$lines[$i] = $1 . $range_min . ".." . $range_max . $2 . "\n";
#				print $lines[$i] . "\n";
				
				last;
			}
			
			$x++;
		}
	
		print SCHEMATIC $lines[$i];
 	}
 	close SCHEMATIC;	
	
	@lines = ();

	# Check out the .iff.
	my $sfilename = "$shared_bin_prefix\\$directory\\shared_$fields[0].iff";
	$sfilename =~ s/\\/\//g;
	my $sp4name = $sfilename;
	system( "p4 edit //depot" . $sp4name );

	# Compile the freaking file.
	system( "templatecompiler -compile $filename" );
}
