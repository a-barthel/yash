#include "shell-readline.hpp"
/** 
 * Wrapper for the write function, given a character.
 * 
 * @param _fd File descriptor on which to write.
 * @param c Character to write.
 * 
 * @return True upon successful write.
 */
bool read_line::write_with_error(int _fd, char & c)
{
    if (!write(_fd, &c, 1)) {
	perror("write");
	return false;
    } return true;
}

/** 
 * Wrapper for the write function, given a C string.
 *
 * This function writes the contents of s to the
 * provided file descriptor. This function will
 * write to the first occurance of a null character,
 * as it calls strlen to determine the length.
 * For those strings whose length, is known, see
 * @see write_with_error(int _fd, const char *& s, size_t len).
 * 
 * @param _fd File descriptor on which to write.
 * @param s String to write.
 * 
 * @return True upon successful write.
 */
bool read_line::write_with_error(int _fd, const char * s)
{
    if (write(_fd, s, strlen(s)) != strlen(s)) {
	perror("write");
	return false;
    } return true;
}

/** 
 * Wrapper for the write function, given a C string
 * and a number of characters.
 *
 * This function writes len bytes of s to the
 * provided file descriptor.
 * 
 * @param _fd File descriptor on which to write.
 * @param s String to write.
 * @param len Number of bytes of s to write.
 * 
 * @return True upon successful write.
 */
bool read_line::write_with_error(int _fd, const char * s, const size_t & len)
{
    if (write(_fd, s, len) != len) {
	perror("write");
	return false;
    } return true;
}

/** 
 * Wrapper for the read function, given a char &
 * in which to store the char and a file descriptor
 * from which we read it.
 * 
 * @param _fd File descriptor from which to read.
 * @param c Character to store the read character.
 * 
 * @return True upon successful read.
 */
bool read_line::read_with_error(int _fd, char & c)
{
    char d; /* temp, for reading */
    if (!read(0, &d, 1)) {
	perror("read");
	return false;
    } else return (c = d), true;
}

/** 
 * Wrapper function for getting the width
 * of the terminal using ioctl.
 * 
 * @return Number of columns in the terminal.
 */
size_t read_line::get_term_width()
{
    struct winsize w;
    ioctl(1, TIOCGWINSZ, &w);
    return w.ws_col;
}

/** 
 * @brief Receive enter key
 * 
 * @param _line The current line of text
 * @param input The last input character
 * 
 * @return True if we should break, false to continue.
 */
bool read_line::handle_enter(std::string & _line, char & input)
{
    char ch;
    // Enter was typed
    if (m_buff.size()) {
	for (; m_buff.size();) {
	    ch = m_buff.top(); m_buff.pop();
	    _line += ch;
	    if (!write_with_error(1, ch)) return false;
	}
    } if (!write_with_error(1, input)) return false;
    history_index = m_history.size();
    return true;
}

/** 
 * @brief Handle ctrl + a
 *
 * This is what we do when the user types ctrl+a.
 * Namely, we move the cursor to the beginning of
 * the text entry point.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true otherwise.
 */
bool read_line::handle_ctrl_a(std::string & _line)
{
    if (!_line.size()) return false;

    register size_t term_width = get_term_width();
			
    for (; _line.size();) {
	m_buff.push(_line.back()); _line.pop_back();
	/* Next check if we need to go up */
	/* @todo this does not quite work -- but it's close */
	if (_line.size() == term_width) {
	    if (!write_with_error(1, "\033[1A\x1b[33;1m$ \x1b[0m")) return false;
	    else if (!write_with_error(1, _line.c_str(), term_width - 3)) return false;

	    for (size_t k = 0; k < term_width - 2; ++k)
		if (!write_with_error(1, "\b", 1)) return true;
	} else if (!write_with_error(1, "\b", 1)) return false;
    }
}

/** 
 * @brief Handle ctrl + e
 *
 * This is how the shell responds to ctrl + e.
 * In particular, the shell should go to the end
 * of the current text.
 * 
 * @param _line Current text.
 * 
 * @return False to continue, true otherwise.
 */
