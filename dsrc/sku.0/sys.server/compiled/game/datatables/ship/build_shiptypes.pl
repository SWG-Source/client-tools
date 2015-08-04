#!/usr/bin/perl

# Constants

$balance_sheet = "ship_balance.txt";
$output_filename  = "shiptype_delta.tab";
$shiptypes = "shiptype.tab";
$shiptypesout = "shiptype_out.tab";
$header = "Name\tSlideDamp\tMass\tShield\tShieldRegen\tArmor\tComponent\tChassis\tDamageMin\tDamageMax\tWeaponDrain\tWeaponRate\tWeaponEffShield\tWeaponEffArmor\tWeaponAmmo\tCapacitorEnergy\tCapacitorRegen\tEngineSpeed\tEngineAccel\tEngineDecel\tEngineYaw\tEngineYawAccel\tEnginePitch\tEnginePitchAccel\tEngineRoll\tEngineRollAccel\tBoosterEnergy\tBoosterRecharge\tBoosterConsumption\tBoosterAccel\tBoosterSpeed";

$chassis_dir = "\\swg\\current\\dsrc\\sku.0\\sys.shared\\compiled\\game\\datatables\\space";
$objects_dir = "\\swg\\current\\dsrc\\sku.0\\sys.server\\compiled\\game\\object\\ship";

# Subroutines

sub processData;
sub readDefaults;
sub readBalance;
sub readModifiers;
sub readComponents;
sub calculateBalance;
sub dumpData;
sub updateShipTypes;
sub createShipChassisTables;

###
# main
###

# Read our balance sheet.
print "\nbuild_shiptypes process started\n";

if ( $ARGV[0] eq undef )
{
	print "\nYou must specify a command line option:\n";
	print " +balance\t- Generate balance data.\n";
	print " +chassis\t- Generate chassis tables.\n";
	print " +objects\t- Generate ship tangible objects.\n";
	print " +xp\t\t- Generate xp data.\n";
	print " +all\t\t- Do it all.\n";
	exit();
}

#print "Output file: $output_filename\n";

# Read balance data.
processData();

# Calculate balance data.
calculateBalance();

# Dump an interim file.
dumpData();

# Update the ship type data.
if ( $ARGV[0] eq "+balance" || $ARGV[0] eq "+all" )
{
	updateShipTypes();
}

# Update xp data.
if ( $ARGV[0] eq "+xp" || $ARGV[0] eq "+all" )
{
	updateXPData();
}

# Hopefully we won't have to do this in the future.
if ( $ARGV[0] eq "+chassis" || $ARGV[0] eq "+all" )
{
	createShipChassisTables();
}
if ( $ARGV[0] eq "+objects" || $ARGV[0] eq "+all" )
{
	createShipObjects();
}

###
# sub processData
# Overall function for reading the balance sheet.
###

sub processData
{
	#print "processData\n";

	open BALANCE, "<$balance_sheet";
	
	$chunk = "";
	while ( <BALANCE> )
	{
		$line = $_;
		for ( $chunk )
		{
			/Defaults/ and do { readDefaults( $line ); next; };
			/Core Balance Data/ and do { readBalance( $line ); next; };
			/Ship Modifiers/ and do { readModifiers( $line ); next; };
			/Component Progression/ and do { readComponents( $line ); next; };
			$chunk = (split /\t/, $line)[0];
		}
	}
	
	close BALANCE;
}

###
# sub readDefaults
# Stores the defaults chunk of the balance sheet.
###

sub readDefaults
{
	# Check for end of chunk.
	if ( $_[0] eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return;
	}
	
	# Read defaults.
	chop($_[0]);
	@fields = split /\t/, $_[0];
	$defaults{$fields[0]} = $fields[1];
}

###
# sub readBalance
# Stores the main balance data.
###

sub readBalance
{
	if ( $_[0] eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return;
	}

	# Read some balance data.
	chop($_[0]);
	@fields = split /\t/, $_[0];
	
	if ( $fields[0] eq "AI Baseline" )
	{
		# Read the baseline.
		@baseline = @fields;
	}
	elsif ( $fields[0] eq "AI Factor" )
	{
		# Read the tier factor.
		@tierfactor = @fields;
	}
	elsif ( $fields[0] eq "Player Baseline" )
	{
		# Read the player's baseline.
		@player_baseline = @fields;
	}
	elsif ( $fields[0] eq "Player Factor" )
	{
		# Read the player's tier factor.
		@player_tierfactor = @fields;
	}
	else
	{
		#print "Skipping '$fields[0]'\n";
	}
}

