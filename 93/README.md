ae - Anthony's Editor
=====================

## 1.  SYNOPSIS

    ae [-f config_file] [filename]


## 2.  OPERANDS

* `config_file`  
  The pathname of an AE configuration file.  The pathname may be absolute, relative the current directory, or relative the user's home directory.

* `filename`  
  The name of a existing or new file to edit.


## 3.  DESCRIPTION

AE is a simple full screen text editor that can be configured for either a modal (VI-style interface) or a modeless (EMACS-style interface).

Text files consist of lines of printable text or tab characters. A line can be of arbitrary length and is delimited by either a newline or the end of file.  Carriage return is mapped to newline on input and ignored on output.  Carriage returns already in the file are preserved.  Tab stops are every eight columns.


## 4.  COMMANDS

Some default configuration files are supplied.  One of them should be renamed to `ae.rc` (Windows) or `.aerc` (Unix) and placed in the user's home directory.  The supplied files are `mode.rc`, `modeless.rc` (original), and `modeless.xterm` (revised) which configure the editor for either modal (VI-style) or modeless (EMACS-style) operation.


### 4.1.  MODAL CONFIGURATION 

#### 4.1.1.  SUPPORT

    :h              Toggle help on/off.
    :map            Macro add, change, delete, or view.
    :r :w           Read and write file to and from the buffer.
    q Q ZZ          Quit with and without query.
    ^R              Refresh the screen.
    V               Display version.

#### 4.1.2.  MOVEMENT

    h j k l         Left, down, up, right cursor movement.
    b w             Word left, right.
    H L M           Page top, bottom, middle.
    ^F ^B           Page down, page up.
    0 $             Start, end of line.
    1G G            Top, bottom of file.
    I n             Incremental search, next match.

#### 4.1.3.  EDIT

    i ESC ERASE     Enter insert mode, escape to leave, backspace.
    ^V              Next character typed will be treated as a literal.
    X x             Delete character left or under the cursor.
    SPACE           Toggle block on/off.
    v               Cut block to scrap.
    P               Paste scrap into buffer before the cursor.
    u               Undo last cut, delete, paste, read, insert, or undo.
    ~               Invert case of letter and advance.

#### 4.1.4.  MACROS

    a               Append after cursor.
    A               Append at the end of a line.
    C               Change to end of line.
    cw              Change word.
    D               Delete from cursor to end of line.
    dd              Delete line.
    dw              Delete word.
    o O             Open line below or above.
    p               Paste scrap into buffer after the cursor.
    y               Yank current block.


### 4.2.  MODELESS CONFIGURATION

#### 4.2.1.  SUPPORT

    F1              Toggle help on/off.
    ^K^M            Macro add, change, delete, or view.
    ^L              Refresh the screen.
    ^R ^W           Read and write file to and from the buffer.
    ^C ^K^C         Quit with and without query.  
    ^K^V            Display version.


#### 4.2.2.  MOVEMENT

    cursor keys     Left, down, up, right cursor movement (ansi defined).
    ^A ^D           Word left,  word right.
    ^F ^E           Front and end of line.
    ^N ^P           Next and previous page.
    ^T ^B           Top and bottom of file.
    ^@ ^G           Incremental search, next match.


#### 4.2.3.  EDIT

    unbound keys    Insert.
    BACKSPACE ^X    Delete character left or under the cursor.
    ^V              Next character typed will be treated as a literal.
    F2              Toggle block on/off.
    F3              Cut block to scrap.
    F4              Paste scrap into buffer.
    ^U              Undo last cut, paste, read, or undo.
    F5              Invert case of letters.

#### 4.2.4.  MACROS

No pre-defined macros.


## 5.  CONFIGURATION

It is possible to redefine copies of the sample configuration files in order to support key-bindings and text messages of the user's choice.   The user can define an interface similar to either Vi or Emacs, and  support multibyte key sequences.  The text messages can be rephrased or translated into any 8-bit character set.  

