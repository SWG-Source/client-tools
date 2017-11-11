@echo off

goto start_p4_space

:show_usage

echo.
echo usage:%0 ^<branch^> ^<operation^> ... (e.g. %0 s4 edit -c 123456)
echo Performs a perforce operation on all space quests

goto :eof

:start_p4_space

if "%1"=="" goto show_usage
if "%2"=="" goto show_usage

set P4BRANCH=%1
set P4OP=

:concat_args

if not "%2"=="" set P4OP=%P4OP%%2 && shift && goto concat_args

p4 %P4OP% //depot/swg/%P4BRANCH%/data/sku.0/sys.server/compiled/game/datatables/spacequest/*/...
p4 %P4OP% //depot/swg/%P4BRANCH%/data/sku.0/sys.shared/built/game/string/en/spacequest/*/...
p4 %P4OP% //depot/swg/%P4BRANCH%/data/sku.0/sys.shared/compiled/game/datatables/questlist/spacequest/*/...
p4 %P4OP% //depot/swg/%P4BRANCH%/data/sku.0/sys.shared/compiled/game/datatables/questtask/spacequest/*/...
p4 %P4OP% //depot/swg/%P4BRANCH%/dsrc/sku.0/sys.server/compiled/game/datatables/spacequest/*/...
p4 %P4OP% //depot/swg/%P4BRANCH%/dsrc/sku.0/sys.shared/compiled/game/datatables/questlist/spacequest/*/...
p4 %P4OP% //depot/swg/%P4BRANCH%/dsrc/sku.0/sys.shared/compiled/game/datatables/questtask/spacequest/*/...