bool read_line::handle_ctrl_e(std::string & _line)
{
    char ctrle[m_buff.size() + 1];
    size_t len = m_buff.size();
    memset(ctrle, 0, m_buff.size() + 1);

    for (char * d = ctrle; m_buff.size();) {
	*(d) = m_buff.top(); m_buff.pop();
	_line += *(d++);
    }

    if (!write_with_error(1, ctrle, len)) return false;
    return true;
}

/** 
 * @brief Handle ctrl + d
 *
 * This is how the shell responds to the
 * delete character. It should remove the
 * highlighted cursor, should one exist.
 * 
 * @param _line Current line of text.
 * 
 * @return True to break, false otherwise.
 */
bool read_line::handle_ctrl_d(std::string & _line)
{
    if (!m_buff.size()) return false;
    if (!_line.size()) {
	m_buff.pop();
	std::stack<char> temp = m_buff;
	for (char d = 0; temp.size();)
	    if (write(1, &(d = (temp.top())), 1)) temp.pop();
	  
	if (!write(1, " ", 1)) std::cerr<<"WAT.\n";
	for (int x = m_buff.size() + 1; x -= write(1, "\b", 1););
	return false;
    } if (m_buff.size()) {
	// Buffer!
	std::stack<char> temp = m_buff;
	temp.pop();
	for (char d = 0; temp.size(); ) {
	    d = temp.top(); temp.pop();
	    if (!write(1, &d, 1)) {
		perror("write");
		return false;
	    }
	}
	char b = ' ';
	if (!write_with_error(1, " ", 1)) return false;	
	else if (!write_with_error(1, "\b", 1)) return false;

	m_buff.pop();
	/* Move cursor to current position. */
	for (size_t x = 0; x < m_buff.size(); ++x) if (!write(1, "\b", 1)) return false;;
    } return false;
}

/** 
 * Handle the tab key.
 * 
 * Tab key performs "tab completion."
 * Any file matching the current text
 * is completed as far as it can be.
 * Moreover, there is some binary completion.
 * If a binary in the PATH variable matches
 * the currently entered text, then it
 * is completed in a similar manner.
 *
 * @param _line The current line.
 * 
 * @return False to continue, true otherwise.
 */
