#!/usr/bin/perl -W

# ================================================================================
#
# QuestChecker.pl - validation rules for quests
# Copyright 2006, Sony Online Entertainment Inc.
#
# ================================================================================

use strict;

use Data::Dumper;
use XML::Simple;

# --------------------------------------------------------------------------------

use constant VERSION => "1.01";

use constant {
	WARNING => 0,
	ERROR => 1,
};

use constant {
	TASK => 0,
	LIST => 1,
};

# --------------------------------------------------------------------------------

my $g_questFileName = "";
my $g_numberOfErrors = 0;
my $g_numberOfWarnings = 0;
my $g_dump = 0;

# --------------------------------------------------------------------------------

sub usage
{
	print STDERR "Usage: $0 [-h] | [-d] <quest file (.qst)>\n";
	print STDERR " Performs rule checks on the quest file and outputs errors and warnings.\n";
	print STDERR " -d    dumps the perl data structure that represents the file\n";
	print STDERR " -h    show usage\n";
	exit 1;
}

# --------------------------------------------------------------------------------

sub getDataValue
{
	my ($node, $name) = @_;

	return "" if !defined($node->{data}->{$name});

	return $node->{data}->{$name}->{value};
}

# --------------------------------------------------------------------------------

sub isTrue
{
	my $value = &getDataValue;

	return !(lc($value) eq "false" || $value eq "0");
}

# --------------------------------------------------------------------------------

sub isFalse
{
	return !&isTrue;
}

# --------------------------------------------------------------------------------

sub isEmpty
{
	my $value = &getDataValue;

	return $value eq "";
}

# --------------------------------------------------------------------------------

sub showSummary
{
	print "\nSummary:\n\n";
	print "Errors - " . $g_numberOfErrors . "\n";
	print "Warnings - " . $g_numberOfWarnings . "\n";

	if ($g_numberOfWarnings == 0 && $g_numberOfErrors == 0)
	{
		print "\n!SUCCESS! Quest has NO warnings or errors\n";
	}
	else
	{
		print "\n!FAILURE! Quest has warnings or errors\n";
	}

	return;
}

# --------------------------------------------------------------------------------

sub displayMessage
{
	my ($node, $nodeType, $field, $messageType, $message) = @_;

	print $g_questFileName . " (";

	if ($nodeType == TASK)
	{
		print "TASK:" . $node->{id} . ":" . $node->{type} . ":";
	}
	elsif ($nodeType == LIST)
	{
		print "LIST:";
	}
	else
	{
		print "UNKNOWN:";
	}

	print $field . ") : ";

	if ($messageType == WARNING)
	{
		print "WARNING";
		++$g_numberOfWarnings;
	}
	elsif ($messageType == ERROR)
	{
		print "ERROR";
		++$g_numberOfErrors;
	}
	else
	{
		print "UNKNOWN";
	}

	print " : " . $message . "\n";
}

# --------------------------------------------------------------------------------

sub checkWaitForTaskEntry
{
	my ($task, $fileName, $taskName, $displayString) = @_;

	if (&isEmpty($task, $fileName) && &isEmpty($task, $taskName) && &isEmpty($task, $displayString))
	{
		return 0;
	}

	if (&isEmpty($task, $fileName))
	{
		&displayMessage($task, TASK, $fileName, ERROR, "Quest filename is required");
	}

	if (&isEmpty($task, $taskName))
	{
		&displayMessage($task, TASK, $taskName, ERROR, "Quest task name is required");
	}

	if (&isEmpty($task, $displayString))
	{
		&displayMessage($task, TASK, $displayString, WARNING, "Display string is empty so the task counter will not update in the journal or in the helper");
	}

	return 1;
}

# --------------------------------------------------------------------------------

