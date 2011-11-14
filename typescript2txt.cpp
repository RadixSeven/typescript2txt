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
 *
 * Gutted and rewritten in C++ by Eric Moyer in 2011
 *
 * Permission is granted to distribute this software under any version
 * of the BSD and GPL licenses.
 *******************************************************************/

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <cctype>

/// Reads typescript output for a linuxterm (and maybe xterm?) and
/// recreates what would be on a very long screen (long enough to hold
/// everything in the file), ignoring color and other formatting
/// characters
class Reader{
  /// The lines that will be output.
  std::vector<std::vector<char> > lines;
  /// The index of the cur
  std::size_t line_idx;
  /// The index of the cursor on the current line, where the next
  /// character will be written.  May be (and very frequently is) one past
  /// the end of the line.
  std::size_t char_idx;
  
  /// Enum to specify the different states the reader can be in
  enum RState{
    SAW_NOTHING,
    SAW_ESC, ///ESC ( ^] ) was seen
    SAW_CSI, ///Control sequence introducer - ESC [ or 0x9B
    SAW_OSC ///Operating system command ESC ]
  };

  /// The current state of the reader (in escape code etc.)
  RState state;
  
  /// Return the current line
  std::vector<char>& cur_line(){ return lines.at(line_idx); }

  /// Perform a line-feed, adding blank lines if necessary
  void line_feed(){ 
    ++line_idx;
    while(line_idx >= lines.size()) {
      lines.push_back("");
    }
    if(char_idx > cur_line().size()){
      char_idx = cur_line().size();
    }
  }


public:
  /// Create an empty reader that has read nothing
  Reader():line_idx(0),char_idx(0){
    lines.push_back(std::vector<char>());
  }

  /// \brief Read from the given typescript output stream using the reader's
  /// \brief current state
  void read_from(std::istream& in);
};

void Reader::read_from(std::istream& in){
}

char* buffer=NULL;
int32_t  buffer_size=0;
int32_t  len;

void addchar(int c) {
  if(len >= buffer_size) {
    if(buffer_size==0) {
      buffer_size=128;
    } else {
      if (buffer_size <= (1<<30)){
	buffer_size*=2;
      }else{
	fprintf(stderr,
		"\nLINE TOO LONG FOR MAXIMUM BUFFER SIZE (len: %d)!!!\n", len);
	exit(2);
      }
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

/// caret converts a charater 'M' into a character '^M'
int caret(int c) { return (c-'A'+1); }

/// dcaret converts a charater '^M' into a character 'M'
int decaret(int c) { return (c+'A'-1); }

enum { RESTART_ESC = 256,
       INTER_ESC = 257,
       EOF_TGCHAR = 258,
};

/// tgetchar like getchar but does "terminal" processing returning the first non-control character
///
/// If the return value is greater than 255 then the last character
/// read requires a state change.  
///
/// RESTART_ESC: must restart any escape sequence in progress
///
/// INTER_ESC: must interupt the escape sequence in progress
///
/// EOF_TGCHAR: ran into eof
int tgetchar(){
  int c;
  while(1){
    c =  getchar();
    if(c == EOF){
      return EOF_TGCHAR;
    }
    if(c < 32){
      switch(decaret(c)){
      case 'H' : len--; if(len < 0){ len = 0; }; break;
      case 'K' : case 'L' : //Same as ^J so no break
      case 'J' : addchar(0); puts(buffer); len=0; break;
      case 'N' : case 'O' : case 'G' : break; //Ignore these
      case '[' : return RESTART_ESC; break;
      case 'X' : return INTER_ESC; break;
      case 'Z' : return INTER_ESC; break;
      default  : addchar(c);
      }
    }else{
      if(c == 0x9B){ //Error on CSI character
	fprintf(stderr, "File contains 0x9B control sequence start.  "
		"Unsupported in this version of typescript2txt.  \n"
		"Replace with ESC [.");
	exit(-1);
      }else if(c != 0x7F){ //Ignore DEL char, return all others
	return c;
      }
    }
  }
}

void handleESC(char c) {
  int first_char; //First character read
  int lc;         //Last character read
  int params[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Numeric parameters
  int iparam=0;   //Maximum valid index in params array
  do{
    do{
      first_char = lc = tgetchar();
    }while(lc == RESTART_ESC);

    params[0] = iparam = 0;
    if (c=='[') {
      while(isdigit(lc)) { 
	params[iparam]=params[iparam]*10+(lc-'0'); 
	lc=tgetchar();
	if(lc == ';'){
	  ++iparam;
	  params[iparam] = 0;
	  lc=tgetchar();
	} 
      }

      if(lc <= 255){
	if (lc=='P'){
	  len-=params[0];
	  if(len<0) {
	    len=0;
	  }
	}
	if (lc=='@'){
	  addchar(' '); //should add n instances of ' '
	}
      }else{
	switch(lc){
	case INTER_ESC: return; break;
	case EOF_TGCHAR: return; break;
	case RESTART_ESC: break;
	default: 
	  fprintf(stderr, "Unexpected return value from tgetchar: %d\n", lc);
	  exit(3);
	}
      }
    } else if (c==']') { // Just strip out all "set icon" and "set title" commands
      while(getchar()>caret('G'));
    }
    //Not sure what this is doing
    if(first_char == 'A'){
      len = 2;
      return;
    }
  }while(lc == RESTART_ESC);
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
      case 'H' : len--; if(len < 0){ len = 0; }; break;
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
  free(buffer);
  return 0;
}
