/* This file automatically generated from win32/build/confutils.js */
MODE_PHPIZE=false;
// Utils for configure script
/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2008 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Wez Furlong <wez@thebrainroom.com>                           |
  +----------------------------------------------------------------------+
*/

// $Id: confutils.js,v 1.60.2.1.2.8.2.33 2009-05-29 07:43:07 kalle Exp $

var STDOUT = WScript.StdOut;
var STDERR = WScript.StdErr;
var WshShell = WScript.CreateObject("WScript.Shell");
var FSO = WScript.CreateObject("Scripting.FileSystemObject");
var MFO = null;
var SYSTEM_DRIVE = WshShell.Environment("Process").Item("SystemDrive");
var PROGRAM_FILES = WshShell.Environment("Process").Item("ProgramFiles");
var PROGRAM_FILESx86 = WshShell.Environment("Process").Item("ProgramFiles(x86)");
var VCINSTALLDIR = WshShell.Environment("Process").Item("VCINSTALLDIR");
var DSP_FLAGS = new Array();
var PHP_SRC_DIR=FSO.GetParentFolderName(WScript.ScriptFullName);

/* Store the enabled extensions (summary + QA check) */
var extensions_enabled = new Array();

/* Store the SAPI enabled (summary + QA check) */
var sapi_enabled = new Array();

/* Store the headers to install */
var headers_install = new Array();

/* Mapping CL version > human readable name */
var VC_VERSIONS = new Array();
VC_VERSIONS[1200] = 'MSVC6 (Visual C++ 6.0)';
VC_VERSIONS[1300] = 'MSVC7 (Visual C++ 2002)';
VC_VERSIONS[1310] = 'MSVC7.1 (Visual C++ 2003)';
VC_VERSIONS[1400] = 'MSVC8 (Visual C++ 2005)';
VC_VERSIONS[1500] = 'MSVC9 (Visual C++ 2008)';
VC_VERSIONS[1600] = 'MSVC10 (Visual C++ 2010)';
VC_VERSIONS[1700] = 'MSVC11 (Visual C++ 2012)';
VC_VERSIONS[1800] = 'MSVC12 (Visual C++ 2013)';

var VC_VERSIONS_SHORT = new Array();
VC_VERSIONS_SHORT[1200] = 'VC6';
VC_VERSIONS_SHORT[1300] = 'VC7';
VC_VERSIONS_SHORT[1310] = 'VC7.1';
VC_VERSIONS_SHORT[1400] = 'VC8';
VC_VERSIONS_SHORT[1500] = 'VC9';
VC_VERSIONS_SHORT[1600] = 'VC10';
VC_VERSIONS_SHORT[1700] = 'VC11';
VC_VERSIONS_SHORT[1800] = 'VC12';

if (PROGRAM_FILES == null) {
	PROGRAM_FILES = "C:\\Program Files";
}

if (MODE_PHPIZE) {
	if (!FSO.FileExists("config.w32")) {
		STDERR.WriteLine("Must be run from the root of the extension source");
		WScript.Quit(10);
	}
} else {
	if (!FSO.FileExists("README.GIT-RULES")) {
		STDERR.WriteLine("Must be run from the root of the php source");
		WScript.Quit(10);
	}
}

var CWD = WshShell.CurrentDirectory;

if (typeof(CWD) == "undefined") {
	CWD = FSO.GetParentFolderName(FSO.GetAbsolutePathName("README.GIT-RULES"));
}

/* defaults; we pick up the precise versions from configure.in */
var PHP_VERSION = 5;
var PHP_MINOR_VERSION = 0;
var PHP_RELEASE_VERSION = 0;
var PHP_EXTRA_VERSION = "";
var PHP_VERSION_STRING = "5.0.0";

function get_version_numbers()
{
	var cin = file_get_contents("configure.in");
	
	if (cin.match(new RegExp("PHP_MAJOR_VERSION=(\\d+)"))) {
		PHP_VERSION = RegExp.$1;
	}
	if (cin.match(new RegExp("PHP_MINOR_VERSION=(\\d+)"))) {
		PHP_MINOR_VERSION = RegExp.$1;
	}
	if (cin.match(new RegExp("PHP_RELEASE_VERSION=(\\d+)"))) {
		PHP_RELEASE_VERSION = RegExp.$1;
	}
	PHP_VERSION_STRING = PHP_VERSION + "." + PHP_MINOR_VERSION + "." + PHP_RELEASE_VERSION;

	if (cin.match(new RegExp("PHP_EXTRA_VERSION=\"([^\"]+)\""))) {
		PHP_EXTRA_VERSION = RegExp.$1;
		if (PHP_EXTRA_VERSION.length) {
			PHP_VERSION_STRING += PHP_EXTRA_VERSION;
		}
	}
	DEFINE('PHP_VERSION_STRING', PHP_VERSION_STRING);
}

configure_args = new Array();
configure_subst = WScript.CreateObject("Scripting.Dictionary");

configure_hdr = WScript.CreateObject("Scripting.Dictionary");
build_dirs = new Array();

extension_include_code = "";
extension_module_ptrs = "";

if (!MODE_PHPIZE) {
	get_version_numbers();
}

/* execute a command and return the output as a string */
function execute(command_line)
{
	var e = WshShell.Exec(command_line);
	var ret = "";

	ret = e.StdOut.ReadAll();

//STDOUT.WriteLine("command " + command_line);
//STDOUT.WriteLine(ret);

	return ret;
}

function probe_binary(EXE, what)
{
	// tricky escapes to get stderr redirection to work
	var command = 'cmd /c ""' + EXE;
	if (what == "version") {
		command = command + '" -v"';
	}
	var version = execute(command + '" 2>&1"');

	if (what == "64") {
		if (version.match(/x64/)) {
			return 1;
		}
	} else {
		if (version.match(/(\d+\.\d+(\.\d+)?(\.\d+)?)/)) {
			return RegExp.$1;
		}
	}
	return 0;
}

function condense_path(path)
{
	path = FSO.GetAbsolutePathName(path);

	if (path.substr(0, CWD.length).toLowerCase()
			== CWD.toLowerCase() &&
			(path.charCodeAt(CWD.length) == 92 || path.charCodeAt(CWD.length) == 47)) {
		return path.substr(CWD.length + 1);
	}

	var a = CWD.split("\\");
	var b = path.split("\\");
	var i, j;

	for (i = 0; i < b.length; i++) {
		if (a[i].toLowerCase() == b[i].toLowerCase())
			continue;
		if (i > 0) {
			/* first difference found */
			path = "";
			for (j = 0; j < a.length - i; j++) {
				path += "..\\";
			}
			for (j = i; j < b.length; j++) {
				path += b[j];
				if (j < b.length - 1)
					path += "\\";
			}
			return path;
		}
		/* on a different drive */
		break;
	}
	
	return path;
}

function ConfigureArg(type, optname, helptext, defval)
{
	var opptype = type == "enable" ? "disable" : "without";

	if (defval == "yes" || defval == "yes,shared") {
		this.arg = "--" + opptype + "-" + optname;
		this.imparg = "--" + type + "-" + optname;
	} else {
		this.arg = "--" + type + "-" + optname;
		this.imparg = "--" + opptype + "-" + optname;
	}
	
	this.optname = optname;
	this.helptext = helptext;
	this.defval = defval;
	this.symval = optname.toUpperCase().replace(new RegExp("-", "g"), "_");
	this.seen = false;
	this.argval = defval;
}

function ARG_WITH(optname, helptext, defval)
{
	configure_args[configure_args.length] = new ConfigureArg("with", optname, helptext, defval);
}

function ARG_ENABLE(optname, helptext, defval)
{
	configure_args[configure_args.length] = new ConfigureArg("enable", optname, helptext, defval);
}

function analyze_arg(argval)
{
	var ret = new Array();
	var shared = false;

	if (argval == "shared") {
		shared = true;
		argval = "yes";
	} else if (argval == null) {
		/* nothing */
	} else if (arg_match = argval.match(new RegExp("^shared,(.*)"))) {
		shared = true;
		argval = arg_match[1];
	} else if (arg_match = argval.match(new RegExp("^(.*),shared$"))) {
		shared = true;
		argval = arg_match[1];
	}

	ret[0] = shared;
	ret[1] = argval;
	return ret;
}

function word_wrap_and_indent(indent, text, line_suffix, indent_char)
{
	if (text == null) {
		return "";
	}
	
	var words = text.split(new RegExp("\\s+", "g"));
	var i = 0;
	var ret_text = "";
	var this_line = "";
	var t;
	var space = "";
	var lines = 0;

	if (line_suffix == null) {
		line_suffix = "";
	}

	if (indent_char == null) {
		indent_char = " ";
	}

	for (i = 0; i < indent; i++) {
		space += indent_char;
	}
	
	for (i = 0; i < words.length; i++) {
		if (this_line.length) {
			t = this_line + " " + words[i];
		} else {
			t = words[i];
		}

		if (t.length + indent > 78) {
			if (lines++) {
				ret_text += space;
			}
			ret_text += this_line + line_suffix + "\r\n";
			this_line = "";
		}

		if (this_line.length) {
			this_line += " " + words[i];
		} else {
			this_line = words[i];
		}
	}

	if (this_line.length) {
		if (lines)
			ret_text += space;
		ret_text += this_line;
	}

	return ret_text;
}

function conf_process_args()
{
	var i, j;
	var configure_help_mode = false;
	var analyzed = false;
	var nice = "cscript /nologo configure.js ";
	var disable_all = false;
	
	args = WScript.Arguments;
	for (i = 0; i < args.length; i++) {
		arg = args(i);
		nice += ' "' + arg + '"';
		if (arg == "--help") {
			configure_help_mode = true;
			break;
		}
		if (arg == "--disable-all") {
			disable_all = true;
			continue;
		}

		// If it is --foo=bar, split on the equals sign
		arg = arg.split("=", 2);
		argname = arg[0];
		if (arg.length > 1) {
			argval = arg[1];
		} else {
			argval = null;
		}

		// Find the arg
		found = false;
		for (j = 0; j < configure_args.length; j++) {
			if (argname == configure_args[j].imparg || argname == configure_args[j].arg) {
				found = true;

				arg = configure_args[j];
				arg.seen = true;

				analyzed = analyze_arg(argval);

				/* Force shared when called after phpize */
				if (MODE_PHPIZE) {
					shared = "shared";
				} else {
					shared = analyzed[0];
				}
				argval = analyzed[1];

				if (argname == arg.imparg) {
					/* we matched the implicit, or default arg */
					if (argval == null) {
						argval = arg.defval;
					}
				} else {
					/* we matched the non-default arg */
					if (argval == null) {
						argval = arg.defval == "no" ? "yes" : "no";
					}
				}
				
				arg.argval = argval;
				eval("PHP_" + arg.symval + " = argval;");
				eval("PHP_" + arg.symval + "_SHARED = shared;");
				break;
			}
		}
		if (!found) {
			STDERR.WriteLine("Unknown option " + argname + "; please try configure.js --help for a list of valid options");
			WScript.Quit(2);
		}
	}

	if (configure_help_mode) {
		STDOUT.WriteLine(word_wrap_and_indent(0,
"Options that enable extensions and SAPI will accept \
'yes' or 'no' as a parameter.  They also accept 'shared' \
as a synonym for 'yes' and request a shared build of that \
module.  Not all modules can be built as shared modules; \
configure will display [shared] after the module name if \
can be built that way. \
"
			));
		STDOUT.WriteBlankLines(1);

		// Measure width to pretty-print the output
		max_width = 0;
		for (i = 0; i < configure_args.length; i++) {
			arg = configure_args[i];
			if (arg.arg.length > max_width)
				max_width = arg.arg.length;
		}

		for (i = 0; i < configure_args.length; i++) {
			arg = configure_args[i];

			n = max_width - arg.arg.length;
			pad = "   ";
			for (j = 0; j < n; j++) {
				pad += " ";
			}
			STDOUT.WriteLine("  " + arg.arg + pad + word_wrap_and_indent(max_width + 5, arg.helptext));
		}
		WScript.Quit(1);
	}

	var snapshot_build_exclusions = new Array(
		'debug', 'crt-debug', 'lzf-better-compression',
		 'php-build', 'snapshot-template', 'ereg',
		 'pcre-regex', 'fastcgi', 'force-cgi-redirect',
		 'path-info-check', 'zts', 'ipv6', 'memory-limit',
		 'zend-multibyte', 'fd-setsize', 'memory-manager',
		 't1lib', 'pgi', 'pgo'
		);
	var force;

	// Now set any defaults we might have missed out earlier
	for (i = 0; i < configure_args.length; i++) {
		arg = configure_args[i];
		if (arg.seen)
			continue;
		analyzed = analyze_arg(arg.defval);
		shared = analyzed[0];
		argval = analyzed[1];
		
		// Don't trust a default "yes" answer for a non-core module
		// in a snapshot build
		if (PHP_SNAPSHOT_BUILD != "no" && argval == "yes" && !shared) {

			force = true;
			for (j = 0; j < snapshot_build_exclusions.length; j++) {
				if (snapshot_build_exclusions[j] == arg.optname) {
					force = false;
					break;
				}
			}

			if (force) {
				/* now check if it is a core module */
				force = false;
				for (j = 0; j < core_module_list.length; j++) {
					if (core_module_list[j] == arg.optname) {
						force = true;
						break;
					}
				}

				if (!force) {
					STDOUT.WriteLine("snapshot: forcing " + arg.arg + " shared");
					shared = true;
				}
			}
		}
		
		if (PHP_SNAPSHOT_BUILD != "no" && argval == "no") {
			force = true;
			for (j = 0; j < snapshot_build_exclusions.length; j++) {
				if (snapshot_build_exclusions[j] == arg.optname) {
					force = false;
					break;
				}
			}
			if (force) {
				STDOUT.WriteLine("snapshot: forcing " + arg.optname + " on");
				argval = "yes";
				shared = true;
			}
		}

		if (disable_all) {
			force = true;
			for (j = 0; j < snapshot_build_exclusions.length; j++) {
				if (snapshot_build_exclusions[j] == arg.optname) {
					force = false;
					break;
				}
			}
			if (force) {
				if (arg.defval == '') {
					argval = '';
				} else {
					argval = "no";
				}
				shared = false;
			}
		}

		eval("PHP_" + arg.symval + " = argval;");
		eval("PHP_" + arg.symval + "_SHARED = shared;");
	}

	MFO = FSO.CreateTextFile("Makefile.objects", true);

	STDOUT.WriteLine("Saving configure options to config.nice.bat");
	var nicefile = FSO.CreateTextFile("config.nice.bat", true);
	nicefile.WriteLine(nice +  " %*");
	nicefile.Close();

	AC_DEFINE('CONFIGURE_COMMAND', nice, "Configure line");
}

function DEFINE(name, value)
{
	if (configure_subst.Exists(name)) {
		configure_subst.Remove(name);
	}
	configure_subst.Add(name, value);
}

// Searches a set of paths for a file;
// returns the dir in which the file was found,
// true if it was found in the default env path,
// or false if it was not found at all.
// env_name is the optional name of an env var
// specifying the default path to search
function search_paths(thing_to_find, explicit_path, env_name)
{
	var i, found = false, place = false, file, env;

	STDOUT.Write("Checking for " + thing_to_find + " ... ");

	thing_to_find = thing_to_find.replace(new RegExp("/", "g"), "\\");

	if (explicit_path != null) {
		if (typeof(explicit_path) == "string") {
			explicit_path = explicit_path.split(";");
		}

		for (i = 0; i < explicit_path.length; i++) {
			file = glob(explicit_path[i] + "\\" + thing_to_find);
			if (file) {
				found = true;
				place = file[0];
				place = place.substr(0, place.length - thing_to_find.length - 1);
				break;
			}
		}
	}

	if (!found && env_name != null) {
		env = WshShell.Environment("Process").Item(env_name);
		env = env.split(";");
		for (i = 0; i < env.length; i++) {
			file = glob(env[i] + "\\" + thing_to_find);
			if (file) {
				found = true;
				place = true;
				break;
			}
		}
	}

	if (found && place == true) {
		STDOUT.WriteLine(" <in default path>");
	} else if (found) {
		STDOUT.WriteLine(" " + place);
	} else {
		STDOUT.WriteLine(" <not found>");
	}
	return place;
}

function PATH_PROG(progname, additional_paths, symbol)
{
	var exe;
	var place;
	var cyg_path = PHP_CYGWIN + "\\bin;" + PHP_CYGWIN + "\\usr\\local\\bin";
	var php_build_bin_path = PHP_PHP_BUILD + "\\bin"

	exe = progname + ".exe";

	if (additional_paths == null) {
		additional_paths = cyg_path;
	} else {
		additional_paths += ";" + cyg_path;
	}

	additional_paths = additional_paths + ";" + php_build_bin_path;

	place = search_paths(exe, additional_paths, "PATH");

	if (place == true) {
		place = exe;
	} else if (place != false) {
		place = place + "\\" + exe;
	}

	if (place) {
		if (symbol == null) {
			symbol = progname.toUpperCase();
		}
		DEFINE(symbol, place);
	}
	return place;
}

function find_pattern_in_path(pattern, path)
{
	if (path == null) {
		return false;
	}

	var dirs = path.split(';');
	var i;
	var items;

	for (i = 0; i < dirs.length; i++) {
		items = glob(dirs[i] + "\\" + pattern);
		if (items) {
			return condense_path(items[0]);
		}
	}
	return false;
}

function CHECK_LIB(libnames, target, path_to_check, common_name)
{
	STDOUT.Write("Checking for library " + libnames + " ... ");

	if (common_name == null && target != null) {
		common_name = target;
	}

	if (path_to_check == null) {
		path_to_check = "";
	}

	// if they specified a common name for the package that contains
	// the library, tag some useful defaults on to the end of the
	// path to be searched
	if (common_name != null) {
		path_to_check += ";" + PHP_PHP_BUILD + "\\" + common_name + "*";
		path_to_check += ";" + PHP_PHP_BUILD + "\\lib\\" + common_name + "*";
		path_to_check += ";..\\" + common_name + "*";
	}

	// Determine target for build flags
	if (target == null) {
		target = "";
	} else {
		target = "_" + target.toUpperCase();
	}

	// Expand path to include general dirs
	path_to_check += ";" + php_usual_lib_suspects;

	// It is common practice to put libs under one of these dir names
	var subdirs = new Array(PHP_DEBUG == "yes" ? "Debug" : (PHP_DEBUG_PACK == "yes"?"Release_Dbg":"Release"), "lib", "libs", "libexec");

	// libnames can be ; separated list of accepted library names
	libnames = libnames.split(';');

	// for debug builds, lib may have _debug appended, we want that first
	if (PHP_DEBUG == "yes") {
		var length = libnames.length;
		for (var i = 0; i < length; i++) {
			var name = new String(libnames[i]);
			rExp = /.lib$/i;
			name = name.replace(rExp,"_debug.lib");
			libnames.unshift(name);
		}
	}

	var i, j, k, libname;
	var location = false;
	var path = path_to_check.split(';');
	
	for (i = 0; i < libnames.length; i++) {
		libname = libnames[i];

		for (k = 0; k < path.length; k++) {
			location = glob(path[k] + "\\" + libname);
			if (location) {
				location = location[0];
				break;
			}
			for (j = 0; j < subdirs.length; j++) {
				location = glob(path[k] + "\\" + subdirs[j] + "\\" + libname);
				if (location) {
					location = location[0];
					break;
				}
			}
			if (location)
				break;
		}

		if (location) {
			location = condense_path(location);
			var libdir = FSO.GetParentFolderName(location);
			libname = FSO.GetFileName(location);
			ADD_FLAG("LDFLAGS" + target, '/libpath:"' + libdir + '" ');
			ADD_FLAG("LIBS" + target, libname);

			STDOUT.WriteLine(location);

			return location;
		}

		// Check in their standard lib path
		location = find_pattern_in_path(libname, WshShell.Environment("Process").Item("LIB"));

		if (location) {
			location = condense_path(location);
			libname = FSO.GetFileName(location);
			ADD_FLAG("LIBS" + target, libname);

			STDOUT.WriteLine("<in LIB path> " + libname);
			return location;
		}

		// Check in their general extra libs path
		location = find_pattern_in_path(libname, PHP_EXTRA_LIBS);
		if (location) {
			location = condense_path(location);
			libname = FSO.GetFileName(location);
			ADD_FLAG("LIBS" + target, libname);
			STDOUT.WriteLine("<in extra libs path>");
			return location;
		}
	}

	STDOUT.WriteLine("<not found>");

	return false;
}

function OLD_CHECK_LIB(libnames, target, path_to_check)
{
	if (target == null) {
		target = "";
	} else {
		target = "_" + target.toUpperCase();
	}
	
	if (path_to_check == null) {
		path_to_check = php_usual_lib_suspects;
	} else {
		path_to_check += ";" + php_usual_lib_suspects;
	}
	var have = 0;
	var p;
	var i;
	var libname;

	var subdir = PHP_DEBUG == "yes" ? "Debug" : (PHP_DEBUG_PACK == "yes"?"Release_Dbg":"Release");

	libnames = libnames.split(';');
	for (i = 0; i < libnames.length; i++) {
		libname = libnames[i];
		p = search_paths(libname, path_to_check, "LIB");

		if (!p) {
			p = search_paths(subdir + "\\" + libname, path_to_check, "LIB");
			if (p) {
				p += "\\" + subdir;
			}
		}

		if (typeof(p) == "string") {
			ADD_FLAG("LDFLAGS" + target, '/libpath:"' + p + '" ');
			ADD_FLAG("LIBS" + target, libname);
			have = 1;
		} else if (p == true) {
			ADD_FLAG("LIBS" + target, libname);
			have = 1;
		} else {
			/* not found in the defaults or the explicit paths,
			 * so check the general extra libs; if we find
			 * it here, no need to add another /libpath: for it as we
			 * already have it covered, but we need to add the lib
			 * to LIBS_XXX */
			if (false != search_paths(libname, PHP_EXTRA_LIBS, null)) {
				ADD_FLAG("LIBS" + target, libname);
				have = 1;
			}
		}

		if (have) {
			break;
		}
	}

//	AC_DEFINE("HAVE_" + header_name.toUpperCase().replace(new RegExp("/\\\\-\.", "g"), "_"), have);

	return have;

}

function CHECK_FUNC_IN_HEADER(header_name, func_name, path_to_check, add_to_flag)
{
	var c = false;
	var sym;

	STDOUT.Write("Checking for " + func_name + " in " + header_name + " ... ");

	c = GREP_HEADER(header_name, func_name, path_to_check);

	sym = func_name.toUpperCase();
	sym = sym.replace(new RegExp("[\\\\/\.-]", "g"), "_");

	if (typeof(add_to_flag) == "undefined") {
		AC_DEFINE("HAVE_" + sym, c ? 1 : 0);
	} else {
		ADD_FLAG(add_to_flag, "/DHAVE_" + sym + "=" + (c ? "1" : "0"));
	}

	if (c) {
		STDOUT.WriteLine("OK");
		return c;
	}
	STDOUT.WriteLine("No");
	return false;	
}

function GREP_HEADER(header_name, regex, path_to_check)
{
	var c = false;

	if (FSO.FileExists(path_to_check + "\\" + header_name)) {
		c = file_get_contents(path_to_check + "\\" + header_name);
	}

	if (!c) {
		/* look in the include path */

		var p = search_paths(header_name, path_to_check, "INCLUDE");
		if (typeof(p) == "string") {
			c = file_get_contents(p);
		} else if (p == false) {
			p = search_paths(header_name, PHP_EXTRA_INCLUDES, null);
			if (typeof(p) == "string") {
				c = file_get_contents(p);
			}
		} 
		if (!c) {
			return false;
		}
	}

	if (typeof(regex) == "string") {
		regex = new RegExp(regex);
	}

	if (c.match(regex)) {
		/* caller can now use RegExp.$1 etc. to get at patterns */
		return true;
	}
	return false;
}

function CHECK_HEADER_ADD_INCLUDE(header_name, flag_name, path_to_check, use_env, add_dir_part, add_to_flag_only)
{
	var dir_part_to_add = "";
	
	if (use_env == null) {
		use_env = true;
	}

	// if true, add the dir part of the header_name to the include path
	if (add_dir_part == null) {
		add_dir_part = false;
	} else if (add_dir_part) {
		var basename = FSO.GetFileName(header_name);
		dir_part_to_add = "\\" + header_name.substr(0, header_name.length - basename.length - 1);
	}

	if (path_to_check == null) {
		path_to_check = php_usual_include_suspects;
	} else {
		path_to_check += ";" + php_usual_include_suspects;
	}
	
	var p = search_paths(header_name, path_to_check, use_env ? "INCLUDE" : null);
	var have = 0;
	var sym;

	if (typeof(p) == "string") {
		ADD_FLAG(flag_name, '/I "' + p + dir_part_to_add + '" ');
	} else if (p == false) {
		/* not found in the defaults or the explicit paths,
		 * so check the general extra includes; if we find
		 * it here, no need to add another /I for it as we
		 * already have it covered, unless we are adding
		 * the dir part.... */
		p = search_paths(header_name, PHP_EXTRA_INCLUDES, null);
		if (typeof(p) == "string" && add_dir_part) {
			ADD_FLAG(flag_name, '/I "' + p + dir_part_to_add + '" ');
		}
	} 
	have = p ? 1 : 0

	sym = header_name.toUpperCase();
	sym = sym.replace(new RegExp("[\\\\/\.-]", "g"), "_");

	if (typeof(add_to_flag_only) == "undefined" &&
			flag_name.match(new RegExp("^CFLAGS_(.*)$"))) {
		add_to_flag_only = true;
	}

	if (typeof(add_to_flag_only) != "undefined") {
		ADD_FLAG(flag_name, "/DHAVE_" + sym + "=" + have);
	} else {
		AC_DEFINE("HAVE_" + sym, have, "have the " + header_name + " header file");
	}

	return p;
}

function generate_version_info_manifest(makefiletarget)
{
	var manifest_name = makefiletarget + ".manifest";

	if (MODE_PHPIZE) {
		MFO.WriteLine("$(BUILD_DIR)\\" + manifest_name + ": " + PHP_DIR + "\\build\\default.manifest");
		MFO.WriteLine("\t@copy " + PHP_DIR + "\\build\\default.manifest $(BUILD_DIR)\\" + makefiletarget + ".manifest");
	} else {
		MFO.WriteLine("$(BUILD_DIR)\\" + manifest_name + ": win32\\build\\default.manifest");
		MFO.WriteLine("\t@copy $(PHP_SRC_DIR)\\win32\\build\\default.manifest $(BUILD_DIR)\\" + makefiletarget + ".manifest");
	}

	return manifest_name;
}

/* emits rule to generate version info for a SAPI
 * or extension.  Returns the name of the .res file
 * that will be generated */
