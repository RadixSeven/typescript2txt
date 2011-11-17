#typescript2txt

typescript2txt is a program to strip out (some of) the extra control
characters from the output of the unix <code>script</code> command.

It attempts to address the bug mentioned in the unix man page:

     Script places everything in the log file, including linefeeds and
     backspaces.  This is not what the naive user expects.

And generate output closer to what the naive user would expect.

#Usage

typescript2txt < output_of_script_cmd > output_as_plain_text

#Compilation

The code is set up to compile under linux using gcc and gmake.

    make

Will compile everything and produce a typescript2txt executable.  No
install target has been created.  You'll need to copy the executable
to an appropriate directory.

#History

This program was inspired by code written by John C. McCabe-Dansted
(gmatht@gmail.com) in 2008 and posted at
https://launchpadlibrarian.net/14571190/script2txt.c as part of a [bug
report on
launchpad](https://bugs.launchpad.net/ubuntu/+source/util-linux/+bug/231340).
It had some memory access problems (that kept RadixSeven from using it
as-is) so he decided to fix it.  In order to share it with the world,
he made a github repo for it.  

After fixing the memory access and trying to fix other things
RadixSeven realized the process of making it usable would be much
faster and produce better code if he just rewrote everything from
scratch.  This resulted in the code you see here.

#Testing Philosophy

You can run the tests assuming you have diff installed.

The tests are **very** minimal.  It takes a lot of time to write good
tests.  The initial author did not want to invest that time.  There is
usually only one test for a code.  So most of the complexity of the
implementation is not tested.

Further, only tests for codes actually implemented are included.  This
means that any code that is supposed to generate a warning or a code
that is ignored is likely to be absent from the test suite.  

Anyone who would like to is welcome to make things more robust by
fleshing out the test cases to really check the functionality.  Other
users should be aware that there are likely to be major bugs that
didn't interfere with the purposes of the earlier authors.

The test cases were created by:

1. Creating a file containing the desired input named *_input.txt

2. Dumping that file to a terminal window (either Gnome's terminal or
Konsole depending on which computer the developer was using at the
time)

3. Copying the new contents of the terminal window to the
expected_output.txt file

#Source for console codes

The files I used to crib the console codes from are in the reference
directory. Their original locations are
http://www.kernel.org/doc/man-pages/online/pages/man4/console_codes.4.html
and http://www.kitebird.com/csh-tcsh-book/ctlseqs.pdf
