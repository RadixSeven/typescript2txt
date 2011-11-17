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
 * http://unixhelp.ed.ac.uk/CGI/man-cgi?console_codes+4 and
 * http://www.kernel.org/doc/man-pages/online/pages/man4/console_codes.4.html
 * which can be seen in many linux distros as man console_codes
 *
 * It would be easy to support colours if we outputted to e.g. HTML.
 *
 * John C. McCabe-Dansted (gmatht@gmail.com) 2008
 *
 * Completely rewritten in C++ by Eric Moyer in 2011
 *
 * Permission is granted to distribute this software under any version
 * of the BSD and GPL licenses.
 *******************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <string>
#include <cctype>
#include <cassert>
#include <algorithm>
#include <stdint.h> 

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

  /// The width (in characters) of the terminal that this Reader emulates
  const static std::size_t width = 80;

  /// The parameters that are used for the CSI sequences - also used
  /// by some of the OSC commands
  std::vector<unsigned> params;
  
  /// Enum to specify the different states the reader can be in
  enum RState{
    SAW_NOTHING,
    SAW_ESC, ///ESC ( ^] ) was seen (but no trailing ] or [ )
    SAW_CSI, ///Control sequence introducer - ESC [ or 0x9B
    SAW_OSC, ///Operating system command ESC ]
    SAW_OSC_EAT_2_BEL, ///Ignore all characters until ^G in OSC
    SAW_OSC_4, ///OSC followed by a 4
    SAW_OSC_5, ///OSC followed by a 5
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

  /// Perform a tab: position the cursor at the next tab stop
  ///
  /// Since tab-stops cannot be set right now, just puts the cursor at
  /// the next multiple of 8
  void tab(){
    char_idx += (8-(char_idx%8))%8;
  }

  /// Perform a carriage return - put the current character index at the beginning of the line
  void carriage_return(){
    char_idx = 0;
  }

  /// Perform a back-space - move back over previous character
  void back_space(){
    if(char_idx > 0){
      --char_idx;
    }
  }

  /// Insert the given character at the current position, not moving
  ///
  /// Inserts \a c at the current position.  Does not change the
  /// current position. If the current character is beyond the end of
  /// the line, inserts enough spaces so that the character is written
  /// at the correct position.
  ///
  /// \param c The character to insert
  void insert_char(char c){
    while(char_idx > cur_line().size()){
      cur_line().push_back(' ');
    }
    assert(char_idx <= cur_line().size());
    if(char_idx == cur_line().size()){
      cur_line().push_back(c);
    }else if(char_idx < cur_line().size()){
      cur_line().insert(char_idx + cur_line().begin(), c);
    }else{
      std::cerr << "SERIOUS WARNING: Impossible value for char_idx "
		<< "in insert_char.  Ignoring.\n"
		<< issue_report_boilerplate();
    }
  }

  /// Set the current character to \a c and advance to the next
  ///
  /// Sets the character a the current position to \a c then advances
  /// the current character to the next position.  If the current
  /// character is beyond the end of the line, inserts enough spaces
  /// so that the character is written at the correct position.
  ///
  /// \param c The new value of the character at the current position
  void put_char(char c){
    if(char_idx >= width){
      std::cerr << "Warning: cursor beyond bounds of window in put_char.";
      char_idx = width - 1;
    }
    while(char_idx > cur_line().size()){
      cur_line().push_back(' ');
    }
    assert(char_idx <= cur_line().size());
    if(char_idx == cur_line().size()){
      cur_line().push_back(c);
      ++char_idx;
      if(char_idx >= width){
	carriage_return(); line_feed();
      }
    }else if(char_idx < cur_line().size()){
      cur_line().at(char_idx) = c;
      ++char_idx;
      if(char_idx >= width){
	carriage_return(); line_feed();
      }
    }else{
      std::cerr << "SERIOUS WARNING: Impossible value for char_idx "
		<< "in put_char.  Ignoring.\n"
		<< issue_report_boilerplate();
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


  /// Performs the insert blank CSI command ESC [ ... @
  ///
  /// If not at the end of a line, inserts the number of blanks
  /// required by \a param.  
  ///
  /// At the end of a line, does nothing.
  ///
  /// \param params is an array with the parameters passed to the
  ///               command.  If no parameters are given, inserts one
  ///               blank.  Otherwise, inserts as many blanks as the
  ///               value of the first parameter.  Prints a warning if
  ///               there is more than one parameter.
  void insert_blank(std::vector<unsigned> params){
    if(params.size() == 0){
      params.push_back(1);
    }else if(params.size() > 1){
      std::cerr << "Warning: too many arguments given to insert "
		<< "blank CSI command ESC [ ... @\n"
		<< "Ignoring extra parameters\n";
    }
    if(char_idx >= cur_line().size()){
      return;
    }
    for(unsigned b = 0; b < params.front(); ++b){
      insert_char(' ');
    }
  }

  /// Performs the cursor up CSI command ESC [ ... A
  ///
  /// Moves the cursor up the number of lines required by \a param.
  /// Does not make new lines above the first line.
  ///
  /// \param params is an array with the parameters passed to the
  ///               command.  If no parameters are given, goes up one
  ///               line blank.  Otherwise, goes up as many lines as
  ///               the value of the first parameter.  Prints a
  ///               warning if there is more than one parameter.  Does
  ///               not go above the first line
  void cursor_up(std::vector<unsigned> params){
    if(params.size() == 0){
      params.push_back(1);
    }else if(params.size() > 1){
      std::cerr << "Warning: too many arguments given to cursor up "
		<< "CSI command ESC [ ... A\n"
		<< "Ignoring extra parameters\n";
    }
    if(line_idx > params.front()){
      line_idx -= params.front();
    }else{
      line_idx = 0;
    }
  }

  /// Performs the cursor down CSI command ESC [ ... B
  ///
  /// Moves the cursor down the number of lines required by \a param.
  /// Does not make new lines below the last line.
  ///
  /// \param params is an array with the parameters passed to the
  ///               command.  If no parameters are given, goes down one
  ///               line.  Otherwise, goes down as many lines as
  ///               the value of the first parameter.  Prints a
  ///               warning if there is more than one parameter.  Does
  ///               not go above the first line
  void cursor_down(std::vector<unsigned> params){
    if(params.size() == 0){
      params.push_back(1);
    }else if(params.size() > 1){
      std::cerr << "Warning: too many arguments given to cursor down "
		<< "CSI command ESC [ ... A\n"
		<< "Ignoring extra parameters\n";
    }
    if(line_idx + params.front() < lines.size()){
      line_idx += params.front();
    }else{
      line_idx = params.size() - 1;
    }
  }

  /// Performs the cursor right CSI command ESC [ ... C
  ///
  /// Moves the cursor right the number of columns required by \a
  /// param.
  /// 
  ///
  /// \param params is an array with the parameters passed to the
  ///               command.  If no parameters are given, goes right one
  ///               column.  Otherwise, goes right as many columns as
  ///               the value of the first parameter.  Prints a
  ///               warning if there is more than one parameter.
  void cursor_right(std::vector<unsigned> params){
    if(params.size() == 0){
      params.push_back(1);
    }else if(params.size() > 1){
      std::cerr << "Warning: too many arguments given to cursor right "
		<< "CSI command ESC [ ... A\n"
		<< "Ignoring extra parameters\n";
    }
    char_idx += params.front();
  }

  /// Performs the delete characters CSI command ESC [ ... P
  ///
  /// Removes the number of characters indicated by \a params from the
  /// line to the right of the current cursor location.  Shifts all
  /// characters after the one to delete to the left to fill in the gap.
  /// If the cursor is past the end of the line, does nothing.
  ///
  /// \param params is an array with the parameters passed to the
  ///               command.  If no parameters are given, deletes one
  ///               character.  Otherwise, deletes as many characters
  ///               as the value of the first parameter.  Prints a
  ///               warning if there is more than one parameter.
  void delete_characters(std::vector<unsigned> params){
    if(params.size() == 0){
      params.push_back(1);
    }else if(params.size() > 1){
      std::cerr << "Warning: too many arguments given to delete characters "
		<< "CSI command ESC [ ... P\n"
		<< "Ignoring extra parameters\n";
    }
    if(params.front() > 0){
      if(char_idx < cur_line().size()){
	unsigned chars_to_right = cur_line().size() - char_idx;
	unsigned chars_to_delete = std::min(chars_to_right, params.front());
	std::vector<char>::iterator del_first = cur_line().begin()+char_idx;
	std::vector<char>::iterator del_end = del_first + chars_to_delete;
	cur_line().erase(del_first, del_end);
      }
    }
  }

  /// Performs the erase line CSI command ESC [ ... K
  /// 
  /// If the parameters array is empty removes all characters in
  /// cur_line() at or after char_idx. 
  ///
  /// If the parameters array holds at least one value, then the first
  /// value must be 1 or 2.  If the first value is neither 1 or 2,
  /// then prints a warning and does nothing.
  /// 
  /// If the first value is 1, the behavior depends on whether
  /// char_idx is past the end of the line.  If there are characters
  /// past char_idx in the line, fills all characters from the
  /// beginning of the line to char_idx (inclusive) with blanks.
  /// Otherwise the behavior is identical to if the param value had
  /// been 2.
  ///
  /// If the first value is 2, deletes all characters in the array,
  /// but leaves char_idx the same.
  ///
  /// If there is more than one argument, prints a warning
  ///
  /// \param params The parameters passed to the CSI K command - see
  ///               the main text for a description of behavior
  void erase_line(std::vector<unsigned> params){
    if(params.size() == 0){
      if(char_idx < cur_line().size()){
	std::vector<char>::iterator erasure_start = cur_line().begin()+char_idx;
	cur_line().erase(erasure_start, cur_line().end());
      }
    }else{
      if(params.size() > 1){
	std::cerr << "Warning: too many arguments given to erase line "
		  << "CSI command ESC [ ... K\n"
		  << "Ignoring extra parameters\n";      
      }
      unsigned p = params.front();
      if(p != 1 && p != 2){
	std::cerr << "Warning: argument " << p << " passed to erase line "
		  << "CSI command ESC [ ... K\n"
		  << "The meaning of this argument is unknown.  "
		  << "Doing nothing.\n";
	return;
      }
      if(p == 2 || (char_idx + 1) >= cur_line().size()){
	//Delete whole line:
	//Param was 2 or we are at or past the last character in the line
	cur_line().clear();
	return;
      }else{
	//Delete chars before and at char_idx when there is at least
	//one character that won't be deleted
	assert(p == 1);
	assert(char_idx + 1 < cur_line().size());
	std::vector<char>::iterator erasure_start = cur_line().begin();
	std::fill(erasure_start, erasure_start + char_idx + 1, ' ');
	return;
      }
    }
  }

  /// \brief Return a string containing instructions for reporting an issue
  /// \brief with the program
  ///
  /// \return a string containing instructions for reporting an issue
  ///         with the program
  char const * issue_report_boilerplate(){
    return "Please submit an issue report to "
      "https://github.com/RadixSeven/typescript2txt/issues "
      "and include the file you were processing by following the instructions "
      "in https://github.com/ned14/Easyshop/issues/1\n";
  }


  /// Do nothing
  ///
  /// This function is called to document that a CSI command is
  /// intentionally not implemented because it is not appropriate to
  /// generating plain text files.
  void ignore_CSI(){ }


  //###################################################
  //###################################################
  //###    Unimplemented Codes That Generate Warnings
  //###################################################
  //###################################################

  /// Print a warning that the given CSI command is unimplemented
  ///
  /// Unlike the regular escape codes which all have different
  /// structrues, I can use a generic warning routine because of the
  /// simple structure of the CSI codes: list of parameters followed
  /// by an action character.
  ///
  /// This warning lets a programmer just implement the codes needed
  /// to parse his files and ignore the rest.
  ///
  /// This is called for CSI codes that may impact plain text output,
  /// so someone may want to implement them in the future.
  ///
  /// \param code The single character command code that identifies
  ///             the CSI command.  This would be @ for insert blanks,
  ///             A for cursor up, etc.
  ///
  /// \param descr A longer description of the code
  ///
  /// \param params The parameters that would have been passed to the code
  void unimplemented_CSI(char code, std::string descr, 
			 std::vector<unsigned> params){
    std::cerr << "Warning: this typescript contains an unimplemented CSI "
	      << "code \"ESC [ ... " << code << "\".  "
	      << "The code has description: \"" << descr << "\" and was "
	      << "passed ";
    if(params.size() == 0){
      std::cerr << "no parameters.  ";
    }else{
      std::cerr << "The parameters: " << params.at(0);
      for(std::size_t i=1; i < params.size(); ++i){
	std::cerr << ", " << params.at(i);
      }
      std::cerr << ".  ";
    }
    std::cerr << "Ignoring.\n";
  }


  /// Execute character set change to \a num - just prints warning right now
  ///
  /// \param num the number of the character set to change to
  void character_set(int num){
    std::cerr << "Warning: typescript file contains command to change to the "
	      << "G" << num << " character set.  This command is ignored by "
	      << "this translator.\n";
  }

  /// Set a horizontal tab stop at the current cursor position
  /// 
  /// Right now, does nothing but print a warning
  void set_htab_stop(){
    std::cerr << "Warning: typescript file contains tab-stop-changing "
	      << "commands that are ignored by this translator.\n";
  }

  /// Save the cursor state for later restoration
  ///
  /// Right now, does nothing but print a warning
  void save_cursor_state(){
    std::cerr << "Warning: typescript file contains cursor-state-saving "
	      << "commands that are ignored by this translator.\n";
  }

  /// Restore the cursor state for later restoration
  ///
  /// Right now, does nothing but print a warning
  void restore_cursor_state(){
    std::cerr << "Warning: typescript file contains cursor-state-restoring "
	      << "commands that are ignored by this translator.\n";
  }

  /// Select character set based on the given \a code which must be @,G, or 8
  ///
  /// Right now, does nothing but print a warning
  ///
  /// \param code @ selects the default (ISO 646/ISO 8859-1), G and 8
  ///             select UTF-8
  void select_character_set(const char code){
    assert(code == '@' || code == 'G' || code == '8');
    if(code == '@'){
      std::cerr << "Warning: typescript contains command to set the "
		<< "character set to ISO 646/ISO 8859-1.  This is "
		<< "currently ignored by this translator.\n";
    }else if(code == 'G' || code == '8'){
      std::cerr << "Warning: typescript contains command to set the "
		<< "character set to UTF8.  This is "
		<< "currently ignored by this translator.\n";
    }else{
      std::cerr << "SERIOUS WARNING: Illegal code passed to "
		<< "select_character_set.  Ignoring.\n"
		<< issue_report_boilerplate();
      return;
    }
  }

  /// Define the character set mapping for G0 or G1 
  ///
  /// Right now does nothing but print a warning.
  ///
  /// Assumes code is one of B,0,U, or X and g_number is 0 or 1
  ///
  /// \param g_number if 0 defines, G0, if 1 defines G1
  ///
  /// \param code The character from the escape code indicating which
  ///             mapping.  From the man page: B = Select default (ISO
  ///             8859-1 mapping), 0 = Select VT100 graphics mapping,
  ///             U = Select null mapping - straight to character ROM,
  ///             K = Select user mapping - the map that is loaded by
  ///             the utility mapscrn(8).
  void  define_g_character_set(const unsigned g_number, const char code){
    assert(code == 'B' || code == '0' || code == 'U' || code == 'K');
    assert(g_number == 0 || g_number == 1);
    std::string mapping;
    switch(code){
    case 'B': mapping = "ISO 8859-1"; break;
    case '0': mapping = "VT100 graphics"; break;
    case 'U': mapping = "the null mapping (straight to character ROM)"; break;
    case 'K': mapping = "the user mapping (loaded by mapscrn(8))"; break;
    default:
      std::cerr << "SERIOUS WARNING: Illegal code passed to "
		<< "define_g_character_set.  Ignoring."
		<< issue_report_boilerplate();
      return;
    }
    char g_char = (g_number == 0?'(':')');
    std::cerr << "Warning: typescript contains command to define the G" 
	      << g_number << " character set as " << mapping << "\n"
	      << "This command -- ESC " << g_char
	      << ' ' << code << " -- is currently ignored by this translator.";
  }

  ///Set the palette entry at \a index to the color described by \a rgb
  ///
  /// Right now, just prints a warning
  ///
  /// \param index The index of of the palette entry to set 0-15 inclusive
  ///
  /// \param rgb The 24 bit number whose lower 3 bytes describe the
  ///            red, green, and blue intensities in that order
  ///            (xxxxxxxxRRRRRRRRGGGGGGGGBBBBBBBB)
  void set_palette(int index, int rgb){
    if(rgb & 0xFF000000){
      std::cerr << "SERIOUS WARNING: Illegal RGB value "
		<< std::hex << ((unsigned)rgb) << " passed to "
		<< "set_palette.  Ignoring."
		<< issue_report_boilerplate();
      return;
    }
    if(index < 0 || index > 15){
      std::cerr << "SERIOUS WARNING: Illegal index value "
		<< index << " passed to "
		<< "set_palette.  Ignoring."
		<< issue_report_boilerplate();
      return;
    }
    int r = (rgb >> 16) & 0xFF;
    int g = (rgb >> 8)  & 0xFF;
    int b = rgb         & 0xFF;
    std::cerr << "Warning: typescript contains command to set palette " 
	      << "entry " << index << " to rgb=("<< r << ',' << g << ','
	      << b << ")  This command is ignored by this translator.\n";
  }

  /// Reset the palette 
  ///
  /// Right now, does nothing but print a warning
  void reset_palette(){
    std::cerr << "Warning: typescript contains command to reset the palette.  " 
	      << "This command is ignored by this translator.\n";
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
      std::vector<std::vector<char> >::const_iterator next_line = line;
      ++next_line;
      std::vector<char>::const_iterator ch;
      for(ch = line->begin(); ch != line->end(); ++ch){
	out << *ch;
      }
      //Output a newline unless this is the last line and it is blank
      //(meaning that it was created by a previous newline but nothing
      //was written to it)
      if(! (line->size() == 0 && next_line == lines.end()) ){
	out << std::endl;
      }
    }
  }
};

void Reader::read_from(std::istream& in){
  while(in){
    RState next_state = SAW_NOTHING;
    int tmp_val;
    char c = in.get();
    if(in.eof()){ break; } //Don't process end of file
    //Process control characters unless in an operating system command
    //that terminates with a BEL character
    if(state != SAW_OSC_EAT_2_BEL){ 
      if(id_and_process_control_char(c)){
	continue;
      }
    }
    switch(state){
    case SAW_NOTHING:
      put_char(c);
      break;
    case SAW_ESC: ///ESC ( ^] ) was seen (but no trailing ] or [ )
      next_state = SAW_NOTHING;
      switch(c){
      case 'c': break; //Terminal reset, do nothing
      case 'D': line_feed(); break; //Line feed
      case 'E': carriage_return(); line_feed(); break; //Newline
      case 'H': set_htab_stop(); break; //Set tab stop
      case 'M': reverse_line_feed(); break; //Reverse line feed
      case 'Z': break; //Ignore dec identification code request
      case '7': save_cursor_state(); break; //Save cursor state
      case '8': restore_cursor_state(); break; //Restore cursor state
      case '[': next_state = SAW_CSI; break; //Control sequence introducer
      case '%': next_state = SAW_ESC_PCT; break; //ESC %
      case '#': next_state = SAW_ESC_NUM; break; //ESC #
      case '(': next_state = SAW_ESC_LPAREN; break; //ESC (
      case ')': next_state = SAW_ESC_RPAREN; break; //ESC )
      case '>': break; //Ignore numeric keypad mode
      case '=': break; //Ignore application keypad mode
      case ']': next_state = SAW_OSC; break; //Operating system command
      default:
	unknown_code("escape",c);
      };
      set_state(next_state);
      break;
    case SAW_CSI: ///Control sequence introducer - ESC [ or 0x9B
      switch(c){
      case '?':
	if(params.size() != 0){
	  std::cerr << "Warning: typescript contains badly formatted CSI code. "
		    << "The ? character appears in the parameter list but is "
		    << "not the first character.  Ignoring.";
	};
	break;
      case '0': 
      case '1': 
      case '2': 
      case '3': 
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': 
	if(params.size() == 0){
	  params.push_back(0);
	}
	tmp_val = c-'0';
	params.back() = (params.back()*10)+tmp_val;
	break;
      case ';':
	if(params.size() == 0){
	  params.push_back(0);
	}
	params.push_back(0);
	break;
      case '@':	insert_blank(params); set_state(SAW_NOTHING); break;
      case 'A': cursor_up(params); set_state(SAW_NOTHING); break;
      case 'B': cursor_down(params); set_state(SAW_NOTHING); break;
      case 'C': cursor_right(params); set_state(SAW_NOTHING); break;
      case 'D': 
	unimplemented_CSI(c, "Cursor left", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'E': 
	unimplemented_CSI(c, "Cursor down and to column 1", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'F': 
	unimplemented_CSI(c, "Cursor up and to column 1", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'G': 
	unimplemented_CSI(c, "Cursor to column", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'H': 
	unimplemented_CSI(c, "Cursor to row, column (origin is 1,1)", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'J': 
	unimplemented_CSI(c, "Erase display", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'K': erase_line(params); set_state(SAW_NOTHING); break;
      case 'L': 
	unimplemented_CSI(c, "Insert lines", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'M': 
	unimplemented_CSI(c, "Delete lines", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'P': delete_characters(params); set_state(SAW_NOTHING); break;
      case 'X': 
	unimplemented_CSI(c, "Erase chars", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'a': 
	unimplemented_CSI(c, "Cursor right", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'c': ignore_CSI(); set_state(SAW_NOTHING); //Ignore VT102 id seq.
	break;
      case 'd': 
	unimplemented_CSI(c, "Cursor to row", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'e': 
	unimplemented_CSI(c, "Cursor down", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'f': 
	unimplemented_CSI(c, "Cursor to row, column", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'g': 
	unimplemented_CSI(c, "Clear tab stop", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'h': 
	unimplemented_CSI(c, "Set mode", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'l': 
	unimplemented_CSI(c, "Reset mode", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'm': ignore_CSI(); set_state(SAW_NOTHING); //Ignore char attributes
	break;
      case 'n': 
	unimplemented_CSI(c, "Device status report", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'q': ignore_CSI(); set_state(SAW_NOTHING); //Ignore LED setting
	break;
      case 'r': 
	unimplemented_CSI(c, "Set srolling region", params); 
	set_state(SAW_NOTHING); 
	break;
      case 's': 
	unimplemented_CSI(c, "Save cursor location", params); 
	set_state(SAW_NOTHING); 
	break;
      case 'u': 
	unimplemented_CSI(c, "Restore cursor location", params); 
	set_state(SAW_NOTHING); 
	break;
      case '`': 
	unimplemented_CSI(c, "Cursor to column", params); 
	set_state(SAW_NOTHING); 
	break;
	//The following parameters were only in references/ctlseqs.pdf
      case 'T': ignore_CSI(); set_state(SAW_NOTHING); //Ignore mouse tracking
	break;
      case 'x': ignore_CSI(); set_state(SAW_NOTHING); //Ignore term params req.
	break;
      default:
	unknown_code("conrol sequence (0x9B or ESC [)",c);
      }
      break;
    case SAW_ESC_PCT: ///   ESC %
      switch(c){
      case '@':	
      case 'G': 
      case '8': select_character_set(c); break;
      default:
	unknown_code("character set select command ESC %",c);
      };
      set_state(SAW_NOTHING);
      break;
    case SAW_ESC_NUM: ///   ESC #
      switch(c){
      case '8':
	std::cerr << "Warning: typescript file contains DEC screen "
		  << "alignment command which is ignored "
		  << "by this translator.\n";
      default:
	unknown_code("DEC screen alignment command ESC #",c);
      };
      set_state(SAW_NOTHING);
      break;
    case SAW_ESC_LPAREN:/// ESC (
      switch(c){
      case 'B': 
      case '0':
      case 'U':
      case 'K': define_g_character_set(0,c); break;
      default:
	unknown_code("define G0 character set command ESC (",c);
      };
      set_state(SAW_NOTHING);
      break;
    case SAW_ESC_RPAREN: /// ESC )
      switch(c){
      case 'B': 
      case '0':
      case 'U':
      case 'K': define_g_character_set(1,c); break;
      default:
	unknown_code("define G1 character set command ESC )",c);
      };
      set_state(SAW_NOTHING);
      break;
    case SAW_OSC: ///Operating system command ESC ]
      next_state = SAW_NOTHING;
      switch(c){
      case 'P': next_state = SAW_OSC_P; break;
      case 'R': reset_palette(); break;
      case '0': next_state = SAW_OSC_EAT_2_BEL; break;
      case '1': next_state = SAW_OSC_EAT_2_BEL; break;
      case '2': next_state = SAW_OSC_EAT_2_BEL; break;
      case '4': next_state = SAW_OSC_4; break;
      case '5': next_state = SAW_OSC_5; break;
      default:
	unknown_code("operating system command ESC ]",c);
      }
      set_state(next_state);
      break;
    case SAW_OSC_EAT_2_BEL:
      if(c=='\x07'){ //^G seen, go back to normal mode
	set_state(SAW_NOTHING);
      }
      break;
    case SAW_OSC_4:
      if(c!='6'){
	unknown_code("operating system command number 4 (ESC ] 4)",c);
      }
      set_state(SAW_OSC_EAT_2_BEL);
      break;
    case SAW_OSC_5:
      if(c!='0'){
	unknown_code("operating system command number 5 (ESC ] 5)",c);
      }
      set_state(SAW_OSC_EAT_2_BEL);
      break;
    case SAW_OSC_P: ///ESC ] P (will read 7 hex digits)
      //params[0] holds the number of hex digits read
      //params[1] holds the index to set
      //params[2] holds the color to set it to (24 bit integer)
      if(params.size() != 3 && params.size() != 0){
	std::cerr << "SERIOUS WARNING: incorrect number of parameters "
		  << "in OSC palette setting command SAW_OSC_P\n"
		  << "Ignoring\n"
		  << issue_report_boilerplate();
	params.clear();
      }
      if(params.size() == 0){
	params.push_back(0); params.push_back(0); params.push_back(0);
      }
      tmp_val = -1;
      switch(c){
      case '0': tmp_val = 0; break;
      case '1': tmp_val = 1; break;
      case '2': tmp_val = 2; break;
      case '3': tmp_val = 3; break;
      case '4': tmp_val = 4; break;
      case '5': tmp_val = 5; break;
      case '6': tmp_val = 6; break;
      case '7': tmp_val = 7; break;
      case '8': tmp_val = 8; break;
      case '9': tmp_val = 9; break;
      case 'A': tmp_val = 10; break;
      case 'B': tmp_val = 11; break;
      case 'C': tmp_val = 12; break;
      case 'D': tmp_val = 13; break;
      case 'E': tmp_val = 14; break;
      case 'F': tmp_val = 15; break;
      default:
	unknown_code("palette set command ESC ] P",c);
      }
      if(tmp_val != -1){
	++params.at(0);
	if(params.at(0) == 1){
	  params.at(1) = tmp_val;
	}else if(params.at(0) > 1 && params.at(0) <= 7){
	  params.at(2) = (params.at(2) << 4) + tmp_val;
	  if(params.at(0) == 7){
	    set_palette(params.at(1), params.at(2));
	  }
	}else{
	  std::cerr << "SERIOUS WARNING: incorrect number of digits read "
		    << '(' << params.at(0) << ')'
		    << "in OSC palette setting command SAW_OSC_P\n"
		    << "Ignoring\n"
		    << issue_report_boilerplate();
	}
      }
      break;
    case SAW_CSI_LBRACKET: /// ESC [ [ or CSI [ (just eats next char)
      set_state(SAW_NOTHING);
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