function generate_version_info_resource(makefiletarget, basename, creditspath, sapi)
{
	var resname = makefiletarget + ".res";
	var res_desc = makefiletarget;
	var res_prod_name = "PHP " + makefiletarget;
	var credits;
	var thanks = "";
	var logo = "";
	var debug = "";
	var project_url = "http://www.php.net";
	var project_header = creditspath + "/php_" + basename + ".h";
	var versioning = "";

	if (sapi) {
		var internal_name = basename.toUpperCase() + " SAPI";
	} else {
		var internal_name = basename.toUpperCase() + " extension";
	}

	if (FSO.FileExists(creditspath + '/CREDITS')) {
		credits = FSO.OpenTextFile(creditspath + '/CREDITS', 1);
		res_desc = credits.ReadLine();
		try {
			thanks = credits.ReadLine();
		} catch (e) {
			thanks = null;
		}
		if (thanks == null) {
			thanks = "";
		} else {
			thanks = "Thanks to " + thanks;
		}
		credits.Close();
	}

	if (creditspath.match(new RegExp("pecl"))) {
		/* PECL project url - this will eventually work correctly for all */
		project_url = "http://pecl.php.net/" + basename;

		/* keep independent versioning PECL-specific for now */
		if (FSO.FileExists(project_header)) {
			if (header = FSO.OpenTextFile(project_header, 1)) {
				contents = header.ReadAll();
				/* allowed: x.x.x[a|b|-alpha|-beta][RCx][-dev] */
				if (contents.match(new RegExp('PHP_' + basename.toUpperCase() + '_VERSION(\\s+)"((\\d+\.\\d+(\.\\d+)?)((a|b)(\\d)?|\-[a-z]{3,5})?(RC\\d+)?(\-dev)?)'))) {
					project_version = RegExp.$2;
					file_version = RegExp.$3.split('.');
					if (!file_version[2]) {
						file_version[2] = 0;
					}
					versioning = '\\"" /d EXT_FILE_VERSION=' + file_version[0] + ',' + file_version[1] + ',' + file_version[2] + ' /d EXT_VERSION="\\"' + project_version;
				}
				header.Close();
			}
		}
	}

	if (makefiletarget.match(new RegExp("\\.exe$"))) {
		logo = " /d WANT_LOGO ";
	}

	if (PHP_DEBUG != "no") {
		debug = " /d _DEBUG";
	}

	/**
	 * Use user supplied template.rc if it exists
	 */
	if (FSO.FileExists(creditspath + '\\template.rc')) {
		MFO.WriteLine("$(BUILD_DIR)\\" + resname + ": " + creditspath + "\\template.rc");
		MFO.WriteLine("\t$(RC) /fo $(BUILD_DIR)\\" + resname + logo + debug +
			' /d FILE_DESCRIPTION="\\"' + res_desc + '\\"" /d FILE_NAME="\\"' +
			makefiletarget + '\\"" /d PRODUCT_NAME="\\"' + res_prod_name +
			versioning + '\\"" /d THANKS_GUYS="\\"' + thanks + '\\"" ' +
			creditspath + '\\template.rc');
		return resname;
	}
	if (MODE_PHPIZE) {
		MFO.WriteLine("$(BUILD_DIR)\\" + resname + ": $(PHP_DIR)\\build\\template.rc");
		MFO.WriteLine("\t$(RC)  /I $(PHP_DIR)/include /n /fo $(BUILD_DIR)\\" + resname + logo + debug +
			' /d FILE_DESCRIPTION="\\"' + res_desc + '\\"" /d FILE_NAME="\\"'
			+ makefiletarget + '\\"" /d URL="\\"' + project_url + 
			'\\"" /d INTERNAL_NAME="\\"' + internal_name + versioning + 
			'\\"" /d THANKS_GUYS="\\"' + thanks + '\\"" $(PHP_DIR)\\build\\template.rc');
	} else {
		MFO.WriteLine("$(BUILD_DIR)\\" + resname + ": win32\\build\\template.rc");
		MFO.WriteLine("\t$(RC) /n /fo $(BUILD_DIR)\\" + resname + logo + debug +
			' /d FILE_DESCRIPTION="\\"' + res_desc + '\\"" /d FILE_NAME="\\"'
			+ makefiletarget + '\\"" /d URL="\\"' + project_url + 
			'\\"" /d INTERNAL_NAME="\\"' + internal_name + versioning + 
			'\\"" /d THANKS_GUYS="\\"' + thanks + '\\"" win32\\build\\template.rc');
	}
	MFO.WriteBlankLines(1);
	return resname;
}

/* Check if PGO is enabled for given module. To disable PGO for a particular module,
define a global variable by the following name scheme before SAPI() or EXTENSION() call
	var PHP_MYMODULE_PGO = false; */
function is_pgo_desired(mod)
{
	var varname = "PHP_" + mod.toUpperCase() + "_PGO";

	/* XXX enable PGO in phpize mode */
	if (MODE_PHPIZE) {
		return false;
	}

	/* don't disable if there's no mention of the varname */
	if (eval("typeof " + varname + " == 'undefined'")) {
		return true;
	}

	return eval("!!" + varname);
}

function SAPI(sapiname, file_list, makefiletarget, cflags, obj_dir)
{
	var SAPI = sapiname.toUpperCase();
	var ldflags;
	var resname;
	var ld;
	var manifest;

	if (typeof(obj_dir) == "undefined") {
		sapiname_for_printing = configure_module_dirname;
	} else {
		sapiname_for_printing = configure_module_dirname + " (via " + obj_dir + ")";
	}

	STDOUT.WriteLine("Enabling SAPI " + sapiname_for_printing);

	MFO.WriteBlankLines(1);
	MFO.WriteLine("# objects for SAPI " + sapiname);
	MFO.WriteBlankLines(1);

	if (cflags) {
		ADD_FLAG('CFLAGS_' + SAPI, cflags);
	}

	ADD_SOURCES(configure_module_dirname, file_list, sapiname, obj_dir);
	MFO.WriteBlankLines(1);
	MFO.WriteLine("# SAPI " + sapiname);
	MFO.WriteBlankLines(1);

	/* generate a .res file containing version information */
	resname = generate_version_info_resource(makefiletarget, sapiname, configure_module_dirname, true);
	
	manifest_name = generate_version_info_manifest(makefiletarget);

	MFO.WriteLine(makefiletarget + ": $(BUILD_DIR)\\" + makefiletarget);
	MFO.WriteLine("\t@echo SAPI " + sapiname_for_printing + " build complete");
	if (MODE_PHPIZE) {
		MFO.WriteLine("$(BUILD_DIR)\\" + makefiletarget + ": $(DEPS_" + SAPI + ") $(" + SAPI + "_GLOBAL_OBJS) $(PHPLIB) $(BUILD_DIR)\\" + resname + " $(BUILD_DIR)\\" + manifest_name);
	} else {
		MFO.WriteLine("$(BUILD_DIR)\\" + makefiletarget + ": $(DEPS_" + SAPI + ") $(" + SAPI + "_GLOBAL_OBJS) $(BUILD_DIR)\\$(PHPLIB) $(BUILD_DIR)\\" + resname + " $(BUILD_DIR)\\" + manifest_name);
	}

	if (makefiletarget.match(new RegExp("\\.dll$"))) {
		ldflags = "/dll $(LDFLAGS)";
		manifest = "-@$(_VC_MANIFEST_EMBED_DLL)";
	} else if (makefiletarget.match(new RegExp("\\.lib$"))) {
		ldflags = "$(LDFLAGS)";
		ld = "$(MAKE_LIB)";
	} else {
		ldflags = "$(LDFLAGS)";
		manifest = "-@$(_VC_MANIFEST_EMBED_EXE)";
	}
	
	if(is_pgo_desired(sapiname) && (PHP_PGI == "yes" || PHP_PGO != "no")) {
		// Add compiler and link flags if PGO options are selected
		if (PHP_DEBUG != "yes" && PHP_PGI == "yes") {
			ADD_FLAG('CFLAGS_' + SAPI, "/GL /O2");
			ADD_FLAG('LDFLAGS_' + SAPI, "/LTCG:PGINSTRUMENT");
		}
		else if (PHP_DEBUG != "yes" && PHP_PGO != "no") {
			ADD_FLAG('CFLAGS_' + SAPI, "/GL /O2");
			ADD_FLAG('LDFLAGS_' + SAPI, "/LTCG:PGUPDATE");
		}

		ldflags += " /PGD:$(PGOPGD_DIR)\\" + makefiletarget.substring(0, makefiletarget.indexOf(".")) + ".pgd";
	}

	if (MODE_PHPIZE) {
		if (ld) {
			MFO.WriteLine("\t" + ld + " /nologo /out:$(BUILD_DIR)\\" + makefiletarget + " " + ldflags + " $(" + SAPI + "_GLOBAL_OBJS) $(PHPLIB) $(LDFLAGS_" + SAPI + ") $(LIBS_" + SAPI + ") $(BUILD_DIR)\\" + resname);
		} else {
			ld = "@$(CC)";
			MFO.WriteLine("\t" + ld + " /nologo " + " $(" + SAPI + "_GLOBAL_OBJS) $(PHPLIB) $(LIBS_" + SAPI + ") $(BUILD_DIR)\\" + resname + " /link /out:$(BUILD_DIR)\\" + makefiletarget + " " + ldflags + " $(LDFLAGS_" + SAPI + ")");
		}
	} else {
		if (ld) {
			MFO.WriteLine("\t" + ld + " /nologo /out:$(BUILD_DIR)\\" + makefiletarget + " " + ldflags + " $(" + SAPI + "_GLOBAL_OBJS) $(BUILD_DIR)\\$(PHPLIB) $(LDFLAGS_" + SAPI + ") $(LIBS_" + SAPI + ") $(BUILD_DIR)\\" + resname);
		} else {
			ld = "@$(CC)";
			MFO.WriteLine("\t" + ld + " /nologo " + " $(" + SAPI + "_GLOBAL_OBJS) $(BUILD_DIR)\\$(PHPLIB) $(LIBS_" + SAPI + ") $(BUILD_DIR)\\" + resname + " /link /out:$(BUILD_DIR)\\" + makefiletarget + " " + ldflags + " $(LDFLAGS_" + SAPI + ")");
		}
	}

	if (manifest) {
		MFO.WriteLine("\t" + manifest);
	}
		
	DEFINE('CFLAGS_' + SAPI + '_OBJ', '$(CFLAGS_' + SAPI + ')');

	if (configure_module_dirname.match("pecl")) {
		ADD_FLAG("PECL_TARGETS", makefiletarget);
	} else {
		ADD_FLAG("SAPI_TARGETS", makefiletarget);
	}

	if (PHP_DSP != "no") {
		generate_dsp_file(sapiname, configure_module_dirname, file_list, false);
	}

	MFO.WriteBlankLines(1);
	sapi_enabled[sapi_enabled.length] = [sapiname];
}

function ADD_DIST_FILE(filename)
{
	if (configure_module_dirname.match("pecl")) {
		ADD_FLAG("PECL_EXTRA_DIST_FILES", filename);
	} else {
		ADD_FLAG("PHP_EXTRA_DIST_FILES", filename);
	}
}	

function file_get_contents(filename)
{
	var f, c;
	try {
		f = FSO.OpenTextFile(filename, 1);
		c = f.ReadAll();
		f.Close();
		return c;
	} catch (e) {
		STDOUT.WriteLine("Problem reading " + filename);
		return false;
	}
}

// Add a dependency on another extension, so that
// the dependencies are built before extname
function ADD_EXTENSION_DEP(extname, dependson, optional)
{
	var EXT = extname.toUpperCase();
	var DEP = dependson.toUpperCase();
	var dep_present = false;
	var dep_shared = false;

	try {
		dep_present = eval("PHP_" + DEP);

		if (dep_present != "no") {
			try {
				dep_shared = eval("PHP_" + DEP + "_SHARED");
			} catch (e) {
				dep_shared = false;
			}
		}

	} catch (e) {
		dep_present = "no";
	}

	if (optional) {
		if (dep_present == "no") {
			MESSAGE("\t" + dependson + " not found: " + dependson + " support in " + extname + " disabled");
			return false;
		}
	}

	var ext_shared = eval("PHP_" + EXT + "_SHARED");

	if (dep_shared) {
		if (!ext_shared) {
			if (optional) {
				MESSAGE("\tstatic " + extname + " cannot depend on shared " + dependson + ": " + dependson + "support disabled");
				return false;
			}
			ERROR("static " + extname + " cannot depend on shared " + dependson);
		}

		ADD_FLAG("LIBS_" + EXT, "php_" + dependson + ".lib");
		if (MODE_PHPIZE) {
			ADD_FLAG("LDFLAGS_" + EXT, "/libpath:$(BUILD_DIR_DEV)\\lib");
			ADD_FLAG("DEPS_" + EXT, "$(BUILD_DIR_DEV)\\lib\\php_" + dependson + ".lib");
		} else {
			ADD_FLAG("LDFLAGS_" + EXT, "/libpath:$(BUILD_DIR)");
			ADD_FLAG("DEPS_" + EXT, "$(BUILD_DIR)\\php_" + dependson + ".lib");
		}

	} else {

		if (dep_present == "no") {
			if (ext_shared) {
				WARNING(extname + " cannot be built: missing dependency, " + dependson + " not found");

				var dllname = ' php_' + extname + '.dll';

				if (!REMOVE_TARGET(dllname, 'EXT_TARGETS')) {
					REMOVE_TARGET(dllname, 'PECL_TARGETS');
				}

				return false;

			}

			ERROR("Cannot build " + extname + "; " + dependson + " not enabled");
			return false;
		}
	} // dependency is statically built-in to PHP
	return true;
}

var static_pgo_enabled = false;

function EXTENSION(extname, file_list, shared, cflags, dllname, obj_dir)
{
	var objs = null;
	var EXT = extname.toUpperCase();
	var extname_for_printing;
	var ldflags;

	if (shared == null) {
		eval("shared = PHP_" + EXT + "_SHARED;");
	} else {
		eval("PHP_" + EXT + "_SHARED = shared;");
	}

	if (cflags == null) {
		cflags = "";
	}

	if (typeof(obj_dir) == "undefined") {
		extname_for_printing = configure_module_dirname;
	} else {
		extname_for_printing = configure_module_dirname + " (via " + obj_dir + ")";
	}

	if (shared) {
		STDOUT.WriteLine("Enabling extension " + extname_for_printing + " [shared]");
		cflags = "/D COMPILE_DL_" + EXT + " /D " + EXT + "_EXPORTS=1 " + cflags;
		ADD_FLAG("CFLAGS_PHP", "/D COMPILE_DL_" + EXT);
	} else {
		STDOUT.WriteLine("Enabling extension " + extname_for_printing);
	}

	MFO.WriteBlankLines(1);
	MFO.WriteLine("# objects for EXT " + extname);
	MFO.WriteBlankLines(1);

	ADD_SOURCES(configure_module_dirname, file_list, extname, obj_dir);
	
	MFO.WriteBlankLines(1);

	if (shared) {
		if (dllname == null) {
			dllname = "php_" + extname + ".dll";
		}
		var libname = dllname.substring(0, dllname.length-4) + ".lib";

		var resname = generate_version_info_resource(dllname, extname, configure_module_dirname, false);
		var ld = "@$(CC)";
		var manifest_name = generate_version_info_manifest(dllname);

		ldflags = "";
		if (is_pgo_desired(extname) && (PHP_PGI == "yes" || PHP_PGO != "no")) {
			// Add compiler and link flags if PGO options are selected
			if (PHP_DEBUG != "yes" && PHP_PGI == "yes") {
				ADD_FLAG('LDFLAGS_' + EXT, "/LTCG:PGINSTRUMENT");
			}
			else if (PHP_DEBUG != "yes" && PHP_PGO != "no") {
				ADD_FLAG('LDFLAGS_' + EXT, "/LTCG:PGUPDATE");
			}

			ADD_FLAG('CFLAGS_' + EXT, "/GL /O2");

			ldflags = " /PGD:$(PGOPGD_DIR)\\" + dllname.substring(0, dllname.indexOf(".")) + ".pgd";
		}

		MFO.WriteLine("$(BUILD_DIR)\\" + libname + ": $(BUILD_DIR)\\" + dllname);
		MFO.WriteBlankLines(1);
		if (MODE_PHPIZE) {
			MFO.WriteLine("$(BUILD_DIR)\\" + dllname + ": $(DEPS_" + EXT + ") $(" + EXT + "_GLOBAL_OBJS) $(PHPLIB) $(BUILD_DIR)\\" + resname + " $(BUILD_DIR)\\" + manifest_name);
			MFO.WriteLine("\t" + ld + " $(" + EXT + "_GLOBAL_OBJS) $(PHPLIB) $(LIBS_" + EXT + ") $(LIBS) $(BUILD_DIR)\\" + resname + " /link /out:$(BUILD_DIR)\\" + dllname + " $(DLL_LDFLAGS) $(LDFLAGS) $(LDFLAGS_" + EXT + ")");
		} else {
			MFO.WriteLine("$(BUILD_DIR)\\" + dllname + ": $(DEPS_" + EXT + ") $(" + EXT + "_GLOBAL_OBJS) $(BUILD_DIR)\\$(PHPLIB) $(BUILD_DIR)\\" + resname + " $(BUILD_DIR)\\" + manifest_name);
			MFO.WriteLine("\t" + ld + " $(" + EXT + "_GLOBAL_OBJS) $(BUILD_DIR)\\$(PHPLIB) $(LIBS_" + EXT + ") $(LIBS) $(BUILD_DIR)\\" + resname + " /link /out:$(BUILD_DIR)\\" + dllname + ldflags + " $(DLL_LDFLAGS) $(LDFLAGS) $(LDFLAGS_" + EXT + ")");
		}
		MFO.WriteLine("\t-@$(_VC_MANIFEST_EMBED_DLL)");
		MFO.WriteBlankLines(1);

		if (configure_module_dirname.match("pecl")) {
			ADD_FLAG("PECL_TARGETS", dllname);
		} else {
			ADD_FLAG("EXT_TARGETS", dllname);
		}
		MFO.WriteLine(dllname + ": $(BUILD_DIR)\\" + dllname);
		MFO.WriteLine("\t@echo EXT " + extname + " build complete");
		MFO.WriteBlankLines(1);
		
		DEFINE('CFLAGS_' + EXT + '_OBJ', '$(CFLAGS_' + EXT + ')');
	} else {
		ADD_FLAG("STATIC_EXT_OBJS", "$(" + EXT + "_GLOBAL_OBJS)");
		ADD_FLAG("STATIC_EXT_LIBS", "$(LIBS_" + EXT + ")");
		ADD_FLAG("STATIC_EXT_LDFLAGS", "$(LDFLAGS_" + EXT + ")");
		ADD_FLAG("STATIC_EXT_CFLAGS", "$(CFLAGS_" + EXT + ")");
		if (is_pgo_desired(extname) && (PHP_PGI == "yes" || PHP_PGO != "no")) {
			if (!static_pgo_enabled) {
				if (PHP_DEBUG != "yes" && PHP_PGI == "yes") {
					ADD_FLAG('STATIC_EXT_LDFLAGS', "/LTCG:PGINSTRUMENT");
				}
				else if (PHP_DEBUG != "yes" && PHP_PGO != "no") {
					ADD_FLAG('STATIC_EXT_LDFLAGS', "/LTCG:PGUPDATE");
				}

				ADD_FLAG("STATIC_EXT_CFLAGS", "/GL /O2");
				static_pgo_enabled = true;
			}
		}

		/* find the header that declares the module pointer,
		 * so we can include it in internal_functions.c */
		var ext_dir = FSO.GetFolder(configure_module_dirname);
		var fc = new Enumerator(ext_dir.Files);
		var re = /\.h$/;
		var s, c;
		for (; !fc.atEnd(); fc.moveNext()) {
			s = fc.item() + "";
			if (s.match(re)) {
				c = file_get_contents(s);
				if (c.match("phpext_")) {
					extension_include_code += '#include "' + configure_module_dirname + '/' + FSO.GetFileName(s) + '"\r\n';
				}
			}
		}
	
		extension_module_ptrs += '\tphpext_' + extname + '_ptr,\r\n';
	
		DEFINE('CFLAGS_' + EXT + '_OBJ', '$(CFLAGS_PHP) $(CFLAGS_' + EXT + ')');
	}
	if (MODE_PHPIZE && FSO.FileExists(PHP_DIR + "/include/main/config.pickle.h")) {
		cflags = "/FI main/config.pickle.h " + cflags;
	}
	ADD_FLAG("CFLAGS_" + EXT, cflags);

	if (PHP_DSP != "no") {
		generate_dsp_file(extname, configure_module_dirname, file_list, shared);
	}

	extensions_enabled[extensions_enabled.length] = [extname, shared ? 'shared' : 'static'];
}

function ADD_SOURCES(dir, file_list, target, obj_dir)
{
	var i;
	var tv;
	var src, obj, sym, flags;

	if (target == null) {
		target = "php";
	}

	sym = target.toUpperCase() + "_GLOBAL_OBJS";
	flags = "CFLAGS_" + target.toUpperCase() + '_OBJ';

	if (configure_subst.Exists(sym)) {
		tv = configure_subst.Item(sym);
	} else {
		tv = "";
	}

	file_list = file_list.split(new RegExp("\\s+"));
	file_list.sort();

	var re = new RegExp("\.[a-z0-9A-Z]+$");

	dir = dir.replace(new RegExp("/", "g"), "\\");
	var objs_line = "";
	var srcs_line = "";

	var sub_build = "$(BUILD_DIR)\\";

	/* if module dir is not a child of the main source dir,
	 * we need to tweak it; we should have detected such a
	 * case in condense_path and rewritten the path to
	 * be relative.
	 * This probably breaks for non-sibling dirs, but that
	 * is not a problem as buildconf only checks for pecl
	 * as either a child or a sibling */
	if (obj_dir == null) {
		var build_dir = dir.replace(new RegExp("^..\\\\"), "");
		var mangle_dir = build_dir.replace(new RegExp("[\\\\/.-]", "g"), "_");
		var bd_flags_name = "CFLAGS_BD_" + mangle_dir.toUpperCase();
	}
	else {
		var build_dir = obj_dir.replace(new RegExp("^..\\\\"), "");
		var mangle_dir = build_dir.replace(new RegExp("[\\\\/.-]", "g"), "_");
		var bd_flags_name = "CFLAGS_BD_" + mangle_dir.toUpperCase();
	}
	
	var dirs = build_dir.split("\\");
	var i, d = "";
	for (i = 0; i < dirs.length; i++) {
		d += dirs[i];
		build_dirs[build_dirs.length] = d;
		d += "\\";
	}
	sub_build += d;


	DEFINE(bd_flags_name, " /Fd" + sub_build + " /Fp" + sub_build + " /FR" + sub_build + " ");

	for (i in file_list) {
		src = file_list[i];
		obj = src.replace(re, ".obj");
		tv += " " + sub_build + obj;

		if (!MODE_PHPIZE && PHP_ONE_SHOT == "yes") {
			if (i > 0) {
				objs_line += " " + sub_build + obj;	
				srcs_line += " " + dir + "\\" + src;
			} else {
				objs_line = sub_build + obj;	
				srcs_line = dir + "\\" + src;
			}
		} else {
			MFO.WriteLine(sub_build + obj + ": " + dir + "\\" + src);

			if (PHP_ANALYZER == "pvs") {
				MFO.WriteLine("\t@\"$(PVS_STUDIO)\" --cl-params $(" + flags + ") $(CFLAGS) $(" + bd_flags_name + ") /c " + dir + "\\" + src + " --source-file "  + dir + "\\" + src
					+ " --cfg PVS-Studio.conf --errors-off \"V122 V117 V111\" ");
			}
			MFO.WriteLine("\t@$(CC) $(" + flags + ") $(CFLAGS) $(" + bd_flags_name + ") /c " + dir + "\\" + src + " /Fo" + sub_build + obj);
		}
	}

	if (!MODE_PHPIZE && PHP_ONE_SHOT == "yes") {
		MFO.WriteLine(objs_line + ": " + srcs_line);
		MFO.WriteLine("\t$(CC) $(" + flags + ") $(CFLAGS) /Fo" + sub_build + " $(" + bd_flags_name + ") /c " + srcs_line);
	}

	DEFINE(sym, tv);
}

function REMOVE_TARGET(dllname, flag)
{
	var dllname = dllname.replace(/\s/g, "");
	var EXT = dllname.replace(/php_(\S+)\.dll/, "$1").toUpperCase();
	var php_flags = configure_subst.Item("CFLAGS_PHP");

	if (configure_subst.Exists(flag)) {
		var targets = configure_subst.Item(flag);

		if (targets.match(dllname)) {
			configure_subst.Remove(flag);
			targets = targets.replace(dllname, "");
			targets = targets.replace(/\s+/, " ");
			targets = targets.replace(/\s$/, "");
			configure_subst.Add(flag, targets);
			configure_hdr.Add("HAVE_" + EXT, new Array(0, ""));
			configure_subst.Item("CFLAGS_PHP") = php_flags.replace(" /D COMPILE_DL_" + EXT, "");
			extensions_enabled.pop();
			return true;
		}
	}
	return false;
}

function generate_internal_functions()
{
	var infile, outfile;
	var indata;

	STDOUT.WriteLine("Generating main/internal_functions.c");
	
	infile = FSO.OpenTextFile("main/internal_functions.c.in", 1);
	indata = infile.ReadAll();
	infile.Close();
	
	indata = indata.replace("@EXT_INCLUDE_CODE@", extension_include_code);
	indata = indata.replace("@EXT_MODULE_PTRS@", extension_module_ptrs);

	if (FSO.FileExists("main/internal_functions.c")) {
		var origdata = file_get_contents("main/internal_functions.c");

		if (origdata == indata) {
			STDOUT.WriteLine("\t[content unchanged; skipping]");
			return;
		}
	}

	outfile = FSO.CreateTextFile("main/internal_functions.c", true);
	outfile.Write(indata);
	outfile.Close();
}

function output_as_table(header, ar_out)
{
	var l = header.length;
	var cols = 80;
	var fixedlength = "";
	var t = 0;
	var i,j,k,m;
	var out = "| ";
	var min = new Array(l);
	var max = new Array(l);

	if (!!ar_out[0] && l != ar_out[0].length) {
		STDOUT.WriteLine("Invalid header argument, can't output the table " + l + " " + ar_out[0].length  );
		return;
	}

	for (j=0; j < l; j++) {
		var tmax, tmin;

		/*Figure out the max length per column */
		tmin = 0;
		tmax = 0;
		for (k = 0; k < ar_out.length; k++) {
			var t = ar_out[k][j].length;
			if (t > tmax) tmax = t;
			else if (t < tmin) tmin = t;
		}
		if (tmax > header[j].length) {
			max[j] = tmax;
		} else {
			max[j] = header[j].length;
		}
		if (tmin < header[j].length) {
			min[j] = header[j].length;
		}
	}

	sep = "";
	k = 0;
	for (i = 0; i < l; i++) {
		k += max[i] + 3;
	}
	k++;

	for (j=0; j < k; j++) {
		sep += "-";
	}

	STDOUT.WriteLine(sep);
	out = "|";
	for (j=0; j < l; j++) {
		out += " " + header[j];
		for (var i = 0; i < (max[j] - header[j].length); i++){
			out += " ";
		}
		out += " |";
	}
	STDOUT.WriteLine(out);

	STDOUT.WriteLine(sep);

	out = "|";
	for (i=0; i < ar_out.length; i++) {
		line = ar_out[i];
		for (j=0; j < l; j++) {
			out += " " + line[j];
			for (var k = 0; k < (max[j] - line[j].length); k++){
				out += " ";
			}
			out += " |";
		}
		STDOUT.WriteLine(out);
		out = "|";
	}

	STDOUT.WriteLine(sep);
}

