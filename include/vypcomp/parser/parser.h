#pragma once

#include <string>

namespace vypcomp {

/**
 * Provides main interface for parsing.
 */
class LangParser {
public:
   LangParser() = default;

   virtual ~LangParser();
   
public:
   /**
    * @brief Parse file provided by path as argument.
    */
   void parse(const std::string &filename);

   /**
    * @brief Parse file provided as input stream.
    */
   void parse(std::istream &file);
};

}
