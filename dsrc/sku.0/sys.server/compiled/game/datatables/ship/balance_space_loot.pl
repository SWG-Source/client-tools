#!/usr/bin/perl

# Constants

$balance_sheet = "ship_balance.txt";
$loot_sheet = "looted_component_balance.txt";
$weapon_loot = "components\\weapon.tab";

$server_prefix = "\\swg\\current\\dsrc\\sku.0\\sys.server\\compiled\\game\\object\\tangible\\ship\\components";
$shared_prefix = "\\swg\\current\\dsrc\\sku.0\\sys.shared\\compiled\\game\\object\\tangible\\ship\\components";

###
# main
###

# Read our balance sheet.
print "\nbalance_space_loot process started\n";
#print "Output file: $output_filename\n";

# Read balance data.
processData();
calculateBalance();
balanceLoot();

###
# sub processData
# Overall function for reading the balance sheet.
###

sub processData
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
		elsif ( /Baseline/ )
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
# sub calculateBalance
###

sub calculateBalance
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
# sub balanceLoot
###

sub balanceLoot
{
	# Read balance data from the loot sheet.
	open LOOT, "<$loot_sheet";
	while ( <LOOT> )
	{
		$line = $_;
		for ( $chunk )
		{
			/Mass Distribution/ and do { $massb = readBaseline( $line, $massb ); next; };
			/Reactor Drain Distribution/ and do { $reactorb = readBaseline( $line, $reactorb ); next; };
			/Armor/ and do { readChunk( $line, \%armor ); next; };
			/Booster/ and do { readChunk( $line, \%booster ); next; };
			/Capacitor/ and do { readChunk( $line, \%capacitor ); next; };
			/Droid Interface/ and do { readChunk( $line, \%droid_interface ); next; };
			/Engine/ and do { readChunk( $line, \%engine ); next; };
			/Reactor/ and do { readChunk( $line, \%reactor ); next; };
			/Shield/ and do { readChunk( $line, \%shield ); next; };
			/Weapon/ and do { readChunk( $line, \%weapon ); next; };
			$chunk = (split /\t/, $line)[0];
		}	
	}
	close LOOT;
	
	# Generate loot!
	outputArmor();
	outputBoosters();
	outputCapacitors();
	outputDroidInterface();
	outputEngines();
	outputReactors();
	outputShields();
	outputWeapons();
}

###
# sub readBaseline
###

sub readBaseline
{
	my ($line, $massb) = @_;

	# Check for end of chunk.
	if ( $line eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return $massb;
	}
	
	# Read some data.
	chop($line);
	@fields = split /\t/, $line;
	if ( $fields[0] ne "BASELINE" )
	{
		return $line;
	}
}


###
# sub readChunk
###

sub readChunk
{
	my ($line, $mref) = @_;

	# Check for end of chunk.
	if ( $line eq "\n" )
	{
		# End of this chunk.
		$chunk = "";
		return;
	}
	
	# Read some data.
	chop($line);
	@fields = split /\t/, $line;
	if ( $fields[0] ne "Name" )
	{
		if ( $fields[0] eq undef )
		{
			$chunk = "";
			return;
		}
		$$mref{$fields[0]} = $line;
	}
}

###
# sub outputArmor
###

