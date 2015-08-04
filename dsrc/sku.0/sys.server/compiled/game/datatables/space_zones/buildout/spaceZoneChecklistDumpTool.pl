$space_tatooine = "space_tatooine";
$space_nova_orion = "space_nova_orion";
$space_yavin4 = "space_yavin4";
$space_naboo = "space_naboo";
$space_lok = "space_lok";
$space_kashyyyk = "space_kashyyyk";
$space_endor = "space_endor";
$space_dantooine = "space_dantooine";
$space_corellia = "space_corellia";
$space_dathomir = "space_dathomir";
$space_light1 = "space_light1";
$space_heavy1 = "space_heavy1";

print "Place this perl script in the space_zones/buildout directory before attempting to parse files.\n\nSpace Zone options:\n";
print " 1. $space_tatooine\n";
print " 2. $space_nova_orion\n";
print " 3. $space_yavin4\n";
print " 4. $space_naboo\n";
print " 5. $space_lok\n";
print " 6. $space_kashyyyk\n";
print " 7. $space_endor\n";
print " 8. $space_dantooine\n";
print " 9. $space_corellia\n";
print " 10. $space_dathomir\n";
print " 11. $space_light1\n";
print " 12. $space_heavy1\n\n";

print "Enter the number of the zone: ";
chomp($input = <STDIN>);

use Switch;
switch ($input) 
{
	case 1	{
			$zone = $space_tatooine;
			$selection = "$space_tatooine.tab";
			$dumpFile = "space_tatooine_dump.tab";
		}
	case 2	{
			$zone = $space_nova_orion;
			$selection = "$space_nova_orion.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 3	{ 
			$zone = $space_yavin4;
			$selection = "$space_yavin4.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 4	{ 
			$zone = $space_naboo;
			$selection = "$space_naboo.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 5	{ 
			$zone = $space_lok;
			$selection = "$space_lok.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 6	{ 
			$zone = $space_kashyyyk;
			$selection = "$space_kashyyyk.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 7	{ 
			$zone = $space_endor;
			$selection = "$space_endor.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 8	{ 
			$zone = $space_tatooine;
			$selection = "$space_dantooine.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 9	{ 
			$zone = $space_tatooine;
			$selection = "$space_corellia.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 10	{ 
			$zone = $space_tatooine;
			$selection = "$space_dathomir.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 11	{ 
			$zone = $space_tatooine;
			$selection = "$space_light1.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}
	case 12	{ 
			$zone = $space_tatooine;
			$selection = "$space_heavy1.tab";
			$dumpFile = "space_nova_orion_dump.tab";
		}	
	else 	{ print "The selection was invalid." }
}

print $selection;

open(FILE, $selection) or die "Can't open $selection for some reason.";
open(OUTFILE, "> $dumpFile");

while (<FILE>)
 {
	# Find 12 columns
	$_ =~ /^\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\.*/;
	
	#We only want spawners
	if($1 eq "object/tangible/space/content_infrastructure/basic_spawner.iff" && (index($11, "fltMaxSpawnDistance") == 0 || index($11, "fltMaxCircleDistance") == 0))
	{
		# Round colums $8, $9, $10 which are the fltPX, fltPY, and fltPZ in datatable
		$roundfltPX = sprintf("%.0f", $8);
		$roundfltPY = sprintf("%.0f", $9);
		$roundfltPZ = sprintf("%.0f", $10);
		
		# Col 11 is strObjVars
		# We have to search for the index of the 'word' .0|5| then add 5 to that index
		
		$where = index($11, ".0|5|");
		$mobStartIdx = $where + 5;
		$stringLength = length $11;
		$mobs = substr($11, $mobStartIdx, $stringLength);
		
		#run it a second time!
		$where = index($mobs, ".0|5|");
		if($where > -1)
		{
			$mobStartIdx = $where + 5;
			$stringLength = length $mobs;
			$mobs = substr($mobs, $mobStartIdx, $stringLength);
		}
		
		# Remove more crap
		
		$searchString = "|strSpawns_mangled.segment.1|5|";
		$where = index($mobs, $searchString);
		if($where > -1)
		{
			$mobsLength = length $mobs;
			$searchStringLength = length $searchString;
			$searchStrEnd = $where + $searchStringLength;
			
			#start at the beginning
			$mobStartIdx = 0;
			$beginningOfMobs = substr($mobs, $mobStartIdx, $where);
			$endOfMobs = substr($mobs, $searchStrEnd, $mobsLength);
			
			$mobs = "$beginningOfMobs$endOfMobs";
		}
		
		# Remove the last :|$|
		$stringLength = length $mobs;		
		$snipOffEndIdx = $stringLength - 4;
		$mobs = substr($mobs, 0, $snipOffEndIdx);

		#replace all colons with commas and a space
		$mobs =~ s/:/, /g;
				
		print OUTFILE "$$roundfltPX\t$roundfltPY\t$roundfltPZ\t$mobs\n";
	}
	$numlines ++;
}

close FILE;
close OUTFILE;