sub checkTask
{
	my $task = shift;

	# check base task fields

	if (&isTrue($task, "isVisible") && &isEmpty($task, "journalEntryTitle"))
	{
		&displayMessage($task, TASK, "journalEntryTitle", ERROR, "Visible journal has missing title");
	}

	if (&isTrue($task, "isVisible") && &isEmpty($task, "journalEntryDescription"))
	{
		&displayMessage($task, TASK, "journalEntryDescription", ERROR, "Visible journal has missing description");
	}

	if (&isTrue($task, "createWaypoint") && &isEmpty($task, "waypointName"))
	{
		&displayMessage($task, TASK, "waypointName", ERROR, "Waypoint missing name");
	}

	if (!&isEmpty($task, "interiorWaypointAppearance") || !&isEmpty($task, "buildingCellName"))
	{
		if (&isEmpty($task, "interiorWaypointAppearance"))
		{
			&displayMessage($task, TASK, "interiorWaypointAppearance", ERROR, "Interior waypoint appearance required for interior waypoints");
		}

		if (&isEmpty($task, "buildingCellName"))
		{
			&displayMessage($task, TASK, "buildingCellName", ERROR, "Building cell name required for interior waypoints");
		}

		if (&getDataValue($task, "LocationY(m)") == 0)
		{
			&displayMessage($task, TASK, "LocationY(m)", ERROR, "Interior waypoint has a zero Y value which is not valid");
		}
	}
	
	if (&isTrue($task, "createEntranceWaypoint") && &isEmpty($task, "entranceWaypointName"))
	{
		&displayMessage($task, TASK, "entranceWaypointName", ERROR, "Entrance Waypoint missing name");
	}
	
	if (&isTrue($task, "createEntranceWaypoint") && !&isTrue($task, "createWaypoint"))
	{
		&displayMessage($task, TASK, "createEntranceWaypoint", ERROR, "Entrance Waypoint on but no non-entrance waypoint defined");
	}
	
	# check task specific fields

	if ($task->{type} eq "Comm Player")
	{
		if (&isEmpty($task, "Comm Message Text"))
		{
			&displayMessage($task, TASK, "Comm Message Text", ERROR, "Message text is required");
		}

		if (&isEmpty($task, "NPC Appearance Server Template"))
		{
			&displayMessage($task, TASK, "NPC Appearance Server Template", ERROR, "Server appearance template is required");
		}
	}
	elsif ($task->{type} eq "Craft Item")
	{
		if (&isEmpty($task, "Server Object Template"))
		{
			&displayMessage($task, TASK, "Server Object Template", ERROR, "Server object template is required");
		}
	}
	elsif ($task->{type} eq "Destroy Multiple")
	{
		if (!&isEmpty($task, "Target Server Template") && !&isEmpty($task, "Social Group"))
		{
			&displayMessage($task, TASK, "Target Server Template", WARNING, "Both server template and social group specified. Server template will be ignored");
		}

		if (&isEmpty($task, "Target Server Template") && &isEmpty($task, "Social Group"))
		{
			&displayMessage($task, TASK, "Target Server Template", ERROR, "A server template or social group is required");
		}

		if (&getDataValue($task, "RewardCredits") != 0)
		{
			&displayMessage($task, TASK, "RewardCredits", WARNING, "Reward credits is deprecated. Use the global quest reward instead");
		}
	}
	elsif ($task->{type} eq "Destroy Multiple and Loot")
	{
		if (!&isEmpty($task, "CreatureType") && !&isEmpty($task, "Social Group"))
		{
			&displayMessage($task, TASK, "CreatureType", WARNING, "Both server template and social group specified. Server template will be ignored");
		}

		if (&isEmpty($task, "CreatureType") && &isEmpty($task, "Social Group"))
		{
			&displayMessage($task, TASK, "CreatureType", ERROR, "A server template or social group is required");
		}

		if (&getDataValue($task, "RewardCredits") != 0)
		{
			&displayMessage($task, TASK, "RewardCredits", WARNING, "Reward credits is deprecated. Use the global quest reward instead");
		}
	}
	elsif ($task->{type} eq "Encounter")
	{
		if (&isEmpty($task, "Creature Type"))
		{
			&displayMessage($task, TASK, "Creature Type", ERROR, "Creature type is required");
		}

		if (&getDataValue($task, "Max Distance") < &getDataValue($task, "Min Distance"))
		{
			&displayMessage($task, TASK, "Max Distance", ERROR, "Max distance cannot be less than min distance");
		}
	}
	elsif ($task->{type} eq "Escort")
	{
		if (&isEmpty($task, "Escort Creature Type"))
		{
			&displayMessage($task, TASK, "Escort Creature Type", ERROR, "Escort creature type is required");
		}
	}
	elsif ($task->{type} eq "Give Item To NPC")
	{
		if (&isEmpty($task, "itemToGive"))
		{
			&displayMessage($task, TASK, "itemToGive", ERROR, "Item to give is required");
		}
	}
	elsif ($task->{type} eq "Go to Location")
	{
		if (&isFalse($task, "createWaypoint"))
		{
			&displayMessage($task, TASK, "createWaypoint", WARNING, "Waypoint not created");
		}
	}
	elsif ($task->{type} eq "Grant Space Quest")
	{
		if (&isEmpty($task, "questToGrant"))
		{
			&displayMessage($task, TASK, "questToGrant", ERROR, "Quest to grant is required");
		}

		if (&isEmpty($task, "questType"))
		{
			&displayMessage($task, TASK, "questType", ERROR, "Quest type is required");
		}
	}
	elsif ($task->{type} eq "Immediately Complete Quest")
	{
	}
	elsif ($task->{type} eq "Immediately Clear Quest")
	{
	}
	elsif ($task->{type} eq "Nothing")
	{
	}
	elsif ($task->{type} eq "Perform")
	{
	}
	elsif ($task->{type} eq "Perform Action On Npc")
	{
		&displayMessage($task, TASK, "none", ERROR, "Do not use this task");
	}
	elsif ($task->{type} eq "Remote Encounter")
	{
		if (&isEmpty($task, "creatureName"))
		{
			&displayMessage($task, TASK, "creatureName", ERROR, "Creature name is required");
		}

		if (&isEmpty($task, "encounterSceneName"))
		{
			&displayMessage($task, TASK, "encounterSceneName", ERROR, "Encounter scene name is required");
		}

		if (&getDataValue($task, "maxDifficulty") < &getDataValue($task, "minDifficulty"))
		{
			&displayMessage($task, TASK, "maxDifficulty", ERROR, "Max difficulty cannot be less than min difficulty");
		}
	}
	elsif ($task->{type} eq "Retrieve Item")
	{
		if (&isEmpty($task, "Server Object Template"))
		{
			&displayMessage($task, TASK, "Server Object Template", ERROR, "Server object template is required");
		}
		if (&isEmpty($task, "ItemName"))
		{
			&displayMessage($task, TASK, "ItemName", ERROR, "Item name is required");
		}
	}
	elsif ($task->{type} eq "Reward")
	{
		&displayMessage($task, TASK, "none", WARNING, "Depricated task. Use the reward fields global to the quest instead");
	}
	elsif ($task->{type} eq "Show Message Box")
	{
		if (&isEmpty($task, "messageBoxTitle"))
		{
			&displayMessage($task, TASK, "messageBoxTitle", ERROR, "Message box title is required");
		}

		if (&isEmpty($task, "messageBoxText"))
		{
			&displayMessage($task, TASK, "messageBoxText", ERROR, "Message box text is required");
		}
	}
	elsif ($task->{type} eq "Static Escort")
	{
		if (&isEmpty($task, "Escort Creature Type"))
		{
			&displayMessage($task, TASK, "Escort Creature Type", ERROR, "Creature type is required");
		}
	}
	elsif ($task->{type} eq "Talk to Npc")
	{
		&displayMessage($task, TASK, "none", WARNING, "Depricated task. Use 'Wait for Signal' instead");
	}
	elsif ($task->{type} eq "Timer")
	{
		if (&getDataValue($task, "Max Time") < &getDataValue($task, "Min Time"))
		{
			&displayMessage($task, TASK, "Max Time", ERROR, "Max time cannot be less than min time");
		}
	}
	elsif ($task->{type} eq "Wait for Signal")
	{
		if (&isEmpty($task, "Signal Name"))
		{
			&displayMessage($task, TASK, "Signal Name", ERROR, "Signal name is required");
		}
	}
	elsif ($task->{type} eq "Wait for Tasks")
	{
		my ($currentDefined, $previousDefined) = (0, 0);
		my ($skippedTask, $firstMissing) = (0, 0);

		$currentDefined = &checkWaitForTaskEntry($task, "Task1 Quest Filename", "Task1 taskName", "Task1 Display String");

	    $firstMissing = !$currentDefined;
		$previousDefined = $currentDefined;

		$currentDefined = &checkWaitForTaskEntry($task, "Task2 Quest Filename", "Task2 taskName", "Task2 Display String");

		$skippedTask = 1 if (!$previousDefined && $currentDefined);
		$previousDefined = $currentDefined;

		$currentDefined = &checkWaitForTaskEntry($task, "Task3 Quest Filename", "Task3 taskName", "Task3 Display String");

		$skippedTask = 1 if (!$previousDefined && $currentDefined);
		$previousDefined = $currentDefined;

		$currentDefined = &checkWaitForTaskEntry($task, "Task4 Quest Filename", "Task4 taskName", "Task4 Display String");

		$skippedTask = 1 if (!$previousDefined && $currentDefined);
		$previousDefined = $currentDefined;

		$currentDefined = &checkWaitForTaskEntry($task, "Task5 Quest Filename", "Task5 taskName", "Task5 Display String");

		$skippedTask = 1 if (!$previousDefined && $currentDefined);
		$previousDefined = $currentDefined;

		$currentDefined = &checkWaitForTaskEntry($task, "Task6 Quest Filename", "Task6 taskName", "Task6 Display String");

		if ($firstMissing)
		{
			&displayMessage($task, TASK, "none", ERROR, "The first task is required");
		}

		if ($skippedTask)
		{
			&displayMessage($task, TASK, "none", ERROR, "A task was skipped. Tasks must be defined consecutively");
		}
	}
}

