/**
 * \file Utility.cpp
 * \brief Implementation for GeographicLib::Utility class
 *
 * Copyright (c) Charles Karney (2011-2020) <charles@karney.com> and licensed
 * under the MIT/X11 License.  For more information, see
 * https://geographiclib.sourceforge.io/
 **********************************************************************/

#include <cstdlib>
#include <GeographicLib/Utility.hpp>

#if defined(_MSC_VER)
// Squelch warnings about unsafe use of getenv
#  pragma warning (disable: 4996)
#endif

namespace GeographicLib {

  using namespace std;

  bool Utility::ParseLine(const std::string& line,
                          std::string& key, std::string& value,
                          char delim) {
    key.clear(); value.clear();
    string::size_type n = line.find('#');
    string linea = trim(line.substr(0, n));
    if (linea.empty()) return false;
    n = delim ? linea.find(delim) : linea.find_first_of(" \t\n\v\f\r");      //
    key = trim(linea.substr(0, n));
    if (key.empty()) return false;
    if (n != string::npos) value = trim(linea.substr(n + 1));
    return true;
  }

  bool Utility::ParseLine(const std::string& line,
                          std::string& key, std::string& value) {
    return ParseLine(line, key, value, '\0');
  }

  int Utility::set_digits(int ndigits) {
#if GEOGRAPHICLIB_PRECISION == 5
    if (ndigits <= 0) {
      char* digitenv = getenv("GEOGRAPHICLIB_DIGITS");
      if (digitenv)
        ndigits = strtol(digitenv, NULL, 0);
      if (ndigits <= 0)
        ndigits = 256;
    }
#endif
    return Math::set_digits(ndigits);
  }

} // namespace GeographicLib