###
# sub readModifiers
# Stores the per-ship modifiers data in a hash.
###

sub readModifiers
{
	if ( $_[0] eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return;
	}
	
	# Read ship specific modifier data.
	chop($_[0]);
	@fields = split /\t/, $_[0];
	$shipmodifiers{$fields[0]} = join '!', @fields;
}

###
# sub readComponents
# Stores the per-ship components data in a hash of hashes.
###

sub readComponents
{
	if ( $comp_chunk eq undef )
	{
		chop($_[0]);
		@fields = split /\t/, $_[0];
		$comp_chunk = $fields[0];
		#print "Handling components for '$comp_chunk'\n";
		return;
	}
	elsif ( $_[0] eq "\n" )
	{
		$comp_chunk = "";
	}
	else
	{
		chop($_[0]);
		@fields = split /\t/, $_[0];
		
		$def = $defaults{$fields[0]};
		
		my $result;
		for ( my $i=1; $i<6; $i++ )
		{
			if ( $fields[$i] ne undef )
			{
				$def = $fields[$i];
			}
			if ( $def eq undef )
			{
				$result .= "0";
			}
			else
			{
				$result .= $def;
			}
			if ( $i < 5 )
			{
				$result .= "|";
			}
		}
		$components{$comp_chunk}{$fields[0]} = $result;
	}
}

###
# sub calculateBalance
###

sub calculateBalance
{
	foreach $key (keys(%shipmodifiers))
	{
		for ( my $tier=1; $tier<6; $tier++ )
		{
			my $name = $key . "_tier" . $tier;
			$balancedata{$name}[0] = $name;
			
			for ( my $i=1; $i<@baseline; $i++ )
			{
				# Modify by the baseline.
				@mods = split '!', $shipmodifiers{$key};
				$f = $mods[$i] * $baseline[$i];
				
				# Modify by the tier factor.
				if ( $tier > 1 )
				{
					@factor = split /!/, $tierfactor[$i];
					$pref = $factor[0];
					$postf = $factor[1];
					
					for ( my $x=1; $x<$tier; $x++ )
					{
						$f *= $pref;
						$pref *= $postf;
					}
				}
				$f = sprintf( "%.2f", $f );
				$balancedata{$name} = $balancedata{$name} . '!' . $f;
			}
		}
	}
}

###
# sub dumpData
###

sub dumpData
{
	print "Writing new ship balance data...\n";
	
	print $balancedata{"z95_tier1"}[1] . "\n";
	
	open OUTFILE, ">$output_filename";
	
	print OUTFILE $header . "\n";
	
	foreach $key (sort(keys(%balancedata)))
	{
		print OUTFILE $key;
		@output = split '!', $balancedata{$key};
		for ( my $i=1; $i<@output; $i++ )
		{
			print OUTFILE "\t";
			printf OUTFILE "%.2f", $output[$i];
		}
		print OUTFILE "\n";
	}

	close OUTFILE;
}

###
# sub updateShipTypes
###