bool read_line::handle_tab(std::string & _line)
{
    Command::currentSimpleCommand = std::unique_ptr<SimpleCommand>(new SimpleCommand());
	
    /* Part 1: add a '*' to the end of the stream. */
    std::string _temp;

    std::vector<std::string> _split;
    if (_line.size()) {
	_split = string_split(_line, ' ');
	_temp = tilde_expand(_split.back()) + "*";
    } else _temp = "*";

    char * _complete_me = strndup(_temp.c_str(), _temp.size());
	
    /* Part 2: Invoke wildcard expand */
    wildcard_expand(_complete_me);

    /**
     * Part 3: If wc_collector.size() <= 1, then complete the tab.
     *         otherwise, run "echo <text>*"
     */
    std::string * array = Command::currentCommand.wc_collector.data();
    std::sort(array, array + Command::currentCommand.wc_collector.size());

    if (Command::currentCommand.wc_collector.size() == 1) {
	/* First check if the line has any spaces! */
	/*     If so, we will wrap in quotes!      */
	bool quote_wrap = false; 
	if (Command::currentCommand.wc_collector[0].find(" ")
	    != std::string::npos) {
	    Command::currentCommand.wc_collector[0].insert(0, "\"");
	    quote_wrap = true;
	}
	char ch[_line.size() + 1]; char sp[_line.size() + 1];
	ch[_line.size()] = '\0'; sp[_line.size()] = '\0';
	memset(ch, '\b', _line.size()); memset(sp, ' ', _line.size());
	if (!write_with_error(1, ch, _line.size())) return false;
	if (!write_with_error(1, sp, _line.size())) return false;
	if (!write_with_error(1, ch, _line.size())) return false;
	_line = "";
	  
	for (size_t x = 0; x < _split.size() - 1; _line += _split[x++] + " ");
	_line += Command::currentCommand.wc_collector[0];
	if (quote_wrap) _line = _line + "\"";
	m_current_line_copy = _line;
	Command::currentCommand.wc_collector.clear();
	Command::currentCommand.wc_collector.shrink_to_fit();
    } else if (Command::currentCommand.wc_collector.size() == 0) {
	/* now we check the binaries! */
	char * _path = getenv("PATH");
	if (!_path) {
	    /* if path isn't set, continue */
	    free(_complete_me); return false;
	} std::string path(_path);
			   
	/* part 1: split the path variable into individual dirs */
	std::vector<std::string> _path_dirs = vector_split(path, ':');
	std::vector<std::string> path_dirs;

	/* part 1.5: remove duplicates */
	for (auto && x : _path_dirs) {
	    /* @todo it would be nice to not do this */
	    bool add = true;
	    for (auto && y : path_dirs) {
		if (x == y) add = false;
	    } if (add) path_dirs.push_back(x);
	}

	/* part 2: go through the paths, coallate matches */
	for (auto && x : path_dirs) {
	    /* add trailing '/' if not already there */
	    if (x.back() != '/') x += '/';
	    /* append _complete_me to current path */
	    x += _complete_me;
	    /* duplicate the string */
	    char * _x_cpy = strndup(x.c_str(), x.size());

	    /* invoke wildcard_expand */
	    wildcard_expand(_x_cpy); free(_x_cpy);
	} std::vector<std::string> wc_expanded =
	      Command::currentCommand.wc_collector;

	/* part 4: check for a single match */
	if (wc_expanded.size() == 1) {
	    /* First check if the line has any spaces! */
	    /*     If so, we will wrap in quotes!      */
	    bool quote_wrap = false; 
	    if (Command::currentCommand.wc_collector[0].find(" ")
		!= std::string::npos) {
		Command::currentCommand.wc_collector[0].insert(0, "\"");
		quote_wrap = true;
	    }
	    char ch[_line.size() + 1]; char sp[_line.size() + 1];
	    ch[_line.size()] = '\0'; sp[_line.size()] = '\0';
	    memset(ch, '\b', _line.size()); memset(sp, ' ', _line.size());
		 
	    if (!write_with_error(1, ch, _line.size())) return false;
	    if (!write_with_error(1, sp, _line.size())) return false;
	    if (!write_with_error(1, ch, _line.size())) return false;
	    _line = "";
	  
	    for (size_t x = 0; x < _split.size() - 1; _line += _split[x++] + " ");
	    _line += Command::currentCommand.wc_collector[0];
	    if (quote_wrap) _line = _line + "\"";
	    m_current_line_copy = _line;
	    Command::currentCommand.wc_collector.clear();
	    Command::currentCommand.wc_collector.shrink_to_fit();
	} else { /* part 5: handle multiple matches */
		 /* @todo appropriately handle multiple matches */
	    free(_complete_me); return false;
	}

	/* free resources and print */
	write_with_error(1, _line.c_str(), _line.size());
	free(_complete_me); return false;
    } else {
	std::cout<<std::endl;
	std::vector<std::string> _wcd = Command::currentCommand.wc_collector;
	std::vector<std::string> cpyd = Command::currentCommand.wc_collector;
	std::string longest_common((longest_substring(cpyd)));
		  
	if (_wcd.size()) {
	    printEvenly(_wcd); char * _echo = strdup("echo");
	    Command::currentSimpleCommand->insertArgument(_echo);
	    Command::currentCommand.wc_collector.clear();
	    Command::currentCommand.wc_collector.shrink_to_fit();
	    Command::currentCommand.execute(); free(_echo);

	    /**
	     * Now we add the largest substring of
	     * the above to the current string so
	     * that the tab completion isn't "butts."
	     *
	     * ~ Ana-Gabriel Perez
	     */

	    if (longest_common.size()) {
		char * to_add = strndup(longest_common.c_str() + strlen(_complete_me) - 1,
					longest_common.size() - strlen(_complete_me) + 1);
		_line += to_add; free(to_add);
		m_current_line_copy = _line;
	    }
	} else { free(_complete_me); return false; }
    } free(_complete_me);

    if (!write_with_error(1, _line.c_str(), _line.size()) != (int)_line.size()) return false;

    return true;
}