sub outputArmor
{
	# Dump the balance data.
	open ARMOR, ">components\\armor.tab";
	print ARMOR "strType\tstrComments\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print ARMOR "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";	
	foreach $key (sort(keys %armor))
	{
		my @fields = split /\t/, $armor{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.25;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print ARMOR "object/tangible/ship/components/armor/$fields[0].iff" . "\t";
		
		# Comments
		print ARMOR "Do not edit." . "\t";
		
		# HP / Armor
		$hp = $balancedata{"Armor_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print ARMOR $hp . "\t";
		print ARMOR $hp . "\t";
		print ARMOR $fields[3] . "\t";
		print ARMOR $hp . "\t";
		print ARMOR $hp ."\t";
		
		# Efficiency
		print ARMOR "1\t";
		
		# Energy Maintenance
		print ARMOR "0\t";
		print ARMOR "0\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[1] * $fields[4] * $fields[6];
		$mass = sprintf( "%.3f", $mass );
		print ARMOR $fields[5] . "\t";
		print ARMOR $mass . "\t";
		
		# Reverse engineering.
		print ARMOR $fields[1] . "\n";
	}
	close ARMOR;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %armor)
#	{
#		open SERVER_OBJ, ">$server_prefix\\armor\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/armor\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\armor\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_reactor_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_armor\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputBoosters
###

sub outputBoosters
{
	# Dump the balance data.
	open BOOSTERS, ">components\\booster.tab";
	print BOOSTERS "strType\tstrComments\tfltCurrentEnergy\tfltMaximumEnergyModifier\tfltMaximumEnergy\tfltRechargeRateModifier\tfltRechargeRate\tfltConsumptionRateModifier\tfltConsumptionRate\tfltAccelerationModifier\tfltAcceleration\tfltMaxSpeedModifier\tfltMaxSpeed\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print BOOSTERS "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %booster))
	{
		my @fields = split /\t/, $booster{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.5;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print BOOSTERS "object/tangible/ship/components/booster/$fields[0].iff" . "\t";
		
		# Comments
		print BOOSTERS "Do not edit." . "\t";
		
		# Booster Energy
		$be = $balancedata{"BoosterEnergy_tier$tier"} * $fields[8] * $mod;
		$be = sprintf( "%.3f", $be );
		print BOOSTERS $be . "\t";
		print BOOSTERS $fields[9] . "\t";
		print BOOSTERS $be . "\t";
		
		# Booster Rate
		$br = $balancedata{"BoosterRecharge_tier$tier"} * $fields[10] * $mod;
		$br = sprintf( "%.3f", $br );
		print BOOSTERS $fields[11] . "\t";
		print BOOSTERS $br . "\t";
		
		# Consumption Rate		
		$bc = $balancedata{"BoosterConsumption_tier$tier"} * $fields[12] * $mod;
		$bc = sprintf( "%.3f", $bc );
		print BOOSTERS $fields[13] . "\t";
		print BOOSTERS $bc . "\t";
		
		# Accel
		$ba = $balancedata{"BoosterAccel_tier$tier"} * $fields[14] * $mod;
		$ba = sprintf( "%.3f", $ba );
		print BOOSTERS $fields[15] . "\t";
		print BOOSTERS $ba . "\t";
		
		# Speed
		$bs = $balancedata{"BoosterSpeed_tier$tier"} * $fields[16] * $mod;
		$bs = sprintf( "%.3f", $bs );
		print BOOSTERS $fields[17] . "\t";
		print BOOSTERS $bs . "\t";
		
		# HP
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print BOOSTERS $hp . "\t";
		print BOOSTERS $hp . "\t";
		print BOOSTERS $fields[3] . "\t";
		print BOOSTERS $hp/2 . "\t";
		print BOOSTERS $hp/2 ."\t";
		
		# Efficiency
		print BOOSTERS "1\t";
		
		# Energy Maintenance
		@rbaseline = split /\t/, $reactorb;
		$maint = $rbaseline[0] * $rbaseline[4] * $fields[4];
		$maint = sprintf( "%.3f", $maint );
		print BOOSTERS $fields[5] . "\t";
		print BOOSTERS $maint . "\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[4] * $fields[6] * $fields[18];
		$mass = sprintf( "%.3f", $mass );
		print BOOSTERS $fields[7] . "\t";
		print BOOSTERS $mass . "\t";
		
		# Reverse engineering.
		print BOOSTERS $fields[1] . "\n";
	}
	close BOOSTERS;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %booster)
#	{
#		open SERVER_OBJ, ">$server_prefix\\booster\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/booster\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\booster\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_booster_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_booster\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputCapacitors
###

sub outputCapacitors
{
	# Dump the balance data.
	open CAPACITORS, ">components\\capacitor.tab";
	print CAPACITORS "strType\tstrComments\tfltCurrentEnergy\tfltMaxEnergyModifier\tfltMaxEnergy\tfltRechargeRateModifier\tfltRechargeRate\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print CAPACITORS "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %capacitor))
	{
		my @fields = split /\t/, $capacitor{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.5;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print CAPACITORS "object/tangible/ship/components/weapon_capacitor/$fields[0].iff" . "\t";
		
		# Comments
		print CAPACITORS "Do not edit." . "\t";
		
		# Capacitor Energy
		$ce = $balancedata{"CapacitorEnergy_tier$tier"} * $fields[8] * $mod;
		$ce = sprintf( "%.3f", $ce );
		print CAPACITORS $ce . "\t";
		print CAPACITORS $fields[9] . "\t";
		print CAPACITORS $ce . "\t";
		
		# Capacitor Rate
		$cr = $balancedata{"CapacitorRegen_tier$tier"} * $fields[10] * $mod;
		$cr = sprintf( "%.3f", $cr );
		print CAPACITORS $fields[11] . "\t";
		print CAPACITORS $cr . "\t";
		
		# HP
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print CAPACITORS $hp . "\t";
		print CAPACITORS $hp . "\t";
		print CAPACITORS $fields[3] . "\t";
		print CAPACITORS $hp/2 . "\t";
		print CAPACITORS $hp/2 ."\t";
		
		# Efficiency
		print CAPACITORS "1\t";
		
		# Energy Maintenance
		@rbaseline = split /\t/, $reactorb;
		$maint = $rbaseline[0] * $rbaseline[5] * $fields[4];
		$maint = sprintf( "%.3f", $maint );
		print CAPACITORS $fields[5] . "\t";
		print CAPACITORS $maint . "\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[5] * $fields[6] * $fields[12];
		$mass = sprintf( "%.3f", $mass );
		print CAPACITORS $fields[7] . "\t";
		print CAPACITORS $mass . "\t";
		
		# Reverse engineering.
		print CAPACITORS $fields[1] . "\n";
	}
	close CAPACITORS;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %capacitor)
