
# Determine what schema should be used based on where we are running this from.
# The schema is the username if we are working in the current branch, or
# username_branchname if working in any other branch.

BRANCH=$(shell pwd |sed -e "/^.*swg\//s///" -e "/.src.*$$/s///")

ifeq ($(BRANCH),current)
	DBUSER=$(USER)
else
	DBUSER=$(USER)_$(BRANCH)
endif

default:
	make delta

delta: templates
	perl database_update.pl --username=$(DBUSER) --delta

create: template_data
	perl database_update.pl --username=$(DBUSER) --create --newschema --dbausername=ausdba --dbapassword=swgdbroot
	@touch ../../data/templates.out #create script will load the template data

drop:
	perl database_update.pl --username=$(DBUSER) --drop
	rm -f ../../data/templates.out

packages:
	perl database_update.pl --username=$(DBUSER) --packages

copy_database:
	sqlplus $(DBUSER)/changeme@swodb @../../queries/copy_from_database.sql
	rm -f ../../data/templates.out

copy_buildcluster:
	perl copy_buildcluster.pl --username=$(DBUSER) --copybuildcluster

wipe_characters:
	sqlplus $(DBUSER)/changeme@swodb @../../queries/wipe_characters.sql

purge:
	sqlplus $(DBUSER)/changeme@swodb @../../queries/purge_deleted_objects.sql

template_data: ../../data/templates.sql

templates: ../../data/templates.out

../../data/templates.out: ../../data/templates.sql
	@echo Loading template list, please wait ...
	sqlplus $(DBUSER)/changeme@swodb @../../data/templates.sql > ../../data/templates.out

../../data/templates.sql: ../../../../../../dsrc/sku.0/sys.server/built/game/misc/object_template_crc_string_table.tab
	perl ../../templates/processTemplateList.pl < ../../../../../../dsrc/sku.0/sys.server/built/game/misc/object_template_crc_string_table.tab > ../../data/templates.sql