/** 
 * Handle ctrl + arrow key
 * 
 * This is the argument hopping feature.
 *
 * @param _line current line
 * 
 * @return False if continue.
 */
bool read_line::handle_ctrl_arrow(std::string & _line)
{
    char ch3[4]; memset(ch3, 0, 4 * sizeof(char));
    if (read(0, ch3, 3) != 3) {
	perror("read");
	return false;
    }

    if (ch3[0] == 59 && ch3[1] == 53 && ch3[2] == 67) {
	/* ctrl + right arrow */
	if (!(m_buff.size())) return false;
	for (;(m_buff.size() &&
	       ((_line += m_buff.top(), m_buff.pop(),_line.back()) != ' ') &&
	       (write(1, &_line.back(), 1) == 1)) ||
		 ((_line.back() == ' ') ? !(write(1, " ", 1)) : 0););
	    
    } else if (ch3[0] == 59 && ch3[1] == 53 && ch3[2] == 68) {
	/* ctrl + left arrow */
	if (!_line.size()) return false;
	for (;(_line.size() &&
	       ((m_buff.push(_line.back()),
		 _line.pop_back(), m_buff.top()) != ' ') &&
	       (write(1, "\b", 1) == 1)) ||
		 ((m_buff.top() == ' ') ? !(write(1, "\b", 1)) : 0););
    } return true;
}

/** 
 * Handle ctrl + k
 *
 * ctrl + k clears the remaining
 * parts of the line.
 * 
 * @param _line Current line.
 * 
 * @return False upon error. 
 */
bool read_line::handle_ctrl_k(std::string & _line)
{
    if (!m_buff.size()) return false;
    size_t count = m_buff.size() + 1;
    /* Clear the stack. On its own thread. */
    std::thread stack_killer([this](){
	    for(;m_buff.size();m_buff.pop());
	}); stack_killer.detach();

    char * spaces = (char*) malloc(count + 1);
    char * bspaces = (char*) malloc(count + 1);

    memset(spaces, ' ', count); spaces[count] = '\0';
    memset(bspaces, '\b', count); bspaces[count] = '\0';

    if (!write_with_error(1, spaces, count)) return false;
    else if (!write_with_error(1, bspaces, count)) return false;
    return false;
}

/** 
 * Handle the backspace key.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_backspace(std::string & _line)
{
    /* backspace */
    if (!_line.size()) return false;

    if (m_buff.size()) {
	// Buffer!
	if (!write_with_error(1, "\b", 1)) return false;
	_line.pop_back();
	std::stack<char> temp = m_buff;
	for (char d = 0; temp.size(); ) {
	    d = temp.top(); temp.pop();
	    if (!write_with_error(1, d)) return false;
	}

	if (!write_with_error(1, " ", 1)) return false;
	else if (!write_with_error(1, "\b", 1)) return false;

	/* get terminal width */
	register size_t term_width = get_term_width();
	register size_t line_size = _line.size() + m_buff.size() + 2;
		  
	for (size_t x = 0; x < m_buff.size(); ++x, --line_size) {
	    /* if the cursor is at the end of a line, print line up */
	    if (line_size && (line_size % term_width == 0)) {
		/* need to go up a line */			  
		const size_t p_len = strlen("\033[1A\x1b[33;1m$ \x1b[0m");

		/* now we print the string */
		if (line_size == term_width) {
		    /**
		     * @todo Make sure you print the correct prompt!
		     * this currently is not going to print that prompt.
		     */
		    if (!write_with_error(1, "\033[1A\x1b[33;1m$ \x1b[0m", p_len)) return false;				  
		    else if (!write_with_error(1, _line.c_str(), _line.size())) return false;			  
		    else break;
		} else {
		    if (!write_with_error(1, "\033[1A \b")) return false;
		    else if (!write_with_error(1, _line.c_str() + (_line.size() - line_size),
					       _line.size() - line_size)) return false;
		}
	    } else if (!write_with_error(1, "\b", 1)) return false;
	}
    } else {
	if (!write_with_error(1, "\b", 1)) return false;
	else if (!write_with_error(1, " ", 1)) return false;
	else if (!write_with_error(1, "\b", 1)) return false;
	_line.pop_back();
    } if (((size_t) history_index == m_history.size()) && m_current_line_copy.size()) m_current_line_copy.pop_back();

    return true;
}