#	{
#		open SERVER_OBJ, ">$server_prefix\\weapon_capacitor\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/weapon_capacitor\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\weapon_capacitor\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_capacitor_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_capacitor\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputDroidInterface
###

sub outputDroidInterface
{
	# Dump the balance data.
	open DROID_INTERFACES, ">components\\droid_interface.tab";
	print DROID_INTERFACES "strType\tstrComments\tfltCommandSpeedModifier\tfltCommandSpeed\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print DROID_INTERFACES "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %droid_interface))
	{
		my @fields = split /\t/, $droid_interface{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.5;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print DROID_INTERFACES "object/tangible/ship/components/droid_interface/$fields[0].iff" . "\t";
		
		# Comments
		print DROID_INTERFACES "Do not edit." . "\t";
		
		# Command Speed
		$cs = $balancedata{"CommandSpeed_tier$tier"} * $fields[8] * $mod;
		$cs = sprintf( "%.3f", $cs );
		print DROID_INTERFACES $fields[9] . "\t";
		print DROID_INTERFACES $cs . "\t";

		# HP
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print DROID_INTERFACES $hp . "\t";
		print DROID_INTERFACES $hp . "\t";
		print DROID_INTERFACES $fields[3] . "\t";
		print DROID_INTERFACES $hp/2 . "\t";
		print DROID_INTERFACES $hp/2 ."\t";
		
		# Efficiency
		print DROID_INTERFACES "1\t";
		
		# Energy Maintenance
		@rbaseline = split /\t/, $reactorb;
		$maint = $rbaseline[0] * $rbaseline[6] * $fields[4];
		$maint = sprintf( "%.3f", $maint );
		print DROID_INTERFACES $fields[5] . "\t";
		print DROID_INTERFACES $maint . "\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[6] * $fields[6] * $fields[10];
		$mass = sprintf( "%.3f", $mass );
		print DROID_INTERFACES $fields[7] . "\t";
		print DROID_INTERFACES $mass . "\t";
		
		# Reverse engineering.
		print DROID_INTERFACES $fields[1] . "\n";
	}
	close DROID_INTERFACES;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %droid_interface)
#	{
#		open SERVER_OBJ, ">$server_prefix\\droid_interface\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/droid_interface\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\droid_interface\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_droidinterface_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_droid_interface\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputEngines
###

