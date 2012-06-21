/* Automatically generated by po2tbl.sed from aria.pot.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "libgettext.h"

const struct _msg_ent _msg_tbl[] = {
  {"", 1},
  {"Usage: aria [OPTION]...", 2},
  {"Yet another download tool", 3},
  {"Options:", 4},
  {"  -g, --get URL                    URL to get", 5},
  {"  -d, --dir SAVEDIR                save directory", 6},
  {"  -r, --ref REFERER                referer", 7},
  {"  -u, --url-list FILE              open URL list", 8},
  {"  -c, --crc-list FILE              open CRC list", 9},
  {"  --basket                         Show basket and iconify Aria", 10},
  {"  -v, --version                    print version information", 11},
  {"  -h, --help                       print this help", 12},
  {"  --initrc                         initialize resource files", 13},
  {"\
  --geometry WIDTHxHEIGHT+XOFF+YOFF\n\
                                   set the size and position of main window", 14},
  {"\
  --thread-limit-override NUM      if you want more simultaneous downloads...", 15},
  {"\
Control Options:\n\
 the following options are effective only when another aria is already \
running", 16},
  {"  -s, --start                      start all downloads", 17},
  {"  -p, --stop                       stop all downloads", 18},
  {"  -a, --again                      re-download all items", 19},
  {"  -q, --quit                       quit Aria(with no confirmation)", 20},
  {"  --list-name                      show the name of the current list", 21},
  {"  --list-prev                      switch to the previous list", 22},
  {"  --list-next                      switch to the next list", 23},
  {"  --list-delete                    delete the current list", 24},
  {"  --clear-complete                 clear downloaded items", 25},
  {"  --ls-complete                    show URLs downloaded", 26},
  {"  --ls-ready                       show URLs waiting in queue", 27},
  {"  --ls-error                       show URLs in error", 28},
  {"  --ls-run                         show URLs now downloading", 29},
  {"  --ls-stop                        show URLs stopped", 30},
  {"  --ls-lock                        show URLs locked", 31},
  {"  --ls-all                         show all URLs in queue", 32},
  {"Compiled date: ", 33},
  {"Copyright (C) 2000-2002 Tatsuhiro Tsujikawa", 34},
  {"\
The part of directory browser is a modified code, the original code was \
written by Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front \
Technologies.\n\
The original code is copyrighted by Peter Alm, Mikael Alm, Olle Hallnas, \
Thomas Nilsson and 4Front Technologies.", 35},
  {"\
The part of MD5 checking is a modified code, the original code was written \
for GnuPG, copyrighted 1995, 1996, 1998, 1999 Free Software Foundation, Inc.", 36},
  {"\
This program uses zlib, Copyright (C) 1995-1998 Jean-loup Gailly and Mark \
Adler", 37},
  {"\
This program uses OpenSSL, Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.\
com)", 38},
  {"Status (Ready/Error/Downloading/Stop/Complete/Lock)", 39},
  {"Split?(Yes/No/split Parts)", 40},
  {"Total ", 41},
  {" URLs", 42},
  {"Another Aria is not running. This option makes no effect.", 43},
  {"Reading Server Template file...", 44},
  {"Server Template status : good", 45},
  {"Server Template status : error", 46},
  {"Reading Command list file...", 47},
  {"Command list status : good", 48},
  {"Command list status : error", 49},
  {"Reading Default item config file...", 50},
  {"Default item config status: error >> using default config", 51},
  {"Default item config status: good", 52},
  {"Reading Application config file...", 53},
  {"Application config status: error >> using default config", 54},
  {"Application config status: good", 55},
  {"URL file read error", 56},
  {"URL file opened", 57},
  {"Saved file opened", 58},
  {"CRC file read error", 59},
  {"CRC file opened", 60},
  {"Ready", 61},
  {"A part of '", 62},
  {"' download complete", 63},
  {"<directory>", 64},
  {"Starting split download", 65},
  {"Download stopped", 66},
  {"Error occurred while deleting files", 67},
  {"Wait for ", 68},
  {" seconds", 69},
  {"Download again", 70},
  {"' download aborted", 71},
  {"Executing program: ", 72},
  {"Command line is '", 73},
  {"Exit status is ", 74},
  {"Program exited abnormally", 75},
  {"Good exit status", 76},
  {"Bad exit status", 77},
  {"Exit status ignored", 78},
  {"Executing command entry '", 79},
  {"Concatenating split files...", 80},
  {"Concatenation successful", 81},
  {"Concatenation failed", 82},
  {"Downloading: ", 83},
  {" Error: ", 84},
  {"#List name: ", 85},
  {"Console", 86},
  {"not selected", 87},
  {"Quit program", 88},
  {"Are you sure to quit?", 89},
  {"Limit download speed (in Kbytes/sec) (0.0: unlimited) ", 90},
  {"CRC is ", 91},
  {"CRC matched", 92},
  {"Invalid CRC. The specified CRC is ", 93},
  {"Created", 94},
  {"unknown", 95},
  {"READY", 96},
  {"STOPPED", 97},
  {"Warning: different version number found in file header", 98},
  {"Unknown protocol", 99},
  {"Ignore CRC error", 100},
  {"Checking MD5 checksum", 101},
  {"Calculated MD5 checksum is ", 102},
  {"MD5 checking failed", 103},
  {"Ignore MD5 error", 104},
  {"MD5 checking successful", 105},
  {"IO error occurred", 106},
  {"Can't resolve host", 107},
  {"Socket error", 108},
  {"Connection refused - ", 109},
  {"Time out", 110},
  {"I/O error - ", 111},
  {"File I/O error - maybe wrong save location or disk full?", 112},
  {"Location retrieval failed", 113},
  {"Cannot find embedded URL", 114},
  {"FTP server returned negative response", 115},
  {"Bind error - ", 116},
  {"Listen error - ", 117},
  {"Accept error - ", 118},
  {"Send error - ", 119},
  {"Recv error - ", 120},
  {"General protocol error", 121},
  {"Unknown error occurred", 122},
  {"Copied", 123},
  {"Partial item added", 124},
  {"Connecting to ", 125},
  {"Can't resolve host: ", 126},
  {", reason: ", 127},
  {"Resolving complete", 128},
  {"Canonical name is ", 129},
  {"Connection refused: ", 130},
  {"Connection established", 131},
  {"Unable to truncate the file '", 132},
  {"'. Rollback disabled", 133},
  {"Failed to make directory '", 134},
  {"UseProxy is not supported yet", 135},
  {"Content forbidden", 136},
  {"Content not found", 137},
  {"Server returned 416 status. Assume download was completed", 138},
  {"Server returned 416 status. This file may be already downloaded", 139},
  {"Access denied", 140},
  {"Unexpected error occurred", 141},
  {"Redirection found", 142},
  {"Redirecting to ", 143},
  {"Connection closed", 144},
  {" is already downloaded", 145},
  {"Resume disabled", 146},
  {"Assume download was completed", 147},
  {"Download aborted due to the file size limitation", 148},
  {"Size of local file is larger than remote file's one. Download again", 149},
  {"Checking whether server supports resuming", 150},
  {"Starting normal download instead", 151},
  {"Splitting file...", 152},
  {"File is too small to be divided", 153},
  {"This file is locked. Aborting download", 154},
  {"Starting download at ", 155},
  {" bytes", 156},
  {"Retrieving embedded URL", 157},
  {"Finding hyperlink", 158},
  {"Hyperlink retrieval complete", 159},
  {"No match size", 160},
  {"Failed to uncompress data", 161},
  {"Sorry, cannot resume", 162},
  {"Data connection established", 163},
  {"Cannot enter PASV mode", 164},
  {"Failed to create symbolic link. Ignored", 165},
  {"Failed to list files", 166},
  {"Failed to login", 167},
  {"Specified directory was not found", 168},
  {"MDTM command failed. Anyway, try resuming", 169},
  {"\
Modification time of remote file is newer than local file's one. Resume \
disabled", 170},
  {"Depth of recursion exceeded. This message can be ignored safely", 171},
  {"Try again assuming as directory", 172},
  {"Specified directory or file was not found", 173},
  {"Download abort due to extension. This message can be ignored safely", 174},
  {"Checking whether ftp server supports resuming", 175},
  {"An error occurred, but this error can be ignored safely", 176},
  {"Redirecting to", 177},
  {"' is already downloaded", 178},
  {", its index is ", 179},
  {", its range is from ", 180},
  {"Specified directory is not found", 181},
  {" item(s) cleared", 182},
  {" item(s) cleared, and its file deleted", 183},
  {"Clear items", 184},
  {"Are you sure to delete these items?", 185},
  {"Clear items and delete their files", 186},
  {"Are you sure to delete these items and their files?", 187},
  {" item(s) deleted", 188},
  {"Start", 189},
  {"Stop", 190},
  {"Clear", 191},
  {"Clear and delete files", 192},
  {"Clear CRC", 193},
  {"Clear MD5", 194},
  {"Check CRC", 195},
  {"Check MD5", 196},
  {"Start all", 197},
  {"Stop all", 198},
  {"Clear all", 199},
  {"Clear all CRC", 200},
  {"Clear all MD5", 201},
  {"Start all lists", 202},
  {"Stop all lists", 203},
  {"Clear all lists", 204},
  {"Check downloaded items' CRC", 205},
  {"Check downloaded items' MD5", 206},
  {"Download", 207},
  {"Checking CRC of '", 208},
  {"' successful", 209},
  {"' failed", 210},
  {"Checking MD5 of '", 211},
  {"Check", 212},
  {"Checking...", 213},
  {" item(s) copied", 214},
  {" item(s) cut", 215},
  {" item(s) pasted", 216},
  {"Select all", 217},
  {"Invert selection", 218},
  {"Paste URL list", 219},
  {"Paste URL list with numerical expansion", 220},
  {"Paste CRC list", 221},
  {"Paste MD5 list", 222},
  {"Copy selected items", 223},
  {"Cut selected items", 224},
  {"Paste items", 225},
  {"Search in current list", 226},
  {"Edit", 227},
  {"Search", 228},
  {"Search complete", 229},
  {"Cancel", 230},
  {"String:", 231},
  {"Autosave done", 232},
  {"Open URL list", 233},
  {"Open CRC list", 234},
  {"Open MD5 list", 235},
  {"Find hyperlink", 236},
  {"Open saved list", 237},
  {"Save current list", 238},
  {"Quit with saving lists", 239},
  {"Quit without saving lists", 240},
  {"File", 241},
  {"Error occurred while reading HTML file", 242},
  {"Find Hyperlink", 243},
  {"OK", 244},
  {"Error occurred while reading URL file", 245},
  {"URL list opened", 246},
  {"Error occurred while reading CRC file", 247},
  {"CRC list opened", 248},
  {"Failed to save lists", 249},
  {"lists saved", 250},
  {"Save list", 251},
  {"Failed to load lists", 252},
  {"Saved list opened", 253},
  {"Open Saved list", 254},
  {"item added", 255},
  {"Error", 256},
  {"Invalid URL", 257},
  {"New item", 258},
  {"Add", 259},
  {"Numerical expansion", 260},
  {"The specified name already exists.", 261},
  {"New list created", 262},
  {"New list", 263},
  {"Ok", 264},
  {"Name:", 265},
  {"Rename list", 266},
  {"1 list deleted", 267},
  {"Delete current list", 268},
  {"Are you sure to delete the current list?", 269},
  {"Ascending", 270},
  {"Descending", 271},
  {"Rename current list", 272},
  {"Move left", 273},
  {"Move right", 274},
  {"Previous list", 275},
  {"Next list", 276},
  {"Sort", 277},
  {"Filename", 278},
  {"Extension", 279},
  {"Downloaded size", 280},
  {"Total size", 281},
  {"Progress", 282},
  {"Retry", 283},
  {"Recursive", 284},
  {"Status", 285},
  {"Save dir", 286},
  {"URL", 287},
  {"Shuffle list", 288},
  {"List", 289},
  {"Lock", 290},
  {"Unlock", 291},
  {"Move item up", 292},
  {"Move item down", 293},
  {"Move item to top", 294},
  {"Move item to bottom", 295},
  {"Lock error items", 296},
  {"Clear downloaded item", 297},
  {"Clear locked items", 298},
  {"Clear error items", 299},
  {"Clear stopped items", 300},
  {"Clear ready items", 301},
  {"Clear error, stopped and ready items", 302},
  {"Item", 303},
  {"Clear system log", 304},
  {"Are you sure to clear system log?", 305},
  {"Selected item option", 306},
  {"Default item option for current list", 307},
  {"Apply default item option", 308},
  {"To selected items", 309},
  {"To all items in current list", 310},
  {"All settings", 311},
  {"Ignore save directory", 312},
  {"Ignore save directory and recursive settings", 313},
  {"Default item option for new list", 314},
  {"Application settings", 315},
  {"Download history", 316},
  {"Toggle basket on/off", 317},
  {"Option", 318},
  {"\
Specified directory does not exist.\n\
Do you wish to make it now?", 319},
  {"Select directory", 320},
  {"Invalid Filename", 321},
  {"Invalid CRC", 322},
  {"Invalid MD5", 323},
  {"General", 324},
  {"Default", 325},
  {"Authentication", 326},
  {"Download 1", 327},
  {"Download 2", 328},
  {"Command", 329},
  {"Referer", 330},
  {"Cookie", 331},
  {"Agent", 332},
  {"Proxy", 333},
  {"Recursive 1", 334},
  {"Recursive 2", 335},
  {"Misc", 336},
  {"Mode", 337},
  {"Save as", 338},
  {"Sync with URL", 339},
  {"Save directory", 340},
  {"select", 341},
  {"Use authentication", 342},
  {"User name", 343},
  {"Password", 344},
  {"Delete item when download completes", 345},
  {"Don't delete item if CRC/MD5 is not set", 346},
  {"Don't check CRC/MD5", 347},
  {"Ignore CRC/MD5 error", 348},
  {"Use Content-MD5", 349},
  {"Do not download already existing files", 350},
  {"Do not redownload if resuming is not available", 351},
  {"Time out (sec) ", 352},
  {"Retry (-1: unlimited) ", 353},
  {"Retry interval (sec) ", 354},
  {"Divide file into ", 355},
  {"Rollback (bytes) ", 356},
  {"Do not download the files less than (bytes) ", 357},
  {"Resume", 358},
  {"Always resume", 359},
  {"Resume if remote contents have not been modified", 360},
  {"Do not resume", 361},
  {"Execute the following command after download", 362},
  {"\
Download again if the exit status is not in the following list (delimiter is \
',')", 363},
  {"none", 364},
  {"user defined", 365},
  {"Delete cookie on restart (resume or error)", 366},
  {"Do not send cookie", 367},
  {"Send user defined cookie", 368},
  {"User Agent", 369},
  {"Select randomly", 370},
  {"Use proxy", 371},
  {"Use cache", 372},
  {"Server : Port", 373},
  {"Delete", 374},
  {"Recursive download", 375},
  {"Depth of recursion ", 376},
  {"With hostname directory", 377},
  {"Convert absolute URLs to relative ones", 378},
  {"Force convert", 379},
  {"Delete comments", 380},
  {"Delete JavaScript", 381},
  {"Delete iframe", 382},
  {"No other host", 383},
  {"Do not ascend to parent directory", 384},
  {"Only relative links", 385},
  {"Referer override", 386},
  {"Follow FTP link", 387},
  {"Assume download is completed if resuming is not available", 388},
  {"Add to paste window", 389},
  {"Tags", 390},
  {"href", 391},
  {"src", 392},
  {"background", 393},
  {"codebase", 394},
  {"Extensions to parse", 395},
  {"Domains to ignore", 396},
  {"Domain", 397},
  {"Patterns to download", 398},
  {"Use this filter", 399},
  {"Pattern", 400},
  {"Extensions to ignore", 401},
  {"Retry even if HTTP status is 404", 402},
  {"Retry even if HTTP status is 503", 403},
  {"HTTP status 416 handling", 404},
  {"Redownload", 405},
  {"Do not follow redirection", 406},
  {"Accept compression encodings", 407},
  {"Use Accept-Language request header", 408},
  {"HTTP version ", 409},
  {"Name of the file to use as a pre-written HTML", 410},
  {"index.html", 411},
  {"User defined", 412},
  {"Via HTTP proxy", 413},
  {"Login procedure", 414},
  {"FTP mode", 415},
  {"Retrieve mode", 416},
  {"Don't send QUIT command", 417},
  {"Don't send CWD command, instead use absolute path.", 418},
  {"Retrieve symbolic links as real files", 419},
  {"Selected items option", 420},
  {"Default item option for ", 421},
  {"' option", 422},
  {" item(s) added", 423},
  {"<DEFAULT>", 424},
  {"Application Option", 425},
  {"Confirmation", 426},
  {"Display", 427},
  {"Basket", 428},
  {"Timer", 429},
  {"Server", 430},
  {"Update Command list", 431},
  {"Update successful", 432},
  {"Update failed", 433},
  {"Use Command list", 434},
  {"Choose commands to use", 435},
  {"Comment", 436},
  {"Update Server Template", 437},
  {"Use Server Template", 438},
  {"Choose templates to use", 439},
  {"Template", 440},
  {"Display sizes in human readable format (e.g. 1.2K 234.1M 2.2G)", 441},
  {"Status Icon", 442},
  {"Icon set", 443},
  {"Apply", 444},
  {"error", 445},
  {"cannot read pixmap file", 446},
  {"Bypass paste window", 447},
  {"Pixmap on DND Basket", 448},
  {"Preview", 449},
  {"Pixmap", 450},
  {"Confirm clear", 451},
  {"Confirm delete list", 452},
  {"Confirm exit", 453},
  {"Confirm clear system log", 454},
  {"Use start timer", 455},
  {"Start (hour : min) ", 456},
  {"Use stop timer", 457},
  {"Stop (hour : min) ", 458},
  {"Do not stop downloads when timer expires", 459},
  {"The number of simultaneous downloads for new list ", 460},
  {"Start downloads automatically", 461},
  {"Only current list", 462},
  {"Force download now (stopping other downloads if necessary)", 463},
  {"Ignore error'ed items in \"Start All\" menu", 464},
  {"Ignore these extensions when finding hyperlink", 465},
  {"Autosave (list, history and options) every ", 466},
  {"minute(s)", 467},
  {"Max entry of download history", 468},
  {"Max value of speed limiter", 469},
  {"Execute command when:", 470},
  {"all downloads are stopped or completed", 471},
  {"timer expires", 472},
  {"Command ", 473},
  {"Quit Aria when all downloads are over or stopped by timer", 474},
  {"*", 475},
  {"Downloaded", 476},
  {"Total", 477},
  {"%", 478},
  {"Speed", 479},
  {"Remaining", 480},
  {"Recurse", 481},
  {"Save", 482},
  {"DIVIDED", 483},
  {"READY TO CONCAT", 484},
  {"CRC ERROR", 485},
  {"EXEC ERROR", 486},
  {"DOWNLOAD ERROR", 487},
  {"LOCKED", 488},
  {"DOWNLOADING", 489},
  {"DOWNLOAD COMPLETE", 490},
  {"Paste URL list from clipboard", 491},
  {"Clear item(s)", 492},
  {"Start downloading selected item(s)", 493},
  {"Stop downloading selected item(s)", 494},
  {"Download again selected item(s)", 495},
  {"Item option", 496},
  {"Lock selected item(s)", 497},
  {"Unlock selected item(s)", 498},
  {"Move up", 499},
  {"Move down", 500},
  {"Track download", 501},
  {"Start downloading all items", 502},
  {"Stop downloading all items", 503},
  {"Adjust the number of working thread", 504},
  {"About Aria", 505},
  {"Rollout: ", 506},
  {"Translations:", 507},
  {"(Czech)", 508},
  {"(Spanish)", 509},
  {"(French)", 510},
  {"(German)", 511},
  {"(Hungarian)", 512},
  {"(Italian)", 513},
  {"(Japanese)", 514},
  {"(Polish)", 515},
  {"(Russian)", 516},
  {"(Traditional Chinese)", 517},
  {"Aria Logo:", 518},
  {"Special Thanks:", 519},
  {"    and all who support Aria", 520},
  {"License:", 521},
  {"\
This program is free software; you can redistribute it and/or modify it \
under the terms of the GNU General Public License as published by the Free \
Software Foundation; either version 2 of the License, or (at your option) \
any later version.\n\
\n\
This program is distributed in the hope that it will be useful, but WITHOUT \
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or \
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for \
more details.\n\
\n\
You should have received a copy of the GNU General Public License along with \
this program; if not, write to the Free Software Foundation, Inc., 59 Temple \
Place - Suite 330, Boston, MA 02111-1307, USA.", 522},
  {"Built on:", 523},
  {"Help", 524},
  {"Yes", 525},
  {"No", 526},
  {"The specified list no longer exists.", 527},
  {"Paste to ", 528},
  {"Start downloading when pasted", 529},
  {"Paste", 530},
  {"History URL list", 531},
  {"Date", 532},
  {"Close", 533},
  {"Query", 534},
};

int _msg_tbl_length = 534;