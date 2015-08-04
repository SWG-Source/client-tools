#!/usr/bin/perl

# Some hard-coded datatypes:
$datatype{"containedBy"}="NetworkId";
$datatype{"slotArrangement"}="int";

while (<>) 
{
    if (/Archive\:\:AutoDeltaVariable(Callback)?<([^,]*).*>\s+(\w+)\s*\;/)
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
    $parent{"ServerObject"}="N\A";
}

open (TEMPLATE,"DecodeTemplateTop_cpp");
while (<TEMPLATE>)
{
    print;
}
close (TEMPLATE);
print "\n";

foreach $class (keys %parent)
{
    #first, determine the starting count by traversing up the tree
    $count=0;
    for ($c=$parent{$class}; $c ne ""; $c=$parent{$c})
    {
	$count+=$sharedattrcount{$c};
    }
    makeFunction("Shared",$count,$class,$parent{$class},$shared{$class});
    print "\n";

    $count=0;
    for ($c=$parent{$class}; $c ne ""; $c=$parent{$c})
    {
	$count+=$serverattrcount{$c};
    }
    makeFunction("Server",$count,$class,$parent{$class},$server{$class});
    print "\n";

    $count=0;
    for ($c=$parent{$class}; $c ne ""; $c=$parent{$c})
    {
	$count+=$clientattrcount{$c};
    }
    makeFunction("Client",$count,$class,$parent{$class},$client{$class});
    print "\n";
}

#Make the functions that switch on type
makeSwitcher("Server");
makeSwitcher("Shared");
makeSwitcher("Client");


sub makeFunction
{
    my($section,$count,$class,$parent,$items) = @_;
    my($baseindex)=($count);
    $class="Object" if ($class eq "ServerObject");
    my($classbuffer);
    if ($class eq "Object")
    {
	$classbuffer="m_objectTableBuffer";
    }
    else
    {
	$classbuffer="m_".lcfirst($class)."Buffer";
    }
    $parent="Object" if ($parent eq "ServerObject");

    open (TEMPLATE,"DecodeTemplate_cpp");
    while (<TEMPLATE>)
    {
	if (($baseindex != 0) || (!(/^\+/)))
	{
	    s/^\+//;
	    s/\$classbuffer/$classbuffer/g;
	    s/\$class/$class/g;
	    s/\$baseindex/$baseindex/g;
	    s/\$parent/$parent/g;
	    s/\$section/$section/g;
	    if (/\$cases/)
	    {
		foreach $item (split (" ",$items))
		{
		    $item =~ s/\s+//g;
		    print "\t\tcase ".$count++.":\n\t\t{\n";
		    if ($item eq "*m_objVars")
		    {
			print "\t\t\tdecodeObjVars(objectId, data);\n";
			print "\t\t\tbreak;\n";
			print "\t\t}\n";
		    }
		    else
		    {
			if ($item eq "m_scriptObject")
			{
			    print "\t\t\tdecodeScriptObject(objectId, data, isBaseline);\n";
			    print "\t\t\tbreak;\n";
			    print "\t\t}\n";
			}
			else
			{
			    print "\t\t\t$datatype{$item} temp;\n";
			    print "\t\t\tArchive::get(data,temp);\n";
			    print "\t\t\trow->".&dbIze($item)."=temp;\n";
			    print "\t\t\tbreak;\n";
			    print "\t\t}\n";
			}
		    }
		}
	    }
	    else
	    {
		print;
	    }
	}
    }
    close (TEMPLATE);
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

sub makeSwitcher
{
    my ($section)=@_;
    print "void SwgSnapshot::decode".$section."Data(NetworkId objectId, Tag typeId, uint16 index, Archive::ByteStream::ReadIterator &bs, bool isBaseline)\n";
    print "{\n";
    print "\tswitch(typeId)\n";
    print "\t{\n";
    foreach $class (keys %parent)
    {
	if ($class =~ /ServerObject/)
	{
	    print "\t\tcase ${class}Template::${class}Template_tag:\n";
	    print "\t\t\tdecode".$section."Object(objectId, index, bs, isBaseline);\n";
	}
	else
	{
	    print "\t\tcase Server${class}Template::Server${class}Template_tag:\n";
	    print "\t\t\tdecode".$section.$class."(objectId, index, bs, isBaseline);\n";
	}
	print "\t\t\tbreak;\n";
    }
    print "\t}\n";
    print "}\n\n";
}