sub outputEngines
{
	# Dump the balance data.
	open ENGINES, ">components\\engine.tab";
	print ENGINES "strType\tstrComments\tfltMaxSpeedModifier\tfltMaxSpeed\tfltMaxPitchModifier\tfltMaxPitch\tfltMaxRollModifier\tfltMaxRoll\tfltMaxYawModifier\tfltMaxYaw\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print ENGINES "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %engine))
	{
		my @fields = split /\t/, $engine{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.25;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print ENGINES "object/tangible/ship/components/engine/$fields[0].iff" . "\t";
		
		# Comments
		print ENGINES "Do not edit." . "\t";
		
		# Max Speed
		$ms = $balancedata{"EngineSpeed_tier$tier"} * $fields[8] * $mod;
		print ENGINES $fields[9] . "\t";
		print ENGINES $ms . "\t";
		
		# Pitch
		$p = $balancedata{"EnginePitch_tier$tier"} * $fields[10] * $mod;
		print ENGINES $fields[11] . "\t";
		print ENGINES $p . "\t";

		# Roll
		$r = $balancedata{"EngineRoll_tier$tier"} * $fields[12] * $mod;
		print ENGINES $fields[13] . "\t";
		print ENGINES $r . "\t";

		# Yaw
		$y = $balancedata{"EngineYaw_tier$tier"} * $fields[14] * $mod;
		print ENGINES $fields[15] . "\t";
		print ENGINES $y . "\t";
		
		# HP
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		print ENGINES $hp . "\t";
		print ENGINES $hp . "\t";
		print ENGINES $fields[3] . "\t";
		print ENGINES $hp/2 . "\t";
		print ENGINES $hp/2 ."\t";
		
		# Efficiency
		print ENGINES "1\t";
		
		# Energy Maintenance
		@rbaseline = split /\t/, $reactorb;
		$maint = $rbaseline[0] * $rbaseline[7] * $fields[4];
		$maint = sprintf( "%.3f", $maint );
		print ENGINES $fields[5] . "\t";
		print ENGINES $maint . "\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[7] * $fields[6] * $fields[16];
		$mass = sprintf( "%.3f", $mass );
		print ENGINES $fields[7] . "\t";
		print ENGINES $mass . "\t";
		
		# Reverse engineering.
		print ENGINES $fields[1] . "\n";
	}
	close ENGINES;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (sort(keys %engine))
#	{
#		open SERVER_OBJ, ">$server_prefix\\engine\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/engine\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\engine\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_engine_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_engine\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputReactors
###

sub outputReactors
{
	# Dump the balance data.
	open REACTORS, ">components\\reactor.tab";
	print REACTORS "strType\tstrComments\tfltEnergyGenerationModifier\tfltEnergyGeneration\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print REACTORS "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %reactor))
	{
		my @fields = split /\t/, $reactor{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.5;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print REACTORS "object/tangible/ship/components/reactor/$fields[0].iff" . "\t";
		
		# Comments
		print REACTORS "Do not edit." . "\t";
		
		# Energy Generation
		$eg = 10000 * $fields[6];
		$eg = sprintf( "%.3f", $eg );
		print REACTORS $fields[7] . "\t";
		print REACTORS $eg . "\t";
		
		# HP / Armor
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print REACTORS $hp . "\t";
		print REACTORS $hp . "\t";
		print REACTORS $fields[3] . "\t";
		print REACTORS $hp/2 . "\t";
		print REACTORS $hp/2 ."\t";
		
		# Efficiency & Maintenance
		print REACTORS "1\t0\t0\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[8] * $fields[4] * $fields[8];
		$mass = sprintf( "%.3f", $mass );
		print REACTORS $fields[5] . "\t";
		print REACTORS $mass . "\t";
		
		# Reverse engineering.
		print REACTORS $fields[1] . "\n";
	}
	close REACTORS;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %reactor)
#	{
#		open SERVER_OBJ, ">$server_prefix\\reactor\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/reactor\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\reactor\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_reactor_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_reactor\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputShields
###

