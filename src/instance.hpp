#ifndef __READERLP_INSTANCE_HPP__
#define __READERLP_INSTANCE_HPP__

#include <vector>

struct Instance {
   unsigned int numvar;
   unsigned int numlincon;
   std::vector<double> c;
   std::vector<double> lb;
   std::vector<double> ub;
   std::vector<unsigned int> astart;
   std::vector<unsigned int> aindex;
   std::vector<double> avalue;
   std::vector<double> rlb;
   std::vector<double> rub;
};

#endif
