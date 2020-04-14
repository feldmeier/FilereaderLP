

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

void test_qap10() {
   Model m = readinstance(std::string(PROJECT_DIR) + "/check/qap10.lp");
}

TEST_CASE( "qap10.lp", "" ) {
    REQUIRE_NOTHROW( test_qap10() );
}

TEST_CASE( "QPLIB_8938.lp", "" ) {
   REQUIRE_NOTHROW( test_validqplibfile() );
}

TEST_CASE( "garbage.lp", "" ) {
   REQUIRE_THROWS_AS( test_filecontentgarbage() , std::invalid_argument );
}
