/*=============================================================================
    A Standard compliant C++ preprocessor

    Copyright (c) 2001-2003 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Permission to copy, use, modify, sell and distribute this software
    is granted provided this copyright notice appears in all copies.
    This software is provided "as is" without express or implied
    warranty, and with no claim as to its suitability for any purpose.

    See Copyright.txt for full copyright notices and acknowledegments.
=============================================================================*/

#include "cpp.hpp"                  // global configuration

#include <iterator>

///////////////////////////////////////////////////////////////////////////////
//  This sample requires the program_options library written by Vladimir Prus,
//  which is currently under Boost review. 
//  It is available here: http://zigzag.cs.msu.su:7813/program_options
//
#include <boost/program_options.hpp>

#include "cpplexer/cpplexer_exceptions.hpp"
#include "cpplexer/cpp_token_ids.hpp"
#include "cpplexer/cpp_lex_iterator.hpp"
#include "cpplexer/slex/util/time_conversion_helper.hpp"

#include "cpp/cpp_exceptions.hpp"
#include "cpp/cpp_context.hpp"

///////////////////////////////////////////////////////////////////////////////
//  include lexer specifics, import lexer names
#if defined(USE_SLEX_CPP_LEXER)
// use the slex based C++ lexer
#include "cpplexer/slex/cpp_slex_token.hpp"

#if !defined(CPP_SEPARATE_LEXER_INSTANTIATION)
#include "cpplexer/slex/cpp_slex_lexer.hpp"
#endif // !defined(CPP_SEPARATE_LEXER_INSTANTIATION)

using namespace cpplexer::slex;

#elif defined(USE_RE2C_CPP_LEXER)
// use the re2c based C++ lexer
#include "cpplexer/re2clex/cpp_re2c_token.hpp"

#if !defined(CPP_SEPARATE_LEXER_INSTANTIATION)
#include "cpplexer/re2clex/cpp_re2c_lexer.hpp"
#endif // !defined(CPP_SEPARATE_LEXER_INSTANTIATION)

using namespace cpplexer::re2clex;

#endif

///////////////////////////////////////////////////////////////////////////////
//  import required names
using namespace boost::spirit;

using std::string;
using std::vector;
using std::getline;
using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::istreambuf_iterator;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

///////////////////////////////////////////////////////////////////////////////
// print the current version

int print_version()
{
// get time of last compilation of this file
cpplexer::slex::util::time_conversion_helper compilation_time(__DATE__ " " __TIME__);

// calculate the number of days since Dec 13 2001 
// (the day the cpp project was started)
std::tm first_day;

    std::memset (&first_day, 0, sizeof(std::tm));
    first_day.tm_mon = 11;           // Dec
    first_day.tm_mday = 13;          // 13
    first_day.tm_year = 101;         // 2001

long seconds = long(std::difftime(compilation_time.get_time(), 
    std::mktime(&first_day)));

    cout 
        << CPP_VERSION_MAJOR << '.' 
        << CPP_VERSION_MINOR << '.'
        << CPP_VERSION_SUBMINOR << '.'
        << seconds/(3600*24);       // get number of days from seconds
    return 1;                       // exit app
}

