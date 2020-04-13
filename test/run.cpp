#include "reader.hpp"

#include <cassert>

bool test_filecontentgarbage() {
   try {
      Model m = readinstance("C:\\Users\\Michael\\Downloads\\garbage.lp");
   } catch(std::invalid_argument ex) {
      return true;
   } catch(std::exception ex) {
      return false;
   }
   return false;
}

bool test_validqplibfile() {
   try{
      Model m = readinstance("C:\\Users\\Michael\\Downloads\\QPLIB_8938.lp");
   } catch(std::exception ex) {
      return false;
   }
   return true;
}

int main(void) {
   assert(test_filecontentgarbage());
   assert(test_validqplibfile());
   return 0;
}
