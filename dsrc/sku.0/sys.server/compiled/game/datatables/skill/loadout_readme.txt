The loadout.tab file contains items that a gm can get from the frog using the "Skill Loadout" option. If the player has the skill in the first column of the table, they will get the objects in the rest of the columns put in their inventory.


The armor column designates an armor set to give the player. It takes the form of "category""level", i.e.:

	battle1   = battle armor, level 1
	assault2  = attault armor, level 2
	recon2    = recon armor, level 2

Armor categories are "assault", "battle", or "recon". Armor levels are 1, 2 or 3.


If you need to add more items for a given skill than there are columns, feel free to add new columns. As long as the column names are unique, and the column type is "s", the script on the frog will handle it.