sub updateShipTypes
{	
	print "Updating shiptype.tab...\n";
	
	# Read the file into a buffer.
	open SHIPTYPES, "<$shiptypes";
	my $i = 0;
	while ( <SHIPTYPES> )
	{
		$shiptype_lines[$i++] = $_;
	}	
	close SHIPTYPES;
	
	open SHIPTYPES, ">$shiptypesout";

	# Write out old records.
	foreach $line (@shiptype_lines)
	{
		$write = 1;
		@fields = split /\t/, $line;
		foreach $key (keys(%balancedata))
		{
			if ( $fields[0] eq $key )
			{
				# Skip this line.
				$write = 0;
			}
		}
		$write and print SHIPTYPES (join "\t", @fields);
	}

	# Build header index, so we can more easily access fields as needed.
	@headers = split /\t/, $header;
	for ( my $j=0; $j<@headers; $j++ )
	{
		$h{$headers[$j]} = $j;
	}
	
	# Write out new records.
	foreach $key (sort(keys(%balancedata)))
	{
		@o = split '!', $balancedata{$key};
		@n = split "_tier", $key;
		
		# name
		print SHIPTYPES $key . "\t";

		# slideDamp
		print SHIPTYPES $o[$h{"SlideDamp"}] . "\t";
		
		# HitPoints
		print SHIPTYPES $o[$h{"Chassis"}] . "\t";
		
		# massMax
		print SHIPTYPES $o[$h{"Mass"}] . "\t";
		
		# reactor, reactor_hp, reactor_arm
		$t = (split /\|/, $components{$n[0]}{"reactor"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES $defaults{"reactor"} . "\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
		}
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		print SHIPTYPES $o[$h{"Component"}] . "\t";

		# engine, engine_hp, engine_arm
		$t = (split /\|/, $components{$n[0]}{"engine"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES $defaults{"engine"} . "\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
		}
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		
		# performance stuff
		print SHIPTYPES $o[$h{"EngineSpeed"}] . "\t";
		print SHIPTYPES $o[$h{"EngineAccel"}] . "\t";
		print SHIPTYPES $o[$h{"EngineDecel"}] . "\t";
		print SHIPTYPES $o[$h{"EngineYaw"}] . "\t";
		print SHIPTYPES $o[$h{"EngineYawAccel"}] . "\t";
		print SHIPTYPES $o[$h{"EnginePitch"}] . "\t";
		print SHIPTYPES $o[$h{"EnginePitchAccel"}] . "\t";
		print SHIPTYPES $o[$h{"EngineRoll"}] . "\t";
		print SHIPTYPES $o[$h{"EngineRollAccel"}] . "\t";
		
		# shield 0 (shield_0, shield_0_hp, shield_0_arm, shield_0_front, shield_0_back, shield_0_regen)
		$t = (split /\|/, $components{$n[0]}{"shield"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES $defaults{"shield"} . "\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
		}
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		print SHIPTYPES $o[$h{"Shield"}] . "\t";
		print SHIPTYPES $o[$h{"Shield"}] . "\t";
		print SHIPTYPES $o[$h{"ShieldRegen"}] . "\t";
		
		# shield 1 (shield_1, shield_1_hp, shield_1_arm, shield_1_front, shield_1_back, shield_1_regen)
		# Nothing for now.
		print SHIPTYPES "\t\t\t\t\t\t";
#		$t = (split /\|/, $components{$n[0]}{"shield"})[$n[1]-1];
#		if ( $t eq "0" )
#		{
#			print SHIPTYPES $defaults{"shield"} . "\t";
#		}
#		else
#		{
#			print SHIPTYPES $t . "\t";
#		}
#		print SHIPTYPES $o[$h{"Component"}] . "\t";
#		print SHIPTYPES $o[$h{"Component"}] . "\t";
#		print SHIPTYPES $o[$h{"Shield"}] . "\t";
#		print SHIPTYPES $o[$h{"Shield"}] . "\t";
#		print SHIPTYPES $o[$h{"ShieldRegen"}] . "\t";

		# armor_0, armor_0_hp, armor_0_arm
		$t = (split /\|/, $components{$n[0]}{"armor"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES $defaults{"armor"} . "\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
		}
		print SHIPTYPES $o[$h{"Armor"}] . "\t";
		print SHIPTYPES $o[$h{"Armor"}] . "\t";
		
		# armor_1, armor_1_hp, armor_1_arm
		$t = (split /\|/, $components{$n[0]}{"armor"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES $defaults{"armor"} . "\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
		}
		print SHIPTYPES $o[$h{"Armor"}] . "\t";
		print SHIPTYPES $o[$h{"Armor"}] . "\t";		

		# capacitor, capacitor_hp, capacitor_arm, capacitor_energy, capacitor_recharge
		$t = (split /\|/, $components{$n[0]}{"capacitor"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES $defaults{"capacitor"} . "\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
		}
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		print SHIPTYPES $o[$h{"Component"}] . "\t";
		print SHIPTYPES $o[$h{"CapacitorEnergy"}] . "\t";
		print SHIPTYPES $o[$h{"CapacitorRegen"}] . "\t";
		
		# booster, booster_hp, booster_arm, booster_energy, booster_recharge, booster_consumption, booster_accel, booster_speed
		$t = (split /\|/, $components{$n[0]}{"booster"})[$n[1]-1];
		if ( $t eq "0" or $t eq undef )
		{
			print SHIPTYPES "\t\t\t\t\t\t\t\t";
		}
		else
		{
			print SHIPTYPES $t . "\t";
			print SHIPTYPES $o[$h{"Component"}] . "\t";
			print SHIPTYPES $o[$h{"Component"}] . "\t";
			print SHIPTYPES $o[$h{"BoosterEnergy"}] . "\t";
			print SHIPTYPES $o[$h{"BoosterRecharge"}] . "\t";
			print SHIPTYPES $o[$h{"BoosterConsumption"}] . "\t";
			print SHIPTYPES $o[$h{"BoosterAccel"}] . "\t";
			print SHIPTYPES $o[$h{"BoosterSpeed"}] . "\t";
		}
	
		# droid_interface, droid_interface_hp, droid_interface_arm
		# Nothing for now.
		print SHIPTYPES "\t\t\t";
		
		# bridge
		# Nothing for now.
		print SHIPTYPES "\t\t\t";
		
		# hangar
		# Nothing for now.
		print SHIPTYPES "\t\t\t";
		
		# targeting_station
		# Nothing for now.
		print SHIPTYPES "\t\t\t";
		
		# Weapons.
		for ( my $x=0; $x<8; $x++ )
		{
			$t = (split /\|/, $components{$n[0]}{"weapon_".$x})[$n[1]-1];
			if ( $t eq "0" or $t eq undef )
			{
				print SHIPTYPES "\t\t\t\t\t\t\t\t\t\t\t";
			}
			else
			{
				# Is this a missile?
				if ( $t =~ m/\"(.*)\((.*)\)\"/ )
				{
					# Pull out the stats.
					@misstats = split ',', $2;
					$missile_rate = $misstats[0];
					$missile_drain = $misstats[1];
					$missile_min_dam = $misstats[2];
					$missile_max_dam = $misstats[3];
					$missile_eff_shield = $misstats[4];
					$missile_eff_armor = $misstats[5];
					$missile_count = $misstats[6];
					$missile_difficulty = $misstats[7];	

					print SHIPTYPES $1 . "\t";
					print SHIPTYPES $o[$h{"Component"}] . "\t";
					print SHIPTYPES $o[$h{"Component"}] . "\t";
					print SHIPTYPES $missile_rate . "\t";
					print SHIPTYPES $o[$h{"WeaponDrain"}]*$missile_drain . "\t";
					print SHIPTYPES $o[$h{"DamageMin"}]*$missile_min_dam . "\t";
					print SHIPTYPES $o[$h{"DamageMax"}]*$missile_max_dam . "\t";
					print SHIPTYPES $missile_eff_shield . "\t";
					print SHIPTYPES $missile_eff_armor . "\t";
					print SHIPTYPES $missile_count . "\t";
					print SHIPTYPES $missile_difficulty . "\t";
				}
				else
				{
					print SHIPTYPES $t . "\t";
					print SHIPTYPES $o[$h{"Component"}] . "\t";
					print SHIPTYPES $o[$h{"Component"}] . "\t";
					print SHIPTYPES $o[$h{"WeaponRate"}] . "\t";
					print SHIPTYPES $o[$h{"WeaponDrain"}] . "\t";
					print SHIPTYPES $o[$h{"DamageMin"}] . "\t";
					print SHIPTYPES $o[$h{"DamageMax"}] . "\t";
					print SHIPTYPES $o[$h{"WeaponEffShield"}] . "\t";
					print SHIPTYPES $o[$h{"WeaponEffArmor"}] . "\t";
					if ( $o[$h{"WeaponAmmo"}] > 1 )
					{
						print SHIPTYPES $o[$h{"WeaponAmmo"}] . "\t";
						print SHIPTYPES "1\t";
					}
					else
					{
						print SHIPTYPES "\t\t";
					}
				}
			}
		}
		
		# Lots consumed.
		# Nothing for now.
#		print SHIPTYPES "\t";
		
		# End of line.		
		print SHIPTYPES "0\n";
	}
	
	close SHIPTYPES;	
}

###
# sub createShipChassisTables
###

sub createShipChassisTables
{
	print "Creating ship chassis tables...\n";
	foreach $key (sort(keys(%balancedata)))
	{
		@fields = split "_tier", $key;
		$oldfile = "$chassis_dir\\ship_chassis_$fields[0].tab";
		$newfile = "$chassis_dir\\ship_chassis_$key.tab";
		$p4name = "$chassis_dir\\ship_chassis_$key.tab";
		$p4name =~ s/\\/\//g;
		
		$outfile = $newfile;
		$outfile =~ s/dsrc/data/;
		$outfile =~ s/\.tab/\.iff/;
		$p4outfile = $outfile;
		$p4outfile =~ s/\\/\//g;
		
#		if ( -f $oldfile and !-f $newfile )
#		{
			# Check out the file.
			system( "p4 edit //depot" . $p4name );
			system( "p4 edit //depot" . $p4outfile );

			# Edit the file.
			system( "copy $oldfile $newfile" );
			
			# Compile the file.			
			system( "datatabletool -i $newfile -o $outfile" );
			
			# Try to add the files.
			system( "p4 add //depot" . $p4name );
			system( "p4 add //depot" . $p4outfile );
#		}
	}	
}

###
# sub createShipObjects
###

sub createShipObjects
{
	print "Creating ship objects from ship_appearances.txt...\n";
	
	my $i = 0;
	open APPEARANCES, "<ship_appearances.txt";
	while ( <APPEARANCES> )
	{
		# Skip the first two lines.
		$i++;
		if ( $i <= 2 )
		{
			next;
		}
		
		@fields = split '\t', $_;
		$ship_appearances{$fields[0]} = $_;
		
	}
	close APPEARANCES;
	
	foreach $key (sort(keys(%ship_appearances)))
	{
		for ( my $tier=1; $tier<6; $tier++ )
		{
			$name = $key . "_tier" . $tier;

			# Snag appearance data.
			@appearancedata = split '\t', $ship_appearances{$key};
			$shiptype = $appearancedata[1] . "_tier" . $tier;
			$appearance = $appearancedata[2];
			$cdf = $appearancedata[3];
			chop($cdf);

			# Server side templates.
			$newfile = "$objects_dir\\$name.tpf";
			$p4name = $newfile;
			$p4name =~ s/\\/\//g;
			$p4outfile = "$objects_dir\\$name.iff";
			$p4outfile =~ s/dsrc/data/g;
			$p4outfile =~ s/\\/\//g;

			# Check out the file.
			system( "p4 edit //depot" . $p4name );
			system( "p4 edit //depot" . $p4outfile );

			# Write the file.
			open NEWFILE, ">$newfile";
			print NEWFILE "\@base object/ship/base/base_fighter_ship.iff\n";
			print NEWFILE "\@class ship_object_template 1\n\n";
			print NEWFILE "shipType = \"$shiptype\"\n\n";
			print NEWFILE "\@class tangible_object_template 0\n";
			print NEWFILE "\@class object_template 2\n\n";
			print NEWFILE "sharedTemplate = \"object/ship/shared_$name.iff\"\n";
			close NEWFILE;

			# Compile the file.
			system( "templatecompiler -compile $newfile" );

			# Add the file.
			system( "p4 add //depot" . $p4name );
			system( "p4 add //depot" . $p4outfile );

			# Shared templates
			$newfile = "$objects_dir\\shared_$name.tpf";
			$newfile =~ s/server/shared/g;
			$p4name = $newfile;
			$p4name =~ s/\\/\//g;
			$p4outfile = "$objects_dir\\shared_$name.iff";
			$p4outfile =~ s/server/shared/g;
			$p4outfile =~ s/dsrc/data/g;
			$p4outfile =~ s/\\/\//g;

			# Check out the file.
			system( "p4 edit //depot" . $p4name );
			system( "p4 edit //depot" . $p4outfile );

			# Write the file.
			open NEWFILE, ">$newfile";
			print NEWFILE "\@base object/ship/base/shared_ship_fighter_base.iff\n";
			print NEWFILE "\@class ship_object_template 0\n";
			print NEWFILE "\@class tangible_object_template 0\n";
			print NEWFILE "\@class object_template 7\n\n";

			print NEWFILE "objectName = \"\" \"\"\n";
			print NEWFILE "detailedDescription = \"\"  \"\"\n";
			print NEWFILE "lookAtText = \"\"  \"\"\n\n";

			print NEWFILE "appearanceFilename = \"appearance/$appearance.apt\"\n";
			print NEWFILE "\n";
			print NEWFILE "\n";
			if ( $cdf ne undef )
			{
				print NEWFILE "clientDataFile = \"clientData/ship/client_shared_$cdf.cdf\"\n";
			}
			close NEWFILE;

			# Compile the file.
			system( "templatecompiler -compile $newfile" );

			# Add the file.
			system( "p4 add //depot" . $p4name );
			system( "p4 add //depot" . $p4outfile );
		}
	}	
}

###
# sub updateXPData
###

sub updateXPData
{
	# Read the appearance data.
	my $i = 0;
	open APPEARANCES, "<ship_appearances.txt";
	$ship_appearances = ();
	while ( <APPEARANCES> )
	{
		# Skip the first two lines.
		$i++;
		if ( $i <= 2 )
		{
			next;
		}
		
		@fields = split '\t', $_;
		$ship_appearances{$fields[0]} = $_;
		
	}
	close APPEARANCES;

	# Check out and open space_mobile.
	$space_mobile = "\\swg\\current\\dsrc\\sku.0\\sys.server\\compiled\\game\\datatables\\space_mobile\\space_mobile.tab";
	$space_mobile_data = "\\swg\\current\\data\\sku.0\\sys.server\\compiled\\game\\datatables\\space_mobile\\space_mobile.iff";
	
	$p4name = $space_mobile;
	$p4name =~ s/\\/\//g;
	system( "p4 edit //depot" . $p4name );

	$p4name = $space_mobile_data;
	$p4name =~ s/\\/\//g;
	system( "p4 edit //depot" . $p4name );
	
	# Read in space_mobile.
	open SPACE_MOB, "<$space_mobile";
	my $i = 0;
	my @space_mobile_lines = ();
	while ( <SPACE_MOB> )
	{
		@fields = split '\t';
		
		# Skip the header.
		if ( $i < 2 )
		{
			$space_mobile_lines[$i] = $_;
			$i++;
			next;
		}
		
		$ship_object = $fields[2];
		@ship_fields = split "_tier", $ship_object;
		$appearance_data = $ship_appearances{$ship_fields[0]};
		@ad_fields = split '\t', $appearance_data;
		$shiptype = $ad_fields[1] . "_tier" . $ship_fields[1];
#		print $shiptype . "\n";

		if ( $ship_fields[0] eq "corvette" )
		{
			$space_mobile_lines[$i] = $_;
			$i++;
			next;
		}

		# Get balance data for this entry.		
		@o = split '!', $balancedata{$shiptype};
		
		# Find the xp multiplier for this entry.
		$xpmod = $o[32];
		$xpout = $xpmod;
		$fields[5] = $xpout;
		
		# Find the loot roll chance.
		@mods = split '!', $shipmodifiers{$ad_fields[1]};
		$fields[6] = (0.18 - ($ship_fields[1] * 0.01)) * $mods[32];
		
		# Find the loot drop roll times.
		if ( $xpmod == 0 )
		{
			# If we don't give xp, we shouldn't give loot either.
			$fields[7] = 0;
		}
		else
		{
			$fields[7] = 1;
		}		
		
		
		
		# Build the output line.
		$space_mobile_lines[$i] = join "\t", @fields;
		
		print "$fields[2] --> $xpout\n";
		
		# Increment line counter.
		$i++;
	}
	close SPACE_MOB;
	
	# Write out space_mobile.
	open SPACE_MOB, ">$space_mobile";
	for ( $i=0; $i<@space_mobile_lines; $i++ )
	{
		print SPACE_MOB $space_mobile_lines[$i];		
	}
	close SPACE_MOB;

	# Compile the file.
	system( "datatabletool -i $space_mobile -o $space_mobile_data" );
}