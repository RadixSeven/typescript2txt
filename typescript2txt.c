/********************************************************************
 * "man script" reports the following bug
 *
 *      Script places everything in the log file, including linefeeds and backspaces.  This is not what the naive user expects.
 *
 * This program converts a script file back into a normal text file
 *
 * USAGE: typescript2txt < script_output > script.txt
 *
 * Although this does not handle all possible xterm output, it appears
 * to work fairly well for normal output from bash etc. 
 * It also works well for linuxterms.
 *
 * http://www.kitebird.com/csh-tcsh-book/ctlseqs.pdf
 * documents other xterm commands that could be handled.
 * The most important xterm commands missing are line up and line down.
 * These can occur in bash if you have a very long commandline
 * in a too small window. Supporting these xterm commands would
 * would require modifying this program to buffer previous lines.
 * (It currently writes out the line every time a newline is recieved).
 *
 * It would be easy to support colours if we outputted to e.g. HTML.
 *
 * This program might be better implemented as a python script.
 *
 * John C. McCabe-Dansted (gmatht@gmail.com) 2008
 * Permission is granted to distribute this software under any version
 * of the BSD and GPL licenses.
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

char* buffer=NULL;
int32_t  buffer_size=0;
int32_t  len;

void addchar(int c) {
	if(len >= buffer_size) {
		if(buffer_size==0) {
			buffer_size=128;
		} else {
			buffer_size*=2;
		}
		buffer=(char*)realloc((void *)buffer, buffer_size);
		if (buffer==NULL) {
			fprintf(stderr,"\nLINE TOO LONG FOR MEMORY!!!\n");
			exit(1);
		}
	}
	buffer[len]=c;
	len++;
}

/* caret converts a charater M into a character '^M' */
int caret(int c) { return (c-'A'+1); }
int decaret(int c) { return (c+'A'-1); }

void handleESC(char c) {
	int s[16];
	int i=0;
	int n=0;
	s[i]=getchar();
	if (isdigit(s[i])) { n=n*10+(s[i]-'0'); i++;  s[i]=getchar();}
	if (isdigit(s[i])) { n=n*10+(s[i]-'0'); i++;  s[i]=getchar();}
	if (isdigit(s[i])) { n=n*10+(s[i]-'0'); i++;  s[i]=getchar();}
	if (isdigit(s[i])) { n=n*10+(s[i]-'0'); i++;  s[i]=getchar();}
	if (c=='[') {
		if (s[0]=='0' && s[1]=='1' && s[2]==';') {
			//  ^[[01;34m == <colour='cyan'>
			//  ^[[01;31m == <colour='red'>
			//  ^[[00m == </colour>
		 i++;  s[i]=getchar();
		 i++;  s[i]=getchar();
		 i++;  s[i]=getchar();
		}
		if (s[i]=='P'){
			len-=n;
			if(len<0) {
				len=0;
			}
		}
		if (s[i]=='@'){
			addchar(' '); //should add n instances of ' '
		}
	} else if (c==']') { // Just strip out all "set icon" and "set title" commands
		while(getchar()>caret('G'));
	}
	if (s[0]=='A') {
		len=2;
	}
}
			
int main () {
	int c,d;
	int lastM=0; // last character was an '^M'
	int lastESC=0; // last character was an '^[' 
	len=0;
 	while ((c=getchar())>=0) {
		d=decaret(c);
		if (lastM && d!='J') {
			len=0;
		}
		lastM=0;
		if (c<32) {
			lastESC=0;
			switch(d) {
				case 'H' : len--; break;
			        case 'J' : addchar(0); puts(buffer); len=0; break;
				case 'M' : lastM=1; break;
				case 'G' : break;
				case '[' : lastESC=1; break;
				default  : addchar('^'); addchar(d); addchar(c);
			}
		} else {
			if (lastESC && (c=='[' || c==']')) {
				handleESC(c);
			} else {
				addchar(c);
			}
		}
	}
	return 0;
}