///////////////////////////////////////////////////////////////////////////////
//  
int do_actual_work (
    po::options_and_arguments const opts, po::variables_map const &vm, 
    vector<string> const &pathes, vector<string> const &syspathes)
{
// current file position is saved for exception handling
file_position current_position;

    try {
    // process the given file
    string file_name(opts.arguments().front());
    ifstream instream(file_name.c_str());
    string instring;

        if (!instream.is_open()) {
            cerr << "cpp: could not open input file: " << file_name << endl;
            return -1;
        }
        instring = string(istreambuf_iterator<char>(instream.rdbuf()),
                          istreambuf_iterator<char>());
        
    // The template lex_token<> is defined in several namespaces (here: 
    // cpplexer::slex and cpplexer::re2clex). The 'using namespace' 
    // directive above tells the compiler, which of them to use.
        typedef cpp::context<lex_token<std::string::iterator> > context_t;

    // The C preprocessor iterator shouldn't be constructed directly. It is 
    // to be generated through a cpp::context<> object. This cpp:context<> 
    // object is additionally to be used to initialize and define different 
    // parameters of the actual preprocessing.
    // The preprocessing of the input stream is done on the fly behind the 
    // scenes during iteration over the context_t::iterator_t stream.
    context_t ctx (instring.begin(), instring.end(), file_name.c_str());

    // add include directories to the include path
        if (vm.count("path")) {
            vector<string>::const_iterator end = pathes.end();
            for (vector<string>::const_iterator cit = pathes.begin(); 
                    cit != end; ++cit)
            {
                ctx.add_include_path((*cit).c_str());
            }
        }
        
    // add system include directories to the include path
        if (vm.count("syspath")) {
            vector<string>::const_iterator end = syspathes.end();
            for (vector<string>::const_iterator cit = syspathes.begin(); 
                    cit != end; ++cit)
            {
                ctx.add_include_path((*cit).c_str(), true);
            }
        }
        
    // analyze the actual file
    context_t::iterator_t first = ctx.begin();
    context_t::iterator_t last = ctx.end();
            
        while (first != last) {
        // print out the string representation of this token (skip comments)
            using namespace cpplexer;
            
        token_id id = token_id(*first);
        
            if (id == T_CPPCOMMENT || id == T_NEWLINE) {
            // C++ comment tokens contain the trailing newline
                cout << endl;
            }
            else if (id != T_CCOMMENT) {
            // print out the current token value
                cout << (*first).get_value();
            }
            ++first;        // advance to the next token
        }
    }
    catch (cpp::cpp_exception &e) {
    // some preprocessing error
        cerr 
            << e.file_name() << "(" << e.line_no() << "): "
            << e.description() << endl;
        return 1;
    }
    catch (cpplexer::lexing_exception &e) {
    // some lexing error
        cerr 
            << e.file_name() << "(" << e.line_no() << "): "
            << e.description() << endl;
        return 2;
    }
    catch (std::exception &e) {
    // use last recognized token to retrieve the error position
        cerr 
            << current_position.file 
            << "(" << current_position.line << "): "
            << "exception caught: " << e.what()
            << endl;
        return 3;
    }
    catch (...) {
    // use last recognized token to retrieve the error position
        cerr 
            << current_position.file 
            << "(" << current_position.line << "): "
            << "unexpected exception caught." << endl;
        return 4;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//  main entry point
int
main (int argc, char const *argv[])
{
    try {
    // analyze the command line options and arguments
    vector<string> pathes;
    vector<string> syspathes;
    po::options_description desc("Usage: cpp [options] file ...");
        
        desc.add_options()
            ("help,h", "", "print out program usage")
            ("version,v", "", "print the version number")
            ("path,I", po::parameter<vector<string> >("dir", &pathes), 
                "specify additional include directory")
            ("syspath,S", po::parameter<vector<string> >("dir", &syspathes), 
                "specify additional system include directory")
        ;

    po::options_and_arguments opts = po::parse_command_line(argc, argv, desc);
    po::variables_map vm;
    
        po::store(opts, vm, desc);
        if (vm.count("help")) {
            cout << desc << endl;
            return 1;
        }
        
        if (vm.count("version")) {
            return print_version();
        }

    // if there is no input file given, then exit
        if (0 == opts.arguments().size()) {
            cerr << "cpp: no input file given, "
                 << "use --help to get a hint." << endl;
            return 5;
        }

    // iterate over all given input files
        return do_actual_work(opts, vm, pathes, syspathes);
    }
    catch (std::exception &e) {
        cout << "cpp: exception caught: " << e.what() << endl;
        return 6;
    }
    catch (...) {
        cerr << "cpp: unexpected exception caught." << endl;
        return 7;
    }
}

