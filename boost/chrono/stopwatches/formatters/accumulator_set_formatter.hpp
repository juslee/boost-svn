//  boost/chrono/stopwatches/stopwatch_formatter.hpp  ------------------------------------------------------------//
//  Copyright 2011 Vicente J. Botet Escriba
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt
//  See http://www.boost.org/libs/chrono/stopwatches for documentation.

#ifndef BOOST_CHRONO_STOPWATCHES_FORMATTERS_ACCUMULATOR_SET_HPP
#define BOOST_CHRONO_STOPWATCHES_FORMATTERS_ACCUMULATOR_SET_HPP

#include <boost/chrono/stopwatches/formatters/base_formatter.hpp>
#include <boost/system/error_code.hpp>
#include <boost/current_function.hpp>
//#include <boost/chrono/stopwatches/detail/adaptive_string.hpp>
#include <boost/format.hpp>
#include <boost/format/group.hpp>
#include <boost/cstdint.hpp>
#include <boost/assert.hpp>
#include <string>
#include <iostream>
#include <cassert>
#include <iomanip>

#define BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT_DEFAULT "count=%1%, sum=%2%, min=%3%, max=%4%, mean=%5%\n"

namespace boost
{
  namespace chrono
  {

    template<typename Ratio = milli, typename CharT = char,
        typename Traits = std::char_traits<CharT>,
        class Alloc = std::allocator<CharT> >
    class basic_accumulator_set_formatter: public base_formatter<CharT, Traits>, public basic_format<CharT, Traits>
    {

    public:
      typedef base_formatter<CharT, Traits> base_type;
      typedef basic_format<CharT, Traits> format_type;
      typedef std::basic_string<CharT, Traits, Alloc> string_type;
      typedef CharT char_type;
      typedef std::basic_ostream<CharT, Traits> ostream_type;

      basic_accumulator_set_formatter() :
        base_type(),
        format_type(BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT_DEFAULT)
      {
      }
      basic_accumulator_set_formatter(ostream_type& os) :
        base_type(os),
        format_type(BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT_DEFAULT)
      {
      }
      basic_accumulator_set_formatter(const char* fmt, ostream_type& os = std::cout) :
        base_type(os), format_type(fmt)
      {
      }
      basic_accumulator_set_formatter(string_type const& fmt, ostream_type& os =
          std::cout) :
        base_type(os), format_type(fmt)
      {
      }

//      static string_type format(const char* s)
//      {
//        string_type res(s);
//        res += boost::chrono::detail::adaptive_string(" : ");
//        res += BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT_DEFAULT;
//        return res;
//      }

      template<class Stopwatch>
      void operator()(Stopwatch & stopwatch_, system::error_code & ec= BOOST_CHRONO_THROWS)
      {


        typedef typename Stopwatch::laps_collector laps_collector;
        laps_collector const& acc = this->get_laps_collector();
        typedef typename Stopwatch::duration duration_t;

        this->os_ << static_cast<format_type&>(*this)
        % boost::accumulators::count(acc)
        % io::group(std::fixed, std::setprecision(this->precision_), duration_fmt(this->duration_style_), boost::chrono::duration<
            double, Ratio>(duration_t(boost::accumulators::sum(acc))))
        % io::group(std::fixed, std::setprecision(this->precision_), duration_fmt(this->duration_style_), boost::chrono::duration<
            double, Ratio>(duration_t((boost::accumulators::min)(acc))))
        % io::group(std::fixed, std::setprecision(this->precision_), duration_fmt(this->duration_style_), boost::chrono::duration<
            double, Ratio>(duration_t((boost::accumulators::max)(acc))))
        % io::group(std::fixed, std::setprecision(this->precision_), duration_fmt(this->duration_style_), ((boost::accumulators::count(acc)
            > 0) ? boost::chrono::duration<double, Ratio>(duration_t(boost::accumulators::sum(acc)
            / boost::accumulators::count(acc))) : boost::chrono::duration<
            double, Ratio>(duration_t(0))))
        ;

      }
    };

    typedef basic_accumulator_set_formatter<milli, char> elapsed_formatter;
    typedef basic_accumulator_set_formatter<milli, wchar_t> welapsed_formatter;

  } // namespace chrono
} // namespace boost

#if 0
#define BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT(F) \
  boost::chrono::detail::adaptive_string(F " : " BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT_DEFAULT)
#ifdef __GNUC__
#define BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FUNCTION_FORMAT \
  boost::chrono::elapsed_formatter::format(BOOST_CURRENT_FUNCTION)
#else
#define BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FUNCTION_FORMAT \
    BOOST_CHRONO_STOPWATCHES_ACCUMULATOR_SET_FORMAT(BOOST_CURRENT_FUNCTION)
#endif
#endif

#endif