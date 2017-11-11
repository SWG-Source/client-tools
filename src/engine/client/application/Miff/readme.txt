Bootprint Entertainment - GT Interactive Austin

Project: mIFF (make IFF)
Author(s): Hideki Ikeda
Last Updated: 01-07-1999 AM


Table of Contents:
==================
* Instructions to compile under MSDev
* Known Bugs
* History and Comments
* mIFF grammar && syntax documentation
* mIFF console command line arguments


Instructions to compile under MSDev:
====================================

1. must have copy of lex (flex.exe) & yacc (bision.exe) in your search
   path of your Tools | Options | Directories | Executables.
2. in DOS level, you MUST have GNU C-Compatible Compiler Preprocessor
   (CCCP.exe aka CPP.exe) in your search directory for debug testing as
   well as for usage in normal level.


Known Bugs:
===========
* 12.15.98 - when using a "\" inside a double-quotes, use "\\" i.e.
  "C:\\Projects" or else the parser thinks it's a special character.
* 12.15.98 - Error message does not report the correct line number and
  columns
* 12.15.98 - when you enter a FORM or CHUNK name that is longer then 4
  characters, the error message does not report the correct line
  number.  And because of the way regular expression is set up, it will
  report other errors (or if none, it will say successfully compiled)
  after it.
* 12.15.98 - I have no clue as to how a float value and double values
  should be stored, and thus, it is stored in %04x format via
  fwrite(&val...) method...


History and Comments:
=====================
v 1.0
* 12.12.98 -  basic lexical analyzer and parser hooked up to Windows
  Editor.
* 12.12.98 - first and foremost, the MFC usage of RichEdit was inspired
  by an article written by Jason Shankel for Jan. 1999 issue of
  Dr.Dobb's Journal.  Without  this tidbit, I would have never been able
  to pass the text buffer to the  parser, and so we would have ended up
  with yet another command line version  of mIFF...
* 12.12.98 - Currently, compile output from lex and yacc are .C file,
  although flex has a -+ option to spit out in .CPP format, it is just
  too much pain in the ass...  so it's all extern'd...  MIFFCompile()
  function is the only communication between  the editor and the
  parser...  The rest is all in C in lex and yacc files...
* 12.12.98 - mIFF is currently functional under MFC to allow usage of
  their RichEdit class.  this makes it easy to cut and paste, copy, etc
  from buffer to buffer  as well as edit with inside the compiler...  no
  more DOS batch...  bummer?  well, maybe  the future version can do
  batch-commands...
* 12.12.98 - Because of all this mish-mash of pre-compiled header, this
  project does NOT use it...  I'm sure we can turn on all but the
  lex/yacc source codes with stdafx and  the rest to be non-PCH
  dependant...  I don't know, I didn't try, and I don't want to try...
  If Jeff asks me to, I will ^_^;
* 12.12.98 - the accompanied .REG file will automagically register .MIF
  file to launch this application.  You can use this .REG file in
  install program...
* 12.12.98 - Bison just sucks the big one when I can't tell it to use
  other filename then bison.simple.  What I hate the most is I cannot
  redirect bison to search for bison.simple in other directories, so it
  has to stay where the .DSW/.DSP files sits...  in another words, it
  has to be at the root of the project workspace in order for the dumb
  compiler (bison) to work...
* 12.12.98 - At this moment, I don't wish to do comments in any other
  form then //comment format.  The /* comment */ requires much more
  complex regular expression.
  For //comment, the regular expression is simple as: //[^\n]*\n which I
  think is nice and sweet ^_^