sub outputShields
{
	# Dump the balance data.
	open SHIELDS, ">components\\shield.tab";
	print SHIELDS "strType\tstrComments\tfltShieldRechargeRateModifier\tfltShieldRechargeRate\tfltShieldHitpointsMaximumFrontModifier\tfltShieldHitpointsMaximumFront\tfltShieldHitpointsMaximumBackModifier\tfltShieldHitpointsMaximumBack\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print SHIELDS "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %shield))
	{
		my @fields = split /\t/, $shield{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.25;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print SHIELDS "object/tangible/ship/components/shield_generator/$fields[0].iff" . "\t";
		
		# Comments
		print SHIELDS "Do not edit." . "\t";
		
		# Shield Recharge
		$sr = $balancedata{"ShieldRegen_tier$tier"} * $fields[8] * $mod;
		$sr = sprintf( "%.3f", $sr );
		print SHIELDS $fields[9] . "\t";
		print SHIELDS $sr . "\t";
		
		# Shield Front
		$sf = $balancedata{"Shield_tier$tier"} * $fields[10] * $mod;
		$sf = sprintf( "%.3f", $sf );
		print SHIELDS $fields[11] . "\t";
		print SHIELDS $sf . "\t";
		
		# Shield Back
		$sb = $balancedata{"Shield_tier$tier"} * $fields[12] * $mod;
		$sb = sprintf( "%.3f", $sb );
		print SHIELDS $fields[13] . "\t";
		print SHIELDS $sb . "\t";
		
		# HP
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print SHIELDS $hp . "\t";
		print SHIELDS $hp . "\t";
		print SHIELDS $fields[3] . "\t";
		print SHIELDS $hp/2 . "\t";
		print SHIELDS $hp/2 ."\t";
		
		# Efficiency
		print SHIELDS "1\t";
		
		# Energy Maintenance
		@rbaseline = split /\t/, $reactorb;
		$maint = $rbaseline[0] * $rbaseline[3] * $fields[4];
		$maint = sprintf( "%.3f", $maint );
		print SHIELDS $fields[5] . "\t";
		print SHIELDS $maint . "\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[3] * $fields[6] * $fields[14];
		$mass = sprintf( "%.3f", $mass );
		print SHIELDS $fields[7] . "\t";
		print SHIELDS $mass . "\t";
		
		# Reverse engineering.
		print SHIELDS $fields[1] . "\n";
	}
	close SHIELDS;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %shield)
#	{
#		open SERVER_OBJ, ">$server_prefix\\shield_generator\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/shield_generator\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\shield_generator\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_shield_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_shield\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}

###
# sub outputWeapons
###

