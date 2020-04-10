#ifndef __READERLP_BUILDER_HPP__
#define __READERLP_BUILDER_HPP__

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "instance.hpp"

enum class VariableType {
   CONTINUOUS,
   BINARY,
   GENERAL,
   SEMICONTINUOUS
};

enum class ObjectiveSense {
   MIN,
   MAX
};

struct Variable {
   VariableType type = VariableType::CONTINUOUS;
   double lowerbound = 0.0;
   double upperbound = std::numeric_limits<double>::infinity();
   std::string name;

   Variable(std::string n="") : name(n) {};
};

struct LinTerm {
   std::shared_ptr<Variable> var;
   double coef;
};

struct QuadTerm {
   std::shared_ptr<Variable> var1;
   std::shared_ptr<Variable> var2;
   double coef;
};

struct Expression {
   std::vector<std::shared_ptr<LinTerm>> linterms;
   std::vector<std::shared_ptr<QuadTerm>> quadterms; 
   double offset;
   std::string name = "";
};

struct Constraint {
   double lowerbound = -std::numeric_limits<double>::infinity();
   double upperbound = std::numeric_limits<double>::infinity();
   std::shared_ptr<Expression> expr;

   Constraint() : expr(std::shared_ptr<Expression>(new Expression)) {};
};

struct Model {
   std::shared_ptr<Expression> objective;
   ObjectiveSense sense;
   std::vector<std::shared_ptr<Constraint>> constraints;
};

struct Builder { 
   std::map<std::string, std::shared_ptr<Variable>> variables; 

   Model model;

   std::shared_ptr<Variable> getvarbyname(std::string name) {
      if (variables.count(name) == 0) {
         variables[name] = std::shared_ptr<Variable>(new Variable(name));
      }
      return variables[name];
   }
};

#endif
