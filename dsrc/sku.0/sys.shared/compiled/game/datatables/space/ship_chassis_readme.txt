This folder contains various datatables for defining properties of ship chassis.

ship_chassis.tab:
	Contains the master list of ship chassis.
	
ship_chassis_<chassisname>.tab
	Each chassis has one of these files.
	For each component that can be installed in this chassis, there is a row.
	For each slot type, there is a column.
	
	In the cell associated with a component & slot, there is a comma seperated
	list of objecttemplate:hardpoint pairs.  The pairing is seperated by a colon.
	The object template name is a shortened form of the attachment template name.
	The shortened form is prepended with 
	"object/tangible/ship/attachment/<componenttype>/shared_" and appended with
	".iff"
	
	The hardpoint may be empty.  This indicates the attachment should be placed 
	at the origin of the ship.

	