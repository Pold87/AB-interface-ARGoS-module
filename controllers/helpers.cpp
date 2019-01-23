#include "helpers.h"
#include <sstream>

/* Convert a number to a string */
template <typename T> std::string NumberToString(T Number) {
  std::ostringstream ss;
  ss << Number;
  return ss.str();
}
/*
  Convert a robot Id (fbxxx) to an integer (xxx)
*/
int Id2Int(std::string id) {

  unsigned int idConversion = id[2] - '0';
  if(id[3]!='\0')
    idConversion = (idConversion * 10) + (id[3] - '0');
  idConversion = (int) idConversion;
  return idConversion;
}