The configuration file layout is fairly simple.  All keywords begin on a line starting with a period, eg. .help.  Messages begin with a message number followed by a colon and end with the first unescaped newline.  Invalid keywords and messages are ignored.

The parameters <string> and <character> can be any text other than whitespace (blank, tab, carriage-return, and newline).  It is possible to specify control keys by prefixing the following characters with a caret, eg. ^H :

    @ a b c d e f g h i j k l m n o 
    p q r s t u v w x y z [ \ ] ^ _

The sequence ^? represents ASCII DEL (0x7f).  The following escape constants are recognised:

    backspace       \b          ^H
    formfeed        \f          ^L
    newline         \n          ^J
    return          \r          ^M
    escape          \e          ^[
    tab             \t          ^I
    space           \s

Several of the above control characters tend to be used for input and cannot be remapped.  Many terminal emulators (putty, kitty) may also map CR to LF or LF to CR.  See also the `stty(1)` input settings and special keys as those will have precedence.  It is sometimes possible to use these keys in combination with a prefix, eg. `^K^I` if the need arises.

Numeric escapes are possible.  The value represented must be between 0 and 255 inclusive.

    decimal         \ddd
    octal           \0ooo
    hex             \0xhh

A literal escape begins with a backslash and is followed by any character that does not specify an escape constant or start a  number, and will represent the character itself.

    .insert_enter    i       <-- single character string
    .insert_exit     ^[      <-- defines ASCII ESC
    .delete_right    \0x7f   <-- defines ASCII DEL
    .cursor_up       ^[[A    <-- defines sequence ESC [ A


## 5.1.  KEYWORDS

### 5.1.1.  SUPPORT

* `.file_read <string>`
* `.file_write <string>`  
  Read or write a file to or from a buffer.

* `.help <string>`  
  Toggle the help text and ruler line on and off.

* `.help_off`  
  Disable initial help message at startup.

* `.itself <character>`  
  The following character represents itself.  This is really a redundant keyword since any key not defined by a keyword, automatically represents itself.

* `.macro <string>`  
  Define a macro during an edit session.  The user will be prompted for an input line consisting of zero, one, or two `<strings>` separated by whitespace. Pressing <return> at the prompt, with no input strings, will display the current set of macros definitions and how many slots have been used versus the total number of slots available. One string entered will remove the macro defined to have that string as the left-hand-side.   Two input strings defines a macro, where the first string, when typed, pushes the second string onto an input stack. Macros may be nested.  It is only possible to delete or change macros that appear in the listing of currently defined macros. All other key-bindings cannot be redefined during an edit session.

* `.macro_define`
* `.macro_define <lhs string> <rhs string>`  
  The first case reserves space for one macro definition that may be defined during the edit session.  The other case will  actually define a macro, where the left-hand-side, when typed will push onto an input stack the right-hand-side.  Either case may be used as many times as desired (memory permitting). Macros may be nested.

* `.quit <string>`
* `.quit_ask <string>`  
  Exit the editor.

* `.redraw <string>`  
  Force a screen redraw.

* `.show_version <string>`  
  Display the release information.


### 5.1.2.  CURSOR MOTION

* `.cursor_up <string>`
* `.cursor_down <string>`
* `.cursor_left <string>`
* `.cursor_right <string>`  
  Cursor motion in four directions.  Typically the arrow keys.

* `.file_top <string>`
* `.file_bottom <string>`  
  Move to the top and bottom of the file buffer.

* `.line_left <string>`
* `.line_right <string>`  
  Move to the beginning or end of the line.

* `.page_top <string>`
* `.page_middle <string>`
* `.page_bottom <string>`  
  First, middle, or last line of the page (screen).

* `.page_up <string>`
* `.page_down <string>`  
  Previous or next screen full of text.

* `.word_left <string>`
* `.word_right <string>`  
  Move the cursor to start of the previous or next word. A word is defined as a sequence of alpha and/or numeric characters.

* `.inc_search <string>`
* `.match_next <string>`
  `.inc_search` starts a new incremental search, as characters are typed, the cursor advances and block selection highlights any matching text.  Press `Enter` or `Esc` to end input.  `.match_next` advances to the next matching pattern.

* `.regex_search <string>`
* `.match_next <string>`
  `.regex_search` starts a new extended regular expression search.  On match the cursor advances and block selection highlights the matching text.  Press `Enter` or `Esc` to end input.  `.match_next` advances to the next matching pattern.


### 5.1.3.  EDIT

* `.block <string>`
* `.cut <string>`
* `.paste <string>`  
  Block on/off toggle, cut block, and paste before.

* `.delete_left <string>`
* `.delete_right <string>`  
  Delete character to the left or right of the cursor.  Can cut a selected block.

* `.insert_enter <string>`
* `.insert_exit <string>`  
  Enter and exit insert mode.  The use of `.insert_enter` denotes a modal user interface.  Insert mode does not perform macro expansion.

* `.literal <string>`  
  Next character entered is treated as a literal.

* `.stty_erase`
* `.stty_kill`  
  Declare that the terminal's values for the erase and kill characters should be used in insert mode to backspace-erase, or discard and restart input.

* `.undo <string>`  
  Undo last cut, delete, insert, paste, read, or undo.

* `.flip_case <string>`  
  Invert the case of letters from lower to upper and visa-versa.  When no region is selected, the cursor will advance right one  character position.


## 5.2.  MESSAGES

Each message has the form:

    number : text

Long messages can be continued by escaping the newline with a backslash (\\).  The first unescaped newline terminates the message text and is not included as part of the text.

The following is a list of messages:

* `1:`  
  Help text. See the sample configuration files for an example.

* `2:%s: Terminated successfully.\n`  
  Exit successfully.  %s is the program name.

* `3:%s: Unspecified error.\n`  
  Exit due to an unknown error.  %s is the program name.

* `4:usage: %s [-f <config>] [file]\n`  
  Exit with usage error.  %s is the program name.

* `5:%s: Failed to initialize the screen.\n`  
  Exit because Curses couldn't be initialized.  %s is the program name.

* `6:%s: Problem with configuration file.\n`  
  Exit due to a problem with the configuration file.  %s is the program name.  Possible causes are:

  o   Configuration file not found.  
  o   Invalid control character, ^X, specified.  
  o   Numeric escape not in range 0 .. 255.  

* `7:%s: Failed to allocate required memory.\n`  
  Exit because required memory is not available.  %s is the program name.

* `8:Ok.`  
  No error.
  
* `9:An unknown error occurred.`  
  Internal error.

* `10:No more memory available.`  
  Requests for additional memory to either grow the edit buffer or macro definitions failed.

* `11:File \"%s\" is too big to load.`  
  The file is too large to load into available memory.  %s is the name of the file that could not be loaded.

* `12:Scrap is empty.  Nothing to paste.`  
  An attempt to paste the scrap buffer failed because is was empty.

* `13:Failed to find file \"%s\".`  
  %s is a file that could not be found.

* `14:Failed to open file \"%s\".`  
  %s is a file that could not be opened.

* `15:Failed to close file \"%s\".`  
  %s is a file that could not be closed.

* `16:Failed to read file \"%s\".`  
  A read error occur for a file %s.

* `17:Failed to write file \"%s\".`  
  A write error occur for a file %s.

* `18:Not a portable POSIX file name.`  
  File names must be portable POSIX file names.

* `19:File \"%s\" %ld bytes.`  
  %ld is the current length of the file named by %s in the buffer.

* `20:File \"%s\" %ld bytes saved.`  
  %ld is the length of the file named by %s just saved.
  
* `21:File \"%s\" %ld bytes read.`  
  %ld is the length of the file named by %s just read.

* `22:File \"%s\" modified.`  
  The file named by %s has been modified.

* `23:Invalid control character or \\number not 0..255.`  
  An invalid control character was specified by ^X, or a numeric escape is not in the range 0 .. 255.

* `24:No such macro defined.`  
  The left-hand-side of a macro is not currently defined and so cannot be changed or deleted.

* `25:No more macro space.`  
  All the macro space, allocated in the configuration file, is currently being used.

* `26:Interrupt.`  
  An interrupt occurred.

* `27:<< EOF >>`  
  End of file marker.

* `28:Macro :`  
  Prompt for a macro to define, delete, or list.

* `29:File not saved.  Quit (y/n) ?`  
  Ask the user if he really wants to quit before he has saved his changes.

* `30:[ Press a key to continue. ]`  
  Prompt the user for a key press in order to proceed.

* `31:Read file :`  
  Prompt the user for a file name to read.

* `32:Write file :`  
  Prompt the user for a file name to save the buffer to.

* `33:Write block :`  
  Prompt the user for a file name to save a block of text to.

* `34:\smore\s`  
  Pause output till user responds with either Q, q, or another key.

* `35:\sy\b`  
  Yes response.

* `36:\sn\b`  
  No response.

* `37:\sq\b`  
  Quit response.

* `38:Nothing to undo.`  
  An attempt was made to undo a change when the buffer had not yet been modified.

* `39:Incremental :`
  Incremental search input.

* `40:No match.`
  No match found from wrap-around incremental search.


## 6.  EXIT STATUS

0   Success.
1   Unknown error.
2   Usage error.
3   Understood failure.


## 7.  INSTALLATION 

The source has been know to compile on a wide variety of machines and compilers like BSD and System V Unix with GNU C, PC machines with  WatCom C or Turbo C, and ATARI ST machines with Sozobon C.  Any machine  that provides at least K&R C and a BSD CURSES library (as described by  Ken Arnolds's paper) should have no trouble getting AE to compile.

To build AE on most unix-like systems, type

    make

The supplied makefile is configured for a BSD environment.  Some systems may require that the macros `CC`, `CFLAGS`, `LD`, `LDFLAGS`, and `LIBS` be configured.

The minimum Curses implementation supported is that defined by Kenneth Arnold's paper "Screen Updating and Cursor Movement Optimization: A Library Package".  Some BSD Curses implementations have been noted to omit the  functions `erasechar()`, `killchar()`, and `idlok()`.  For BSD systems with poor Curses implementations, alter the following macro:

    CFLAGS = -O -DBADCURSES

For a System V environment alter the following macros to:

    CFLAGS = -O
    LIBS = -lcurses

If the constants CHUNK or CONFIG are not defined by CFLAGS then the defaults used are

    CHUNK   = 8096L
    CONFIG  = "ae.rc"

CHUNK is the size by which the buffer is expanded when the buffer becomes full while inserting text.  CONFIG is the name of the default  configuration file.  The name chosen aims to satisfy both unix and  personal systems.  Unix aficionados may want to reconfigure this  to `.aerc`.

Most EBCDIC machines use block mode terminals.  This is a problem that has not been addressed and/or tested for.


## 8.  BUGS

No known bugs.


## 9.  REFERENCES

* Craig A. Finseth, "Theory and Practice of Text Editors or A Cookbook For An EMACS",  
  TM-165, MIT Lab. for Computer Science  
  <https://dspace.mit.edu/handle/1721.1/15905>

* Craig A. Finseth, "Craft Of Text Editing", 1991  
  Springer-Verlag, ISBN 0-387-97616-7, ISBN 3-3540-97616-7

* Craig A. Finseth, "Craft Of Text Editing", 1999  
  <http://www.finseth.com/craft/>

* Kernighan & Plauger, "Software Tools in Pascal",  
  Addison-Wesley, 81, chapter 6

* Eugene W. Myers & Webb Miller, "Row-replacement Algorithms for Screen Editors",  
  TR 86-19, Dept. of Computer Science, U. of Arizona

* Eugene W. Myers & Webb Miller, "A simple row-replacement method",  
  TR 86-28, Dept. of Computer Science, U. of Arizona

* Webb Miller, "A Software Tools Sampler", Prentice Hall, 87  
  ISBN 0-13-822305-X, chapter 5

* "Editor 101/102" articles from comp.editors
