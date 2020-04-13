

#define CATCH_CONFIG_MAIN 
#include "../external/catch/catch.hpp"

#include "config.hpp"
#include "reader.hpp"

void test_filecontentgarbage() {
   Model m = readinstance(std::string(PROJECT_DIR) + "/check/garbage.lp");
}

void test_validqplibfile() {
   Model m = readinstance(std::string(PROJECT_DIR) + "/check/QPLIB_8938.lp");
}

TEST_CASE( "", "" ) {
   REQUIRE_THROWS_AS( test_filecontentgarbage() , std::invalid_argument );
}

TEST_CASE("", "" ) {
   REQUIRE_NOTHROW( test_validqplibfile() );
}