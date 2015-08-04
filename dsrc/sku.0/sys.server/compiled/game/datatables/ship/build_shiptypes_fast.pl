#!/usr/bin/perl

###
# main
###

$shiptype_dir = "\\swg\\current\\dsrc\\sku.0\\sys.server\\compiled\\game\\datatables\\ship";
$shiptype_out_dir = "\\swg\\current\\data\\sku.0\\sys.server\\compiled\\game\\datatables\\ship";

$p4name = "$shiptype_dir\\shiptype.tab";
$p4name =~ s/\\/\//g;
system( "p4 edit //depot" . $p4name );

$p4name = "$shiptype_out_dir\\shiptype.iff";
$p4name =~ s/\\/\//g;
system( "p4 edit //depot" . $p4name );

system( "perl build_shiptypes.pl +balance" );
system( "copy $shiptype_dir\\shiptype_out.tab $shiptype_dir\\shiptype.tab" );
system( "datatabletool -i $shiptype_dir\\shiptype.tab -o $shiptype_out_dir\\shiptype.iff" );