function write_summary()
{
	var ar = new Array();

	STDOUT.WriteBlankLines(2);

	STDOUT.WriteLine("Enabled extensions:");
	output_as_table(["Extension", "Mode"], extensions_enabled.sort());
	STDOUT.WriteBlankLines(2);
	if (!MODE_PHPIZE) {
		STDOUT.WriteLine("Enabled SAPI:");
		output_as_table(["Sapi Name"], sapi_enabled);
		STDOUT.WriteBlankLines(2);
	}
	ar[0] = ['Build type', PHP_DEBUG == "yes" ? "Debug" : "Release"];
	ar[1] = ['Thread Safety', PHP_ZTS == "yes" ? "Yes" : "No"];
	ar[2] = ['Compiler', VC_VERSIONS[VCVERS]];
	ar[3] = ['Architecture', X64 ? 'x64' : 'x86'];
	if (PHP_PGO == "yes") {
		ar[4] = ['Optimization', "PGO"];
	} else if (PHP_PGI == "yes") {
		ar[4] = ['Optimization', "PGI"];
	} else {
		ar[4] = ['Optimization', PHP_DEBUG == "yes" ? "disabled" : "PGO disabled"];
	}
	if (PHP_ANALYZER == "vs") {
		ar[5] = ['Static analyzer', 'Visual Studio'];
	} else if (PHP_ANALYZER == "pvs") {
		ar[5] = ['Static analyzer', 'PVS-Studio'];
	} else {
		ar[5] = ['Static analyzer', 'disabled'];
	}

	output_as_table(["",""], ar);
	STDOUT.WriteBlankLines(2);
}

function generate_files()
{
	var i, dir, bd, last;

	STDOUT.WriteBlankLines(1);
	STDOUT.WriteLine("Creating build dirs...");
	dir = get_define("BUILD_DIR");
	build_dirs.sort();
	last = null;

	if (!FSO.FolderExists(dir)) {
		FSO.CreateFolder(dir);
	}

	for (i = 0; i < build_dirs.length; i++) {
		bd = FSO.BuildPath(dir, build_dirs[i]);
		if (bd == last) {
			continue;
		}
		last = bd;

		build_dir = get_define('BUILD_DIR');
		build_dir = build_dir.replace(new RegExp("\\\\", "g"), "\\\\");
		if (build_dir.substr(build_dir.Length - 2, 2) != '\\\\') {
			build_dir += '\\\\';
		}
		ADD_FLAG("BUILD_DIRS_SUB", bd.replace(new RegExp(build_dir), ''));

		if (!FSO.FolderExists(bd)) {
			FSO.CreateFolder(bd);
		}
	}

	if (PHP_DSP != "no") {
		generate_dsp_file("TSRM", "TSRM", null, false);
		generate_dsp_file("Zend", "Zend", null, false);
		generate_dsp_file("win32", "win32", null, false);
		generate_dsp_file("main", "main", null, false);
		generate_dsp_file("streams", "main\\streams", null, false);
		copy_dsp_files();
	}

	STDOUT.WriteLine("Generating files...");
	generate_makefile();
	if (!MODE_PHPIZE) {
		generate_internal_functions();
		generate_config_h();
		generate_phpize();
	} else {
		generate_config_pickle_h();
		generate_ext_pickle();
	}
	STDOUT.WriteLine("Done.");
	STDOUT.WriteBlankLines(1);
	write_summary();

	if (PHP_SNAPSHOT_BUILD != "no") {
		STDOUT.WriteLine("Type 'nmake snap' to build a PHP snapshot");
	} else {
		STDOUT.WriteLine("Type 'nmake' to build PHP");
	}
}

function generate_ext_pickle()
{
	var content;
	var DEPS = null;
	var dest;
	var deps_lines = new Array();

	var build_var_name = function(name) {
		return "PHP_" + name.toUpperCase();
	}

	STDOUT.WriteLine("Generating pickle deps");
	dest = PHP_DIR + "/script/";

	if (!FSO.FolderExists(dest)) {
		FSO.CreateFolder(dest);
	}

	if (FSO.FileExists(dest + "/ext_pickle.js")) {
		DEPS = FSO.OpenTextFile(dest + "/ext_pickle.js", 1);

		while (!DEPS.AtEndOfStream) {
			var ln = DEPS.ReadLine();
			var found = false;

			for (var i in extensions_enabled) {
				var reg0 = new RegExp(build_var_name(extensions_enabled[i][0]) + "\s*=.+", "g");
				var reg1 = new RegExp(build_var_name(extensions_enabled[i][0]) + "_SHARED" + "\s*=.+", "g");

				if (ln.match(reg1) || ln.match(reg0)) {
					found = true;
					break;
				}
			}

			if (!found) {
				deps_lines.push(ln);
			}
		}
	}

	for (var i in extensions_enabled) {
		deps_lines.push(build_var_name(extensions_enabled[i][0]) + "=true;");
		deps_lines.push(build_var_name(extensions_enabled[i][0]) + "_SHARED=" + (extensions_enabled[i][1] == 'shared' ? 'true' : 'false') + ";");
	}

	if (!!DEPS) {
		DEPS.Close();
		DEPS = null;
	}

	/* Replace the ext_pickle.js with the new content */
	DEPS = FSO.CreateTextFile(dest + "/ext_pickle.js", true);

	for (var j in deps_lines) {
		DEPS.WriteLine(deps_lines[j]);
	}

	DEPS.Close();
}

function generate_config_pickle_h()
{
	var outfile = null;
	var lines = new Array();
	var keys = (new VBArray(configure_hdr.Keys())).toArray();
	dest = PHP_DIR + "/include/main";

	var ignore_key = function(key) {
		var ignores = [ "CONFIGURE_COMMAND", "PHP_COMPILER_ID", "COMPILER", "ARCHITECTURE", "HAVE_STRNLEN", "PHP_DIR" ];

		for (var k in ignores) {
			if (ignores[k] == key) {
				return true;
			}
		}

		return false;
	}


	STDOUT.WriteLine("Generating main/config.pickle.h");

	if (FSO.FileExists(dest + "/config.pickle.h")) {
		outfile = FSO.OpenTextFile(dest + "/config.pickle.h", 1);

		while (!outfile.AtEndOfStream) {
			var found = false;
			var ln = outfile.ReadLine();

			for (var i in keys) {
				var reg = new RegExp("#define[\s ]+" + keys[i] + "[\s ]*.*", "g");

				if (ln.match(reg)) {
					found = true;
					break;
				}
			}

			if (!found) {
				lines.push(ln);
			}
		}
	}

	for (var i in keys) {
		var item = configure_hdr.Item(keys[i]);

		if (ignore_key(keys[i])) {
			continue;
		}

		/* XXX fix comment handling */
		/*if (!lines[j].match(/^#define.+/g)) {
			continue;
		}*/

		lines.push("#define " + keys[i] + " " + item[0]);
	}

	if (outfile) {
		outfile.Close();
		outfile = null;
	}

	outfile = FSO.CreateTextFile(dest + "/config.pickle.h", true);

	for (var k in lines) {
		outfile.WriteLine(lines[k]);
	}

	outfile.Close();
}

function generate_config_h()
{
	var infile, outfile;
	var indata;
	var prefix;

	prefix = PHP_PREFIX.replace(new RegExp("\\\\", "g"), "\\\\");

	STDOUT.WriteLine("Generating main/config.w32.h");
	
	infile = FSO.OpenTextFile("win32/build/config.w32.h.in", 1);
	indata = infile.ReadAll();
	infile.Close();
	
	outfile = FSO.CreateTextFile("main/config.w32.h", true);

	indata = indata.replace(new RegExp("@PREFIX@", "g"), prefix);
	outfile.Write(indata);

	var keys = (new VBArray(configure_hdr.Keys())).toArray();
	var i, j;
	var item;
	var pieces, stuff_to_crack, chunk;

	outfile.WriteBlankLines(1);
	outfile.WriteLine("/* values determined by configure.js */");

	for (i in keys) {
		item = configure_hdr.Item(keys[i]);
		outfile.WriteBlankLines(1);
		pieces = item[0];

		if (item[1] != undefined) {
			outfile.WriteLine("/* " + item[1] + " */");
		}

		if (typeof(pieces) == "string" && pieces.charCodeAt(0) == 34) {
			/* quoted string have a maximal length of 2k under vc.
			 * solution is to crack them and let the compiler concat
			 * them implicitly */
			stuff_to_crack = pieces;
			pieces = "";

			while (stuff_to_crack.length) {
				j = 65;
				while (stuff_to_crack.charCodeAt(j) != 32 && j < stuff_to_crack.length)
					j++;

				chunk = stuff_to_crack.substr(0, j);
				pieces += chunk;
				stuff_to_crack = stuff_to_crack.substr(chunk.length);
				if (stuff_to_crack.length)
					pieces += '" "';
			}
		}
		
		outfile.WriteLine("#define " + keys[i] + " " + pieces);
	}
	
	outfile.Close();
}

function generate_phpize()
{
	STDOUT.WriteLine("Generating phpize");
	dest = get_define("BUILD_DIR") + '/devel';

	if (!FSO.FolderExists(dest)) {
		FSO.CreateFolder(dest);
	}

	var MF = FSO.CreateTextFile(dest + "/phpize.js", true);
	var DEPS = FSO.CreateTextFile(dest + "/ext_deps.js", true);
	
	prefix = get_define("PHP_PREFIX");
	prefix = prefix.replace(new RegExp("/", "g"), "\\");
	prefix = prefix.replace(new RegExp("\\\\", "g"), "\\\\");
	MF.WriteLine("var PHP_PREFIX=" + '"' + prefix + '"');
	MF.WriteLine("var PHP_ZTS=" + '"' + (PHP_ZTS.toLowerCase() == "yes" ? "Yes" : "No") + '"');
	MF.WriteLine("var VC_VERSION=" + VCVERS);
	MF.WriteLine("var PHP_VERSION=" + PHP_VERSION);
	MF.WriteLine("var PHP_MINOR_VERSION=" + PHP_MINOR_VERSION);
	MF.WriteLine("var PHP_RELEASE_VERSION=" + PHP_RELEASE_VERSION);
	MF.WriteBlankLines(1);
	MF.WriteLine("/* Genereted extensions list with mode (static/shared) */");

	var count = extensions_enabled.length;
	for (i in extensions_enabled) {
		out = "PHP_" + extensions_enabled[i][0].toUpperCase() + "_SHARED=" + (extensions_enabled[i][1] == 'shared' ? 'true' : 'false') + ";";
		DEPS.WriteLine("PHP_" + extensions_enabled[i][0].toUpperCase() + "=true;");
		DEPS.WriteLine(out);
		MF.WriteLine(out);
	}

	MF.WriteBlankLines(2);
	MF.WriteLine("/* Genereted win32/build/phpize.js.in */");
	MF.WriteBlankLines(1);
	MF.Write(file_get_contents("win32/build/phpize.js.in"));
	MF.Close();
	DEPS.Close();

	/* Generate flags file */
	/* spit out variable definitions */
	CJ = FSO.CreateTextFile(dest + "/config.phpize.js");

	CJ.WriteLine("var PHP_ZTS =" + '"' + PHP_ZTS + '"');
	CJ.WriteLine("var PHP_DLL_LIB =" + '"' + get_define('PHPLIB') + '"');
	CJ.WriteLine("var PHP_DLL =" + '"' + get_define('PHPDLL') + '"');

	/* The corresponding configure options aren't enabled through phpize,
		thus these dummy declarations are required. */
	CJ.WriteLine("var PHP_ANALYZER =" + '"no"');
	CJ.WriteLine("var PHP_PGO =" + '"no"');
	CJ.WriteLine("var PHP_PGI =" + '"no"');
	CJ.WriteLine("var PHP_ALL_SHARED =" + '"no"');

	CJ.WriteBlankLines(1);
	CJ.Close();
}

function generate_makefile()
{
	STDOUT.WriteLine("Generating Makefile");
	var MF = FSO.CreateTextFile("Makefile", true);

	MF.WriteLine("# Generated by configure.js");
	/* spit out variable definitions */
	var keys = (new VBArray(configure_subst.Keys())).toArray();
	var i;
	MF.WriteLine("PHP_SRC_DIR =" + PHP_SRC_DIR);
	for (i in keys) {
		// The trailing space is needed to prevent the trailing backslash
		// that is part of the build dir flags (CFLAGS_BD_XXX) from being
		// seen as a line continuation character
		MF.WriteLine(keys[i] + "=" + 
			//word_wrap_and_indent(1, configure_subst.Item(keys[i]), ' \\', '\t') + " "
			configure_subst.Item(keys[i]) + " "
			);
		MF.WriteBlankLines(1);
	}

	MF.WriteBlankLines(1);
	if (MODE_PHPIZE) {
		var TF = FSO.OpenTextFile(PHP_DIR + "/script/Makefile.phpize", 1);
	} else {
		var TF = FSO.OpenTextFile("win32/build/Makefile", 1);
	}

	MF.Write(TF.ReadAll());

	MF.WriteLine("build-headers:");
	MF.WriteLine("	@if not exist $(BUILD_DIR_DEV)\\include mkdir $(BUILD_DIR_DEV)\\include >nul");
	MF.WriteLine("	@for %D in ($(INSTALL_HEADERS_DIR)) do @if not exist $(BUILD_DIR_DEV)\\include\\%D mkdir $(BUILD_DIR_DEV)\\include\\%D >nul");
	for (i in headers_install) {
		if (headers_install[i][2] != "") {
				MF.WriteLine("	@if not exist $(BUILD_DIR_DEV)\\include\\" + headers_install[i][2] + " mkdir $(BUILD_DIR_DEV)\\include\\" + 
												headers_install[i][2] + ">nul");
				MF.WriteLine("	@copy " + headers_install[i][0] + " " + "$(BUILD_DIR_DEV)\\include\\" + headers_install[i][2] + " /y >nul");
		}
	}
	MF.WriteLine("	@for %D in ($(INSTALL_HEADERS_DIR)) do @copy %D*.h $(BUILD_DIR_DEV)\\include\\%D /y >nul");
	if (MODE_PHPIZE) {
		MF.WriteBlankLines(1);
		MF.WriteLine("build-bins:");
		for (var i in extensions_enabled) {
			var lib = "php_" + extensions_enabled[i][0] + ".lib";
			var dll = "php_" + extensions_enabled[i][0] + ".dll";
			MF.WriteLine("	@copy $(BUILD_DIR)\\" + lib + " $(BUILD_DIR_DEV)\\lib");
			MF.WriteLine("  @if not exist $(PHP_PREFIX) mkdir $(PHP_PREFIX) >nul");
			MF.WriteLine("	@copy $(BUILD_DIR)\\" + dll + " $(PHP_PREFIX)");
		}
	} else {
		MF.WriteBlankLines(1);
		MF.WriteLine("build-ext-libs:");
		MF.WriteLine("	@if not exist $(BUILD_DIR_DEV)\\lib mkdir $(BUILD_DIR_DEV)\\lib >nul");
		for (var i in extensions_enabled) {
			var lib;

			lib = "php_" + extensions_enabled[i][0] + "*.lib";

			if ('shared' == extensions_enabled[i][1]) {
				MF.WriteLine("	@if exist $(BUILD_DIR)\\" + lib + " copy $(BUILD_DIR)\\" + lib + " $(BUILD_DIR_DEV)\\lib");
			}
		}
	}
	TF.Close();

	MF.WriteBlankLines(2);

	MFO.Close();
	TF = FSO.OpenTextFile("Makefile.objects", 1);
	if (!TF.AtEndOfStream) {
		MF.Write(TF.ReadAll());
	}
	TF.Close();

	MF.Close();	
}

function ADD_FLAG(name, flags, target)
{
	if (target != null) {
		name = target.toUpperCase() + "_" + name;
	}
	if (configure_subst.Exists(name)) {
		var curr_flags = configure_subst.Item(name);

		if (curr_flags.indexOf(flags) >= 0) {
			return;
		}
		
		flags = curr_flags + " " + flags;
		configure_subst.Remove(name);
	}
	configure_subst.Add(name, flags);

	if (PHP_DSP != "no") {
		if (flags && (name.substr(name.length-3) != "PHP") && (name.substr(0, 7) == "CFLAGS_")) {
			DSP_FLAGS[DSP_FLAGS.length] = new Array(name, flags);
		}
	}
}

function get_define(name)
{
	if (configure_subst.Exists(name)) {
		return configure_subst.Item(name);
	}
	return "";
}

// Add a .def to the core to export symbols
function ADD_DEF_FILE(name)
{
	if (!configure_subst.Exists("PHPDEF")) {
		DEFINE("PHPDEF", "$(BUILD_DIR)\\$(PHPDLL).def");
		ADD_FLAG("PHP_LDFLAGS", "/def:$(PHPDEF)");
	}
	ADD_FLAG("PHP_DLL_DEF_SOURCES", name);
}

function AC_DEFINE(name, value, comment, quote)
{
	if (quote == null) {
		quote = true;
	}
	if (quote && typeof(value) == "string") {
		value = '"' + value.replace(new RegExp('(["\\\\])', "g"), '\\$1') + '"';
	} else if (value.length == 0) {
		value = '""';
	}
	var item = new Array(value, comment);
	if (configure_hdr.Exists(name)) {
		var orig_item = configure_hdr.Item(name);
		STDOUT.WriteLine("AC_DEFINE[" + name + "]=" + value + ": is already defined to " + orig_item[0]);
	} else {
		configure_hdr.Add(name, item);
	}
}

function MESSAGE(msg)
{
	STDOUT.WriteLine("" + msg);
}

function ERROR(msg)
{
	STDERR.WriteLine("ERROR: " + msg);
	WScript.Quit(3);
}

function WARNING(msg)
{
	STDERR.WriteLine("WARNING: " + msg);
	STDERR.WriteBlankLines(1);
}

function copy_and_subst(srcname, destname, subst_array)
{
	if (!FSO.FileExists(srcname)) {
		srcname = configure_module_dirname + "\\" + srcname;
		destname = configure_module_dirname + "\\" + destname;
	}

	var content = file_get_contents(srcname);
	var i;

	for (i = 0; i < subst_array.length; i+=2) {
		var re = subst_array[i];
		var rep = subst_array[i+1];

		content = content.replace(re, rep);
	}
	
	var f = FSO.CreateTextFile(destname, true);
	f.Write(content);
	f.Close();
}

// glob using simple filename wildcards
// returns an array of matches that are found
// in the filesystem
function glob(path_pattern)
{
	var path_parts = path_pattern.replace(new RegExp("/", "g"), "\\").split("\\");
	var p;
	var base = "";
	var is_pat_re = /\*/;

//STDOUT.WriteLine("glob: " + path_pattern);

	if (FSO.FileExists(path_pattern)) {
		return new Array(path_pattern);
	}
	
	// first, build as much as possible that doesn't have a pattern
	for (p = 0; p < path_parts.length; p++) {
		if (path_parts[p].match(is_pat_re))
			break;
		if (p)
			base += "\\";
		base += path_parts[p];	
	}

	return _inner_glob(base, p, path_parts);
}

function _inner_glob(base, p, parts)
{
	var pat = parts[p];
	var full_name = base + "\\" + pat;
	var re = null;
	var items = null;

	if (p == parts.length) {
		return false;
	}

//STDOUT.WriteLine("inner: base=" + base + " p=" + p + " pat=" + pat);

	if (FSO.FileExists(full_name)) {
		if (p < parts.length - 1) {
			// we didn't reach the full extent of the pattern
			return false;
		}
		return new Array(full_name);
	}

	if (FSO.FolderExists(full_name) && p == parts.length - 1) {
		// we have reached the end of the pattern; no need to recurse
		return new Array(full_name);
	}

	// Convert the pattern into a regexp
	re = new RegExp("^" + pat.replace(/\./g, '\\.').replace(/\*/g, '.*').replace(/\?/g, '.') + "$", "i");

	items = new Array();

	if (!FSO.FolderExists(base)) {
		return false;
	}

	var folder = FSO.GetFolder(base);
	var fc = null;
	var subitems = null;
	var item_name = null;
	var j;

	fc = new Enumerator(folder.SubFolders);
	for (; !fc.atEnd(); fc.moveNext()) {
		item_name = FSO.GetFileName(fc.item());

		if (item_name.match(re)) {
			// got a match; if we are at the end of the pattern, just add these
			// things to the items array
			if (p == parts.length - 1) {
				items[items.length] = fc.item();
			} else {
				// we should recurse and do more matches
				subitems = _inner_glob(base + "\\" + item_name, p + 1, parts);
				if (subitems) {
					for (j = 0; j < subitems.length; j++) {
						items[items.length] = subitems[j];
					}
				}
			}
		}
	}

	// if we are at the end of the pattern, we should match
	// files too
	if (p == parts.length - 1) {
		fc = new Enumerator(folder.Files);
		for (; !fc.atEnd(); fc.moveNext()) {
			item_name = FSO.GetFileName(fc.item());
			if (item_name.match(re)) {
				items[items.length] = fc.item();
			}
		}
	}

	if (items.length == 0)
		return false;

	return items;
}

function PHP_INSTALL_HEADERS(dir, headers_list)
{
	headers_list = headers_list.split(new RegExp("\\s+"));
	headers_list.sort();
	if (dir.length > 0 && dir.substr(dir.length - 1) != '/' && dir.substr(dir.length - 1) != '\\') {
		dir += '/';
	}
	dir = dir.replace(new RegExp("/", "g"), "\\");

	for (i in headers_list) {
		found = false;
		src = headers_list[i];
		src = src.replace(new RegExp("/", "g"), "\\");
		isdir = FSO.FolderExists(dir + src);
		isfile = FSO.FileExists(dir + src);
		if (isdir) {
			if (src.length > 0 && src.substr(src.length - 1) != '/' && src.substr(src.length - 1) != '\\') {
				src += '\\';
			}
			headers_install[headers_install.length] = [dir + src, 'dir',''];
			ADD_FLAG("INSTALL_HEADERS_DIR", dir + src);
			found = true;
		} else if (isfile) {
			dirname = FSO.GetParentFolderName(dir + src);
			headers_install[headers_install.length] = [dir + src, 'file', dirname];
			ADD_FLAG("INSTALL_HEADERS", dir + src);
			found = true;
		} else {
			path =  configure_module_dirname + "\\"+ src;
			isdir = FSO.FolderExists(path);
			isfile = FSO.FileExists(path);
			if (isdir) {
				if (src.length > 0 && src.substr(src.length - 1) != '/' && src.substr(src.length - 1) != '\\') {
					src += '\\';
				}
				headers_install[headers_install.length] = [path, 'dir',''];
				ADD_FLAG("INSTALL_HEADERS_DIR", path);
			} else if (isfile) {
				dirname = FSO.GetParentFolderName(path);
				headers_install[headers_install.length] = [path, 'file', dir];
				ADD_FLAG("INSTALL_HEADERS", dir + src);
				found = true;
			}
		}

		if (found == false) {
			STDOUT.WriteLine(headers_list);
			ERROR("Cannot find header " + dir + src);
		}
	}
}

// for snapshot builders, this option will attempt to enable everything
// and you can then build everything, ignoring fatal errors within a module
// by running "nmake snap"
PHP_SNAPSHOT_BUILD = "no";
if (!MODE_PHPIZE) {
	ARG_ENABLE('snapshot-build', 'Build a snapshot; turns on everything it can and ignores build errors', 'no');

	// one-shot build optimizes build by asking compiler to build
	// several objects at once, reducing overhead of starting new
	// compiler processes.
	ARG_ENABLE('one-shot', 'Optimize for fast build - best for release and snapshot builders, not so hot for edit-and-rebuild hacking', 'no');
}

PHP_DSP="no"