* 12.12.98 - fopen() is used to create output files (defined by
  #pragma's) and therefore, if the directory specified in #pragma's
  does not exist, it will produce an error.  You may think this is not
  ellagant for it should pop up a file-folder dialog, but it is the
  most appropriate way since mIFF is just trying to open a file defined
  EXACTLY what #pragma states.
* 12.12.98 - all mIFF source codes MUST have "#pragma drive", "#pragma
  directory", "#pragma filename" and "#pragma extension" somewhere at
  the top of the mIFF source in order to write file output.  Any of
  these missing will cause an error.
* 12.14.98 - currently, you can have FORM's inside a FORM, which can
  have CHUNKs and with inside the CHUNK, you can have FORM's.  In
  another words, there is NO  restrictions as to how deep a FORM and
  CHUNK can go, and there are NO rules that says you can't have FORM's
  inside a CHUNK.
* 12.16.98 - fixed the RTF bug.  mIFF now only reads in TEXT file, and
  does not force or reformat to stupid RTF mode.  The reason why it had
  to originally be in RTF mode was due to the fact that I had no clue
  how to serialize a stream I/O without the
  GetRichEditCtrl().StreamOut() functions streaming the text buffer into
  the parser/compiler...  I now force the m_bRTF to FALSE in mIFFDoc
  file so the serializations will assume the format to be SF_TEXT
  always!
* 12.22.98 - C preprocessor will expand the #define macros and #include
  source text files.
* 12.22.98 - there is a difference between
    #include "filename.src"
  versus
    include "filename.bin"
  the first has a #include (# pound sign) which the C preprocessor will
  automagically include the source text file.
* 12.22.98 - While linking, make sure to tell the linker to ignore
  libcmtd.lib (debug) and libcmt.lib (release) lib files for the
  external functions found in these library is already included in
  engine.lib (oh, and if you're compiling as release mode, make sure to
  compile the engine under release mode first or else it wont link *duh*

v 1.0 DOS mode 01.06.99
* 01.06.99 - implemented DOS version of mIFF and called it mIFFDOS.exe.
  The inspiration to all this (the console version) was because mIFF
  (the MFC version) had a crappy time passing the args from DOS prompt
  correctly.  It was necessary to pass args to mIFF to allow batch file
  processing for multiple datas in loads of levels.  The args are found
  when you do mIFFDOS -h, but for the sake of completion to a
  documentation, it is also provided at the end of this document.
* 01.06.99 -  This version has capability of using GNU C Preprocessor
  via spawning a process to preprocess the miff source and then pass it
  to the miff parser.  The preprocessor will expand the #define macros
  and #include source files into one big source for mIFF's parser to
  evaluate.
* 01.06.99 - there are 3 types of include's in mIFF source code.
  See/read the mIFF grammar section for details on the differences.
* 01.07.99 - woo hoo!  as of today, I've removed all reference (except
  for the .exe file) of the MFC version of mIFF.  The crummy problem of
  COMSPEC/SHELL under 4DOS made it crappy result for CCCP to
  CreateProcess(), and therefore, I removed it...  I must've been on
  crack when I made that decision back in December to do mIFF in MFC...
  Well, at that time, I thought it was cool to have the compiler in the
  editor environment, but hell...  I should have listened to Jeff when
  his response was "Hmmm, interesting..." which probably meant "Hmmm,
  this is going to be interesting how you're gonna recover from bowing
  down to Microsoft's evil library called MFC!"  I should have saw it
  coming...  Only thing that bugs the shit out of me right now is that
  it's using system() which will still look at the COMSPEC to find out
  which COMMAND.COM (or 4DOS.COM) it's using, and in NT, Jess claims
  CMD.COM is more reliable then COMMAND.COM...  Well, I'll let system()
  which looks for the envr[] determine this...  At least under console
  version, I'm not having that problem of DIR changing on me when
  process is spawned via system() to call CCCP...
* 01.07.99 - next goal is to make mIFF use the engine's library so I can
  get rid of my write() function to create IFF files.  I want to make
  sure it's 100% compatible to the engine, and therefore, I believe it's
  essential.  Another goal is to call CreateProcess() rather then
  system() as Jeff suggests.  This will be a bit more challenging to
  do...
* 01.08.99 - spent few hours snooping thru CreateProcess(), made a
  project based on console mode and another based on MFC mode.
  CreateProcess() will exit to the parent process in console mode
  because it's already calling from the console mode.  But under MFC
  mode, CreateProcess() will open a console window, execute CCCP, and
  leave the console open and hanging there.  I've tried many
  combinations for the flag switches of CreateProcess() that I thought
  were logically the correct flag choices...  I've also attempted to
  close the process manually by calling TerminateProcess().
  TerminateProcess() DID kill and close the console window for me, but
  it killed it before the process has been completed (sometimes
  earlier). Basically, it was because the console was running async
  to the MFC thread.  So... I've attempted to call
  WaitForSingleObject(process) so CCCP's console window can finish what
  it's doing first before I called TerminateProcess(process) to close
  the console window down.  Well, guess what?  After CCCP finished
  compiling, the console window just said "Finished" in the damn title
  bar...  WaitForSingleObject() is waiting for completion of the console
  window, which apparently, will not get out of this polling until I
  click on the [x] "close the damn window" button.  It then, gets out of
  the WaitForSingleObject() which by then, TerminateProcess() is useless
  since I've already manually closed the f'king console window... ugh!
  I've learned a valuable lesson within the past 2 days...  never
  attempt to mix or call console applications from Win32 mode, it just
  won't work damn it!  So, to make the long story short, mIFF is not
  going to be in Win32/MFC mode.  The project no longer exists in VSS
  anyways, but I wanted to make the one last attempt to be sure.
* 01.08.99 - now that I've got the damn CreateProcess() out of the way,
  I can concentrate on applying Jeff's engine library IFF class into
  mIFF so in the future, _IF_ the byte order of the data changes, etc it
  will be just a click of <F7> button to re-compile and voila...  I'll
  start on this...  Last time I tried to implement engine .lib into
  mIFF (that was during the MFC version of mIFF) I had problems with
  operator overloaded new().  So let's see what happens...
* 01.11.99 - Added the feature of 16bit string via wstring <str_literal>
  syntax.  I have used mbstowcs() [MultiByteString to WideCharString]
  conversion routine so if in the future, the byte order changed from
  little end-in to big end-in in the stdio.h, it will be just a compile
  button away...
* 01.11.99 - added -q and -c option.  -q is to prevent useless string to
  be displayed unless we are debugging.  In batch command mode, we want
  to keep it quiet as possible except for errors.  -c option is because
  I like the name CCCP rather then CPP.  If you enter -c, it will search
  for CCCP.exe rather then CPP.exe.  The default is CPP.exe, and
  therefore, unless you've renamed your CPP.EXE to CCCP.EXE, you don't
  have to use this switch.



==============================================================================
mIFF grammar && syntax:
~~~~~~~~~~~~~~~~~~~~~~

* The following are reserved words for mIFF's lexical analyzer:
      form FORM
      chunk CHUNK
      int32 uint32
      int16 uint16
      int8 uint8
      float double
      string wstring
      sin cos tan asin acos atan
      #define
      #include
      include
      includeiff includeIFF
      #pragma drive directory filename extension
      //

* String literals are ALWAYS enclosed with a "double quotes".

* Numbers are integer, floating points, hexadecimals, and even
  mathmatical expressions (i.e. addition, subtraction, etc).

* ALL comments are referred to in C++ style comment style //.  Any
  string that follows // until the end of line is ignored by the parser.

* For issues on specifications, see the separate document SPECS.TXT
  which contains threads of e-mail discussion.  This document also
  contains sample mIFF source code.

===== form FORM =====
form <string_literal> { <form_body> }
FORM <string_literal> { <form_body> }

 example:
    form "foo"
    {
      form "foo2"
      {
        chunk "bar"
        {
          int32 0   // ID
          float 0   // x
          float 0   // y
          float 0   // z
        }
      }
      chunk "bar2"
      {
        include "binary.data"
      }
    }

 Both form and FORM are the same, but since mIFF was compiled in case
 sensitive mode (because we pass it thru GNU C's preprocessor, we want
 to make sure #include is used instead of #INCLUDE or some other dumb
 combination - and therefore, we made sure it's case sensitive by
 general rule).
 By definition, a form can be empty, and the maximum string literal
 length is 4 character.  mIFF will default to error message if longer
 then 4 character.  If the string literal is less then 4 character, it
 will pack it with spaces.  For instance form "foo" will be actually
 treated as "foo ".  Finally, it is illegal to have:
   form "form"
   {
   }
 Form name cannot be "form" (or "FORM") for algorithm used for seeking
 IFF's form is dependant to search for this key word, and it would
 definitely get confused...  mIFF will check for grammar error of this
 and flag an error.
 It is legal to have multiple depth of FORM inside a FORM.  But it is
 illegal to have a FORM inside a CHUNK.

===== chunk CHUNK =====
chunk <string_literal> { <chunk_body> }
CHUNK <string_literal> { <chunk_body> }

  example: (see form example)

  Just like form, chunk's string literal is 4 characters maximum, if
  greater 4, it will appropriately inform the user of this.  You cannot
  use the string literal "FORM" for chunk name.  This will confuse the
  IFF seeking routine for entering forms and such (see FORM).
  As the rule is defined, you cannot have FORM inside a CHUNK.

===== memory allocations by types =====
int32 <integer value>
uint32 <integer value>
int16 <integer value>
uint16 <integer value>
int8 <integer value>
uint8 <integer value>
float <floating point>
double <floating point>
string <string literal>
wstring <string literal>

  example:
    chunk "data"
    {
      uint32 0
      float  3.1592965
      string "this is a string" 
      int32  (5 * 6 - 2) 
      float  sin(45)
      wstring "this is wide 16bit string"
    }

  Types you define before the literals define the memory allocation
  size.  mIFF allows mathmatical expressions for data space allocation,
  but however if you request a memory allocation to be in uint32 and
  your calculation results in negative, mIFF will take the unsigned
  value of that and store it as unsigned. Integer values can be in
  decimal or hex. Floating point does not require a decimal to represent
  its a float. String literals MUST be defined with a double quotes, and
  special characters must use a '\' to indicate it.  For example
  "C:\\MyDir" will be parsed as: C:\MyDir, or "This is inside a \"double
  quotes\"" will be parsed as: This is inside a "double quotes".
  Keyword wstring is a wide version (16 bit) of string.  It is currently
  in the form of little-end-in (0 is packed on the following byte) and
  is terminated in 16bit NULL.

===== mathmatical expressions =====
<expression> + <expression>
<expression> - <expression>
<expression> * <expression>
<expression> / <expression>
<expression> ^ <expression>
<expression> n
( <expression> )
sin( <expression> )
cos( <expression> )
tan( <expression> )
acos( <expression> )
asin( <expression> )
atan( <expression> )

  example: (see memory allocation by types)

  An <expression> at the smallest entity is a numerical literal (in
  decimal, float, or hexadecimal).  An <expression> can consist of
  recursion of expressions within expression.
  i.e. (5 * (3 + cos(45 + 2)) / 180)
  The operators of the expressions are all familiar and self explanatory
  except for maybe ^ and n.
  <expression> ^ <expression> is a form of power.  i.e. 8^2 == 64
  <expression> n is a negation operator.  i.e. 64n == -64.  The 'n'
  operator was used instead of a '-' sign to reduce the syntax
  ambiguity. i.e. ( 5 - -2) requires more complex lexical analisys, and
  therefore, it should be written as (5 - 2n).

===== preprocessors: #define =====
#define <string_literal> <expression>

  example:
    #define coordinate(x,y,z) float x    float y    float z

  The #define syntax is stated here for the purpose of completion to
  this documentation.  mIFF will accept the syntax, but will do nothing
  for the grammar.  mIFF instead, will pass the original source to GNU's
  CCCP (C-Compatible Compiler Preprocessor) and have it expand the
  macros into what is defined.  In case the source didn't go through
  CCCP first, mIFF may accept some of the syntax, but will probably
  complain. If you decide you want to pass mIFF a pre-processed source
  file, or you wish to debug your mIFF source perhaps because you want
  to see the expanded source with all the #include and #define's
  expanded, you can do so with what mIFF uses as a parameter to call
  CCCP:
    cccp.exe -P -pedantic -nostdinc -nostdinc++ -H -lang-c++ <in> <out>
  where <in> is your mIFF source code, and <out> is the output file with
  all the #define and #include's expanded.  For more details on how to
  use CCCP, see the manual provided by GNU.

===== preprocessors: #pragma =====
#pragma drive <string_literal>
#pragma directory <string_literal>
#pragma filename <string_literal>
#pragma extension <string_literal>

  example:
    #pragma drive "C:"
    #pragma directory "\\Projects\\Game\\dSrc"
    #pragma filename "myData"
    #pragma extension "iff"

  #pragma's are only used if -i option is provided in the command line
  argument when calling mIFF.  In MFC version by default, #pragma's are
  enabled.  When #pragma's are enabled, ALL the #pragma's MUST be there,
  meaning you cannot just define drive but no directory, etc.  It's use
  all or none.  In the command line version, if -i is provided but the
  #pragma's are not defined in the mIFF source code, you will get a
  syntax error.
  The string literals all must use a double-slash \\ to represent a
  single-slash \.
  For more details on #pragmas, see also the -o, -d, -p, -f, and -e
  options in command line arguments section of the console version of
  mIFF.

===== #include, include, includeIFF =====
#include <string_literal>
include <string_literal>
includeIFF <string_literal>
includeiff <string_literal>

  example:
    #include "source.txt"
    form "foo1"
    {
      chunk "bar1"
      {
        include "binary.raw"
      }
    }

    form "foo2"
    {
      includeIFF "myOtherBinFile.IFF"
    }


  mIFF will accept includeIFF or includeiff for syntactical checking.
  mIFF is after all, case sensitive compiler...

  1.  #include "source.txt" - this is strictly for preprocessor, and
  mIFF will ingore it IF it finds it in the source. Therefore, make sure
  to run it thru the GNU C preprocessor. Currently, main.cpp will pass
  the input miff source file to CCCP first before compiling, and
  therefore, it's automagic.  For more details on preprocessor issue,
  see #define.

  2. include "binary.raw" - note the include has no # at th front. This
  syntax is only found in CHUNK level.  It is strictly used to include
  raw binary file inside a chunk.  If you try to use include "blah.iff"
  (an IFF file) inside a chunk, which is perfectly legal, but you're
  pretty much screwed when you're trying to traverse thru the chunk to
  see the raw IFF you tried to include will be consided a DATA, not an
  actual IFF tree... In another words, YOU CANNOT HAVE FORM INSIDE A
  CHUNK - and the grammar check does give you error.

  3. includeiff "rawform.iff" - from the observation of wishing to
  include a raw binary IFF inside a mIFF source, here it is. The grammar
  checker will make sure you can only use this in the FORM level and NOT
  in the CHUNK level (see #2 - include)


==============================================================================
mIFF Console version command line args
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usage: mIFF [-i] [-o FULLoutputFileName] [-d drive letter] [-p pathDir]
            [-f filename] [-e extension] inputSourceFile

Return Value: IF no errors were found, mIFF will return 0 to the caller
  application and/or batch file.

Parameters:
    -i          [optional] install #pragma found in the IFF source file
                (inputFilename) if -i is not specified, even if the
                source contains #pragma's, it will be ignored.
    -o fname    [optional] FULL pathname with extension, dir, and the
                filename to be used to spit out the result IFF file that
                mIFF compiles.
    -d drive    [optional] drive letter of the destination target file.
                IF -o is specified, this option is ignored! MUST
                provide -p with this option because it would be stupid
                if you're in C:\myDir and you request -d D but D:\myDir
                doesn't exist...
    -p path     [optional] path of the destination target file.  IF -o
                is specified, this option is ignored!
    -f          [optional] destination target filename.  IF -o is
                specified, this option is ignored.
    -e          [optional] destination target extension.  IF -o is
                specified, this option is ignored.
    inFile      [REQUIRED] input source file that is required to
                generate IFF file.
    -c          [optional] Use CCCP.EXE rather then CPP.EXE [default].
                If for some reason you have renamed CPP.EXE to CCCP.EXE
                in your search path, then use this switch to search run
                CCCP rather then CPP.
    -q          [optional] Quiet mode. If you do not enter -q switch,
                all me messages and informations are displayed when
                possible. In a batch file when you pipe output for
                errors, it might be annoying to have them useless
                informations, and thus you can turn this switch on. -q
                does not suppress error messages.
    -h          [very optional] this help screen.

Example:
    mIFF foo.bar
                this will generate an iff file foo.iff (default if no
                parm specified) in the current working directory.
                Even if foo.bar contains #pragma, it will create
                foo.iff because -i was not specified.

    mIFF -o "C:\my project\myData\foo.iff" foo.bar
                notice that if you have space in your dirname, use "
                to encapsilate it.

    mIFF -e foo foo.bar
                will generate output file foo.yfoo in the current
                working directory.
