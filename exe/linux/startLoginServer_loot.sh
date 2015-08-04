#!/bin/bash

export ORACLE_HOME=/opt/app/oracle/product/9.2.0
export PATH=/opt/app/oracle/product/9.2.0/bin:$PATH
export LD_LIBRARY_PATH=/opt/app/oracle/product/9.2.0/lib:$LD_LIBRARY_PATH
export NLS_LANG=american_america.utf8

cd /swg/swg/s1/exe/linux
killall -9 LoginServer
debug/LoginServer -- @loginServer_loot.cfg &