# --------------------------------------------------------------------------------

sub checkTasks
{
	my $node = shift;

	return if !defined($node->{task});

	for my $task (@{$node->{task}})
	{
		&checkTask($task);
		&checkTasks($task);
	}
}

# --------------------------------------------------------------------------------

sub checkList
{
	my $list = shift;

	# check list fields

	if (&isEmpty($list, "category"))
	{
		&displayMessage($list, LIST, "category", ERROR, "Quest journal category is required");
	}

	if (&isEmpty($list, "journalEntryTitle"))
	{
		&displayMessage($list, LIST, "journalEntryTitle", ERROR, "Quest journal title is required");
	}

	if (&isEmpty($list, "journalEntryDescription"))
	{
		&displayMessage($list, LIST, "journalEntryDescription", ERROR, "Quest journal description is required");
	}
}

# --------------------------------------------------------------------------------

sub main
{
	my $ref = XMLin($g_questFileName, keyattr => ['name'], forcearray => ['task']);

	if ($g_dump)
	{
		print Dumper($ref);
	}
	else
	{
		print "Checking quest '$g_questFileName'\n\n";

		print "*** checking tasks ***\n";
		&checkTasks($ref->{tasks});

		print "*** checking list ***\n";
		&checkList($ref->{list});

		&showSummary;
	}
}

# ================================================================================

print "\n# Quest Checker Version " . VERSION . "\n\n";

&usage if $#ARGV < 0;

while ($#ARGV > 0)
{
	$g_dump = 1 if ($ARGV[0] eq "-d");

	&usage if ($ARGV[0] eq "-h");

	&usage if ($ARGV[0] ne "-h" && $ARGV[0] ne "-d");

	shift;
}

$g_questFileName = $ARGV[0];

unless (-e $g_questFileName)
{
	print "File does not exist: '$g_questFileName'\n";
	&usage;
}

&main;

# ================================================================================
