#!/usr/bin/perl

$sections[0]="shared";
$sections[1]="server";
$sections[2]="client";

# Some hard-coded datatypes:
$datatype{"containedBy"}="NetworkId";
$datatype{"slotArrangement"}="int";

&parse();
&figureBaseCounts();
&readTemplate();
&copyTop();
&makeEncoders();
&makeSwitcher();

sub parse
{
    while (<>) 
    {
	if (/Archive\:\:AutoDeltaVariable(Callback)?<([\w\:]+).*>\s+(\w+)\s*\;/)
	{
	    $datatype{$3}=$2;
	}
	if (/(\w+Object)\s*\:\s*public\s+(\w+Object)/)
	{
	    $parent{$1}=$2;
	}
	if (/(\w+Object)\:\:\1/) # looks like a constructor
	{
	    $classname = $1;
	}
	if (/addSharedVariable\s*\((\*?\w+)\)/)
	{
	    $shared{$classname}.="$1 ";
	    $sharedattrcount{$classname}++;
	}
	if (/addServerVariable\s*\((\*?\w+)\)/)
	{
	    $server{$classname}.="$1 ";
	    $serverattrcount{$classname}++;
	}
	if (/addClientVariable\s*\((\*?\w+)\)/)
	{
	    $client{$classname}.="$1 ";
	    $clientattrcount{$classname}++;
	}
	if (/const\s+int\s+GOT_(\w*Object)\s*=\s*(\d*);/)
	{
	    $GOT{$1}=$2;
	}
    }
    $parent{"ServerObject"}="N/A";
}

sub figureBaseCounts
# Compute how many attributes are in the parents/ancestors of each class
{
    my($class,$sharedCount,$servercount,$clientcount);
    foreach $class (keys %parent)
    {
	#determine the starting count by traversing up the tree
	$sharedcount=0;
	$servercount=0;
	$clientcount=0;
	for ($c=$parent{$class}; $c ne ""; $c=$parent{$c})
	{
	    $sharedcount+=$sharedattrcount{$c};
	    $servercount+=$serverattrcount{$c};
	    $clientcount+=$clientattrcount{$c};
	}
	$sharedbasecount{$class}=$sharedcount;
	$serverbasecount{$class}=$servercount;
	$clientbasecount{$class}=$clientcount;
    }
}

sub readTemplate
{
    open (TEMPLATE,"EncodeTemplate_cpp");
    while (<TEMPLATE>)
    {
	$template.=$_;
    }
    close (TEMPLATE);
}

sub copyTop
{
    open (TOP,"EncodeTemplateTop_cpp");
    while (<TOP>)
    {
	print;
    }
    close (TOP);
}

sub makeEncoders
# makes the encoding functions from the template
{
    foreach $classname (keys %parent)
    {
	$parent=$parent{$classname};

	$class=$classname;
	$class="Object" if ($class eq "ServerObject");
	if ($class eq "Object")
	{
	    $classbuffer="m_objectTableBuffer";
	}
	else
	{
	    $classbuffer="m_".lcfirst($class)."Buffer";
	}
	$parent="Object" if ($parent eq "ServerObject");

	foreach $section (@sections)
	{
	    eval("\$basecount = \$".$section."basecount{\$classname}");
	    eval("\$totalcount = \$basecount + \$".$section."attrcount{\$classname}");
	    @items=split(" ",eval("\$${section}{\$classname}"));

	    eval $template;
	    print "error is $@\n" if $@;
	}
    }
}

sub makeSwitcher
{
    foreach $section (@sections)
    {
	print <<END;
void SwgSnapshot::encode\u${section}Data(NetworkId objectId, Tag typeId, const GameServerConnection &connection) const
{
\tArchive::ByteStream bs;
\tswitch (typeId)
\t{
END

	foreach $class (keys %parent)
	{
	    $class="Object" if ($class eq "ServerObject");
		print "\tcase Server${class}Template::Server${class}Template_tag:\n";
	    print "\t\tencode\u${section}$class(objectId, bs);\n";
	    print "\t\tbreak;\n";
	}
        print "\t}\n\n";
        print <<END;
	// test for 0-length message
	unsigned short packedSize;
	Archive::ByteStream::ReadIterator ri = bs.begin();
	Archive::get(ri, packedSize);
	ri = bs.begin();
	if (packedSize == 0)
		return;

END
        $secname=uc($section);
        $secname="CLIENT_SERVER" if ($secname eq "CLIENT");   
        print "\tBaselinesMessage msg(objectId,typeId,BaselinesMessage::BASELINES_".$secname.",ri);\n";
        print <<END;
\tconnection.send(msg,true);
}

END

    }
}

sub dbIze
{ 
    my($name)=@_;
    $name =~ s/m\_//;
    $name =~ s/\s+$//;
    $name =~ s/([a-z])([A-Z])/$1_$2/g;
    $name =~ tr/[A-Z]/[a-z]/;
    return $name;
}
