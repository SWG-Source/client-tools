############################################################################
#
# script_prep.py - Script preprocessor. Turns .script files into .java files.
# If the .java file is created successfully, runs the javac program on it.
#
###########################################################################

import os
import re
import sys
import script_util
import ScriptFile

############################################################################
# globals
############################################################################

JavaCOptions = ()


############################################################################
# functions
############################################################################

##
# Shorthand for script_util.print_error().
##
def print_error(msg, linenum):
    script_util.print_error(msg, linenum)
#end print_error()

##
# Parse a #define statement.
##
def parse_define(line, table):
    if (len(line) > 0):
        print "defining <%s>" % (line)
        #print table
        if (line in table):
            return 1
        # test for DEBUG and RELEASE
        if (line == "DEBUG"):
            if ("RELEASE" in table):
                print_error("cannot define DEBUG, RELEASE already defined", 0)
                return 0
        elif (line == "RELEASE"):
            if ("DEBUG" in table):
                print_error("cannot define RELEASE, DEBUG already defined", 0)
                return 0
        table.append(line)
    return 1
#end parse_define()

##
# Parses the C++ ScriptFunctionTable.cpp file to get the available triggers
##
def parseScriptFuncTable():
    global TriggerProtos

    script_util.TriggerProtos.clear()

    # guess the path to ScriptFunctionTable.cpp
    filename = ""
    path = os.path.abspath(os.curdir)
    while (len(path) > 0):
        splitpath = os.path.split(path)
        if (splitpath[1] == "current"):
            filename = os.path.join(path, "src", "engine", "server", "library", "serverScript", "src", "shared", "ScriptFunctionTable.cpp")
            break
        else:
            path = splitpath[0]
    #endwhile

    try:
        fp = open(filename, 'r')
    except IOError, err:
        sys.stderr.write("Mocha warning: ScriptFunctionTable.cpp not found, triggers will not be checked for correctness\n")
        return 1

    while 1:
        line = fp.readline()
        if (len(line) == 0):     # end of file
            break
        #endif

        # see if this line is a trigger definition
        result = re.search('^\s*{(Scripting::TRIG_[A-Z0-9_]+),\s*"(\w+)",\s*"([A-Za-z\*\[]*)"', line)
        if (result <> None):
            name = result.group(2)
            params = result.group(3)
            # the first param is the obj_id self
            name += "#obj_id"
            # go through the param chars to create the function prototype for the trigger
            i = 0
            while i < (len(params)):
                if (params[i] == 'i'):
                    if (i < len(params) - 1 and params[i + 1] == '*'):
                        i += 1
                        name += "#modifiable_int"
                    else:
                        name += "#int"
                    #endif
                elif (params[i] == 'f'):
                    if (i < len(params) - 1 and params[i + 1] == '*'):
                        i += 1
                        name += "#modifiable_float"
                    else:
                        name += "#float"
                elif (params[i] == 's' or params[i] == 'u'):
                    name += "#String"
                elif (params[i] == 'O'):
                    name += "#obj_id"
                elif (params[i] == 'S'):
                    if (i < len(params) - 1 and params[i + 1] == '*'):
                        i += 1
                        name += "#modifiable_string_id"
                    else:
                        name += "#string_id"
                elif (params[i] == 'E'):
                    name += "#slot_data"
                elif (params[i] == 'A'):
                    name += "#attrib_mod"
                elif (params[i] == 'M'):
                    name += "#mental_state_mod[]"
                elif (params[i] == 'm'):
                    if (i < len(params) - 1 and params[i + 1] == '*'):
                        i += 1
                        name += "#menu_info"
                    else:
                        name += "#menu_info"
                elif (params[i] == '['):
                    name += "[]"
                else:
                    print_error("Unknown ScriptFunctionTable.cpp, name = " + name, 0)
                    return 0
                #endif
                i += 1
            #endwhile
#            print "Trigger proto " + name
            script_util.TriggerProtos[name] = 1
        #endif
    #endwhile

    fp.close()
    return 1
#end parseScriptFuncTable

##
# Parse script files given on the command line.
##
def parse(args):
    global JavaCOptions

    nocompile = 0
    commandDefines = [] # names defined by passing the -d param in the command line

    # get the .script filename from the command line
    if (len(args) <= 1):
        print "Command syntax:"
        print "script_prep [-nocompile] [-d<name> ...] script1 ..."
        print "options:"
        print "-nocompile: create the java file, but don't compile it"
        print "-d<name>: define the flag <name>"
        print "script1 ...: scripts to process"
        return
    #endif

    for i in xrange(1, len(args)):
        param = args[i]
        if (re.search("^-events", param) <> None):
            # flag to generate a list of events the script handles
            pass
        if (re.search("^-d", param) <> None):
           parse_define(param[2:], commandDefines)
        elif (re.search("^-nocompile", param) <> None):
            nocompile = 1
        elif (re.search("\.(script|jawa)$", param) <> None):
            print "processing script %s" % param
            scriptFile = ScriptFile.ScriptFile()
            scriptFile.parseFile(param, commandDefines, nocompile, JavaCOptions)
        elif (re.search("\.(scriptlib)$", param) <> None):
            print "processing script library %s" % param
            scriptFile = ScriptFile.ScriptFile()
            scriptFile.parseLibrary(param, commandDefines, nocompile, JavaCOptions)
        else:
            print_error("unknown file type for %s" % (param), 0)
            #sys.stderr.write("ERROR: unknown file type for %s\n" % (param))
    #end for i
#end parse()

##
# Sets up paths for sending to the Java compiler
##
def setupJava(args):
    global JavaCOptions

    # find the name of the script file from the args to use as a base directory
    scriptPath = None
    for i in xrange(1, len(args)):
        param = args[i]
        if (re.search("\.(script|jawa)$", param) <> None):
            scriptPath = param
        elif (re.search("\.(scriptlib)$", param) <> None):
            scriptPath = param
    #end for i
    if (scriptPath == None):
        print_error("cannot find script name in args", 0)
        return 0
    #endif

    if (os.name == "nt" or os.name == "dos"):
        extension = ".exe"
    else:
        extension = ""

    paths = re.split(os.pathsep, os.environ["PATH"])
    for path in paths:
        javaPath = path + os.sep + "javac" + extension
        if (os.access(javaPath, os.F_OK) == 1):
            if (os.access(javaPath, os.X_OK) == 0):
                print_error("cannot execute javac compiler, check file permissions", 0)
                return 0
            # find the path with the "BaseScript.java" file
            if (script_util.find_root_script_dir(scriptPath) != 1):
                return 0
#            JavaCOptions = "-classpath " + os.curdir + os.pathsep + script_util.RootScriptDir
            JavaCOptions = (javaPath, \
                            "-sourcepath", os.path.join(script_util.RootScriptDir, ".."), \
                            "-classpath", os.path.join(script_util.RootClassDir, ".."), \
                            "-d", os.path.join(script_util.RootClassDir, ".."), \
                            "-deprecation")
            print "JavaCOptions = ", JavaCOptions
            return 1
        # end if
    # end for path
    print_error("cannot find javac compiler, check your PATH environment variable", 0)
    return 0
#end setupJava()

#def foo(args):
#    if (setupJava(args) == 1):
#        parse(args)
#end foo()

if (setupJava(sys.argv) == 1):
    parse(sys.argv)

