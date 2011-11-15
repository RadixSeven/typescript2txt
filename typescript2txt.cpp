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
 *
 * The rewrite was accomplished mainly by referring to
 * http://unixhelp.ed.ac.uk/CGI/man-cgi?console_codes+4 which can be
 * seen in many linux distros as man console_codes
 *
 * It would be easy to support colours if we outputted to e.g. HTML.
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
#include <cassert>

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

  /// The parameters that are used for the CSI sequences - also used
  /// by some of the OSC commands
  std::vector<unsigned> params;
  
  /// Enum to specify the different states the reader can be in
  enum RState{
    SAW_NOTHING,
    SAW_ESC, ///ESC ( ^] ) was seen (but no trailing ] or [ )
    SAW_CSI, ///Control sequence introducer - ESC [ or 0x9B
    SAW_OSC, ///Operating system command ESC ]
    SAW_OSC_P, ///ESC ] P (will read 7 hex digits)
    SAW_ESC_NUM, ///   ESC #
    SAW_ESC_PCT, ///   ESC %
    SAW_ESC_LPAREN,/// ESC (
    SAW_ESC_RPAREN, /// ESC )
    SAW_CSI_LBRACKET /// ESC [ [ or CSI [ (just eats next char)
  };

  /// The current state of the reader (in escape code etc.)
  RState state;
  
  /// Return the current line
  std::vector<char>& cur_line(){ return lines.at(line_idx); }

  /// Perform a line-feed, adding blank lines and spaces if necessary
  void line_feed(){ 
    ++line_idx;
    while(line_idx >= lines.size()) {
       lines.push_back(std::vector<char>());
    }
    while(char_idx > cur_line().size()){
      cur_line().push_back(' ');
    }
  }

  /// Perform a reverse line-feed - go up one line
  void reverse_line_feed(){
    if(line_idx > 0){
      --line_idx;
    }else{
      assert(line_idx == 0); //line_idx should never be negative
      lines.insert(lines.begin(),std::vector<char>());
    }
    while(char_idx > cur_line().size()){
      cur_line().push_back(' ');
    }
  }

  /// Perform a tab: Insert enough spaces at the current position to move to a multiple of 8
  void tab(){
    while(char_idx % 8 != 0){
      put_char(' ');
    }
  }

  /// Perform a carriage return - put the current character index at the beginning of the line
  void carriage_return(){
    char_idx = 0;
  }

  /// Perform a back-space - delete previous char but not past beginning of line
  void back_space(){
    if(char_idx > 0){
      unsigned idx_to_delete = char_idx - 1;
      cur_line().erase(cur_line().begin() + idx_to_delete);
    }
  }

  /// Execute character set change to \a num - just prints warning right now
  ///
  /// \param num the number of the character set to change to
  void character_set(int /*num*/){
    std::cerr << "Warning: typescript file contains character-set-changing "
	      << "commands that are ignored by this translator.\n";
  }

  /// Insert the given character at the current position, moving to the next one
  ///
  /// \param c There character to insert
  void insert_char(char c){
    if(char_idx == cur_line().size()){
      cur_line().push_back(c);
      ++char_idx;
    }else if(char_idx < cur_line().size()){
      cur_line().insert(char_idx + cur_line().begin(), c);
      ++char_idx;
    }else{
      std::cerr << "ERROR: char_idx more than one space beyond end "
		<< "of line in insert\n";
      std::exit(-1);
    }
  }

  /// Set the current character to \a c and advance to the next
  ///
  /// Sets the character a the current position to \a c then advances
  /// the current character to the next position.  If the current
  /// character is beyond the end of the line, inserts \a c at the end
  /// of the line
  ///
  /// \param c The new value of the character at the current position
  void put_char(char c){
    if(char_idx == cur_line().size()){
      cur_line().push_back(c);
      ++char_idx;
    }else if(char_idx < cur_line().size()){
      cur_line().at(char_idx) = c;
      ++char_idx;
    }else{
      std::cerr << "ERROR: char_idx more than one space beyond end of "
		<< "line in put\n";
      std::exit(-1);
    }
  }

  /// Set the state to new_state and clear parameter array
  ///
  /// Sets the state of the reader to new_state and (since the
  /// parameter array is in one sense part of the command reading
  /// state) clear that array.
  ///
  /// \param new_state the new state of the reader after set_state
  void set_state(RState new_state){
    state = new_state;
    params.clear();
  }

  /// Set a horizontal tab stop at the current cursor position
  /// 
  /// Right now, does nothing but print a warning
  void set_htab_stop(){
    std::cerr << "Warning: typescript file contains tab-stop-changing "
	      << "commands that are ignored by this translator.\n";
  }

  /// Print a warning about \a c whose meaning is unknown in the given context
  ///
  /// \param context a string describing the context in which \c is unknown
  ///
  /// \param c the character whose meaning is unknown in the given context
  void unknown_code(std::string context, unsigned char c){
    using std::cerr;
    cerr << "Warning: the meaning of the character '";
    if(!isprint(c)){
      if(c < 0x20){
	cerr << '^'  << (c+'@');
      }else{
	if(c == 0x7F){
	  cerr << "DEL";
	}else{
	  cerr << "Unknown char";
	}
      }
      cerr << "' (" << ((unsigned int)c) << " decimal) ";
    }else{
      cerr << c << "' ";
    }
    cerr << "is unknown in the context of a " << context << ".\n";
  }

  /// Return true if the given char is a control character (and process it)
  ///
  /// If \a c is a control character, changes the reader's state to
  /// reflect its having been received and returns true.  Otherwise,
  /// does nothing and returns false.
  ///
  /// \param c the character to identify and process
  ///
  /// \return true if c is a control character, false otherwise
  bool id_and_process_control_char(unsigned char c){
    if(c >= 0x20 && c != 0x7F && c != 0x9B){ return false; }
    c = c+'@'; //Make c into a printable character for easier coding
    switch(c){
    case 'G': return true;
    case 'H': back_space(); return true;
    case 'I': tab(); return true;
    case 'J': case 'K': case 'L':carriage_return(); line_feed(); return true;
    case 'M': carriage_return(); return true;
    case 'N': character_set(1); return true;
    case 'O': character_set(0); return true;
    case 'X': case 'Z': set_state(SAW_NOTHING); return true;
    case '[': set_state(SAW_ESC); return true;
    case (0x7F+'@'): /*DEL=0x7F*/ return true;
    case (0x9B+'@'): /*CSI=0x9B*/ set_state(SAW_CSI); return true;
    default: return false;
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

  /// \brief Write the contents of this reader to the given stream
  ///
  /// The contents of the reader are the interpreted inputs it has
  /// read, not those inputs themselves.
  void write_to(std::ostream& out) const{
    std::vector<std::vector<char> >::const_iterator line;
    for(line = lines.begin(); line != lines.end(); ++line){
      std::vector<char>::const_iterator ch;
      for(ch = line->begin(); ch != line->end(); ++ch){
	out << *ch;
      }
      out << std::endl;
    }
  }
};

void Reader::read_from(std::istream& in){
  while(in){
    RState next_state = SAW_NOTHING;
    char c = in.get();
    if(in.eof()){ break; } //Don't process end of line
    if(id_and_process_control_char(c)){
      continue;
    }
    switch(state){
    case SAW_NOTHING:
      //      std::cerr << "sn\n"; //DEBUG
      put_char(c);
      break;
    case SAW_ESC: ///ESC ( ^] ) was seen (but no trailing ] or [ )
      //      std::cerr << "se\n"; //DEBUG
      next_state = SAW_NOTHING;
      switch(c){
      case 'c': break; //Terminal reset, do nothing
      case 'D': line_feed(); break; //Line feed
      case 'E': carriage_return(); line_feed(); break; //Newline
      case 'H': set_htab_stop(); break; //Set tab stop
      default:
	unknown_code("escape",c);
      };
      set_state(next_state);
      break;
    case SAW_CSI: ///Control sequence introducer - ESC [ or 0x9B
      //      std::cerr << "scsi\n"; //DEBUG
      unknown_code("conrol sequence (0x9B or ESC [)",c);
      break;
    case SAW_OSC: ///Operating system command ESC ]
      unknown_code("operating system command ESC ]",c);
      break;
    case SAW_OSC_P: ///ESC ] P (will read 7 hex digits)
      unknown_code("palette set command ESC ] P",c);
      break;
    case SAW_ESC_NUM: ///   ESC #
      unknown_code("DEC screen alignment command ESC #",c);
      break;
    case SAW_ESC_PCT: ///   ESC %
      unknown_code("character set select command ESC %",c);
      break;
    case SAW_ESC_LPAREN:/// ESC (
      unknown_code("G0 character set select command ESC (",c);
      break;
    case SAW_ESC_RPAREN: /// ESC )
      unknown_code("G1 character set select command ESC )",c);
      break;
    case SAW_CSI_LBRACKET: /// ESC [ [ or CSI [ (just eats next char)
      break;
    default:
      std::cerr << "ERROR: Unknown reader state (" << ((unsigned)state)
		<< ") encountered in read_from\n";
      exit(-2);
    }
  }
}

int main(){
  Reader r;
  r.read_from(std::cin);
  r.write_to(std::cout);
}
