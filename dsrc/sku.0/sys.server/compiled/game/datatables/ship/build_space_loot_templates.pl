#!/usr/bin/perl

# Constants

$loot_sheet = "looted_component_balance.txt";
$ship_components = "j:\\swg\\current\\dsrc\\sku.0\\sys.shared\\compiled\\game\\datatables\\space\\ship_components.tab";
$dsrc_server_prefix = "\\swg\\current\\dsrc\\sku.0\\sys.server\\compiled\\game\\object\\tangible\\ship\\components";
$dsrc_shared_prefix = "\\swg\\current\\dsrc\\sku.0\\sys.shared\\compiled\\game\\object\\tangible\\ship\\components";
$data_server_prefix = "\\swg\\current\\data\\sku.0\\sys.server\\compiled\\game\\object\\tangible\\ship\\components";
$data_shared_prefix = "\\swg\\current\\data\\sku.0\\sys.shared\\compiled\\game\\object\\tangible\\ship\\components";


###
# MAIN
###

readSegments();

sub readSegments
{
	# Read balance data from the loot sheet.
	open LOOT, "<$loot_sheet";
	while ( <LOOT> )
	{
		$line = $_;
		for ( $chunk )
		{
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
	makeArmorTemplates();
	makeBoosterTemplates();
	makeCapacitorTemplates();
	makeDroidInterfaceTemplates();
	makeEngineTemplates();
	makeReactorTemplates();
	makeShieldTemplates();
	makeWeaponTemplates();
	
}

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

sub makeArmorTemplates
{
	foreach $key (keys %armor)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $armor{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\armor\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/armor\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\armor\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_armor_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_armor_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_armor\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\armor\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\armor\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\armor\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\armor\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/armor/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/armor/$key.iff\tobject/tangible/ship/components/armor/shared_$key.iff\tarmor\tarm_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}


sub makeBoosterTemplates
{
	foreach $key (keys %booster)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $booster{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\booster\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/booster\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\booster\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_booster_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_booster_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_booster\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\booster\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\booster\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\booster\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\booster\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/booster/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/booster/$key.iff\tobject/tangible/ship/components/booster/shared_$key.iff\tbooster\tbst_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}


sub makeCapacitorTemplates
{
	foreach $key (keys %capacitor)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $capacitor{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\weapon_capacitor\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/capacitor\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\weapon_capacitor\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_capacitor_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_capacitor_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_capacitor\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\weapon_capacitor\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\weapon_capacitor\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\weapon_capacitor\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\weapon_capacitor\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/weapon_capacitor/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/weapon_capacitor/$key.iff\tobject/tangible/ship/components/weapon_capacitor/shared_$key.iff\tcapacitor\tcap_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}

sub makeDroidInterfaceTemplates
{
	foreach $key (keys %droid_interface)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $droid_interface{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\droid_interface\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/droid_interface\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\droid_interface\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_droid_interface_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_droidinterface_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_droid_interface\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\droid_interface\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\droid_interface\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\droid_interface\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\droid_interface\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/droid_interface/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/droid_interface/$key.iff\tobject/tangible/ship/components/droid_interface/shared_$key.iff\tdroid_interface\tddi_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}

sub makeEngineTemplates
{
	foreach $key (keys %engine)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $engine{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\engine\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/engine\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\engine\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_engine_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_engine_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_engine\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\engine\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\engine\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\engine\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\engine\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/engine/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/engine/$key.iff\tobject/tangible/ship/components/engine/shared_$key.iff\tengine\teng_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}

sub makeReactorTemplates
{
	foreach $key (keys %reactor)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $reactor{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\reactor\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/reactor\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\reactor\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_reactor_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_reactor_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_reactor\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\reactor\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\reactor\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\reactor\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\reactor\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/reactor/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/reactor/$key.iff\tobject/tangible/ship/components/reactor/shared_$key.iff\treactor\trct_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}

sub makeShieldTemplates
{
	foreach $key (keys %shield)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $shield{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\shield_generator\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/shield_generator\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\shield_generator\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_shield_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_shield_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_shield\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\shield_generator\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\shield_generator\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\shield_generator\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\shield_generator\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/shield_generator/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/shield_generator/$key.iff\tobject/tangible/ship/components/shield_generator/shared_$key.iff\tshield\tshd_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}

sub makeWeaponTemplates
{
	foreach $key (keys %weapon)
	{
		print "\nStarting process for key $key";
		my @fields = split /\t/, $weapon{$key};
		
		$cert = $fields[1];
		if ( $cert == 10 )
		{
			$cert = "ten";
		}
		
		open SERVER_OBJ, ">$dsrc_server_prefix\\weapon\\$key.tpf";
			print SERVER_OBJ "\@base object\/tangible\/ship\/components\/base\/ship_component_loot_base.iff\n";
			print SERVER_OBJ "\@class tangible_object_template 1\n";
			print SERVER_OBJ "\@class object_template 5\n";
			print SERVER_OBJ "sharedTemplate = \"object\/tangible\/ship\/components\/weapon\/shared_$key.iff\"\n";
			print SERVER_OBJ "moveFlags = [ MF_player ]\n";
		close SERVER_OBJ;
		
		open SHARED_OBJ, ">$dsrc_shared_prefix\\weapon\\shared_$key.tpf";
			print SHARED_OBJ "\@base object\/tangible\/ship\/components\/base\/shared_ship_component_loot_base.iff\n";
			print SHARED_OBJ "\@class tangible_object_template 8\n";
			print SHARED_OBJ "certificationsRequired = [\"cert_ordnance_level$cert\"]\n";
			print SHARED_OBJ "\@class object_template 7\n";
			print SHARED_OBJ "clearFloraRadius = 0\n";
			print SHARED_OBJ "objectName = \"space/space_item\" \"".$key."_n\"\n";
			print SHARED_OBJ "detailedDescription = \"space/space_item\" \"generic_weapon_d\"\n";
			print SHARED_OBJ "containerType = CT_none\n";
			print SHARED_OBJ "appearanceFilename = \"appearance\/ship_component_weapon_s01.apt\"\n";
			print SHARED_OBJ "portalLayoutFilename = \"\"\n";
			print SHARED_OBJ "clientDataFile = \"\"\n";
			print SHARED_OBJ "gameObjectType = GOT_ship_component_weapon\n";
		close SHARED_OBJ;
		
		my $serverfilename = "$dsrc_server_prefix\\weapon\\$key.tpf";
		my $sharedfilename = "$dsrc_shared_prefix\\weapon\\shared_$key.tpf";
		
		system( "templatecompiler -compile $serverfilename" );
		system( "templatecompiler -compile $sharedfilename" );
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		my $serverfilename = "$data_server_prefix\\weapon\\$key.iff";
		my $sharedfilename = "$data_shared_prefix\\weapon\\shared_$key.iff";
		
		$serverfilename =~ s/\\/\//g;
		$sharedfilename =~ s/\\/\//g;
		
		system( "p4 add //depot" . $serverfilename );
		system( "p4 add //depot" . $sharedfilename );
		
		open  COMPONENTS, "<$ship_components";
		
		@file = <COMPONENTS>;
		
		close COMPONENTS;
		
		my $found = 0;
		
		foreach my $y (@file)
		{
		
			my @fields = split (/\t/, $y);
			if ( $fields[1] eq "object/tangible/ship/components/weapon/$key.iff")
			{
				$found = 1;
			}
			
		}
		
		
		
		if ($found == 0)
		{
			open  COMPONENTS, ">>$ship_components";
			
			print COMPONENTS "$key\tobject/tangible/ship/components/weapon/$key.iff\tobject/tangible/ship/components/weapon/shared_$key.iff\tweapon\twpn_0\n";
			
			close COMPONENTS;	
		}
		
		
		
		
		
		
	}

}