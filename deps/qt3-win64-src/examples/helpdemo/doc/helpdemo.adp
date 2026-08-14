<!DOCTYPE DCF>

<assistantconfig version="3.2.0">

<profile>
    <property name="name">HelpExample</property>
    <property name="title">Help Example</property>
    <property name="applicationicon">logo.png</property>
    <property name="startpage">index.html</property>
    <property name="aboutmenutext">About Help</property>
    <property name="abouturl">../about.txt</property>
    <property name="assistantdocs">../../../doc/html</property>
</profile>

<DCF ref="index.html" icon="handbook.png" title="Help example">
    <section ref="./manual.html" title="How to use this Example">
	<keyword ref="./manual.html#onlydoc">Example Profile</keyword>
	<keyword ref="./manual.html#lineedit">Filename</keyword>
	<keyword ref="./manual.html#hide">Hide Sidebar</keyword>
	<keyword ref="./manual.html#openqabutton">Open Assistant</keyword>
	<keyword ref="./manual.html#closeqabutton">Close Assistant</keyword>
	<keyword ref="./manual.html#displaybutton">Display</keyword>
        <keyword ref="./manual.html#closebutton">Close</keyword>
    </section>
</DCF>

</assistantconfig>