core_module_list = new Array(
"TSRM",
"aolserver",
"apache",
"apache2filter",
"apache2handler",
"apache-hooks",
"cgi",
"cli",
"embed",
"isapi",
"nsapi",
"phpdbg",
"pi3web",
"bcmath",
"bz2",
"calendar",
"com-dotnet",
"ctype",
"curl",
"date",
"dba",
"dom",
"enchant",
"ereg",
"exif",
"fileinfo",
"filter",
"ftp",
"gd",
"gettext",
"gmp",
"hash",
"iconv",
"imap",
"interbase",
"intl",
"json",
"ldap",
"libxml",
"mbstring",
"mcrypt",
"mssql",
"mysql",
"mysqli",
"mysqlnd",
"oci8",
"odbc",
"opcache",
"openssl",
"pcre",
"pdo",
"pdo-dblib",
"pdo-firebird",
"pdo-mysql",
"pdo-oci",
"pdo-odbc",
"pdo-pgsql",
"pdo-sqlite",
"pgsql",
"phar",
"pspell",
"reflection",
"session",
"shmop",
"simplexml",
"snmp",
"soap",
"sockets",
"spl",
"sqlite3",
"standard",
"sybase-ct",
"tidy",
"tokenizer",
"wddx",
"xml",
"xmlreader",
"xmlrpc",
"xmlwriter",
"xsl",
"zip",
"zlib",
false // dummy
);
try {
ARG_WITH('cygwin', 'Path to cygwin utilities on your system', '\\cygwin');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('object-out-dir', 'Alternate location for binary objects during build', '');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('debug', 'Compile with debugging symbols', "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('debug-pack', 'Release binaries with external debug symbols (--enable-debug must not be specified)', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('pgi', 'Generate PGO instrumented binaries', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('pgo', 'Compile optimized binaries using training data from folder', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('zts', 'Thread safety', 'yes');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('prefix', 'where PHP will be installed', '');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('mp', 'Tell VC9+ use up to [n,auto,disable] processes for compilation', 'auto');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('php-build', 'Path to where you extracted the development libraries (http://wiki.php.net/internals/windows/libs). Assumes that it is a sibling of this source dir (..\\deps) if not specified', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('extra-includes', 'Extra include path to use when building everything', '');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('extra-libs', 'Extra library path to use when linking everything', '');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("ipv6", "Disable IPv6 support (default is turn it on if available)", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('fd-setsize', "Set maximum number of sockets for select(2)", "256");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("snapshot-template", "Path to snapshot builder template dir", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("security-flags", "Disable the compiler security flags", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("analyzer", "Enable static analyzer. Pass vs for Visual Studio, pvs for PVS-Studio", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('aolserver', 'Build AOLserver support', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('apache', 'Build Apache 1.3.x version of PHP', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('apache-includes', 'Where to find Apache 1.3 headers', null);
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('apache-libs', 'Where to find Apache 1.3 libraries', null);
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('apache2filter', 'Build Apache 2.x filter', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('apache2-2filter', 'Build Apache 2.2.x filter', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('apache2handler', 'Build Apache 2.x handler', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('apache2-2handler', 'Build Apache 2.2.x handler', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('apache2-4handler', 'Build Apache 2.4.x handler', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('apache-hooks', 'Build Apache 1.3.x (hooks) version of PHP', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('cgi', 'Build CGI version of PHP', 'yes');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('cli', 'Build CLI version of PHP', 'yes');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('crt-debug', 'Enable CRT memory dumps for debugging sent to STDERR', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('cli-win32', 'Build console-less CLI version of PHP', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('embed', 'Embedded SAPI library', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('isapi', 'Build ISAPI version of PHP', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('nsapi', 'Build NSAPI for Netscape/iPlanet/SunONE webservers', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('nsapi-includes', 'Where to find NSAPI headers', null);
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('nsapi-libs', 'Where to find NSAPI libraries', null);
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('phpdbg', 'Build phpdbg', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE('phpdbgs', 'Build phpdbg shared', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH('pi3web', 'Pi3Web', 'no');
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("bcmath", "bc style precision math functions", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("bz2", "BZip2", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("calendar", "calendar conversion support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("com-dotnet", "COM and .Net support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("ctype", "ctype", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("curl", "cURL support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("dba", "DBA support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("enchant", "Enchant Support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("ereg", "POSIX extended regular expressions", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("fileinfo", "fileinfo support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("filter", "Filter Support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("ftp", "ftp support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("gd", "Bundled GD support", "yes,shared");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("t1lib", "t1lib support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("libvpx", "vpx support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("gettext", "gettext support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("gmp", "Include GNU MP support.", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("hash", "enable hash support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("mhash", "mhash support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("iconv", "iconv support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("imap", "IMAP Support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("interbase", "InterBase support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("intl", "Enable internationalization support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("json", "JavaScript Object Serialization support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("ldap", "LDAP support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("libmbfl", "use external libmbfl", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("mbstring", "multibyte string functions", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("mbregex", "multibyte regex support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("mbregex-backtrack", "check multibyte regex backtrack", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("mcrypt", "mcrypt support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("mssql", "mssql support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("dblib", "mssql support with freetds", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("mysqlnd", "Mysql Native Client Driver", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("oci8", "OCI8 support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("oci8-11g", "OCI8 support using Oracle 11g Instant Client", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("oci8-12c", "OCI8 support using Oracle Database 12c Instant Client", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("odbc", "ODBC support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("odbcver", "Force support for the passed ODBC version. A hex number is expected, default 0x0300. Use the special value of 0 to prevent an explicit ODBCVER to be defined.", "0x0300");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("opcache", "whether to enable Zend OPcache support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("openssl", "OpenSSL support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pgsql", "PostgreSQL support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pspell", "pspell/aspell (whatever it's called this month) support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("session", "session support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("shmop", "shmop support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("snmp", "SNMP support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("sockets", "SOCKETS support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("sqlite3", "SQLite 3 support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("config-file-scan-dir", "Dir to check for additional php ini files", "");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("sybase-ct", "SYBASE_CT support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("tidy", "TIDY support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("tokenizer", "tokenizer support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("zip", "ZIP support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("zlib", "ZLIB support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("libxml", "LibXML support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("dom", "DOM support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("exif", "exif", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("mysql", "MySQL support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("mysqli", "MySQLi support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("pdo", "Enable PHP Data Objects support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-dblib", "freetds dblib (Sybase, MS-SQL) support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-mssql", "Native MS-SQL support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-firebird", "Firebird support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-mysql", "MySQL support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-oci", "Oracle OCI support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-odbc", "ODBC support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-pgsql", "PostgreSQL support for PDO", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-sqlite", "for pdo_sqlite support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("pdo-sqlite-external", "for pdo_sqlite support from an external dll", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("phar", "disable phar support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("phar-native-ssl", "enable phar with native OpenSSL support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("simplexml", "Simple XML support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("soap", "SOAP support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("wddx", "WDDX support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("xml", "XML support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("xmlreader", "XMLReader support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("xmlrpc", "XMLRPC-EPI support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_ENABLE("xmlwriter", "XMLWriter support", "yes");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}
try {
ARG_WITH("xsl", "xsl support", "no");
} catch (e) {
	STDOUT.WriteLine("problem: " + e);
}

conf_process_args();

// vim:ft=javascript
// $Id$
// "Master" config file; think of it as a configure.in
// equivalent.

/* ARG_WITH('cygwin', 'Path to cygwin utilities on your system', '\\cygwin'); */
PHP_CL = PATH_PROG('cl', null, 'PHP_CL');
if (!PHP_CL) {
	ERROR("MS C++ compiler is required");
}

/* For the record here: */
// 1200 is VC6
// 1300 is vs.net 2002
// 1310 is vs.net 2003
// 1400 is vs.net 2005
// 1500 is vs.net 2008
// 1600 is vs.net 2010
// Which version of the compiler do we have?
VCVERS = probe_binary(PHP_CL).substr(0, 5).replace('.', '');
STDOUT.WriteLine("  Detected compiler " + VC_VERSIONS[VCVERS]);

if (VCVERS < 1500) {
	ERROR("Unsupported MS C++ Compiler, VC9 (2008) minimum is required");
}

AC_DEFINE('COMPILER', VC_VERSIONS[VCVERS], "Detected compiler version");
DEFINE("PHP_COMPILER_SHORT", VC_VERSIONS_SHORT[VCVERS]);
AC_DEFINE('PHP_COMPILER_ID', VC_VERSIONS_SHORT[VCVERS], "Compiler compatibility ID");

// do we use x64 or 80x86 version of compiler?
X64 = probe_binary(PHP_CL, 64, null, 'PHP_CL');
if (X64) {
	STDOUT.WriteLine("  Detected 64-bit compiler");
} else {
	STDOUT.WriteLine("  Detected 32-bit compiler");
}
AC_DEFINE('ARCHITECTURE', X64 ? 'x64' : 'x86', "Detected compiler architecture");
DEFINE("PHP_ARCHITECTURE", X64 ? 'x64' : 'x86');

// cygwin now ships with link.exe.  Avoid searching the cygwin path
// for this, as we want the MS linker, not the fileutil
PATH_PROG('link', WshShell.Environment("Process").Item("PATH"));
PATH_PROG('nmake');

// we don't want to define LIB, as that will override the default library path
// that is set in that env var
PATH_PROG('lib', null, 'MAKE_LIB');
if (!PATH_PROG('bison')) {
	ERROR('bison is required')
}

// There's a minimum requirement for re2c..
MINRE2C = "0.13.4";

RE2C = PATH_PROG('re2c');
if (RE2C) {
	var intvers, intmin;
	var pattern = /\./g;

	RE2CVERS = probe_binary(RE2C, "version");
	STDOUT.WriteLine('  Detected re2c version ' + RE2CVERS);

	intvers = RE2CVERS.replace(pattern, '') - 0;
	intmin = MINRE2C.replace(pattern, '') - 0;

	if (intvers < intmin) {
		STDOUT.WriteLine('WARNING: The minimum RE2C version requirement is ' + MINRE2C);
		STDOUT.WriteLine('Parsers will not be generated. Upgrade your copy at http://sf.net/projects/re2c');
		DEFINE('RE2C', '');
	} else {
		DEFINE('RE2C_FLAGS', '');
	}
} else {
	STDOUT.WriteLine('Parsers will not be regenerated');
}
PATH_PROG('zip');
PATH_PROG('lemon');

// avoid picking up midnight commander from cygwin
PATH_PROG('mc', WshShell.Environment("Process").Item("PATH"));

// Try locating manifest tool
if (VCVERS > 1200) {
	PATH_PROG('mt', WshShell.Environment("Process").Item("PATH"));
}

// stick objects somewhere outside of the source tree
/* ARG_ENABLE('object-out-dir', 'Alternate location for binary objects during build', ''); */
if (PHP_OBJECT_OUT_DIR.length) {
	PHP_OBJECT_OUT_DIR = FSO.GetAbsolutePathName(PHP_OBJECT_OUT_DIR);
	if (!FSO.FolderExists(PHP_OBJECT_OUT_DIR)) {
		ERROR('you chosen output directory ' + PHP_OBJECT_OUT_DIR + ' does not exist');
	}
	PHP_OBJECT_OUT_DIR += '\\';
} else if (X64) {
	if (!FSO.FolderExists("x64")) {
		FSO.CreateFolder("x64");
	}
	PHP_OBJECT_OUT_DIR = 'x64\\';
}

/* ARG_ENABLE('debug', 'Compile with debugging symbols', "no"); */
/* ARG_ENABLE('debug-pack', 'Release binaries with external debug symbols (--enable-debug must not be specified)', 'no'); */
if (PHP_DEBUG == "yes" && PHP_DEBUG_PACK == "yes") {
	ERROR("Use of both --enable-debug and --enable-debug-pack not allowed.");
}

/* ARG_ENABLE('pgi', 'Generate PGO instrumented binaries', 'no'); */
/* ARG_WITH('pgo', 'Compile optimized binaries using training data from folder', 'no'); */
if (PHP_PGI == "yes" || PHP_PGO != "no") {
	PGOMGR = PATH_PROG('pgomgr', WshShell.Environment("Process").Item("PATH"));
	if (!PGOMGR) {
		ERROR("--enable-pgi and --with-pgo options can only be used if PGO capable compiler is present.");
	}
	if (PHP_PGI == "yes" && PHP_PGO != "no") {
		ERROR("Use of both --enable-pgi and --with-pgo not allowed.");
	}
}

/* ARG_ENABLE('zts', 'Thread safety', 'yes'); */
// Configures the hard-coded installation dir
/* ARG_WITH('prefix', 'where PHP will be installed', ''); */
if (PHP_PREFIX == '') {
	PHP_PREFIX = "C:\\php";
	if (PHP_DEBUG == "yes")
		PHP_PREFIX += "\\debug";
}
DEFINE('PHP_PREFIX', PHP_PREFIX);

DEFINE("BASE_INCLUDES", "/I . /I main /I Zend /I TSRM /I ext ");

// CFLAGS for building the PHP dll
DEFINE("CFLAGS_PHP", "/D _USRDLL /D PHP5DLLTS_EXPORTS /D PHP_EXPORTS \
/D LIBZEND_EXPORTS /D TSRM_EXPORTS /D SAPI_EXPORTS /D WINVER=0x500");

DEFINE('CFLAGS_PHP_OBJ', '$(CFLAGS_PHP) $(STATIC_EXT_CFLAGS)');

// General CFLAGS for building objects
DEFINE("CFLAGS", "/nologo /FD $(BASE_INCLUDES) /D _WINDOWS \
/D ZEND_WIN32=1 /D PHP_WIN32=1 /D WIN32 /D _MBCS /W3 ");

if (VCVERS < 1400) {
	// Enable automatic precompiled headers
	ADD_FLAG('CFLAGS', ' /YX ');

	if (PHP_DEBUG == "yes") {
		// Set some debug/release specific options
		ADD_FLAG('CFLAGS', ' /GZ ');
	}
}

if (VCVERS >= 1400) {
	// fun stuff: MS deprecated ANSI stdio and similar functions
	// disable annoying warnings.  In addition, time_t defaults
	// to 64-bit.  Ask for 32-bit.
	if (X64) {
		ADD_FLAG('CFLAGS', ' /wd4996 ');
	} else {
		ADD_FLAG('CFLAGS', ' /wd4996 /D_USE_32BIT_TIME_T=1 ');
	}

	if (PHP_DEBUG == "yes") {
		// Set some debug/release specific options
		ADD_FLAG('CFLAGS', ' /RTC1 ');
	}
}

/* ARG_WITH('mp', 'Tell VC9+ use up to [n,auto,disable] processes for compilation', 'auto'); */
if (VCVERS >= 1500 && PHP_MP != 'disable') {
		// no from disable-all 
		if(PHP_MP == 'auto' || PHP_MP == 'no') {
			 ADD_FLAG('CFLAGS', ' /MP ');
		} else {
			if(parseInt(PHP_MP) != 0) {
				ADD_FLAG('CFLAGS', ' /MP'+ PHP_MP +' ');
			} else {
				STDOUT.WriteLine('WARNING: Invalid argument for MP: ' + PHP_MP);
			}
		}
}

// General link flags

if (VCVERS >= 1700) {
	DEFINE("LDFLAGS", "/nologo ");
} else {
	DEFINE("LDFLAGS", "/nologo /version:" +
			PHP_VERSION + "." + PHP_MINOR_VERSION + "." + PHP_RELEASE_VERSION);
}

// General DLL link flags
DEFINE("DLL_LDFLAGS", "/dll ");

// PHP DLL link flags
DEFINE("PHP_LDFLAGS", "$(DLL_LDFLAGS)");

// General libs
// urlmon.lib ole32.lib oleaut32.lib uuid.lib gdi32.lib winspool.lib comdlg32.lib
DEFINE("LIBS", "kernel32.lib ole32.lib user32.lib advapi32.lib shell32.lib ws2_32.lib Dnsapi.lib");

// Set some debug/release specific options
if (PHP_DEBUG == "yes") {
	ADD_FLAG("CFLAGS", "/LDd /MDd /W3 /Gm /Od /D _DEBUG /D ZEND_DEBUG=1 " +
		(X64?"/Zi":"/ZI"));
	ADD_FLAG("LDFLAGS", "/debug");
	// Avoid problems when linking to release libraries that use the release
	// version of the libc
	ADD_FLAG("PHP_LDFLAGS", "/nodefaultlib:msvcrt");
} else {
	// Generate external debug files when --enable-debug-pack is specified
	if (PHP_DEBUG_PACK == "yes") {
		ADD_FLAG("CFLAGS", "/Zi");
		ADD_FLAG("LDFLAGS", "/incremental:no /debug /opt:ref,icf");
	}
	// Equivalent to Release_TSInline build -> best optimization
	ADD_FLAG("CFLAGS", "/LD /MD /W3 /Ox /D NDebug /D NDEBUG /D ZEND_WIN32_FORCE_INLINE /GF /D ZEND_DEBUG=0");

	// if you have VS.Net /GS hardens the binary against buffer overruns
	// ADD_FLAG("CFLAGS", "/GS");
}

if (PHP_ZTS == "yes") {
	ADD_FLAG("CFLAGS", "/D ZTS=1");
	ADD_FLAG("ZTS", "1");
} else {
	ADD_FLAG("ZTS", "0");
}

DEFINE("PHP_ZTS_ARCHIVE_POSTFIX", PHP_ZTS == "yes" ? '' : "-nts");


// we want msvcrt in the PHP DLL
ADD_FLAG("PHP_LDFLAGS", "/nodefaultlib:libcmt");

// set up the build dir and DLL name
if (PHP_DEBUG == "yes" && PHP_ZTS == "yes") {
	DEFINE("BUILD_DIR", PHP_OBJECT_OUT_DIR + "Debug_TS");
	DEFINE("PHPDLL", "php" + PHP_VERSION + "ts_debug.dll");
	DEFINE("PHPLIB", "php" + PHP_VERSION + "ts_debug.lib");
} else if (PHP_DEBUG == "yes" && PHP_ZTS == "no") {
	DEFINE("BUILD_DIR", PHP_OBJECT_OUT_DIR + "Debug");
	DEFINE("PHPDLL", "php" + PHP_VERSION + "_debug.dll");
	DEFINE("PHPLIB", "php" + PHP_VERSION + "_debug.lib");
} else if (PHP_DEBUG == "no" && PHP_ZTS == "yes") {
	DEFINE("BUILD_DIR", PHP_OBJECT_OUT_DIR + "Release_TS");
	DEFINE("PHPDLL", "php" + PHP_VERSION + "ts.dll");
	DEFINE("PHPLIB", "php" + PHP_VERSION + "ts.lib");
} else if (PHP_DEBUG == "no" && PHP_ZTS == "no") {
	DEFINE("BUILD_DIR", PHP_OBJECT_OUT_DIR + "Release");
	DEFINE("PHPDLL", "php" + PHP_VERSION + ".dll");
	DEFINE("PHPLIB", "php" + PHP_VERSION + ".lib");
}

// CFLAGS, LDFLAGS and BUILD_DIR are defined
// Add compiler and link flags if PGO options are selected
if (PHP_DEBUG != "yes" && PHP_PGI == "yes") {
	ADD_FLAG("STATIC_EXT_CFLAGS", "/GL /O2");
	DEFINE("PGOPGD_DIR", "$(BUILD_DIR)");
}
else if (PHP_DEBUG != "yes" && PHP_PGO != "no") {
	ADD_FLAG("STATIC_EXT_CFLAGS", "/GL /O2");
	DEFINE("PGOPGD_DIR", ((PHP_PGO.length == 0 || PHP_PGO == "yes") ? "$(BUILD_DIR)" : PHP_PGO));
}

// Find the php_build dir - it contains headers and libraries
// that we need
/* ARG_WITH('php-build', 'Path to where you extracted the development libraries (http://wiki.php.net/internals/windows/libs). Assumes that it is a sibling of this source dir (..\\deps) if not specified', 'no'); */

if (PHP_PHP_BUILD == 'no') {
	if (FSO.FolderExists("..\\deps")) {
		PHP_PHP_BUILD = "..\\deps";
	} else {
		if (FSO.FolderExists("..\\php_build")) {
			PHP_PHP_BUILD = "..\\php_build";
		} else {
			if (X64) {
				if (FSO.FolderExists("..\\win64build")) {
					PHP_PHP_BUILD = "..\\win64build";
				} else if (FSO.FolderExists("..\\php-win64-dev\\php_build")) {
					PHP_PHP_BUILD = "..\\php-win64-dev\\php_build";
				}
			} else {
				if (FSO.FolderExists("..\\win32build")) {
					PHP_PHP_BUILD = "..\\win32build";
				} else if (FSO.FolderExists("..\\php-win32-dev\\php_build")) {
					PHP_PHP_BUILD = "..\\php-win32-dev\\php_build";
				}
			}
		}
	}
	PHP_PHP_BUILD = FSO.GetAbsolutePathName(PHP_PHP_BUILD);
}
DEFINE("PHP_BUILD", PHP_PHP_BUILD);

/* ARG_WITH('extra-includes', 'Extra include path to use when building everything', ''); */
/* ARG_WITH('extra-libs', 'Extra library path to use when linking everything', ''); */

var php_usual_include_suspects = PHP_PHP_BUILD+"\\include";
var php_usual_lib_suspects = PHP_PHP_BUILD+"\\lib";

ADD_FLAG("CFLAGS", '/I "' + php_usual_include_suspects + '" ');
ADD_FLAG("LDFLAGS", '/libpath:"' + php_usual_lib_suspects + '" ');

// Poke around for some headers
function probe_basic_headers()
{
	var p;

	if (PHP_PHP_BUILD != "no") {
		php_usual_include_suspects += ";" + PHP_PHP_BUILD + "\\include";
		php_usual_lib_suspects += ";" + PHP_PHP_BUILD + "\\lib";
	}
}

function add_extra_dirs()
{
	var path, i, f;

	if (PHP_EXTRA_INCLUDES.length) {
		path = PHP_EXTRA_INCLUDES.split(';');
		for (i = 0; i < path.length; i++) {
			f = FSO.GetAbsolutePathName(path[i]);
			if (FSO.FolderExists(f)) {
				ADD_FLAG("CFLAGS", '/I "' + f + '" ');
			}
		}
	}
	if (PHP_EXTRA_LIBS.length) {
		path = PHP_EXTRA_LIBS.split(';');
		for (i = 0; i < path.length; i++) {
			f = FSO.GetAbsolutePathName(path[i]);
			if (FSO.FolderExists(f)) {
				if (VCVERS <= 1200 && f.indexOf(" ") >= 0) {
					ADD_FLAG("LDFLAGS", '/libpath:"\\"' + f + '\\"" ');
				} else {
					ADD_FLAG("LDFLAGS", '/libpath:"' + f + '" ');
				}
			}
		}
	}

}

probe_basic_headers();
add_extra_dirs();

//DEFINE("PHP_BUILD", PHP_PHP_BUILD);

STDOUT.WriteBlankLines(1);
STDOUT.WriteLine("Build dir: " + get_define('BUILD_DIR'));
STDOUT.WriteLine("PHP Core:  " + get_define('PHPDLL') + " and " + get_define('PHPLIB'));

ADD_SOURCES("Zend", "zend_language_parser.c zend_language_scanner.c \
	zend_ini_parser.c zend_ini_scanner.c zend_alloc.c zend_compile.c \
	zend_constants.c zend_dynamic_array.c zend_exceptions.c \
	zend_execute_API.c zend_highlight.c \
	zend_llist.c zend_vm_opcodes.c zend_opcode.c zend_operators.c zend_ptr_stack.c \
	zend_stack.c zend_variables.c zend.c zend_API.c zend_extensions.c \
	zend_hash.c zend_list.c zend_indent.c zend_builtin_functions.c \
	zend_sprintf.c zend_ini.c zend_qsort.c zend_multibyte.c zend_ts_hash.c \
	zend_stream.c zend_iterators.c zend_interfaces.c zend_objects.c \
	zend_object_handlers.c zend_objects_API.c \
	zend_default_classes.c zend_execute.c zend_strtod.c zend_gc.c zend_closures.c \
	zend_float.c zend_string.c zend_generators.c zend_virtual_cwd.c zend_ast.c");

if (VCVERS == 1200) {
	AC_DEFINE('ZEND_DVAL_TO_LVAL_CAST_OK', 1);
}

ADD_SOURCES("main", "main.c snprintf.c spprintf.c getopt.c fopen_wrappers.c \
	php_scandir.c php_ini.c SAPI.c rfc1867.c php_content_types.c strlcpy.c \
	strlcat.c mergesort.c reentrancy.c php_variables.c php_ticks.c network.c \
	php_open_temporary_file.c output.c internal_functions.c php_sprintf.c");
ADD_SOURCES("win32", "inet.c fnmatch.c sockets.c");

// Newer versions have it
if (VCVERS <= 1300) {
	ADD_SOURCES("win32", "strtoi64.c");
}
if (VCVERS >= 1400) {
	AC_DEFINE('HAVE_STRNLEN', 1);
}

ADD_SOURCES("main/streams", "streams.c cast.c memory.c filter.c plain_wrapper.c \
	userspace.c transports.c xp_socket.c mmap.c glob_wrapper.c");

ADD_SOURCES("win32", "glob.c readdir.c \
	registry.c select.c sendmail.c time.c winutil.c wsyslog.c globals.c");

PHP_INSTALL_HEADERS("", "Zend/ TSRM/ main/ main/streams/ win32/");

STDOUT.WriteBlankLines(1);


/* Can we build with IPv6 support? */
/* ARG_ENABLE("ipv6", "Disable IPv6 support (default is turn it on if available)", "yes"); */

var main_network_has_ipv6 = 0;
if (PHP_IPV6 == "yes") {
	main_network_has_ipv6 = CHECK_HEADER_ADD_INCLUDE("wspiapi.h", "CFLAGS") ? 1 : 0;
}
if (main_network_has_ipv6) {
	STDOUT.WriteLine("Enabling IPv6 support");
}
AC_DEFINE('HAVE_GETADDRINFO', main_network_has_ipv6);
AC_DEFINE('HAVE_GAI_STRERROR', main_network_has_ipv6);
AC_DEFINE('HAVE_IPV6', main_network_has_ipv6);

/* this allows up to 256 sockets to be select()ed in a single
 * call to select(), instead of the usual 64 */
/* ARG_ENABLE('fd-setsize', "Set maximum number of sockets for select(2)", "256"); */
ADD_FLAG("CFLAGS", "/D FD_SETSIZE=" + parseInt(PHP_FD_SETSIZE));

AC_DEFINE('HAVE_USLEEP', 1);
AC_DEFINE('HAVE_STRCOLL', 1);

/* For snapshot builders, where can we find the additional
 * files that make up the snapshot template? */
/* ARG_WITH("snapshot-template", "Path to snapshot builder template dir", "no"); */

if (PHP_SNAPSHOT_TEMPLATE == "no") {
	/* default is as a sibling of the php_build dir */
	if (FSO.FolderExists(PHP_PHP_BUILD + "\\template")) {
		PHP_SNAPSHOT_TEMPLATE = FSO.GetAbsolutePathName(PHP_PHP_BUILD + "\\template");
	} else if (FSO.FolderExists(PHP_PHP_BUILD + "\\..\\template")) {
		PHP_SNAPSHOT_TEMPLATE = FSO.GetAbsolutePathName(PHP_PHP_BUILD + "\\..\\template");
	}
}

DEFINE('SNAPSHOT_TEMPLATE', PHP_SNAPSHOT_TEMPLATE);

if (PHP_DSP != "no") {
	if (FSO.FolderExists("tmp")) {
		FSO.DeleteFolder("tmp");
	}
	FSO.CreateFolder("tmp");
}

/* ARG_ENABLE("security-flags", "Disable the compiler security flags", "yes"); */
if (PHP_SECURITY_FLAGS == "yes") {
	ADD_FLAG("LDFLAGS", "/NXCOMPAT /DYNAMICBASE ");
}

/* XXX add and implement clang keyword for clang analyzer */
/* ARG_WITH("analyzer", "Enable static analyzer. Pass vs for Visual Studio, pvs for PVS-Studio", "no"); */
if (PHP_ANALYZER == "vs") {
	ADD_FLAG("CFLAGS", " /analyze ");
	ADD_FLAG("CFLAGS", " /wd6308 ");
} else if (PHP_ANALYZER == "pvs") {
	var pvs_studio = false;

	if (FSO.FileExists(PROGRAM_FILES + "\\PVS-Studio\\x64\\PVS-Studio.exe")) {
		pvs_studio = PROGRAM_FILES + "\\PVS-Studio\\x86\\PVS-Studio.exe";
	} else if (FSO.FileExists(PROGRAM_FILESx86 + "\\PVS-Studio\\x64\\PVS-Studio.exe")) {
		pvs_studio = PROGRAM_FILESx86 + "\\PVS-Studio\\x64\\PVS-Studio.exe";
	}

	if (!pvs_studio) {
		WARNING("Couldn't find PVS-Studio binaries, static analyze was disabled");
		PHP_ANALYZER = "no";
	} else {
		var pvscfg = FSO.CreateTextFile("PVS-Studio.conf", true);
		DEFINE("PVS_STUDIO", pvs_studio);

		pvscfg.WriteLine("exclude-path = " + VCINSTALLDIR);
		if (FSO.FolderExists(PROGRAM_FILESx86 + "\\windows kits\\")) {
			pvscfg.WriteLine("exclude-path = " + PROGRAM_FILESx86 + "\\windows kits\\");
		} else if (FSO.FolderExists(PROGRAM_FILES + "\\windows kits\\")) {
			pvscfg.WriteLine("exclude-path = " + PROGRAM_FILES + "\\windows kits\\");
		}
		pvscfg.WriteLine("vcinstalldir = " + VCINSTALLDIR);
		pvscfg.WriteLine("platform = " + (X64 ? 'x64' : 'Win32'));
		pvscfg.WriteLine("preprocessor = visualcpp");
		pvscfg.WriteLine("language = C");
	}
} else {
	PHP_ANALYZER = "no"
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\TSRM\\config.w32'));
// vim:ft=javascript
// $Id$

ADD_SOURCES("TSRM", "TSRM.c tsrm_strtok_r.c tsrm_win32.c");

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\aolserver\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_WITH('aolserver', 'Build AOLserver support', 'no'); */

if (PHP_AOLSERVER != "no") {
	if (PHP_ZTS == "no") {
		WARNING("AOLSERVER module requires an --enable-zts build of PHP");
	} else {
		if (CHECK_HEADER_ADD_INCLUDE("ns.h", "CFLAGS_AOLSERVER", PHP_AOLSERVER) && CHECK_LIB("nsd.lib", "aolserver", PHP_AOLSERVER)) {
			SAPI('aolserver', 'aolserver.c', 'php' + PHP_VERSION + 'aolserver.so', '/D XP_WIN32 ');
		} else {
			WARNING("sapi/aolserver not enabled: Could not find libraries/headers");
		}
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\apache\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('apache', 'Build Apache 1.3.x version of PHP', 'no'); */

/* ARG_WITH('apache-includes', 'Where to find Apache 1.3 headers', null); */
/* ARG_WITH('apache-libs', 'Where to find Apache 1.3 libraries', null); */

if (PHP_APACHE != "no") {
	if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE", php_usual_include_suspects +
				";" + PROGRAM_FILES + "\\Apache Group\\Apache\\include" +
				";" + PHP_PHP_BUILD + "\\apache\\src\\include") &&
			CHECK_LIB("ApacheCore.lib", "apache", php_usual_lib_suspects +
				';' + PROGRAM_FILES + '\\Apache Group\\Apache\\libexec' +
				";" + PHP_PHP_BUILD + "\\apache\\src\\corer")) {
		// We need to play tricks to get our readdir.h used by apache
		// headers
		SAPI('apache', 'mod_php5.c sapi_apache.c php_apache.c',
				'php' + PHP_VERSION + 'apache.dll',
				'/D APACHEPHP5_EXPORTS /D APACHE_READDIR_H /I win32');
	} else {
		WARNING("Could not find apache libraries/headers");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\apache2filter\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('apache2filter', 'Build Apache 2.x filter', 'no'); */

if (PHP_APACHE2FILTER != "no") {
	if (PHP_ZTS == "no") {
		WARNING("Apache2 module requires an --enable-zts build of PHP on windows");
	} else if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE2FILTER", PHP_PHP_BUILD + "\\include\\apache2") &&
			CHECK_LIB("libhttpd.lib", "apache2filter", PHP_PHP_BUILD + "\\lib\\apache2") &&
			CHECK_LIB("libapr.lib", "apache2filter", PHP_PHP_BUILD + "\\lib\\apache2") &&
			CHECK_LIB("libaprutil.lib", "apache2filter", PHP_PHP_BUILD + "\\lib\\apache2")
			) {
		SAPI('apache2filter', 'sapi_apache2.c apache_config.c php_functions.c',
				'php' + PHP_VERSION + 'apache2_filter.dll',
				'/D PHP_APACHE2_EXPORTS /I win32');
	} else {
		WARNING("Could not find apache2 filter libraries/headers");
	}
}

/* ARG_ENABLE('apache2-2filter', 'Build Apache 2.2.x filter', 'no'); */

if (PHP_APACHE2_2FILTER != "no") {
	if (PHP_ZTS == "no") {
		WARNING("Apache2 module requires an --enable-zts build of PHP on windows");
	} else if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE2_2FILTER", PHP_PHP_BUILD + "\\include\\apache2_2") &&
			CHECK_LIB("libhttpd.lib", "apache2_2filter", PHP_PHP_BUILD + "\\lib\\apache2_2") &&
			CHECK_LIB("libapr-1.lib", "apache2_2filter", PHP_PHP_BUILD + "\\lib\\apache2_2") &&
			CHECK_LIB("libaprutil-1.lib", "apache2_2filter", PHP_PHP_BUILD + "\\lib\\apache2_2")
			) {
		SAPI('apache2_2filter', 'sapi_apache2.c apache_config.c php_functions.c',
				'php' + PHP_VERSION + 'apache2_2_filter.dll',
				'/D PHP_APACHE2_EXPORTS /I win32',
				'sapi\\apache2_2filter');
	} else {
		WARNING("Could not find apache2.2 filter libraries/headers");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\apache2handler\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('apache2handler', 'Build Apache 2.x handler', 'no'); */

if (PHP_APACHE2HANDLER != "no") {
	if (PHP_ZTS == "no") {
		WARNING("Apache 2.0 module requires an --enable-zts build of PHP on windows");
	} else if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE2HANDLER", PHP_PHP_BUILD + "\\include\\apache2") &&
			CHECK_LIB("libhttpd.lib", "apache2handler", PHP_PHP_BUILD + "\\lib\\apache2") &&
			CHECK_LIB("libapr.lib", "apache2handler", PHP_PHP_BUILD + "\\lib\\apache2") &&
			CHECK_LIB("libaprutil.lib", "apache2handler", PHP_PHP_BUILD + "\\lib\\apache2")
			) {
		SAPI('apache2handler', 'mod_php5.c sapi_apache2.c apache_config.c php_functions.c',
				'php' + PHP_VERSION + 'apache2.dll',
				'/D PHP_APACHE2_EXPORTS /I win32');
	} else {
		WARNING("Could not find apache2 libraries/headers");
	}
}

/* ARG_ENABLE('apache2-2handler', 'Build Apache 2.2.x handler', 'no'); */

if (PHP_APACHE2_2HANDLER != "no") {
	if (PHP_ZTS == "no") {
		WARNING("Apache 2.2 module requires an --enable-zts build of PHP on windows");
	} else if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE2_2HANDLER", PHP_PHP_BUILD + "\\include\\apache2_2") &&
			CHECK_LIB("libhttpd.lib", "apache2_2handler", PHP_PHP_BUILD + "\\lib\\apache2_2") &&
			CHECK_LIB("libapr-1.lib", "apache2_2handler", PHP_PHP_BUILD + "\\lib\\apache2_2") &&
			CHECK_LIB("libaprutil-1.lib", "apache2_2handler", PHP_PHP_BUILD + "\\lib\\apache2_2")
			) {
		SAPI('apache2_2handler', 'mod_php5.c sapi_apache2.c apache_config.c php_functions.c',
				'php' + PHP_VERSION + 'apache2_2.dll',
				'/D PHP_APACHE2_EXPORTS /I win32',
				'sapi\\apache2_2handler');
	} else {
		WARNING("Could not find apache2.2 libraries/headers");
	}
}

/* ARG_ENABLE('apache2-4handler', 'Build Apache 2.4.x handler', 'no'); */
if (PHP_APACHE2_4HANDLER != "no") {
	if (PHP_ZTS == "no") {
		WARNING("Apache 2.4 module requires an --enable-zts build of PHP on windows");
	} else if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE2_4HANDLER", PHP_PHP_BUILD + "\\include\\apache2_4") &&
			CHECK_LIB("libhttpd.lib", "apache2_4handler", PHP_PHP_BUILD + "\\lib\\apache2_4") &&
			CHECK_LIB("libapr-1.lib", "apache2_4handler", PHP_PHP_BUILD + "\\lib\\apache2_4") &&
			CHECK_LIB("libaprutil-1.lib", "apache2_4handler", PHP_PHP_BUILD + "\\lib\\apache2_4")
			) {
		SAPI('apache2_4handler', 'mod_php5.c sapi_apache2.c apache_config.c php_functions.c',
				'php' + PHP_VERSION + 'apache2_4.dll',
				'/D PHP_APACHE2_EXPORTS /I win32',
				'sapi\\apache2handler');
	} else {
		WARNING("Could not find apache 2.4 libraries/headers");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\apache_hooks\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_WITH('apache-hooks', 'Build Apache 1.3.x (hooks) version of PHP', 'no'); */

if (PHP_APACHE_HOOKS != "no") {
	if (CHECK_HEADER_ADD_INCLUDE("httpd.h", "CFLAGS_APACHE_HOOKS", php_usual_include_suspects +
				";" + PROGRAM_FILES + "\\Apache Group\\Apache\\include" +
				";" + PHP_PHP_BUILD + "\\apache\\src\\include") &&
			CHECK_LIB("ApacheCore.lib", "apache_hooks", php_usual_lib_suspects +
				';' + PROGRAM_FILES + '\\Apache Group\\Apache\\libexec' +
				";" + PHP_PHP_BUILD + "\\apache\\src\\corer")) {
		// We need to play tricks to get our readdir.h used by apache
		// headers
		SAPI('apache_hooks', 'mod_php5.c sapi_apache.c php_apache.c',
				'php' + PHP_VERSION + 'apache_hooks.dll',
				'/D APACHEPHP5_EXPORTS /D APACHE_READDIR_H /I win32');
	} else {
		WARNING("Could not find apache libraries/headers");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\cgi\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('cgi', 'Build CGI version of PHP', 'yes'); */

if (PHP_CGI == "yes") {
	ADD_FLAG("LDFLAGS_CGI", "/stack:8388608");
	SAPI('cgi', 'cgi_main.c fastcgi.c', 'php-cgi.exe');
	ADD_FLAG('LIBS_CGI', 'ws2_32.lib kernel32.lib advapi32.lib');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\cli\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('cli', 'Build CLI version of PHP', 'yes'); */
/* ARG_ENABLE('crt-debug', 'Enable CRT memory dumps for debugging sent to STDERR', 'no'); */
/* ARG_ENABLE('cli-win32', 'Build console-less CLI version of PHP', 'no'); */

if (PHP_CLI == "yes") {
	SAPI('cli', 'php_cli.c php_http_parser.c php_cli_server.c php_cli_process_title.c ps_title.c', 'php.exe');
	ADD_FLAG("LIBS_CLI", "ws2_32.lib");
	if (PHP_CRT_DEBUG == "yes") {
		ADD_FLAG("CFLAGS_CLI", "/D PHP_WIN32_DEBUG_HEAP");
	}
	ADD_FLAG("LDFLAGS_CLI", "/stack:8388608");
}

if (PHP_CLI_WIN32 == "yes") {
	SAPI('cli_win32', 'cli_win32.c php_cli_process_title.c ps_title.c', 'php-win.exe');
	ADD_FLAG("LDFLAGS_CLI_WIN32", "/stack:8388608");
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\embed\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('embed', 'Embedded SAPI library', 'no'); */

var PHP_EMBED_PGO = false;

if (PHP_EMBED != "no") {
	SAPI('embed', 'php_embed.c', 'php' + PHP_VERSION + 'embed.lib');
	PHP_INSTALL_HEADERS("sapi/embed", "php_embed.h");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\isapi\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('isapi', 'Build ISAPI version of PHP', 'no'); */

if (PHP_ISAPI == "yes") {
	if (PHP_ZTS == "no") {
		WARNING("ISAPI module requires an --enable-zts build of PHP");
	} else {
		SAPI('isapi', 'php5isapi.c', 'php' + PHP_VERSION + 'isapi.dll', '/D PHP5ISAPI_EXPORTS');
		ADD_FLAG('LDFLAGS_ISAPI', '/DEF:sapi\\isapi\\php5isapi.def');
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\nsapi\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_ENABLE('nsapi', 'Build NSAPI for Netscape/iPlanet/SunONE webservers', 'no'); */

/* ARG_WITH('nsapi-includes', 'Where to find NSAPI headers', null); */
/* ARG_WITH('nsapi-libs', 'Where to find NSAPI libraries', null); */

if (PHP_NSAPI != "no") {
	if (PHP_ZTS == "no") {
		WARNING("NSAPI module requires an --enable-zts build of PHP");
		PHP_NSAPI = "no"
	} else if (CHECK_HEADER_ADD_INCLUDE("nsapi.h", "CFLAGS_NSAPI",
			PHP_NSAPI + ';' + PHP_NSAPI_INCLUDES) &&
			CHECK_LIB("ns-httpd*.lib", "nsapi", PHP_NSAPI + ";" + PHP_NSAPI_LIBS)) {
		SAPI('nsapi', 'nsapi.c', 'php' + PHP_VERSION + 'nsapi.dll', '/D XP_WIN32 ');
	} else {
		WARNING("Could not find NSAPI headers/libraries");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\phpdbg\\config.w32'));
/* ARG_ENABLE('phpdbg', 'Build phpdbg', 'no'); */
/* ARG_ENABLE('phpdbgs', 'Build phpdbg shared', 'no'); */

PHPDBG_SOURCES='phpdbg.c phpdbg_prompt.c phpdbg_cmd.c phpdbg_info.c phpdbg_help.c phpdbg_break.c phpdbg_print.c phpdbg_bp.c phpdbg_opcode.c phpdbg_list.c phpdbg_utils.c phpdbg_set.c phpdbg_frame.c phpdbg_watch.c phpdbg_win.c phpdbg_btree.c phpdbg_parser.c phpdbg_lexer.c';
PHPDBG_DLL='php' + PHP_VERSION + 'phpdbg.dll';
PHPDBG_EXE='phpdbg.exe';

if (PHP_PHPDBG == "yes") {
	SAPI('phpdbg', PHPDBG_SOURCES, PHPDBG_EXE);
	ADD_FLAG("LIBS_PHPDBG", "ws2_32.lib user32.lib");
	ADD_FLAG("CFLAGS_PHPDBG", "/D YY_NO_UNISTD_H");
}

if (PHP_PHPDBGS == "yes") {
	SAPI('phpdbgs', PHPDBG_SOURCES, PHPDBG_DLL, '/D PHP_PHPDBG_EXPORTS /I win32');
	ADD_FLAG("LIBS_PHPDBGS", "ws2_32.lib user32.lib");
	ADD_FLAG("CFLAGS_PHPDBGS", "/D YY_NO_UNISTD_H");
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\sapi\\pi3web\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_WITH('pi3web', 'Pi3Web', 'no'); */

if (PHP_PI3WEB != "no") {
	if (CHECK_HEADER_ADD_INCLUDE('PiAPI.h', 'CFLAGS_PI3WEB', PHP_PHP_BUILD + "\\Pi3Web\\include;" + PHP_PI3WEB) &&
		   CHECK_LIB('piapi.lib', 'pi3web', PHP_PHP_BUILD + "\\Pi3Web\\lib;" + PHP_PI3WEB) &&
		   CHECK_LIB('pi2api.lib', 'pi3web', PHP_PHP_BUILD + "\\Pi3Web\\lib;" + PHP_PI3WEB) &&
		   CHECK_LIB('pi3api.lib', 'pi3web', PHP_PHP_BUILD + "\\Pi3Web\\lib;" + PHP_PI3WEB)) {
		SAPI('pi3web', 'pi3web_sapi.c', 'php' + PHP_VERSION + 'pi3web.dll', '/D PHP5PI3WEB_EXPORTS');
		AC_DEFINE('WITH_PI3WEB', 1);
	} else {
		WARNING('Pi3Web not enabled; headers/libraries not found');
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\bcmath\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("bcmath", "bc style precision math functions", "yes"); */

if (PHP_BCMATH == "yes") {
	EXTENSION("bcmath", "bcmath.c",	null, "-Iext/bcmath/libbcmath/src");
	ADD_SOURCES("ext/bcmath/libbcmath/src", "add.c div.c init.c neg.c \
		outofmem.c raisemod.c rt.c sub.c compare.c divmod.c int2num.c \
		num2long.c output.c recmul.c sqrt.c zero.c debug.c doaddsub.c \
		nearzero.c num2str.c raise.c rmzero.c str2num.c", "bcmath");

	AC_DEFINE('HAVE_BCMATH', 1, 'Have BCMATH library');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\bz2\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("bz2", "BZip2", "no"); */

if (PHP_BZ2 != "no") {
	if (CHECK_LIB("libbz2_a.lib;libbz2.lib", "bz2", PHP_BZ2) &&
			CHECK_HEADER_ADD_INCLUDE("bzlib.h", "CFLAGS_BZ2")) {
		EXTENSION("bz2", "bz2.c bz2_filter.c");
		AC_DEFINE('HAVE_BZ2', 1, 'Have BZ2 library');
		// BZ2 extension does this slightly differently from others
		if (PHP_BZ2_SHARED) {
			ADD_FLAG("CFLAGS_BZ2", "/D PHP_BZ2_EXPORTS ");
		}
	} else {
		WARNING("bz2 not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\calendar\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("calendar", "calendar conversion support", "yes"); */

if (PHP_CALENDAR == "yes") {
	EXTENSION("calendar", "calendar.c dow.c french.c gregor.c jewish.c \
		julian.c easter.c cal_unix.c");
	AC_DEFINE('HAVE_CALENDAR', 1, 'Have calendar');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\com_dotnet\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("com-dotnet", "COM and .Net support", "yes"); */

if (PHP_COM_DOTNET == "yes") {
	CHECK_LIB('oleaut32.lib', 'com_dotnet');
	EXTENSION("com_dotnet", "com_com.c com_dotnet.c com_extension.c \
		com_handlers.c com_iterator.c com_misc.c com_olechar.c \
		com_typeinfo.c com_variant.c com_wrapper.c com_saproxy.c com_persist.c");
	AC_DEFINE('HAVE_COM_DOTNET', 1, 'Have COM_DOTNET support');
	CHECK_HEADER_ADD_INCLUDE('mscoree.h', 'CFLAGS_COM_DOTNET');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\ctype\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("ctype", "ctype", "yes"); */

if (PHP_CTYPE == "yes") {
	EXTENSION("ctype", "ctype.c");
	AC_DEFINE('HAVE_CTYPE', 1, 'Have ctype');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\curl\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("curl", "cURL support", "no"); */

if (PHP_CURL != "no") {
	if (CHECK_LIB("libcurl_a.lib;libcurl.lib", "curl", PHP_CURL) &&
			CHECK_HEADER_ADD_INCLUDE("curl/easy.h", "CFLAGS_CURL") &&
			CHECK_LIB("ssleay32.lib", "curl", PHP_CURL) &&
			CHECK_LIB("libeay32.lib", "curl", PHP_CURL) 
		&& CHECK_LIB("winmm.lib", "curl", PHP_CURL)
		&& CHECK_LIB("wldap32.lib", "curl", PHP_CURL)
		&& (((PHP_ZLIB=="no") && (CHECK_LIB("zlib_a.lib;zlib.lib", "curl", PHP_CURL))) || 
			(PHP_ZLIB_SHARED && CHECK_LIB("zlib.lib", "curl", PHP_CURL)) || (PHP_ZLIB == "yes" && (!PHP_ZLIB_SHARED)))
		) {
		EXTENSION("curl", "interface.c multi.c share.c curl_file.c", true);
		AC_DEFINE('HAVE_CURL', 1, 'Have cURL library');
		AC_DEFINE('HAVE_CURL_SSL', 1, 'Have SSL suppurt in cURL');
		AC_DEFINE('HAVE_CURL_EASY_STRERROR', 1, 'Have curl_easy_strerror in cURL');
		AC_DEFINE('HAVE_CURL_MULTI_STRERROR', 1, 'Have curl_multi_strerror in cURL');
		ADD_FLAG("CFLAGS_CURL", "/D CURL_STATICLIB");
		// TODO: check for curl_version_info
	} else {
		WARNING("curl not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\date\\config.w32'));
// $Id$
// vim:ft=javascript

EXTENSION("date", "php_date.c", false, "/Iext/date/lib /DHAVE_TIMELIB_CONFIG_H=1");
ADD_SOURCES("ext/date/lib", "astro.c timelib.c dow.c parse_date.c parse_tz.c tm2unixtime.c unixtime2tm.c parse_iso_intervals.c interval.c", "date");
AC_DEFINE('HAVE_DATE', 1, 'Have date/time support');

ADD_FLAG('CFLAGS_DATE', "/wd4244");

var tl_config = FSO.CreateTextFile("ext/date/lib/timelib_config.h", true);
tl_config.WriteLine("#include \"config.w32.h\"");
tl_config.WriteLine("#include <php_stdint.h>");
tl_config.WriteLine("#include \"win32/php_inttypes.h\"");
tl_config.WriteLine("#define TIMELIB_OMIT_STDINT 1");
tl_config.Close();

PHP_INSTALL_HEADERS("ext/date/", "php_date.h lib/timelib.h lib/timelib_structs.h lib/timelib_config.h");
AC_DEFINE('HAVE_TIMELIB_CONFIG_H', 1, 'Have timelib_config.h')
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\dba\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("dba", "DBA support", "no"); */

if (PHP_DBA != "no") {
	if (CHECK_LIB("libdb31s.lib", "dba", PHP_DBA) &&
		CHECK_HEADER_ADD_INCLUDE("db.h", "CFLAGS_DBA")) {
		EXTENSION("dba", "dba.c dba_cdb.c dba_db1.c dba_db2.c dba_db3.c dba_dbm.c dba_flatfile.c dba_gdbm.c dba_ndbm.c dba_inifile.c");
		ADD_SOURCES("ext/dba/libcdb", "cdb.c cdb_make.c uint32.c", "dba");
		ADD_SOURCES("ext/dba/libflatfile", "flatfile.c", "dba");
		ADD_SOURCES("ext/dba/libinifile", "inifile.c", "dba");
		AC_DEFINE('HAVE_DBA', 1, 'DBA support');
		ADD_FLAG("CFLAGS_DBA", "/D DBA_DB1=0 /D DB1_VERSION=\"\\\"Berkeley DB 1.85 emulation in DB3\\\"\" /D DB1_INCLUDE_FILE=\"\\\"db_185.h\\\"\" /D DBA_DB3=1 /D DB3_INCLUDE_FILE=\"\\\"db.h\\\"\" /D DBA_FLATFILE=1 /D DBA_CDB=1 /D DBA_CDB_MAKE=1 /D DBA_CDB_BUILTIN=1 /D DBA_INIFILE=1");
	} else {
		WARNING("dba not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\enchant\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("enchant", "Enchant Support", "no"); */

if (PHP_ENCHANT == "yes") {
	if (CHECK_HEADER_ADD_INCLUDE("enchant.h", "CFLAGS_ENCHANT", PHP_ENCHANT+ ";" + PHP_PHP_BUILD + "\\include\\enchant") &&
			CHECK_HEADER_ADD_INCLUDE("glib.h", "CFLAGS_ENCHANT", PHP_ENCHANT+ ";" + PHP_PHP_BUILD + "\\include\\glib-2.0") &&
			CHECK_LIB("libenchant.lib", "enchant", PHP_ENCHANT) ) {
		EXTENSION("enchant", "enchant.c");
		AC_DEFINE('HAVE_ENCHANT', 1, 'Have Enchant support', false);
		AC_DEFINE('HAVE_ENCHANT_BROKER_SET_PARAM', 1);
		ADD_FLAG("CFLAG_ENCHANT", "/D _WIN32");
	} else {
		WARNING('Could not find enchant.h; skipping');
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\ereg\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("ereg", "POSIX extended regular expressions", "yes"); */
if (PHP_EREG != "no") {

	EXTENSION("ereg", "ereg.c", PHP_EREG_SHARED, "-Dregexec=php_regexec -Dregerror=php_regerror -Dregfree=php_regfree -Dregcomp=php_regcomp -Iext/ereg/regex");
	ADD_SOURCES("ext/ereg/regex", "regcomp.c regexec.c regerror.c regfree.c", "ereg");
	AC_DEFINE('REGEX', 1, 'Bundled regex');
	AC_DEFINE('HSREGEX', 1, 'Bundled regex');
	PHP_INSTALL_HEADERS("ext/ereg", "php_ereg.h php_regex.h regex/");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\fileinfo\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("fileinfo", "fileinfo support", "no"); */

if (PHP_FILEINFO != 'no') {
	LIBMAGIC_SOURCES=" apprentice.c apptype.c ascmagic.c \
			cdf.c cdf_time.c compress.c \
			encoding.c fsmagic.c funcs.c \
			is_tar.c magic.c print.c \
			readcdf.c softmagic.c";

	if (VCVERS < 1500) {
		ADD_FLAG('CFLAGS', '/Zm1000');
	}

	EXTENSION('fileinfo', 'fileinfo.c', true, "/I" + configure_module_dirname + "/libmagic /I" + configure_module_dirname);
	ADD_SOURCES(configure_module_dirname + '\\libmagic', LIBMAGIC_SOURCES, "fileinfo");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\filter\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("filter", "Filter Support", "yes"); */

if (PHP_FILTER == "yes") {
	EXTENSION("filter", "filter.c sanitizing_filters.c logical_filters.c callback_filter.c");
	PHP_INSTALL_HEADERS("ext/filter", "php_filter.h");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\ftp\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("ftp", "ftp support", "yes"); */

if (PHP_FTP == "yes") {
	EXTENSION("ftp", "php_ftp.c ftp.c");
	AC_DEFINE('HAVE_FTP', 1, 'Have FTP support');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\gd\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("gd", "Bundled GD support", "yes,shared"); */
/* ARG_WITH("t1lib", "t1lib support", "yes"); */
/* ARG_WITH("libvpx", "vpx support", "yes"); */

if (PHP_GD != "no") {
	if (
		CHECK_LIB("libjpeg_a.lib;libjpeg.lib", "gd", PHP_GD) &&
		CHECK_LIB("freetype_a.lib;freetype.lib", "gd", PHP_GD) &&
		CHECK_HEADER_ADD_INCLUDE("ft2build.h", "CFLAGS_GD", PHP_GD + ";" + PHP_PHP_BUILD + "\\include" + ";" + PHP_PHP_BUILD + "\\include\\freetype") &&
		CHECK_LIB("libpng_a.lib;libpng.lib", "gd", PHP_GD) &&
		CHECK_HEADER_ADD_INCLUDE("gd.h", "CFLAGS_GD", PHP_GD + ";ext\\gd\\libgd") &&
		(CHECK_HEADER_ADD_INCLUDE("png.h", "CFLAGS_GD", PHP_GD +  ";" + PHP_PHP_BUILD + "\\include\\libpng15") ||
		CHECK_HEADER_ADD_INCLUDE("png.h", "CFLAGS_GD", PHP_GD +  ";" + PHP_PHP_BUILD + "\\include\\libpng12")) &&
		(CHECK_LIB("libiconv_a.lib;libiconv.lib", "gd", PHP_GD) || CHECK_LIB("iconv_a.lib;iconv.lib", "gd", PHP_GD)) &&
		 CHECK_HEADER_ADD_INCLUDE("iconv.h", "CFLAGS_GD", PHP_GD) && 
		(((PHP_ZLIB=="no") && (CHECK_LIB("zlib_a.lib;zlib.lib", "gd", PHP_GD) )) || 
			(PHP_ZLIB_SHARED && CHECK_LIB("zlib.lib", "gd", PHP_GD)) || (PHP_ZLIB == "yes" && (!PHP_ZLIB_SHARED))) &&
		CHECK_LIB("libXpm_a.lib", "gd", PHP_GD) &&
		CHECK_HEADER_ADD_INCLUDE("xpm.h", "CFLAGS_GD", PHP_GD + ";" + PHP_PHP_BUILD + "\\include\\X11")
		) {
		if (PHP_T1LIB != "no") {
			if (CHECK_LIB("T1_StaticMD.lib", "gd", PHP_GD) &&
				CHECK_HEADER_ADD_INCLUDE("t1lib.h", "CFLAGS_GD", PHP_GD + ";" + PHP_PHP_BUILD + "\\include\\t1lib")
				) {
				ADD_FLAG("CFLAGS_GD", "/D HAVE_LIBT1");
				} else {
				WARNING("t1lib not enabled; libraries and headers not found");
			}
		}

		if (PHP_LIBVPX != "no") {
			if (CHECK_LIB("vpxmt.lib", "gd", PHP_GD) &&
				CHECK_HEADER_ADD_INCLUDE("vp8.h", "CFLAGS_GD", PHP_GD + ";" + PHP_PHP_BUILD + "\\include\\vpx")) {
				ADD_FLAG("CFLAGS_GD", "/D HAVE_LIBVPX /D HAVE_GD_WEBP");
			} else {
				WARNING("libvpx not enabled; libraries and headers not found");
			}
		}
		CHECK_LIB("User32.lib", "gd", PHP_GD);
		CHECK_LIB("Gdi32.lib", "gd", PHP_GD);

		EXTENSION("gd", "gd.c", null, "-Iext/gd/libgd", "php_gd2.dll");
		ADD_SOURCES("ext/gd/libgd", "gd2copypal.c gd_arc_f_buggy.c gd.c \
			gdcache.c gdfontg.c gdfontl.c gdfontmb.c gdfonts.c gdfontt.c \
			gdft.c gd_gd2.c gd_gd.c gd_gif_in.c gd_gif_out.c gdhelpers.c gd_io.c gd_io_dp.c \
			gd_io_file.c gd_io_ss.c gd_jpeg.c gdkanji.c gd_png.c gd_ss.c \
			gdtables.c gd_topal.c gd_wbmp.c gdxpm.c wbmp.c xbm.c gd_security.c gd_transform.c \
			gd_filter.c gd_pixelate.c gd_arc.c gd_rotate.c gd_color.c webpimg.c gd_webp.c \
			gd_crop.c gd_interpolation.c gd_matrix.c", "gd");
		AC_DEFINE('HAVE_LIBGD', 1, 'GD support');
		ADD_FLAG("CFLAGS_GD", " \
/D HAVE_GD_DYNAMIC_CTX_EX=1 \
/D HAVE_GD_BUNDLED=1  \
/D HAVE_GD_GD2  \
/D HAVE_GD_GIF_READ=1  \
/D HAVE_GD_GIF_CREATE=1  \
/D HAVE_GDIMAGECOLORRESOLVE=1  \
/D HAVE_GD_IMAGESETBRUSH=1  \
/D HAVE_GD_IMAGESETTILE=1 \
/D HAVE_GD_FONTCACHESHUTDOWN=1 \
/D HAVE_GD_FONTMUTEX=1 \
/D HAVE_LIBFREETYPE=1 \
/D HAVE_GD_JPG  \
/D HAVE_GD_PNG  \
/D HAVE_GD_STRINGFTEX=1  \
/D HAVE_GD_STRINGTTF=1  \
/D HAVE_GD_WBMP  \
/D HAVE_GD_XBM  \
/D HAVE_GD_XPM  \
/D HAVE_GD_FREETYPE=1  \
/D HAVE_LIBGD13=1  \
/D HAVE_LIBGD15=1  \
/D HAVE_LIBGD20=1  \
/D HAVE_LIBGD204=1 \
/D HAVE_LIBJPEG  \
/D HAVE_LIBPNG  \
/D HAVE_XPM  \
/D HAVE_COLORCLOSESTHWB  \
/D USE_GD_IMGSTRTTF  \
/D USE_GD_IOCTX \
/D MSWIN32 \
		");
		
		PHP_INSTALL_HEADERS("", "ext/gd ext/gd/libgd" );
	} else {
		WARNING("gd not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\gettext\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("gettext", "gettext support", "no"); */

if (PHP_GETTEXT != "no") {
	if (CHECK_LIB("libintl_a.lib;libintl.lib", "gettext", PHP_GETTEXT) && CHECK_HEADER_ADD_INCLUDE("libintl.h", "CFLAGS_GETTEXT")) {
		EXTENSION("gettext", "gettext.c", PHP_GETTEXT_SHARED, "-DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_DNGETTEXT=1 -DHAVE_NGETTEXT=1 -DHAVE_LIBINTL=1 -DHAVE_DCNGETTEXT=1");
	} else {
		WARNING("gettext not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\gmp\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("gmp", "Include GNU MP support.", "no"); */

if (PHP_GMP != "no") {
	if (CHECK_LIB("mpir_a.lib", "gmp", PHP_GMP) &&
		CHECK_HEADER_ADD_INCLUDE("gmp.h", "CFLAGS_GMP", PHP_GMP +  ";" + PHP_PHP_BUILD + "\\include\\mpir")) {
		EXTENSION("gmp", "gmp.c");
		AC_DEFINE('HAVE_GMP', 1, 'GMP support');
		AC_DEFINE('HAVE_MPIR', 1, 'MPIR support');
	} else {
		WARNING("GMP not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\hash\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("hash", "enable hash support", "yes"); */
/* ARG_WITH("mhash", "mhash support", "no"); */ 

if (PHP_MHASH != "no") {
	if (PHP_HASH == "no") {
		PHP_HASH = "yes";
	}
	AC_DEFINE('PHP_MHASH_BC', 1);
}

if (PHP_HASH != "no") {
	AC_DEFINE('HAVE_HASH_EXT', 1);
	EXTENSION("hash", "hash.c hash_md.c hash_sha.c hash_ripemd.c hash_haval.c "
		+ "hash_tiger.c hash_gost.c hash_snefru.c hash_whirlpool.c "
		+ "hash_adler32.c hash_crc32.c hash_joaat.c hash_fnv.c");

		PHP_INSTALL_HEADERS("ext/hash/", "php_hash.h php_hash_md.h php_hash_sha.h php_hash_ripemd.h " +
		"php_hash_haval.h php_hash_tiger.h php_hash_gost.h php_hash_snefru.h " + 
		"php_hash_whirlpool.h php_hash_adler32.h php_hash_crc32.h");
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\iconv\\config.w32'));
// $Id$
// vim: ft=javascript

/* ARG_WITH("iconv", "iconv support", "yes"); */

if (PHP_ICONV != "no") {
	if ((CHECK_LIB("libiconv_a.lib", "iconv", PHP_ICONV) || CHECK_LIB("libiconv.lib", "iconv", PHP_ICONV) ||
			CHECK_LIB("iconv_a.lib", "iconv", PHP_ICONV) || CHECK_LIB("iconv.lib", "iconv", PHP_ICONV)) &&
		CHECK_HEADER_ADD_INCLUDE("iconv.h", "CFLAGS_ICONV", PHP_ICONV)) {

		EXTENSION("iconv", "iconv.c");
		
		AC_DEFINE("HAVE_ICONV", 1, "Define if iconv extension is enabled");
		AC_DEFINE("HAVE_LIBICONV", 1, "Define if libiconv is available");
		AC_DEFINE("ICONV_ALIASED_LIBICONV", 1, "The iconv function is called iconv() in libiconv");
		AC_DEFINE("PHP_ICONV_IMPL", "\"libiconv\"", "Which iconv implementation to use");
		AC_DEFINE("ICONV_SUPPORTS_ERRNO", 1, "Whether iconv supports errno or not");
		ADD_FLAG("CFLAGS_ICONV", "/D PHP_ICONV_EXPORTS ");
		if (!PHP_ICONV_SHARED) {
			ADD_DEF_FILE("ext\\iconv\\php_iconv.def");
		}
		PHP_INSTALL_HEADERS("", "ext/iconv");
	} else {
		WARNING("iconv support can't be enabled, libraries or headers are missing")
		PHP_ICONV = "no";
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\imap\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("imap", "IMAP Support", "no"); */

if (PHP_IMAP == "yes") {
	if (CHECK_LIB("cclient_a.lib;cclient.lib", "imap") &&
			(CHECK_HEADER_ADD_INCLUDE("c-client.h", "CFLAGS_IMAP")||
			CHECK_HEADER_ADD_INCLUDE("c-client/c-client.h", "CFLAGS_IMAP", null, null, true)) ||
			(CHECK_HEADER_ADD_INCLUDE("utf8aux.h", "CFLAGS_IMAP")||
			CHECK_HEADER_ADD_INCLUDE("c-client/utf8aux.h", "CFLAGS_IMAP", null, null, true))
			) {
		CHECK_LIB("winmm.lib", "imap");
		CHECK_LIB("ws2_32.lib", "imap");
		CHECK_LIB("Secur32.lib", "imap");
		CHECK_LIB("crypt32.lib", "imap");
		EXTENSION("imap", "php_imap.c");

		ADD_FLAG("CFLAGS_IMAP", "/D HAVE_IMAP2000=1 /D HAVE_IMAP2004=1 /D HAVE_IMAP2007a=1 /D HAVE_IMAP2007b=1 /D HAVE_IMAP_SSL=1");
		AC_DEFINE('HAVE_IMAP', 1, 'Have IMAP support', true);
		AC_DEFINE('HAVE_NEW_MIME2TEXT', 1, 'Have utf8_mime2text', true);
		AC_DEFINE('HAVE_RFC822_OUTPUT_ADDRESS_LIST', 1, 'Have rfc822_output_address_list', true);
		AC_DEFINE('HAVE_IMAP_MUTF7', 1, 'Have modified utf7 support', true);
		AC_DEFINE('HAVE_NEW_MIME2TEXT', 1, 'Whether utf8_mime2text() has new signature');
	} else {
		WARNING("imap not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\interbase\\config.w32'));
// vim:ft=javascript

/* ARG_WITH("interbase", "InterBase support", "no"); */

if (PHP_INTERBASE != "no") {

	if (CHECK_HEADER_ADD_INCLUDE("ibase.h", "CFLAGS_INTERBASE", 
				PHP_PHP_BUILD + "\\include\\interbase;" + PHP_PHP_BUILD + "\\interbase\\include;" + PHP_INTERBASE) &&
			(CHECK_LIB("fbclient_ms.lib", "interbase", PHP_PHP_BUILD + "\\interbase\\lib_ms;" + PHP_INTERBASE) ||
			 CHECK_LIB("gds32_ms.lib", "interbase", PHP_PHP_BUILD + "\\interbase\\lib_ms;" + PHP_INTERBASE))) {

		EXTENSION("interbase", "interbase.c ibase_query.c ibase_service.c ibase_events.c ibase_blobs.c");
		AC_DEFINE('HAVE_IBASE', 1, 'Have interbase library');
	} else {
		WARNING("interbase not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\intl\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("intl", "Enable internationalization support", "no"); */

if (PHP_INTL != "no") {
	if (CHECK_LIB("icuuc.lib", "intl", PHP_INTL) &&
					CHECK_HEADER_ADD_INCLUDE("unicode/utf.h", "CFLAGS_INTL")) {
		// always build as shared - zend_strtod.c/ICU type conflict
		EXTENSION("intl", "php_intl.c intl_convert.c intl_convertcpp.cpp intl_error.c ", true,
								"/I \"" + configure_module_dirname + "\"");
		ADD_SOURCES(configure_module_dirname + "/collator", "\
				collator.c \
				collator_attr.c \
				collator_class.c \
				collator_compare.c \
				collator_convert.c \
				collator_create.c \
				collator_error.c \
				collator_is_numeric.c \
				collator_locale.c \
				collator_sort.c \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/common", "\
				common_error.c \
				common_enum.cpp \
				common_date.cpp \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/converter", "\
				converter.c \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/formatter", "\
				formatter.c \
				formatter_attr.c \
				formatter_class.c \
				formatter_data.c \
				formatter_format.c \
				formatter_main.c \
				formatter_parse.c \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/locale", "\
				locale.c \
				locale_class.c \
				locale_methods.c \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/msgformat", "\
				msgformat.c \
				msgformat_attr.c \
				msgformat_class.c \
				msgformat_data.c \
				msgformat_format.c \
				msgformat_helpers.cpp \
				msgformat_parse.c \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/grapheme", "\
                                grapheme_string.c grapheme_util.c  \
                                ", "intl");
		ADD_SOURCES(configure_module_dirname + "/normalizer", "\
				normalizer.c \
				normalizer_class.c \
				normalizer_normalize.c \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/dateformat", "\
				dateformat.c \
				dateformat_class.c \
				dateformat_attr.c \
				dateformat_format.c \
				dateformat_format_object.cpp \
				dateformat_parse.c \
				dateformat_data.c \
				dateformat_attrcpp.cpp \
				dateformat_helpers.cpp \
				dateformat_create.cpp \
				", "intl");
		ADD_SOURCES(configure_module_dirname + "/idn", "\
				idn.c",
				"intl");
		ADD_SOURCES(configure_module_dirname + "/resourcebundle", "\
				resourcebundle.c \
				resourcebundle_class.c \
				resourcebundle_iterator.c",
				"intl");

		if (CHECK_HEADER_ADD_INCLUDE("unicode/uspoof.h", "CFLAGS_INTL")) {
			ADD_SOURCES(configure_module_dirname + "/spoofchecker", "\
					spoofchecker.c \
					spoofchecker_class.c \
					spoofchecker_create.c \
					spoofchecker_main.c",
					"intl");
					AC_DEFINE("HAVE_INTL", 1, "Internationalization support enabled");
		}

		ADD_SOURCES(configure_module_dirname + "/transliterator", "\
				transliterator.c \
				transliterator_class.c \
				transliterator_methods.c",
				"intl");
		
		ADD_SOURCES(configure_module_dirname + "/timezone", "\
				timezone_class.cpp \
				timezone_methods.cpp",
				"intl");
				
		ADD_SOURCES(configure_module_dirname + "/calendar", "\
				calendar_methods.cpp \
				gregoriancalendar_methods.cpp \
				calendar_class.cpp",
				"intl");
				
		ADD_SOURCES(configure_module_dirname + "/breakiterator", "\
				breakiterator_class.cpp \
				breakiterator_methods.cpp \
				breakiterator_iterators.cpp \
				rulebasedbreakiterator_methods.cpp \
				codepointiterator_internal.cpp \
				codepointiterator_methods.cpp ",
				"intl");
		
		ADD_FLAG("LIBS_INTL", "icudt.lib icuin.lib icuio.lib icule.lib iculx.lib");
		ADD_FLAG("CFLAGS_INTL", "/EHsc");
		AC_DEFINE("HAVE_INTL", 1, "Internationalization support enabled");
	} else {
		WARNING("intl not enabled; libraries and/or headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\json\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("json", "JavaScript Object Serialization support", "yes"); */

if (PHP_JSON != "no") {
	EXTENSION('json', 'json.c', PHP_JSON_SHARED, "");
	ADD_SOURCES(configure_module_dirname, "JSON_parser.c", "json");
	PHP_INSTALL_HEADERS("ext/json/", "php_json.h");
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\ldap\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("ldap", "LDAP support", "no"); */

if (PHP_LDAP != "no") {

	if (CHECK_HEADER_ADD_INCLUDE("ldap.h", "CFLAGS_LDAP", PHP_PHP_BUILD + "\\include\\openldap;" + PHP_PHP_BUILD + "\\openldap\\include;" + PHP_LDAP) &&
			CHECK_HEADER_ADD_INCLUDE("lber.h", "CFLAGS_LDAP", PHP_PHP_BUILD + "\\include\\openldap;" + PHP_PHP_BUILD + "\\openldap\\include;" + PHP_LDAP)
			&&
			CHECK_LIB("ssleay32.lib", "ldap", PHP_LDAP) &&
			CHECK_LIB("libeay32.lib", "ldap", PHP_LDAP) &&
			CHECK_LIB("oldap32_a.lib", "ldap", PHP_LDAP) &&
			CHECK_LIB("olber32_a.lib", "ldap", PHP_LDAP)&&
			CHECK_LIB("libsasl.lib", "ldap", PHP_LDAP)) {
		EXTENSION('ldap', 'ldap.c');

		AC_DEFINE('HAVE_LDAP_PARSE_RESULT', 1);
		AC_DEFINE('HAVE_LDAP_PARSE_REFERENCE', 1);
		AC_DEFINE('HAVE_LDAP_START_TLS_S', 1);
		AC_DEFINE('HAVE_LDAP', 1);
		AC_DEFINE('HAVE_LDAP_SASL', 1);
		AC_DEFINE('HAVE_LDAP_SASL_SASL_H', 1);
		AC_DEFINE('LDAP_DEPRECATED', 1);
		AC_DEFINE('HAVE_LDAP_CONTROL_FIND', 1);

	} else {
		WARNING("ldap not enabled; libraries and headers not found");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\mbstring\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("libmbfl", "use external libmbfl", "no"); */
/* ARG_ENABLE("mbstring", "multibyte string functions", "no"); */
/* ARG_ENABLE("mbregex", "multibyte regex support", "no"); */
/* ARG_ENABLE("mbregex-backtrack", "check multibyte regex backtrack", "yes"); */

if (PHP_MBSTRING != "no") {

	EXTENSION("mbstring", "mbstring.c php_unicode.c mb_gpc.c", PHP_MBSTRING_SHARED);

	if (PHP_LIBMBFL != "no" &&
			CHECK_HEADER_ADD_INCLUDE("mbfl/mbfilter.h", "CFLAGS_LIBMBFL", PHP_LIBMBFL + "\\include") &&
			CHECK_LIB("mbfl.lib", "libmbfl", PHP_LIBMBFL + "\\lib")) {

		ADD_FLAG("LIBS_MBSTRING", get_define("LIBS_LIBMBFL"));
		ADD_FLAG("LDFLAGS_MBSTRING", get_define("LDFLAGS_LIBMBFL"));
		ADD_FLAG("CFLAGS_MBSTRING", get_define("CFLAGS_LIBMBFL") +
			" /I ext/mbstring/oniguruma /D NOT_RUBY=1 \
		          /D HAVE_STDARG_PROTOTYPES=1 /D HAVE_STDLIB_H \
		          /D HAVE_STRICMP /D EXPORT");

		PHP_INSTALL_HEADERS("ext/mbstring", "mbstring.h oniguruma/oniguruma.h php_mbregex.h php_onig_compat.h");
	} else {
		STDOUT.WriteLine("Using bundled libmbfl...");

		ADD_FLAG("CFLAGS_MBSTRING", "-Iext/mbstring/libmbfl -Iext/mbstring/libmbfl/mbfl \
			-Iext/mbstring/oniguruma /D NOT_RUBY=1 /D LIBMBFL_EXPORTS=1 \
		        /D HAVE_STDARG_PROTOTYPES=1 /D HAVE_CONFIG_H /D HAVE_STDLIB_H \
		        /D HAVE_STRICMP /D MBFL_DLL_EXPORT=1 /D EXPORT")

		FSO.CopyFile("ext\\mbstring\\libmbfl\\config.h.w32",
		   	"ext\\mbstring\\libmbfl\\config.h", true);

		ADD_SOURCES("ext/mbstring/libmbfl/filters", "html_entities.c \
			mbfilter_7bit.c mbfilter_ascii.c mbfilter_base64.c mbfilter_big5.c \
			mbfilter_byte2.c mbfilter_byte4.c mbfilter_cp1251.c mbfilter_cp1252.c \
			mbfilter_cp866.c mbfilter_cp932.c mbfilter_cp936.c mbfilter_cp51932.c \
			mbfilter_euc_cn.c mbfilter_euc_jp.c mbfilter_euc_jp_win.c mbfilter_euc_kr.c \
			mbfilter_euc_tw.c mbfilter_htmlent.c mbfilter_hz.c mbfilter_iso2022_kr.c \
			mbfilter_iso8859_1.c mbfilter_iso8859_10.c mbfilter_iso8859_13.c \
			mbfilter_iso8859_14.c mbfilter_iso8859_15.c mbfilter_iso8859_16.c \
			mbfilter_iso8859_2.c mbfilter_iso8859_3.c mbfilter_iso8859_4.c \
			mbfilter_iso8859_5.c mbfilter_iso8859_6.c mbfilter_iso8859_7.c \
			mbfilter_iso8859_8.c mbfilter_iso8859_9.c mbfilter_jis.c \
			mbfilter_iso2022_jp_ms.c mbfilter_gb18030.c mbfilter_sjis_2004.c \
			mbfilter_koi8r.c mbfilter_qprint.c mbfilter_sjis.c mbfilter_ucs2.c \
			mbfilter_ucs4.c mbfilter_uhc.c mbfilter_utf16.c mbfilter_utf32.c \
			mbfilter_utf7.c mbfilter_utf7imap.c mbfilter_utf8.c mbfilter_utf8_mobile.c \
			mbfilter_koi8u.c mbfilter_cp1254.c mbfilter_euc_jp_2004.c \
			mbfilter_uuencode.c mbfilter_armscii8.c mbfilter_cp850.c \
			mbfilter_cp5022x.c mbfilter_sjis_open.c mbfilter_sjis_mobile.c \
			mbfilter_sjis_mac.c \
			mbfilter_iso2022jp_2004.c  mbfilter_iso2022jp_mobile.c \
			mbfilter_tl_jisx0201_jisx0208.c", "mbstring");

		ADD_SOURCES("ext/mbstring/libmbfl/mbfl", "mbfilter.c mbfilter_8bit.c \
			mbfilter_pass.c mbfilter_wchar.c mbfl_convert.c mbfl_encoding.c \
			mbfl_filter_output.c mbfl_ident.c mbfl_language.c mbfl_memory_device.c \
			mbfl_string.c mbfl_allocators.c", "mbstring");

		ADD_SOURCES("ext/mbstring/libmbfl/nls", "nls_de.c nls_en.c nls_ja.c \
			nls_kr.c nls_neutral.c nls_ru.c nls_uni.c nls_zh.c nls_hy.c \
			nls_ua.c nls_tr.c", "mbstring");

		PHP_INSTALL_HEADERS("ext/mbstring", "mbstring.h oniguruma/oniguruma.h php_mbregex.h php_onig_compat.h libmbfl/config.h libmbfl/mbfl/eaw_table.h libmbfl/mbfl/mbfilter.h libmbfl/mbfl/mbfilter_8bit.h libmbfl/mbfl/mbfilter_pass.h libmbfl/mbfl/mbfilter_wchar.h libmbfl/mbfl/mbfl_allocators.h libmbfl/mbfl/mbfl_consts.h libmbfl/mbfl/mbfl_convert.h libmbfl/mbfl/mbfl_defs.h libmbfl/mbfl/mbfl_encoding.h libmbfl/mbfl/mbfl_filter_output.h libmbfl/mbfl/mbfl_ident.h libmbfl/mbfl/mbfl_language.h libmbfl/mbfl/mbfl_memory_device.h libmbfl/mbfl/mbfl_string.h");
	}

	AC_DEFINE('HAVE_MBSTRING', 1, 'Have mbstring support');
	AC_DEFINE('HAVE_MBSTR_CN', 1, 'CN');
	AC_DEFINE('HAVE_MBSTR_JA', 1, 'JA');
	AC_DEFINE('HAVE_MBSTR_KR', 1, 'KR');
	AC_DEFINE('HAVE_MBSTR_RU', 1, 'RU');
	AC_DEFINE('HAVE_MBSTR_TW', 1, 'TW');

	FSO.CopyFile("ext\\mbstring\\oniguruma\\win32\\config.h",
		"ext\\mbstring\\oniguruma\\config.h", true);

	if (PHP_MBREGEX != "no") {
		AC_DEFINE('HAVE_STDARG_PROTOTYPES', 1, 'have stdarg.h');
		AC_DEFINE('HAVE_MBREGEX', 1);
		AC_DEFINE('HAVE_ONIG', 1);
		if (PHP_MBREGEX_BACKTRACK != "no") {
			AC_DEFINE('USE_COMBINATION_EXPLOSION_CHECK', 1);
		}
		ADD_SOURCES("ext/mbstring/oniguruma", "regcomp.c regerror.c \
			regenc.c regexec.c reggnu.c regparse.c regposerr.c \
			regext.c regsyntax.c regtrav.c regversion.c st.c", "mbstring");
		ADD_SOURCES("ext/mbstring/oniguruma/enc", "ascii.c big5.c \
			euc_jp.c euc_kr.c euc_tw.c iso8859_1.c iso8859_2.c \
			iso8859_3.c iso8859_4.c iso8859_5.c iso8859_6.c \
			iso8859_7.c iso8859_8.c iso8859_9.c iso8859_10.c \
			iso8859_11.c iso8859_13.c iso8859_14.c iso8859_15.c iso8859_16.c \
			koi8.c koi8_r.c sjis.c utf8.c unicode.c utf16_be.c utf16_le.c \
			utf32_be.c utf32_le.c gb18030.c", "mbstring");
		ADD_SOURCES("ext/mbstring", "php_mbregex.c", "mbstring");
		PHP_INSTALL_HEADERS("ext/mbstring", "php_mbregex.h");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\mcrypt\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("mcrypt", "mcrypt support", "no"); */

if (PHP_MCRYPT != "no") {

	if (CHECK_HEADER_ADD_INCLUDE('mcrypt.h', 'CFLAGS_MCRYPT') &&
			CHECK_LIB('libmcrypt_a.lib;libmcrypt.lib', 'mcrypt') &&
			CHECK_LIB('Advapi32.lib', 'mcrypt')
			) {

		EXTENSION('mcrypt', 'mcrypt.c mcrypt_filter.c', false);
		AC_DEFINE('HAVE_LIBMCRYPT', 1);
		AC_DEFINE('HAVE_LIBMCRYPT24', 1);
	} else {
		WARNING("mcrypt not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\mssql\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("mssql", "mssql support", "no"); */

if (PHP_MSSQL != "no") {
	if (CHECK_LIB("ntwdblib.lib", "mssql", "\\MSSQL7\\DevTools\\Lib;\\MSSQL7\\DevTools\\Lib;" + PHP_MSSQL) &&
			CHECK_HEADER_ADD_INCLUDE("sqlfront.h", "CFLAGS_MSSQL", "\\MSSQL7\\DevTools\\Include;\\MSSQL7\\DevTools\\Include" + PHP_MSSQL)) {
		EXTENSION("mssql", "php_mssql.c");
		AC_DEFINE('HAVE_MSSQL', 1, 'Have MSSQL support');
		ADD_FLAG("CFLAGS_MSSQL", "/D DBNTWIN32=1 /D MSSQL70=1");
	} else {
		WARNING("mssql not enabled; libraries and headers not found");
	}
}

/* ARG_WITH("dblib", "mssql support with freetds", "no"); */

if (PHP_DBLIB != "no") {
	if (CHECK_LIB("dblib.lib", "dblib", PHP_PHP_BUILD + "\\freetds") &&
			CHECK_HEADER_ADD_INCLUDE("sybfront.h", "CFLAGS_DBLIB", PHP_PHP_BUILD + "\\freetds;" + PHP_DBLIB)) {
		EXTENSION("dblib", "php_mssql.c", null, null, null, "ext\\dblib");
		AC_DEFINE('HAVE_MSSQL', 1, 'Have MSSQL support');
		ADD_FLAG("CFLAGS_DBLIB", "/D HAVE_FREETDS=1 /D DBNTWIN32=1 /D MSSQL70=1 /D MSSQL_EXPORTS /D COMPILE_DL_MSSQL");
	} else {
		WARNING("dblib not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\mysqlnd\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("mysqlnd", "Mysql Native Client Driver", "yes"); */
if (PHP_MYSQLND != "no") {

	if (CHECK_LIB("ws2_32.lib", "mysqlnd")) {
		mysqlnd_source = 
			"mysqlnd.c " +
			"mysqlnd_alloc.c " +
			"mysqlnd_auth.c " +
			"mysqlnd_block_alloc.c " +
			"mysqlnd_bt.c " +
			"mysqlnd_charset.c " +
			"mysqlnd_debug.c " +
			"mysqlnd_driver.c " +
			"mysqlnd_ext_plugin.c " +
			"mysqlnd_loaddata.c " +
			"mysqlnd_reverse_api.c " +
			"mysqlnd_net.c " +
			"mysqlnd_plugin.c " +
			"mysqlnd_ps.c " +
			"mysqlnd_ps_codec.c " +
			"mysqlnd_result.c " +
			"mysqlnd_result_meta.c " +
			"mysqlnd_statistics.c " +
			"mysqlnd_wireprotocol.c " +
			"php_mysqlnd.c ";
		EXTENSION("mysqlnd", mysqlnd_source, false);
		if (((PHP_ZLIB=="no") && (CHECK_LIB("zlib_a.lib;zlib.lib", "mysqlnd", PHP_MYSQLND))) || 
			(PHP_ZLIB_SHARED && CHECK_LIB("zlib.lib", "mysqlnd", PHP_MYSQLND)) || (PHP_ZLIB == "yes" && (!PHP_ZLIB_SHARED)))
		{
			AC_DEFINE("MYSQLND_COMPRESSION_ENABLED", 1, "Compression support");
			AC_DEFINE("MYSQLND_SSL_SUPPORTED", 1, "SSL support");
		}
		PHP_INSTALL_HEADERS("", "ext/mysqlnd");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\oci8\\config.w32'));
// $Id$
// vim:ft=javascript

if (PHP_OCI8 != "no" && PHP_OCI8_11G != "no") {
	if (!PHP_OCI8_SHARED && !PHP_OCI8_11G_SHARED) {
		WARNING("oci8 and oci8-11g provide the same extension and cannot both be built statically");
		PHP_OCI8 = "no"
	}
}

if (PHP_OCI8 != "no" && PHP_OCI8_12C != "no") {
	if (!PHP_OCI8_SHARED && !PHP_OCI8_12C_SHARED) {
		WARNING("oci8 and oci8-12c provide the same extension and cannot both be built statically");
		PHP_OCI8 = "no"
	}
}

if (PHP_OCI8_11G != "no" && PHP_OCI8_12C != "no") {
	if (!PHP_OCI8_11G_SHARED && !PHP_OCI8_12C_SHARED) {
		WARNING("oci8-11g and oci8-12c provide the same extension and cannot both be built statically");
		PHP_OCI8_11G = "no"
	}
}

/* ARG_WITH("oci8", "OCI8 support", "no"); */

if (PHP_OCI8 != "no") {

	oci8_dirs = new Array(
		PHP_OCI8
	);

	oci8_lib_paths = "";
	oci8_inc_paths = "";

	// find the Oracle install
	for (i = 0; i < oci8_dirs.length; i++) {
		oci8_lib_paths += oci8_dirs[i] + "\\lib;";
		oci8_lib_paths += oci8_dirs[i] + "\\lib\\msvc;";
		oci8_inc_paths += oci8_dirs[i] + "\\include;";
	}

	oci8_inc_paths += PHP_PHP_BUILD + "\\include\\instantclient;"
	oci8_lib_paths += PHP_PHP_BUILD + "\\lib\\instantclient;";

	if (CHECK_HEADER_ADD_INCLUDE("oci.h", "CFLAGS_OCI8", oci8_inc_paths) &&
			CHECK_LIB("oci.lib", "oci8", oci8_lib_paths))
	{
		EXTENSION('oci8', 'oci8.c oci8_lob.c oci8_statement.c oci8_collection.c oci8_interface.c');

		AC_DEFINE('HAVE_OCI8', 1);
		AC_DEFINE('HAVE_OCI_INSTANT_CLIENT', 1);

	} else {
		WARNING("oci8 not enabled: Oracle Database client libraries or Oracle 10g Instant Client not found");
		PHP_OCI8 = "no"
	}
}

/* ARG_WITH("oci8-11g", "OCI8 support using Oracle 11g Instant Client", "no"); */

if (PHP_OCI8_11G != "no") {

	oci8_11g_dirs = new Array(
		PHP_OCI8_11G
	);

	oci8_11g_lib_paths = "";
	oci8_11g_inc_paths = "";

	// find the Oracle install
	for (i = 0; i < oci8_11g_dirs.length; i++) {
		oci8_11g_lib_paths += oci8_11g_dirs[i] + "\\lib;";
		oci8_11g_lib_paths += oci8_11g_dirs[i] + "\\lib\\msvc;";
		oci8_11g_inc_paths += oci8_11g_dirs[i] + "\\include;";
	}

	oci8_11g_inc_paths += PHP_PHP_BUILD + "\\include\\instantclient_11;"
	oci8_11g_lib_paths += PHP_PHP_BUILD + "\\lib\\instantclient_11;";

	if (CHECK_HEADER_ADD_INCLUDE("oci.h", "CFLAGS_OCI8_11G", oci8_11g_inc_paths) &&
			CHECK_LIB("oci.lib", "oci8_11g", oci8_11g_lib_paths))
	{
		EXTENSION('oci8_11g', 'oci8.c oci8_lob.c oci8_statement.c oci8_collection.c oci8_interface.c', null, null, null, "ext\\oci8_11g")

		AC_DEFINE('HAVE_OCI8', 1);
		AC_DEFINE('HAVE_OCI_INSTANT_CLIENT', 1);

	} else {
		WARNING("oci8-11g not enabled: Oracle Database client libraries or Oracle 11g Instant Client not found");
		PHP_OCI8_11G = "no"
	}
}

/* ARG_WITH("oci8-12c", "OCI8 support using Oracle Database 12c Instant Client", "no"); */

if (PHP_OCI8_12C != "no") {

	oci8_12c_dirs = new Array(
		PHP_OCI8_12C
	);

	oci8_12c_lib_paths = "";
	oci8_12c_inc_paths = "";

	// find the Oracle install
	for (i = 0; i < oci8_12c_dirs.length; i++) {
		oci8_12c_lib_paths += oci8_12c_dirs[i] + "\\lib;";
		oci8_12c_lib_paths += oci8_12c_dirs[i] + "\\lib\\msvc;";
		oci8_12c_inc_paths += oci8_12c_dirs[i] + "\\include;";
	}

	oci8_12c_inc_paths += PHP_PHP_BUILD + "\\include\\instantclient_12;"
	oci8_12c_lib_paths += PHP_PHP_BUILD + "\\lib\\instantclient_12;";

	if (CHECK_HEADER_ADD_INCLUDE("oci.h", "CFLAGS_OCI8_12C", oci8_12c_inc_paths) &&
			CHECK_LIB("oci.lib", "oci8_12c", oci8_12c_lib_paths))
	{
		EXTENSION('oci8_12c', 'oci8.c oci8_lob.c oci8_statement.c oci8_collection.c oci8_interface.c', null, null, null, "ext\\oci8_12c")

		AC_DEFINE('HAVE_OCI8', 1);
		AC_DEFINE('HAVE_OCI_INSTANT_CLIENT', 1);
	} else {
		WARNING("oci8-12c not enabled: Oracle Database client libraries or Oracle Database 12c Instant Client not found");
		PHP_OCI8_12C = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\odbc\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("odbc", "ODBC support", "yes"); */
/* ARG_WITH("odbcver", "Force support for the passed ODBC version. A hex number is expected, default 0x0300. Use the special value of 0 to prevent an explicit ODBCVER to be defined.", "0x0300"); */

if (PHP_ODBC == "yes") {
	if (CHECK_LIB("odbc32.lib", "odbc") && CHECK_LIB("odbccp32.lib", "odbc")
	&& CHECK_HEADER_ADD_INCLUDE("sql.h", "CFLAGS_ODBC")
	&& CHECK_HEADER_ADD_INCLUDE("sqlext.h", "CFLAGS_ODBC")) {
		EXTENSION("odbc", "php_odbc.c");
		AC_DEFINE("HAVE_UODBC", 1, "ODBC support");
		if ("no" == PHP_ODBCVER) {
			AC_DEFINE("ODBCVER", "0x0300", "The highest supported ODBC version", false);
		} else if ("0" != PHP_ODBCVER) {
			AC_DEFINE("ODBCVER", PHP_ODBCVER, "The highest supported ODBC version", false);
		}
	} else {
		WARNING("odbc support can't be enabled, libraries or header are missing (SDK)")
		PHP_ODBC = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\opcache\\config.w32'));
/* ARG_ENABLE("opcache", "whether to enable Zend OPcache support", "yes"); */

var PHP_OPCACHE_PGO = false;

if (PHP_OPCACHE != "no") {

	EXTENSION('opcache', "\
		ZendAccelerator.c \
		zend_accelerator_blacklist.c \
		zend_accelerator_debug.c \
		zend_accelerator_hash.c \
		zend_accelerator_module.c \
		zend_accelerator_util_funcs.c \
		zend_persist.c \
		zend_persist_calc.c \
		zend_shared_alloc.c \
		shared_alloc_win32.c", true);

	ADD_SOURCES(configure_module_dirname + "/Optimizer", "zend_optimizer.c", "opcache", "OptimizerObj");

  
	ADD_FLAG('CFLAGS_OPCACHE', "/I " + configure_module_dirname);

	ADD_FLAG('CFLAGS_OPCACHE', "/Dregexec=php_regexec /Dregerror=php_regerror /Dregfree=php_regfree /Dregcomp=php_regcomp /Iext/ereg/regex");

}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\openssl\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("openssl", "OpenSSL support", "no"); */

if (PHP_OPENSSL != "no") {
	if (CHECK_LIB("ssleay32.lib", "openssl", PHP_OPENSSL) &&
			CHECK_LIB("libeay32.lib", "openssl", PHP_OPENSSL) &&
			CHECK_LIB("crypt32.lib", "openssl") &&
			CHECK_HEADER_ADD_INCLUDE("openssl/ssl.h", "CFLAGS_OPENSSL")) {
		EXTENSION("openssl", "openssl.c xp_ssl.c");

		AC_DEFINE("HAVE_OPENSSL_EXT", PHP_OPENSSL_SHARED ? 0 : 1, "Have openssl");
		AC_DEFINE("HAVE_OPENSSL", 1);
		AC_DEFINE("HAVE_DSA_DEFAULT_METHOD", 1);
	}
}


configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pcre\\config.w32'));
// $Id$
// vim:ft=javascript

EXTENSION("pcre", "php_pcre.c", false /* never shared */,
		"-Iext/pcre/pcrelib");
ADD_SOURCES("ext/pcre/pcrelib", "pcre_chartables.c pcre_ucd.c pcre_compile.c pcre_config.c pcre_exec.c pcre_fullinfo.c pcre_get.c pcre_globals.c pcre_maketables.c pcre_newline.c pcre_ord2utf8.c pcre_refcount.c pcre_study.c pcre_tables.c pcre_valid_utf8.c pcre_version.c pcre_xclass.c pcre_jit_compile.c", "pcre");
ADD_DEF_FILE("ext\\pcre\\php_pcre.def");

AC_DEFINE('HAVE_BUNDLED_PCRE', 1, 'Using bundled PCRE library');
AC_DEFINE('HAVE_PCRE', 1, 'Have PCRE library');
PHP_PCRE="yes";
PHP_INSTALL_HEADERS("ext/pcre", "php_pcre.h pcrelib/");
ADD_FLAG("CFLAGS_PCRE", " /D HAVE_CONFIG_H");
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pgsql\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pgsql", "PostgreSQL support", "no"); */

if (PHP_PGSQL != "no") {
	if (CHECK_LIB("libpq.lib", "pgsql", PHP_PGSQL) &&
		CHECK_HEADER_ADD_INCLUDE("libpq-fe.h", "CFLAGS_PGSQL", PHP_PGSQL + "\\include;" + PHP_PHP_BUILD + "\\include\\pgsql;" + PHP_PHP_BUILD + "\\include\\libpq;" + PHP_PGSQL)) {
		EXTENSION("pgsql", "pgsql.c");
		AC_DEFINE('HAVE_PGSQL', 1, 'Have PostgreSQL library');
		ADD_FLAG("CFLAGS_PGSQL", "/D HAVE_PG_CONFIG_H /D PGSQL_EXPORTS /D HAVE_PQSETNONBLOCKING /D HAVE_PQCMDTUPLES /D HAVE_PQCLIENTENCODING /D HAVE_PQESCAPE /D HAVE_PQPARAMETERSTATUS /D HAVE_PGTRANSACTIONSTATUS /D HAVE_PQEXECPARAMS /D HAVE_PQPREPARE /D HAVE_PQEXECPREPARED /D HAVE_PQRESULTERRORFIELD /D HAVE_PQSENDQUERYPARAMS /D HAVE_PQSENDPREPARE /D HAVE_PQSENDQUERYPREPARED /D HAVE_PQPUTCOPYDATA /D HAVE_PQPUTCOPYEND /D HAVE_PQGETCOPYDATA /D HAVE_PQSETERRORVERBOSITY /D HAVE_PQUNESCAPEBYTEA /D HAVE_PQFTABLE /D HAVE_PQESCAPE_CONN /D HAVE_PQESCAPE_BYTEA_CONN /D HAVE_PQFREEMEM /D HAVE_PGSQL_WITH_MULTIBYTE_SUPPORT /D HAVE_PQPROTOCOLVERSION /D HAVE_PG_LO_CREATE /D HAVE_PG_LO_IMPORT_WITH_OID /D HAVE_PG_LO_TRUNCATE /D HAVE_PG_LO64 /D HAVE_PQESCAPELITERAL");
	} else {
		WARNING("pgsql not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pspell\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pspell", "pspell/aspell (whatever it's called this month) support", "no"); */

if (PHP_PSPELL != "no") {

	if (CHECK_HEADER_ADD_INCLUDE("pspell.h", "CFLAGS_PSPELL", PHP_PHP_BUILD + "\\include\\pspell;" + PHP_PSPELL) &&
			CHECK_LIB("aspell*.lib", "pspell", PHP_PSPELL)) {
		EXTENSION('pspell', 'pspell.c');
		AC_DEFINE('HAVE_PSPELL', 1);
	} else {
		WARNING("pspell not enabled; libraries and headers not found");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\reflection\\config.w32'));
// $Id$
// vim:ft=javascript

EXTENSION("reflection", "php_reflection.c", false /* never shared */);
AC_DEFINE('HAVE_REFLECTION', 1, 'Reflection support enabled');
PHP_REFLECTION="yes";configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\session\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("session", "session support", "yes"); */

if (PHP_SESSION == "yes") {
	EXTENSION("session", "mod_user_class.c session.c mod_files.c mod_mm.c mod_user.c", false /* never shared */);
	AC_DEFINE("HAVE_PHP_SESSION", 1, "Session support");
	PHP_INSTALL_HEADERS("ext/session/", "mod_mm.h php_session.h mod_files.h mod_user.h");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\shmop\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("shmop", "shmop support", "no"); */

if (PHP_SHMOP == "yes") {
	EXTENSION("shmop", "shmop.c");
	AC_DEFINE('HAVE_SHMOP', 1, 'Have SHMOP support');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\snmp\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("snmp", "SNMP support", "no"); */

if (PHP_SNMP != "no") {
	if (CHECK_HEADER_ADD_INCLUDE("snmp.h", "CFLAGS_SNMP", PHP_PHP_BUILD + "\\include\\net-snmp;" + PHP_SNMP)) {
		if (CHECK_LIB("netsnmp.lib", "snmp", PHP_SNMP)) {
			EXTENSION('snmp', 'snmp.c');
			CHECK_LIB("libeay32.lib", "snmp", PHP_SNMP)
			AC_DEFINE('HAVE_SNMP', 1);
			AC_DEFINE("HAVE_NET_SNMP", 1);
		} else {
			WARNING("snmp not enabled; libraries and headers not found");
		}
	} else {
		WARNING("snmp not enabled; libraries and headers not found");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\sockets\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("sockets", "SOCKETS support", "no"); */

if (PHP_SOCKETS != "no") {
	if (CHECK_LIB("ws2_32.lib", "sockets", PHP_SOCKETS)
	&& CHECK_LIB("Iphlpapi.lib", "sockets", PHP_SOCKETS)
	&& CHECK_HEADER_ADD_INCLUDE("winsock.h", "CFLAGS_SOCKETS")) {
		EXTENSION('sockets', 'sockets.c multicast.c conversions.c sockaddr_conv.c sendrecvmsg.c');
		AC_DEFINE('HAVE_SOCKETS', 1);
		PHP_INSTALL_HEADERS("ext/sockets", "php_sockets.h");
	} else {
		WARNING("sockets not enabled; libraries and headers not found");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\spl\\config.w32'));
// $Id$
// vim:ft=javascript

EXTENSION("spl", "php_spl.c spl_functions.c spl_engine.c spl_iterators.c spl_array.c spl_directory.c spl_exceptions.c spl_observer.c spl_dllist.c spl_heap.c spl_fixedarray.c", false /*never shared */);
AC_DEFINE('HAVE_SPL', 1);
PHP_SPL="yes";
PHP_INSTALL_HEADERS("ext/spl", "php_spl.h spl_array.h spl_directory.h spl_engine.h spl_exceptions.h spl_functions.h spl_iterators.h spl_observer.h spl_dllist.h spl_heap.h spl_fixedarray.h");
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\sqlite3\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("sqlite3", "SQLite 3 support", "no"); */

if (PHP_SQLITE3 != "no") {
	ADD_FLAG("CFLAGS_SQLITE3", "/D SQLITE_THREADSAFE=" + (PHP_ZTS == "yes" ? "1" : "0") + " /D SQLITE_ENABLE_FTS3=1 /D SQLITE_ENABLE_COLUMN_METADATA=1 /D SQLITE_CORE=1 /D SQLITE_API=__declspec(dllexport) ");
	EXTENSION("sqlite3", "sqlite3.c", null, "/I" + configure_module_dirname + "/libsqlite /I" + configure_module_dirname);

	ADD_SOURCES(configure_module_dirname + "/libsqlite", "sqlite3.c", "sqlite3");

	AC_DEFINE("HAVE_SQLITE3", 1, "SQLite support");
	PHP_INSTALL_HEADERS("ext/sqlite3", "libsqlite/sqlite3.h");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\standard\\config.w32'));
// vim:ft=javascript
// $Id$

/* ARG_WITH("config-file-scan-dir", "Dir to check for additional php ini files", ""); */

AC_DEFINE("PHP_CONFIG_FILE_SCAN_DIR", PHP_CONFIG_FILE_SCAN_DIR);
AC_DEFINE("PHP_USE_PHP_CRYPT_R", 1);

CHECK_HEADER_ADD_INCLUDE("timelib_config.h", "CFLAGS_STANDARD", "ext/date/lib");

EXTENSION("standard", "array.c base64.c basic_functions.c browscap.c \
	crc32.c crypt.c crypt_freesec.c crypt_blowfish.c crypt_sha256.c \
	crypt_sha512.c  php_crypt_r.c \
	cyr_convert.c datetime.c dir.c dl.c dns.c dns_win32.c exec.c \
	file.c filestat.c formatted_print.c fsock.c head.c html.c image.c \
	info.c iptc.c lcg.c link_win32.c mail.c math.c md5.c metaphone.c microtime.c \
	pack.c pageinfo.c quot_print.c rand.c soundex.c \
	string.c scanf.c syslog.c type.c uniqid.c url.c var.c \
	versioning.c assert.c strnatcmp.c levenshtein.c incomplete_class.c \
	url_scanner_ex.c ftp_fopen_wrapper.c http_fopen_wrapper.c \
	php_fopen_wrapper.c credits.c css.c var_unserializer.c ftok.c sha1.c \
	user_filters.c uuencode.c filters.c proc_open.c password.c \
	streamsfuncs.c http.c flock_compat.c", false /* never shared */);
	PHP_INSTALL_HEADERS("", "ext/standard");
if (PHP_MBREGEX != "no") {
	CHECK_HEADER_ADD_INCLUDE("oniguruma.h", "CFLAGS_STANDARD", PHP_MBREGEX + ";ext\\mbstring\\oniguruma")   
}
PHP_INSTALL_HEADERS("", "ext/standard");
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\sybase_ct\\config.w32'));

// $Id$
// vim:ft=javascript

/* ARG_WITH("sybase-ct", "SYBASE_CT support", "no"); */

if (PHP_SYBASE_CT != "no") {

	if (CHECK_HEADER_ADD_INCLUDE("ctpublic.h", "CFLAGS_SYBASE_CT", PHP_PHP_BUILD + "\\sybase\\include;" + PHP_SYBASE_CT) &&
			(!X64 && CHECK_LIB("libsybcs.lib", "sybase_ct", PHP_PHP_BUILD + "\\sybase\\lib;" + PHP_SYBASE_CT) &&
			CHECK_LIB("libsybct.lib", "sybase_ct", PHP_PHP_BUILD + "\\sybase\\lib;" + PHP_SYBASE_CT) ||
			X64 && CHECK_LIB("libsybcs64.lib", "sybase_ct", PHP_PHP_BUILD + "\\sybase\\lib;" + PHP_SYBASE_CT) &&
			CHECK_LIB("libsybct64.lib", "sybase_ct", PHP_PHP_BUILD + "\\sybase\\lib;" + PHP_SYBASE_CT))
		) {
		EXTENSION('sybase_ct', 'php_sybase_ct.c');
		AC_DEFINE('HAVE_SYBASE_CT', 1);
	} else {
		WARNING("sybase_ct not enabled; libraries and headers not found");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\tidy\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("tidy", "TIDY support", "no"); */

if (PHP_TIDY != "no") {
	if (CHECK_LIB("libtidy_a.lib;libtidy.lib", "tidy", PHP_TIDY) &&
			(
				CHECK_HEADER_ADD_INCLUDE("tidy.h", "CFLAGS_TIDY") ||
				CHECK_HEADER_ADD_INCLUDE("tidy/tidy.h", "CFLAGS_TIDY", null, null, true) ||
				CHECK_HEADER_ADD_INCLUDE("libtidy/tidy.h", "CFLAGS_TIDY", null, null, true)
			)) {
		EXTENSION("tidy", "tidy.c");
		AC_DEFINE('HAVE_TIDY', 1, 'Have TIDY library');
		if (!PHP_TIDY_SHARED) {
			ADD_DEF_FILE("ext\\tidy\\php_tidy.def");
		}
	} else {
		WARNING("tidy not enabled; libraries and headers not found");
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\tokenizer\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("tokenizer", "tokenizer support", "yes"); */

if (PHP_TOKENIZER == "yes") {
	EXTENSION("tokenizer", "tokenizer.c tokenizer_data.c");
	AC_DEFINE("HAVE_TOKENIZER", 1, "Tokenizer support");
}


configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\zip\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("zip", "ZIP support", "yes"); */

if (PHP_ZIP != "no") {
	if (CHECK_HEADER_ADD_INCLUDE("zlib.h", "CFLAGS_ZIP", "..\\zlib;" + PHP_ZIP) &&
		CHECK_HEADER_ADD_INCLUDE("zipconf.h", "CFLAGS_ZIP", configure_module_dirname + "\\lib;" + PHP_ZIP) &&
		(((PHP_ZLIB=="no") && (CHECK_LIB("zlib_a.lib", "zip", PHP_ZIP) ||  CHECK_LIB("zlib.lib", "zip", PHP_ZIP))) || 
			(PHP_ZLIB_SHARED && CHECK_LIB("zlib.lib", "zip", PHP_ZIP)) || (PHP_ZLIB == "yes" && (!PHP_ZLIB_SHARED)))
		) {
		EXTENSION('zip', 'php_zip.c zip_stream.c');
		ADD_SOURCES(configure_module_dirname + "/lib", "zip_add.c zip_add_dir.c zip_add_entry.c\
			zip_close.c zip_delete.c zip_dir_add.c zip_dirent.c zip_discard.c zip_entry.c\
			zip_err_str.c zip_error.c zip_error_clear.c zip_error_get.c zip_error_get_sys_type.c\
			zip_error_strerror.c zip_error_to_str.c zip_extra_field.c zip_extra_field_api.c\
			zip_fclose.c zip_fdopen.c zip_file_add.c zip_file_error_clear.c zip_file_error_get.c\
			zip_file_get_comment.c zip_file_get_offset.c zip_file_rename.c zip_file_replace.c\
			zip_file_set_comment.c zip_file_strerror.c zip_filerange_crc.c zip_fopen.c\
			zip_fopen_encrypted.c zip_fopen_index.c zip_fopen_index_encrypted.c zip_fread.c\
			zip_get_archive_comment.c zip_get_archive_flag.c zip_get_compression_implementation.c\
			zip_get_encryption_implementation.c zip_get_file_comment.c zip_get_name.c zip_get_num_entries.c \
			zip_get_num_files.c zip_memdup.c zip_name_locate.c zip_new.c zip_open.c zip_rename.c zip_replace.c\
			zip_set_archive_comment.c zip_set_archive_flag.c zip_set_default_password.c zip_set_file_comment.c\
			zip_set_file_compression.c zip_set_name.c zip_source_buffer.c zip_source_close.c zip_source_crc.c\
			zip_source_deflate.c zip_source_error.c zip_source_file.c zip_source_filep.c zip_source_free.c\
			zip_source_function.c zip_source_layered.c zip_source_open.c zip_source_pkware.c zip_source_pop.c\
			zip_source_read.c zip_source_stat.c zip_source_window.c zip_source_zip.c zip_source_zip_new.c\
			zip_stat.c zip_stat_index.c zip_stat_init.c zip_strerror.c zip_string.c zip_unchange.c zip_unchange_all.c\
			zip_unchange_archive.c zip_unchange_data.c zip_utf-8.c mkstemp.c \
			zip_file_set_external_attributes.c zip_file_get_external_attributes.c", "zip");

		AC_DEFINE('HAVE_ZIP', 1);
		ADD_FLAG("CFLAGS_ZIP", "/D _WIN32");
	} else {
		WARNING("zip not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\zlib\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("zlib", "ZLIB support", "yes"); */

if (PHP_ZLIB == "yes") {
	if (CHECK_LIB("zlib_a.lib;zlib.lib", "zlib", PHP_ZLIB) &&
		CHECK_HEADER_ADD_INCLUDE("zlib.h", "CFLAGS", "..\\zlib;" + php_usual_include_suspects)) {

		EXTENSION("zlib", "zlib.c zlib_fopen_wrapper.c zlib_filter.c", null, "/D ZLIB_EXPORTS");
		AC_DEFINE("HAVE_ZLIB", 1, "ZLIB support");

		if (!PHP_ZLIB_SHARED) {
			ADD_DEF_FILE("ext\\zlib\\php_zlib.def");
		}
	} else {
		WARNING("zlib support can't be enabled, zlib is missing")
		PHP_ZLIB = "no"
	}
}


configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\libxml\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("libxml", "LibXML support", "yes"); */

if (PHP_LIBXML == "yes") {
	if (CHECK_LIB("libxml2_a_dll.lib;libxml2_a.lib", "libxml") &&
			CHECK_LIB("libiconv_a.lib;iconv_a.lib;libiconv.lib;iconv.lib", "libxml") &&
			CHECK_HEADER_ADD_INCLUDE("libxml/parser.h", "CFLAGS_LIBXML") &&
			ADD_EXTENSION_DEP('libxml', 'iconv')) {

		EXTENSION("libxml", "libxml.c", false /* never shared */);
		AC_DEFINE("HAVE_LIBXML", 1, "LibXML support");
		ADD_FLAG("CFLAGS_LIBXML", "/D LIBXML_STATIC /D LIBXML_STATIC_FOR_DLL /D HAVE_WIN32_THREADS ");
		if (!PHP_LIBXML_SHARED) {
			ADD_DEF_FILE("ext\\libxml\\php_libxml2.def");
		}
		PHP_INSTALL_HEADERS("ext/libxml/", "php_libxml.h");
	} else {
		WARNING("libxml support can't be enabled, iconv or libxml are missing")
		PHP_LIBXML = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\dom\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("dom", "DOM support", "yes"); */

if (PHP_DOM == "yes") {
	if (PHP_LIBXML == "yes" && ADD_EXTENSION_DEP('dom', 'libxml')) {
		EXTENSION("dom", "php_dom.c attr.c document.c domerrorhandler.c \
			domstringlist.c domexception.c namelist.c processinginstruction.c \
			cdatasection.c documentfragment.c domimplementation.c element.c \
			node.c string_extend.c characterdata.c documenttype.c \
			domimplementationlist.c entity.c nodelist.c text.c comment.c \
			domconfiguration.c domimplementationsource.c entityreference.c \
			notation.c xpath.c dom_iterators.c typeinfo.c domerror.c \
			domlocator.c namednodemap.c userdatahandler.c");

		AC_DEFINE("HAVE_DOM", 1, "DOM support");

		if (!PHP_DOM_SHARED) {
			ADD_FLAG("CFLAGS_DOM", "/D LIBXML_STATIC ");
		}
		PHP_INSTALL_HEADERS("ext/dom", "xml_common.h");
	} else {
		WARNING("dom support can't be enabled, libxml is not enabled")
		PHP_DOM = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\exif\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("exif", "exif", "no"); */

if (PHP_EXIF == "yes") {
	if (ADD_EXTENSION_DEP('exif', 'mbstring')) {
		EXTENSION("exif", "exif.c");
		AC_DEFINE('HAVE_EXIF', 1, 'Have exif');
	} else {
		WARNING("exif support can't be enabled, libxml is not enabled")
		PHP_EXIF = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\mysql\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("mysql", "MySQL support", "no"); */

if (PHP_MYSQL != "no") {
	if ((PHP_MYSQL == "yes" || PHP_MYSQL == "mysqlnd")) {
		AC_DEFINE('MYSQL_USE_MYSQLND', 1, 'Using MySQL native driver');
		if (!ADD_EXTENSION_DEP('mysql', 'mysqlnd', true)) {
			PHP_MYSQL = "no";
			WARNING("mysql not enabled; mysqlnd is not enabled");
		} else {
			EXTENSION("mysql", "php_mysql.c");
			AC_DEFINE('HAVE_MYSQL', 1, 'Have MySQL library');
			MESSAGE("\tusing mysqlnd build");
		}
	} else if (PHP_MYSQL != "") {
		if (CHECK_LIB("libmysql.lib", "mysql", PHP_MYSQL) &&
			CHECK_HEADER_ADD_INCLUDE("mysql.h", "CFLAGS_MYSQL",
			PHP_MYSQL + "\\include;" + PHP_PHP_BUILD + "\\include\\mysql;" + PHP_MYSQL)) {
			EXTENSION("mysql", "php_mysql.c");
			AC_DEFINE('HAVE_MYSQL', 1, 'Have MySQL library');
			MESSAGE("\tusing libmysql");
		} else {
			WARNING("mysql not enabled; libraries and headers not found");
		}
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\mysqli\\config.w32'));
// $Id$
// vim:ft=javascript

// Note: The extension name is "mysqli", you enable it with "--with-mysqli".
// Passing value "mysqlnd" to it enables the bundled
// client library to connect to the MySQL server, i.e. no external MySQL
// client library is needed to perform the build.

/* ARG_WITH("mysqli", "MySQLi support", "no"); */

if (PHP_MYSQLI != "no") {
	mysqli_source =
		"mysqli.c " +
		"mysqli_api.c " +
		"mysqli_driver.c " +
		"mysqli_embedded.c " +
		"mysqli_exception.c " +
		"mysqli_fe.c " +
		"mysqli_nonapi.c " +
		"mysqli_prop.c " +
		"mysqli_result_iterator.c " +
		"mysqli_report.c " +
		"mysqli_warning.c";

	if (PHP_MYSQLI == "yes" || PHP_MYSQLI == "mysqlnd") {
		EXTENSION("mysqli", mysqli_source);
		AC_DEFINE('MYSQLI_USE_MYSQLND', 1, 'Using MySQL native driver');
		AC_DEFINE('HAVE_MYSQLILIB', 1, 'Have MySQLi library');
		ADD_EXTENSION_DEP('mysqli', 'mysqlnd', true);
		MESSAGE("\tmysqlnd build");
		PHP_INSTALL_HEADERS("ext/mysqli", "php_mysqli_structs.h");
	} else {
		if (CHECK_LIB("libmysql.lib", "mysqli", PHP_MYSQLI) &&
			CHECK_HEADER_ADD_INCLUDE("mysql.h", "CFLAGS_MYSQLI", PHP_MYSQLI + 
										"\\include;" + PHP_PHP_BUILD +
										"\\include\\mysql;" + PHP_MYSQLI)) {
			EXTENSION("mysqli", mysqli_source);
			AC_DEFINE('HAVE_MYSQLILIB', 1, 'Have MySQLi library');
			MESSAGE("\tlibmysql build");
			PHP_INSTALL_HEADERS("ext/mysqli", "php_mysqli_structs.h");
		} else {
			WARNING("mysqli not enabled; libraries and headers not found");
			PHP_MYSQLI = "no"
		}
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("pdo", "Enable PHP Data Objects support", "no"); */

if (PHP_PDO != "no") {
	EXTENSION('pdo', 'pdo.c pdo_dbh.c pdo_stmt.c pdo_sql_parser.c pdo_sqlstate.c', false /* force static, PHP_PDO_SHARED is broken yet somehow */);
	ADD_EXTENSION_DEP('pdo', 'spl', true);
	PHP_INSTALL_HEADERS("ext/pdo", "php_pdo.h php_pdo_driver.h php_pdo_error.h");
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_dblib\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pdo-dblib", "freetds dblib (Sybase, MS-SQL) support for PDO", "no"); */

if (PHP_PDO_DBLIB != "no") {
	/* if they pointed us to a freetds dir, pick that up,
	 * otherwise we'll poke around and look for MSSQL libs */

	if (CHECK_LIB("sybdb.lib", "pdo_dblib", PHP_PDO_DBLIB) &&
			CHECK_HEADER_ADD_INCLUDE("sybfront.h", "CFLAGS_PDO_DBLIB",
				PHP_PDO_DBLIB, null, null, true))
	{
		EXTENSION("pdo_dblib", "pdo_dblib.c dblib_driver.c dblib_stmt.c");
		ADD_FLAG('CFLAGS_PDO_DBLIB', "/D PDO_DBLIB_FLAVOUR=\\\"freetds\\\"");
		ADD_EXTENSION_DEP('pdo_dblib', 'pdo');
	} else {
		WARNING("pdo_dblib not enabled, libraries or headers not found")
	}
}

/* ARG_WITH("pdo-mssql", "Native MS-SQL support for PDO", "no"); */

if (PHP_PDO_MSSQL != "no") {
	/* if they pointed us to a freetds dir, pick that up,
	 * otherwise we'll poke around and look for MSSQL libs */

	pdo_mssql_locations = new Array(
		PHP_PDO_MSSQL,
		"\\Program Files\\Microsoft SQL Server\\80",
		"\\MSSQL8",
		"\\MSSQL7",
		PHP_PHP_BUILD + "\\MSSQL80"
	);
	PDO_DBLIB_FLAVOUR = 0;

	if (CHECK_LIB("sybdb.lib", "pdo_mssql", PHP_PDO_MSSQL) &&
			CHECK_HEADER_ADD_INCLUDE("sybfront.h", "CFLAGS_PDO_MSSQL",
			PHP_PDO_MSSQL, null, null, true)) {
		/* smells like FreeTDS (or maybe native sybase dblib) */
		PDO_DBLIB_FLAVOUR = "freetds";
	} else {
		
		for (i = 0; i < pdo_mssql_locations.length; i++) {
			if (CHECK_LIB("ntwdblib.lib", "pdo_mssql",
					pdo_mssql_locations[i] + "\\DevTools\\Lib") &&
					CHECK_HEADER_ADD_INCLUDE("sqlfront.h", "CFLAGS_PDO_MSSQL",
					pdo_mssql_locations[i] + "\\DevTools\\Include",
					null, null, true)) {
				if (pdo_mssql_locations[i] == 'yes') {
					PDO_DBLIB_FLAVOUR = 70;
				} else {
					pdo_mssql_locations[i].match(new RegExp("(\\d)"));
					PDO_DBLIB_FLAVOUR = RegExp.$1 + 0;
				}
				break;
			}
		}
	}

	if (PDO_DBLIB_FLAVOUR != 0) {
		EXTENSION("pdo_mssql", "pdo_dblib.c dblib_driver.c dblib_stmt.c", null, null, null, "ext\\pdo_mssql");
		if (PDO_DBLIB_FLAVOUR != "freetds") {
			ADD_FLAG("CFLAGS_PDO_MSSQL", "/DPHP_DBLIB_IS_MSSQL=1 /DDBNTWIN32=1 /DMSSQL" + PDO_DBLIB_FLAVOUR + "0=1 /DMSSQL_VERSION=\\\"" + PDO_DBLIB_FLAVOUR + ".0\\\"");
			ADD_FLAG("CFLAGS_PDO_MSSQL", "/DPDO_DBLIB_IS_MSSQL=" + PDO_DBLIB_FLAVOUR);
			PDO_DBLIB_FLAVOUR = "MSSQL_" + PDO_DBLIB_FLAVOUR;
		}
		ADD_FLAG('CFLAGS_PDO_MSSQL', "/D PDO_DBLIB_FLAVOUR=\\\"" + PDO_DBLIB_FLAVOUR + "\\\"");
		ADD_EXTENSION_DEP('pdo_mssql', 'pdo');
	} else {
		WARNING("pdo_mssql not enabled, libraries or headers not found")
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_firebird\\config.w32'));
// vim:ft=javascript

/* ARG_WITH("pdo-firebird", "Firebird support for PDO", "no"); */

if (PHP_PDO_FIREBIRD != "no") {

	if ((CHECK_LIB("fbclient_ms.lib", "pdo_firebird", PHP_PHP_BUILD + "\\interbase\\lib_ms;" + PHP_PDO_FIREBIRD)
			|| CHECK_LIB("gds32_ms.lib", "pdo_firebird", PHP_PHP_BUILD + "\\interbase\\lib_ms;" + PHP_PDO_FIREBIRD)
		) && CHECK_HEADER_ADD_INCLUDE("ibase.h", "CFLAGS_PDO_FIREBIRD", 
				PHP_PHP_BUILD + "\\include\\interbase;" + PHP_PHP_BUILD + "\\interbase\\include;" + PHP_PDO_FIREBIRD)
		) {

		EXTENSION("pdo_firebird", "pdo_firebird.c firebird_driver.c firebird_statement.c");
	} else {
		WARNING("pdo_firebird not enabled; libraries and headers not found");
	}
	ADD_EXTENSION_DEP('pdo_firebird', 'pdo');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_mysql\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pdo-mysql", "MySQL support for PDO", "no"); */

if (PHP_PDO_MYSQL != "no") {
	if (PHP_PDO_MYSQL == "yes" || PHP_PDO_MYSQL == "mysqlnd") {
		AC_DEFINE('PDO_USE_MYSQLND', 1, 'Using MySQL native driver');
		STDOUT.WriteLine("INFO: mysqlnd build");
		EXTENSION("pdo_mysql", "pdo_mysql.c mysql_driver.c mysql_statement.c");
		ADD_EXTENSION_DEP('pdo_mysql', 'pdo');
	} else {
		if (CHECK_LIB("libmysql.lib", "pdo_mysql", PHP_PDO_MYSQL) &&
				CHECK_HEADER_ADD_INCLUDE("mysql.h", "CFLAGS_PDO_MYSQL", PHP_PHP_BUILD + "\\include\\mysql;" + PHP_PDO_MYSQL)) {
			EXTENSION("pdo_mysql", "pdo_mysql.c mysql_driver.c mysql_statement.c");
		} else {
			WARNING("pdo_mysql not enabled; libraries and headers not found");
		}
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_oci\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pdo-oci", "Oracle OCI support for PDO", "no"); */

if (PHP_PDO_OCI != "no") {

	pdo_oci_dirs = new Array(
		PHP_PDO_OCI,
		PHP_PDO_OCI + "\\oci",
		PHP_PHP_BUILD + "\\instantclient10_*\\sdk",
		PHP_PHP_BUILD + "\\oci92"
	);

	pdo_oci_lib_paths = "";
	pdo_oci_inc_paths = "";

	// find the oracle install
	for (i = 0; i < pdo_oci_dirs.length; i++) {
		pdo_oci_lib_paths += pdo_oci_dirs[i] + "\\lib;";
		pdo_oci_lib_paths += pdo_oci_dirs[i] + "\\lib\\msvc;";
		pdo_oci_inc_paths += pdo_oci_dirs[i] + "\\include;";
	}
	
	pdo_oci_inc_paths += PHP_PHP_BUILD + "\\include\\instantclient;"
	pdo_oci_lib_paths += PHP_PHP_BUILD + "\\lib\\instantclient;";

	pdo_oci_header = CHECK_HEADER_ADD_INCLUDE("oci.h", "CFLAGS_PDO_OCI", pdo_oci_inc_paths, null, null, true);

	if (pdo_oci_header && CHECK_LIB("oci.lib", "pdo_oci", pdo_oci_lib_paths)) {

		pdo_oci_inc_dir = FSO.GetParentFolderName(pdo_oci_header);

		EXTENSION('pdo_oci', 'pdo_oci.c oci_driver.c oci_statement.c');

		/* probe for some functions not present in older versions */
		pdo_oci_inc_dir = FSO.GetFolder(pdo_oci_header);
		CHECK_FUNC_IN_HEADER('oci.h', 'OCIEnvCreate', pdo_oci_inc_dir, 'CFLAGS_PDO_OCI');
		CHECK_FUNC_IN_HEADER('ociap.h', 'OCIStmtFetch2', pdo_oci_inc_dir, 'CFLAGS_PDO_OCI');
		CHECK_FUNC_IN_HEADER('ociap.h', 'OCIEnvNlsCreate', pdo_oci_inc_dir, 'CFLAGS_PDO_OCI');

	} else {
		WARNING("pdo-oci not enabled; libraries and headers not found");
	}
	ADD_EXTENSION_DEP('pdo_oci', 'pdo');
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_odbc\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pdo-odbc", "ODBC support for PDO", "no"); */

if (PHP_PDO_ODBC != "no") {
	if (CHECK_LIB("odbc32.lib", "pdo_odbc") && CHECK_LIB("odbccp32.lib", "pdo_odbc")
	&& CHECK_HEADER_ADD_INCLUDE('sql.h', 'CFLAGS_PDO_ODBC')
	&& CHECK_HEADER_ADD_INCLUDE('sqlext.h', 'CFLAGS_PDO_ODBC')) {

		EXTENSION("pdo_odbc", "pdo_odbc.c odbc_driver.c odbc_stmt.c");
		ADD_EXTENSION_DEP('pdo_odbc', 'pdo');

	} else {
		WARNING("pdo_odbc support can't be enabled, headers or libraries are missing (SDK)")
		PHP_PDO_ODBC = "no"
	}
}


configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_pgsql\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pdo-pgsql", "PostgreSQL support for PDO", "no"); */

if (PHP_PDO_PGSQL != "no") {
	if (CHECK_LIB("libpq.lib", "pdo_pgsql", PHP_PDO_PGSQL) &&
			CHECK_HEADER_ADD_INCLUDE("libpq-fe.h", "CFLAGS_PDO_PGSQL", PHP_PDO_PGSQL + ";" + PHP_PHP_BUILD + "\\include\\pgsql;" + PHP_PHP_BUILD + "\\include\\libpq;")) {
		EXTENSION("pdo_pgsql", "pdo_pgsql.c pgsql_driver.c pgsql_statement.c");

		if (CHECK_HEADER_ADD_INCLUDE("pg_config.h", "CFLAGS_PDO_PGSQL", PHP_PDO_PGSQL + ";" + PHP_PHP_BUILD + "\\include\\pgsql")) {
			ADD_FLAG('CFLAGS_PDO_PGSQL', "/D HAVE_PG_CONFIG_H");
		}

		AC_DEFINE('HAVE_PDO_PGSQL',		1, 'Have PostgreSQL library');

		ADD_EXTENSION_DEP('pdo_pgsql', 'pdo');
	} else {
		WARNING("pdo_pgsql not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\pdo_sqlite\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("pdo-sqlite", "for pdo_sqlite support", "no"); */

if (PHP_PDO_SQLITE != "no") {
	EXTENSION("pdo_sqlite", "pdo_sqlite.c sqlite_driver.c sqlite_statement.c", null, "/DSQLITE_THREADSAFE=" + (PHP_ZTS == "yes" ? "1" : "0") + " /D SQLITE_ENABLE_FTS3=1 /D SQLITE_ENABLE_COLUMN_METADATA=1 /D SQLITE_CORE=1 /I" + configure_module_dirname + "/../sqlite3/libsqlite /I" + configure_module_dirname);
	
	ADD_EXTENSION_DEP('pdo_sqlite', 'pdo');
	// If pdo_sqlite is static, and sqlite3 is also static, then we don't add a second copy of the sqlite3 libs
	if (PHP_PDO_SQLITE_SHARED || PHP_SQLITE3_SHARED || PHP_SQLITE3 == 'no') {
		ADD_SOURCES(configure_module_dirname + "/../sqlite3/libsqlite", "sqlite3.c", "pdo_sqlite");
	}
}

/* ARG_WITH("pdo-sqlite-external", "for pdo_sqlite support from an external dll", "no"); */
if (PHP_PDO_SQLITE_EXTERNAL != "no") {
	if (CHECK_HEADER_ADD_INCLUDE("sqlite3.h", "CFLAGS_PDO_SQLITE_EXTERNAL", PHP_PDO_SQLITE_EXTERNAL + ";" + PHP_PHP_BUILD + "\\include\\sqlite3") &&
	CHECK_LIB("sqlite3-import.lib", "pdo_sqlite_external", PHP_PDO_SQLITE_EXTERNAL + ";" + PHP_PHP_BUILD + "\\lib")
	&& ADD_EXTENSION_DEP('pdo_sqlite_external', 'pdo') ) {
		EXTENSION("pdo_sqlite_external", "pdo_sqlite.c sqlite_driver.c sqlite_statement.c", null, "/I" + configure_module_dirname, null, "ext\\pdo_sqlite_external");
	} else {
		WARNING("pdo-sqlite-external support can't be enabled, libraries or headers are missing")
		PHP_PDO_SQLITE_EXTERNAL = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\phar\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("phar", "disable phar support", "yes"); */
/* ARG_ENABLE("phar-native-ssl", "enable phar with native OpenSSL support", "no"); */

if (PHP_PHAR_NATIVE_SSL != "no") {
	PHP_PHAR = PHP_PHAR_NATIVE_SSL;
}

if (PHP_PHAR != "no") {
	EXTENSION("phar", "dirstream.c func_interceptors.c phar.c phar_object.c phar_path_check.c stream.c tar.c util.c zip.c");
	if (PHP_PHAR_SHARED || (PHP_PHAR_NATIVE_SSL_SHARED && PHP_SNAPSHOT_BUILD == "no")) {
		ADD_FLAG("CFLAGS_PHAR", "/D COMPILE_DL_PHAR ");
	}
	if (PHP_PHAR_NATIVE_SSL != "no") {
		if (CHECK_LIB("libeay32st.lib", "phar")) {
			/* We don't really need GDI for this, but there's no
			way to avoid linking it in the static openssl build */
			ADD_FLAG("LIBS_PHAR", "libeay32st.lib gdi32.lib");
			if (PHP_DEBUG == "no") {
				/* Silence irrelevant-to-us warning in release builds */
				ADD_FLAG("LDFLAGS_PHAR", "/IGNORE:4089 ");
			}
			AC_DEFINE('PHAR_HAVE_OPENSSL', 1);
			STDOUT.WriteLine('        Native OpenSSL support in Phar enabled');
		} else {
			WARNING('Could not enable native OpenSSL support in Phar');
		}
	} else {
		if (PHP_OPENSSL != "no" && !PHP_OPENSSL_SHARED && !PHP_PHAR_SHARED) {
			AC_DEFINE('PHAR_HAVE_OPENSSL', 1);
			STDOUT.WriteLine('        Native OpenSSL support in Phar enabled');
		} else {
			STDOUT.WriteLine('        Native OpenSSL support in Phar disabled');
		}
	}
	if (PHP_HASH != "no") {
		if (!PHP_HASH_SHARED) {
			AC_DEFINE("PHAR_HASH_OK", 1);
		} else {
			WARNING('Phar: sha256/sha512 signature support disabled if ext/hash is built shared');
		}
	}
	ADD_EXTENSION_DEP('phar', 'spl', true);
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\simplexml\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("simplexml", "Simple XML support", "yes"); */

if (PHP_SIMPLEXML == "yes") {
	if(PHP_LIBXML == "yes"
	&& ADD_EXTENSION_DEP('simplexml', 'libxml')) {
		EXTENSION("simplexml", "simplexml.c sxe.c");
		AC_DEFINE("HAVE_SIMPLEXML", 1, "Simple XML support");
		if (!PHP_SIMPLEXML_SHARED) {
			ADD_FLAG("CFLAGS_SIMPLEXML", "/D LIBXML_STATIC");
		}

		if (!ADD_EXTENSION_DEP('simplexml', 'spl', true)) {
			MESSAGE("\tSPL support in simplexml disabled");
		}
		ADD_FLAG("CFLAGS_SIMPLEXML", "/D PHP_SIMPLEXML_EXPORTS ");
		PHP_INSTALL_HEADERS("ext/simplexml/", "php_simplexml.h php_simplexml_exports.h");
	} else {
		PHP_SIMPLEXML = "no";
		WARNING("simplexml not enabled; libraries and headers not found");
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\soap\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("soap", "SOAP support", "no"); */

if (PHP_SOAP != "no") {
	if (PHP_LIBXML == "yes" && ADD_EXTENSION_DEP('soap', 'libxml')) {
		EXTENSION('soap', 'soap.c php_encoding.c php_http.c php_packet_soap.c php_schema.c php_sdl.c php_xml.c');
		AC_DEFINE('HAVE_PHP_SOAP', 1, "SOAP support");

		if (!PHP_SOAP_SHARED) {
			ADD_FLAG('CFLAGS_SOAP', "/D LIBXML_STATIC ");
		}
	} else {
			WARNING("soap not enabled, libxml not found");
			PHP_SOAP = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\wddx\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("wddx", "WDDX support", "yes"); */

if (PHP_WDDX == "yes" && PHP_LIBXML == "yes") {
	EXTENSION("wddx", "wddx.c");
	AC_DEFINE("HAVE_WDDX", 1, "WDDX support");
	ADD_EXTENSION_DEP('wddx', 'libxml');
	CHECK_HEADER_ADD_INCLUDE("timelib_config.h", "CFLAGS_WDDX", "ext/date/lib");
}


configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\xml\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("xml", "XML support", "yes"); */

if (PHP_XML == "yes") {
	if (PHP_LIBXML == "yes"
	&& ADD_EXTENSION_DEP('xml', 'libxml')) {
		EXTENSION("xml", "xml.c compat.c");
		AC_DEFINE("HAVE_XML", 1, "XML support");
		if (!PHP_XML_SHARED) {
			ADD_FLAG("CFLAGS_XML", "/D LIBXML_STATIC ");
		}
		PHP_INSTALL_HEADERS("", "ext/xml");
	} else {
		WARNING("xml support can't be enabled, libraries or headers are missing")
		PHP_ZLIB = "no"
	}
}
configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\xmlreader\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("xmlreader", "XMLReader support", "yes"); */

if (PHP_XMLREADER == "yes" && PHP_LIBXML == "yes") {
	EXTENSION("xmlreader", "php_xmlreader.c");
	AC_DEFINE("HAVE_XMLREADER", 1, "XMLReader support");
	if (!PHP_XMLREADER_SHARED) {
		ADD_FLAG("CFLAGS_XMLREADER", "/D LIBXML_STATIC");
	}
	ADD_EXTENSION_DEP('xmlreader', 'libxml');
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\xmlrpc\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_WITH("xmlrpc", "XMLRPC-EPI support", "no"); */

if (PHP_XMLRPC != "no") {
	if (CHECK_HEADER_ADD_INCLUDE("xmlrpc.h", "CFLAGS_XMLRPC", configure_module_dirname + "/libxmlrpc")
		&& CHECK_HEADER_ADD_INCLUDE("iconv.h", "CFLAGS")
		&& ADD_EXTENSION_DEP('xmlrpc', 'libxml')) {
		EXTENSION('xmlrpc', 'xmlrpc-epi-php.c', PHP_XMLRPC_SHARED, "-DVERSION=\"0.50\"");
		ADD_SOURCES(configure_module_dirname + "/libxmlrpc", "base64.c simplestring.c xml_to_dandarpc.c \
		xmlrpc_introspection.c encodings.c system_methods.c xml_to_xmlrpc.c \
		queue.c xml_element.c xmlrpc.c xml_to_soap.c", "xmlrpc");

	} else {
		WARNING("xmlrpc support can't be enabled, libraries or headers are missing")
		PHP_XMLRPC = "no";
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\xmlwriter\\config.w32'));
// $Id$
// vim:ft=javascript

/* ARG_ENABLE("xmlwriter", "XMLWriter support", "yes"); */

if (PHP_XMLWRITER == "yes" && PHP_LIBXML == "yes") {
	if (CHECK_HEADER_ADD_INCLUDE('libxml/xmlwriter.h', 'CFLAGS_XMLWRITER', PHP_XMLWRITER)) {
		EXTENSION("xmlwriter", "php_xmlwriter.c");
		AC_DEFINE("HAVE_XMLWRITER", 1, "XMLWriter support");
		if (!PHP_XMLWRITER_SHARED) {
			ADD_FLAG("CFLAGS_XMLWRITER", "/D LIBXML_STATIC");
		}
		ADD_EXTENSION_DEP('xmlwriter', 'libxml');
	} else {
		WARNING('Could not find xmlwriter.h');
	}
}

configure_module_dirname = condense_path(FSO.GetParentFolderName('C:\\Users\\NJU-SEG\\huawei\\llvm\\llvm-3.6.0.src\\tools\\clang\\tools\\huawei-checker\\test\\php-5.6.16\\php-5.6.16\\ext\\xsl\\config.w32'));
// $Id$
// vim: ft=javascript

/* ARG_WITH("xsl", "xsl support", "no"); */

if (PHP_XSL != "no") {
	if (PHP_DOM == "yes" && PHP_LIBXML == "yes"
	&& ADD_EXTENSION_DEP('xsl', 'libxml')
	&& ADD_EXTENSION_DEP('xsl', 'dom')) {
		var ext_xsl_lib_found = false;
		var ext_exslt_lib_found = false;

		if (CHECK_LIB("libxslt_a.lib", "xsl", PHP_XSL)) {
			ext_xsl_lib_found = true;
			ADD_FLAG("CFLAGS_XSL", "/D LIBXSLT_STATIC ");
			if (CHECK_LIB("libexslt_a.lib", "xsl", PHP_XSL)) {
				ADD_FLAG("CFLAGS_XSL", "/D LIBEXSLT_STATIC ");
				ext_exslt_lib_found = true;
			}
		} else if (CHECK_LIB("libxslt.lib", "xsl", PHP_XSL)) {
			ext_xsl_lib_found = true;
			if (CHECK_LIB("libexslt.lib", "xsl", PHP_XSL)) {
				ext_exslt_lib_found = true;
			}
		}

		if (ext_xsl_lib_found && CHECK_HEADER_ADD_INCLUDE("libxslt\\xslt.h", "CFLAGS_XSL")) {
			if (ext_exslt_lib_found) {
				if (CHECK_HEADER_ADD_INCLUDE("libexslt\\exslt.h", "CFLAGS_XSL")) {
					AC_DEFINE("HAVE_XSL_EXSLT", 1, "");
				}
			}
			EXTENSION("xsl", "php_xsl.c xsltprocessor.c", PHP_XSL_SHARED);
			AC_DEFINE("HAVE_XSL", 1, "Define if xsl extension is enabled");
			if (! PHP_XSL_SHARED) {
				ADD_FLAG("CFLAGS_XSL", "/D DOM_EXPORTS /D LIBXML_STATIC");
			} else {
				if (PHP_DEBUG == "yes") {
					ADD_FLAG("LDFLAGS_XSL", "/nodefaultlib:msvcrt");
				}
			}
			
		} else {
			WARNING("xsl not enabled; libraries and headers not found");
		}
	} else {
		WARNING("xsl not enabled; DOM extension required");
	}
}

// vim:ft=javascript
// $Id$
// tail end of configure

if (!MODE_PHPIZE && sapi_enabled.length < 1) {
	MESSAGE("");
	ERROR("No SAPI selected, please enable at least one SAPI.");
}

generate_files();

