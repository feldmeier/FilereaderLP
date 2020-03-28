#ifndef __READERLP_INSTANCE_HPP__
#define __READERLP_INSTANCE_HPP__

#include <vector>

class Instance {
   unsigned int numvar;
   unsigned int numlincon;
   unsigned int numquadcon;
   std::vector<double> c;
   std::vector<unsigned int> qstart;
   std::vector<unsigned int> qindex;
   std::vector<double> qvalue;
   // TODO 

};

#endif
