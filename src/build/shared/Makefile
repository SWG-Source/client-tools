#
# Copyright 2001, Sony Online Entertainment, Inc.
# All Rights Reserved.
#
# Makefile for creating Emacs workspace data file for fast workspace
# file opening.

WORKSPACE_FILENAME=swg.ews
PERL=perl

all:
	$(PERL) -w make_workspace.pl ../.. > ${WORKSPACE_FILENAME}
	$(PERL) -w make_workspace.pl ../../../tools >> ${WORKSPACE_FILENAME}
	$(PERL) -w make_workspace.pl ../../../dsrc >> ${WORKSPACE_FILENAME}

clean:
	rm -f ${WORKSPACE_FILENAME}
