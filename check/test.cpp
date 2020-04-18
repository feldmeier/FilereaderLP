

#define CATCH_CONFIG_MAIN 
#include "../external/catch/catch.hpp"

#include "config.hpp"
#include "reader.hpp"
#include "writer.hpp"

void test_filecontentgarbage() {
   Model m = readinstance(std::string(PROJECT_DIR) + "/check/garbage.lp");
}

void test_validqplibfile() {
   Model m = readinstance(std::string(PROJECT_DIR) + "/check/QPLIB_8938.lp");
}

void test_qap10() {
   Model m = readinstance(std::string(PROJECT_DIR) + "/check/qap10.lp");
}

void test_writer() {
   Model m1 = readinstance(std::string(PROJECT_DIR) + "/check/QPLIB_8938.lp");
   REQUIRE_NOTHROW( writeinstance("test.lp", m1) );
   Model m2 = readinstance("test.lp");
   REQUIRE(m1.sense == m2.sense);
   REQUIRE(m1.variables.size() == m2.variables.size());
   REQUIRE(m1.constraints.size() == m2.constraints.size());
}

TEST_CASE( "writer", "" ) {
   test_writer();
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
