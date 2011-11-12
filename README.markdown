#typescript2txt

typescript2txt is a program to strip out (some of) the extra control
characters from the output of the unix <code>script</code> command.

It attempts to address the bug mentioned in the unix man page:

     Script places everything in the log file, including linefeeds and
     backspaces.  This is not what the naive user expects.

And generate output closer to what the naive user would expect.

#Usage

typescript2txt < output_of_script_cmd > output_as_plain_text

#History

This started life as code written by John C. McCabe-Dansted
(gmatht@gmail.com) in 2008 and posted at
https://launchpadlibrarian.net/14571190/script2txt.c as part of a (bug
report on launchpad)
[https://bugs.launchpad.net/ubuntu/+source/util-linux/+bug/231340].
It had some memory access problems (that kept RadixSeven from using it
as-is) so he decided to fix it.  In order to share it with the world,
he made a github repo for it.
