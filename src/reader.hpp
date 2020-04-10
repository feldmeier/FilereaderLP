#ifndef __READERLP_READER_HPP__
#define __READERLP_READER_HPP__

#include <string>

#include "model.hpp"

Model readinstance(std::string filename);
void writeinstance(Model&, std::string filename);

#endif