/** 
 * Handle the delete button.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_delete(std::string & _line)
{
    char ch3;

    if (!read_with_error(0, ch3)) return false;

    if (ch3 == 126) {
	if (!m_buff.size()) return false;
	if (!_line.size()) {
	    m_buff.pop();
	    std::stack<char> temp = m_buff;
	    for (char d = 0; temp.size();) {
		if (write_with_error(1, &(d = (temp.top())), 1)) temp.pop();
		else return false;
	    }
	    if (!write(1, " ", 1)) std::cerr<<"WAT.\n";
	    for (int x = m_buff.size() + 1; x-- && write_with_error(1, "\b", 1););
	    return false;
	}

	if (m_buff.size()) {
	    // Buffer!
	    std::stack<char> temp = m_buff;
	    temp.pop();
	    for (char d = 0; temp.size(); ) {
		d = temp.top(); temp.pop();
		if (!write_with_error(1, d)) return false;
	    }
	    char b = ' ';
	    if (!write_with_error(1, " ", 1)) return false;
	    else if (!write_with_error(1, "\b", 1)) return false;
	    m_buff.pop();
	    // Move cursor to current position.
	    for (size_t x = 0; x < m_buff.size(); ++x) {
		if (!write_with_error(1, "\b", 1)) return false;
	    }
	} else return false;
	if ((size_t) history_index == m_history.size()) m_current_line_copy.pop_back();
    } return true;
}

/** 
 * Handle the "!!" or "!-<n>" keys.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_bang(std::string & _line)
{
    if (!write_with_error(0, "!", 1)) return false;

    /* Check for "!!" and "!-<n>" */
    if (!m_history.size()) {
	_line += "!";
	return false;
    }
	  
    char ch1;
    /* read next char from stdin */
    if (!read_with_error(0, ch1)) return false;

    /* print newline and stop looping */
    if ((ch1 == '\n') && !write_with_error(1, "\n", 1)) return true;			   
    else if (ch1 == '!') {
	// "!!" = run prior command
	if (!write_with_error(1, "!", 1)) return false;

	_line += m_history[m_history.size() - 1];
	_line.pop_back();
	m_show_line = true;
	return false;
    } else if (ch1 == '-') {
	if (!write_with_error(1, "-", 1)) return false;

	auto && is_digit = [](char b) { return '0' <= b && b <= '9'; };
				  
/* "!-<n>" = run what I did n commands ago. */
char * buff = (char*) alloca(20); char * b;
	  for (b=buff;read(0,b,1)&&write(1,b,1)&&is_digit(*b);*(++b+1)=0);
	  int n = atoi(buff); bool run_cmd = false;
	  if (*b=='\n') run_cmd = true;
	  if (n > 0) {
		 int _idx = m_history.size() - n;
		 _line += m_history[(_idx >= 0) ? _idx : 0];
		 _line.pop_back();
		 m_show_line = true;
		 if (run_cmd) {
			if (m_buff.size()) {
			   char ch;
			   for (; m_buff.size();) {
				  ch = m_buff.top(); m_buff.pop();
				  _line += ch;
							  
				  if (!write_with_error(1, ch)) return false;
			   }
			}
			history_index = m_history.size();
			return true;
		 }
	  }
   } else {
	  if (!write_with_error(1, ch1)) return false;
	  _line += "!"; _line += ch1;
	  return false;
   }
}