sub outputWeapons
{
	# Dump the balance data.
	open WEAPONS, ">components\\weapon.tab";
	print WEAPONS "strType\tstrComments\tfltMinDamageModifier\tfltMinDamage\tfltMaxDamageModifier\tfltMaxDamage\tfltShieldEffectivenessModifier\tfltShieldEffectiveness\tfltArmorEffectivenessModifier\tfltArmorEffectiveness\tfltEnergyPerShotModifier\tfltEnergyPerShot\tfltRefireRateModifier\tfltRefireRate\tfltCurrentHitpoints\tfltMaximumHitpoints\tfltMaximumArmorHitpointsMod\tfltMaximumArmorHitpoints\tfltCurrentArmorHitpoints\tfltEfficiency\tfltEnergyMaintenanceModifier\tfltEnergyMaintenance\tfltMassModifier\tfltMass\treverseEngineeringLevel\n";
	print WEAPONS "s\tc\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\tf\ti\n";
	foreach $key (sort(keys %weapon))
	{
		my @fields = split /\t/, $weapon{$key};

		# Determine our tier.
		$tier = sprintf( "%.0f", $fields[1]/2 );
		$step = ($fields[1] % 2) == 0;
		$mod = 1;
		if ( $step )
		{
			$mod = 1.25;
		}
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
	
		# Name
		print WEAPONS "object/tangible/ship/components/weapon/$fields[0].iff" . "\t";
		
		# Comments
		print WEAPONS "Do not edit." . "\t";
		
		# Min Damage
		$md = $balancedata{"DamageMin_tier$tier"} * $fields[8] * $mod;
		$md = sprintf( "%.3f", $md );
		print WEAPONS $fields[9] . "\t";
		print WEAPONS $md . "\t";
		
		# Max Damage
		$md = $balancedata{"DamageMax_tier$tier"} * $fields[10] * $mod;
		$md = sprintf( "%.3f", $md );
		print WEAPONS $fields[11] . "\t";
		print WEAPONS $md . "\t";
		
		# Shield Eff
		$se = $balancedata{"WeaponEffShield_tier$tier"} * $fields[12] * $mod;
		$se = sprintf( "%.3f", $se );
		print WEAPONS $fields[13] . "\t";
		print WEAPONS $se . "\t";
		
		# ArmorEff
		$ae = $balancedata{"WeaponEffArmor_tier$tier"} * $fields[14] * $mod;
		$ae = sprintf( "%.3f", $ae );
		print WEAPONS $fields[15] . "\t";
		print WEAPONS $ae . "\t";
		
		# Drain
		$wd = $balancedata{"WeaponDrain_tier$tier"} * $fields[16] * $mod;
		$wd = sprintf( "%.3f", $wd );
		print WEAPONS $fields[17] . "\t";
		print WEAPONS $wd . "\t";
		
		# Rate
		$wr = $balancedata{"WeaponRate_tier$tier"} * $fields[18] * $mod;
#		$wr = sprintf( "%.3f", $wr );
		print WEAPONS $fields[19] . "\t";
		print WEAPONS $wr . "\t";
		
		# HP
		$hp = $balancedata{"Component_tier$tier"} * $fields[2] * $mod;
		$hp = sprintf( "%.3f", $hp );
		print WEAPONS $hp . "\t";
		print WEAPONS $hp . "\t";
		print WEAPONS $fields[3] . "\t";
		print WEAPONS $hp/2 . "\t";
		print WEAPONS $hp/2 ."\t";
		
		# Efficiency
		print WEAPONS "1\t";
		
		# Energy Maintenance
		@rbaseline = split /\t/, $reactorb;
		$maint = $rbaseline[0] * $rbaseline[2] * $fields[4];
		$maint = sprintf( "%.3f", $maint );
		print WEAPONS $fields[5] . "\t";
		print WEAPONS $maint . "\t";
		
		# Mass
		@mbaseline = split /\t/, $massb;
		$mass = $mbaseline[0] * $mbaseline[2] * $fields[6] * $fields[20];
		$mass = sprintf( "%.3f", $mass );
		print WEAPONS $fields[7] . "\t";
		print WEAPONS $mass . "\t";
		
		# Reverse engineering.
		print WEAPONS $fields[1] . "\n";
	}
	close WEAPONS;
	
	# Check out the objects from P4?
	
	# Create the objects.
#	foreach $key (keys %weapon)
#	{
#		open SERVER_OBJ, ">$server_prefix\\weapon\\$key.tpf";
#		print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
#		print SERVER_OBJ "\@class tangible_object_template 1\n";
#		print SERVER_OBJ "\@class object_template 5\n";
#		print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/weapon\/shared_$key.iff\"\n";
#		print SERVER_OBJ "moveFlags = [ MF_player ]\n";
#		close SERVER_OBJ;
		
#		open SHARED_OBJ, ">$shared_prefix\\weapon\\shared_$key.tpf";
#		print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
#		print SHARED_OBJ "\@class tangible_object_template 8\n";
#		print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
#		print SHARED_OBJ "\@class object_template 7\n";
#		print SHARED_OBJ "clearFloraRadius = 0\n";
#		print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
#		print SHARED_OBJ "detailedDescription = \"space/space_item\" \"".$key."_d\"\n";
#		print SHARED_OBJ "containerType = CT_none\n";
#		print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_weapon_s01.apt\"\n";
#		print SHARED_OBJ "portalLayoutFilename = \"\"\n";
#		print SHARED_OBJ "clientDataFile = \"\"\n";
#		print SHARED_OBJ "gameObjectType = GOT_ship_component_weapon\n";
#		close SHARED_OBJ;
#	}
	
	# Add the objects to P4?
}
