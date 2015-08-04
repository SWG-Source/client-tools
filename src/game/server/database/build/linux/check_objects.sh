#!/bin/bash
if [ $# -lt 1 ]
then
  echo "Usage: $0 <username/password@service>"
  exit
fi

sqlplus -s /nolog <<EOF
connect $1
set heading off
set feedback off
set term off
set pages 0

spool check_objects.sql

select 'alter '||decode(object_type,'TYPE','TYPE','PACKAGE BODY','PACKAGE',object_type)||' '
       ||object_name||' compile'||decode(object_type,'PACKAGE BODY',' BODY;',';')
from user_objects
where status = 'INVALID'
order by object_type;

select 'exit;' from dual;

spool off

EOF