/** 
 * Handle the up arrow.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_up_arrow(std::string & _line)
{
   /* we will print the line prior from history. */
   if (!m_history.size()) return false;

   /* clear input so far */
   char ch[_line.size() + 1]; char sp[_line.size() + 1];
   memset(ch, '\b',_line.size()); memset(sp, ' ', _line.size());
   if (write(1, ch, _line.size()) != (int) _line.size()) {
	  perror("write");
	  return false;
   } else if (write(1, sp, _line.size()) != (int) _line.size()) {
	  perror("write");
	  return false;
   } else if (write(1, ch, _line.size()) != (int) _line.size()) {
	  perror("write");
	  return false;
   }

   if ((size_t) history_index == m_history.size()) --history_index;
   /* only decrement if we are going beyond the first command (duh) */
   _line = m_history[history_index];
   history_index = (!history_index) ? history_index : history_index - 1;
   /* print the line */
   if (_line.size()) _line.pop_back();
   if (write(1, _line.c_str(), _line.size()) != (int) _line.size()) {
	  perror("write");
	  return false;
   }
}

/** 
 * Handle the down arrow.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_down_arrow(std::string & _line)
{
   if (!m_history.size()) return false;
   if ((size_t) history_index == m_history.size()) return false;
   // Clear input so far
   for (size_t x = 0; x < _line.size(); ++x) {
	  if (!write_with_error(1, "\b", 1)) return false;
   }
  
   for (size_t x = 0; x < _line.size(); ++x) {
	  if (!write_with_error(1, " ", 1)) return false;
   }
  
   for (size_t x = 0; x < _line.size(); ++x) {
	  if (!write_with_error(1, "\b", 1)) return false;
   }
	  
   history_index = ((size_t) history_index == m_history.size()) ? m_history.size()
	  : history_index + 1;
   if ((size_t) history_index == m_history.size()) _line = m_current_line_copy;
   else _line = m_history[history_index];
   if (_line.size() && (size_t) history_index != m_history.size()) _line.pop_back();
   // Print the line
   if (write(1, _line.c_str(), _line.size()) != (int) _line.size()) {
	  perror("write");
	  return false;
   }
}

/** 
 * Handle the right arrow key.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_right_arrow(std::string & _line)
{
   /* Right Arrow Key */
   if (!m_buff.size()) return false;

   char wrt = m_buff.top();
   if (!write(1, &wrt, 1)) {
	  perror("write");
	  return false;
   }
   _line += wrt; m_buff.pop();
}

/** 
 * Handle the left arrow key.
 * 
 * @param _line Current line of text.
 * 
 * @return False to continue, true to break.
 */
bool read_line::handle_left_arrow(std::string & _line)
{
   if (!_line.size()) return false;
   /* Left Arrow Key */

   /* grab width of terminal */
   struct winsize w;
   ioctl(1, TIOCGWINSZ, &w);
   size_t term_width = w.ws_col;

   /* check if we need to go up a line */
   /*  The plus 2 comes from the "$ "  */
   if ((_line.size() == (term_width - 2))) {
	  /* need to go up a line */		   
	  const size_t p_len = strlen("\033[1A\x1b[33;1m$ \x1b[0m");

	  /* now we print the string */
	  if (!write(1, "\033[1A\x1b[33;1m$ \x1b[0m", p_len)) {
		 /**
		  * @todo Make sure you print the correct prompt!
		  */
		 perror("write");
		 return false;
	  } else if (!write(1, _line.c_str(), term_width - 2)) {
		 perror("write");
		 return false;
	  }
   } else if (!((_line.size() + 2) % (term_width))) {
	  /* This case is for more than one line of backtrack */
	  if (!write(1, "\033[1A \b", strlen("\033[1A \b"))) perror("write");
	  else if (!write(1, _line.c_str() - 2 +
			  (term_width  * ((_line.size() - 2)
					  / term_width)), term_width)) {
		 perror("write");
		 return false;
	  }
   }
   m_buff.push(_line.back());
   char bsp = '\b';
   if (!write(1, &bsp, 1)) {
	  perror("write");
	  return false;
   }
   _line.pop_back();
}
