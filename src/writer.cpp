#include "writer.hpp"

#include <cstdarg>
#include <cstdio>

#include "def.hpp"

const std::string LP_COMMENT_FILESTART = "File written by FilereaderLP (https://github.com/feldmeier/FilereaderLP)";

class Writer {
private:
   FILE* file;
   char linebuffer[LP_MAX_LINE_LENGTH];
   unsigned int linelength = 0;

   void writetofile(const char* format, ...);
   void writelineend();

   void writeexpression(std::shared_ptr<Expression> expr);

public:
   Writer(std::string filename) : file(fopen(filename.c_str(), "w")) {
      lpassert(file != nullptr);
   };

   ~Writer() {
      fclose(file);
   }

   void write(const Model& model);
};

void writeinstance(std::string filename, const Model& model) {
   Writer writer(filename);
   writer.write(model);
}

void Writer::writetofile(const char* format, ...) {
   va_list argptr;
   va_start(argptr, format);
   unsigned int tokenlength = (unsigned int)vsprintf(this->linebuffer, format, argptr);
   if (linelength + tokenlength >= LP_MAX_LINE_LENGTH) {
      fprintf(this->file, "\n");
      fprintf(this->file, "%s", this->linebuffer);
      this->linelength = tokenlength;
   } else {
      fprintf(this->file, "%s", this->linebuffer);
      this->linelength += tokenlength;
   }
}

void Writer::writelineend() {
  fprintf(this->file, "\n");
  this->linelength = 0;
}

void Writer::writeexpression(std::shared_ptr<Expression> expr) {
   if (expr->name != "") {
      writetofile("%s: ", expr->name.c_str());
   }

   for (unsigned int i = 0; i < expr->linterms.size(); i++) {
      std::shared_ptr<LinTerm> lt = expr->linterms[i];
      writetofile("%+g %s ", lt->coef, lt->var->name.c_str());
   }

   if (expr->quadterms.size() > 0) {
      if (expr->linterms.size() > 0) {
         writetofile("+ ");
      }
      writetofile("[");
      
      for (unsigned int i=0; i<expr->quadterms.size(); i++) {
         std::shared_ptr<QuadTerm> qt = expr->quadterms[i];
         writetofile("%+g ", qt->coef);
         if (qt->var1 == qt->var2) {
            writetofile("%s^2", qt->var1->name.c_str());
         } else {
            writetofile("%s * %s ", qt->var1->name.c_str(), qt->var2->name.c_str());
         }
      }

      writetofile("]\\2");
   }
}

void Writer::write(const Model& model) {
   // write comment
   writetofile("\\ %s", LP_COMMENT_FILESTART.c_str());
   writelineend();

   // write objective section
   writetofile("%s", model.sense == ObjectiveSense::MIN ? LP_KEYWORD_MIN[0].c_str() : LP_KEYWORD_MAX[0].c_str());
   writelineend();
   writeexpression(model.objective);
   writelineend();

   // write constraints
   writetofile("%s", LP_KEYWORD_ST[0].c_str());
   writelineend();
   for (unsigned int i=0; i<model.constraints.size(); i++) {
      std::shared_ptr<Constraint> con = model.constraints[i];
      
      if (con->lowerbound == con->upperbound) {
         writeexpression(con->expr);
         writetofile("= %+g", con->lowerbound);
      } else if (con->lowerbound == -std::numeric_limits<double>::infinity()) {
         writeexpression(con->expr);
         writetofile("<= %+g", con->upperbound);
      } else if (con->upperbound == std::numeric_limits<double>::infinity()) {
         writeexpression(con->expr);
         writetofile(">= %+g", con->lowerbound);
      } else {
         writeexpression(con->expr);
         writetofile("<= %+g", con->upperbound);
         writeexpression(con->expr);
         writetofile(">= %+g", con->lowerbound);
      }
      writelineend();
   }

   // write bounds
   writetofile("%s", LP_KEYWORD_BOUNDS[0].c_str());
   writelineend();
   for (unsigned int i=0; i<model.variables.size(); i++) {
      std::shared_ptr<Variable> var = model.variables[i];
      writetofile("%+g <= %s <= %+g", var->lowerbound, var->name.c_str(), var->upperbound);
      writelineend();
   }

   // write bin section
   writetofile("%s", LP_KEYWORD_BIN[0].c_str());
   writelineend();
   for (unsigned int i=0; i<model.variables.size(); i++) {
      std::shared_ptr<Variable> var = model.variables[i];
      if (var->type == VariableType::BINARY) {
         writetofile(" %s", var->name.c_str());
         writelineend();
      }
   }

   // write gen section
   writetofile("%s", LP_KEYWORD_GEN[0].c_str());
   writelineend();
   for (unsigned int i=0; i<model.variables.size(); i++) {
      std::shared_ptr<Variable> var = model.variables[i];
      if (var->type == VariableType::GENERAL) {
         writetofile(" %s", var->name.c_str());
         writelineend();
      }
   }

   // write semi section
   writetofile("%s", LP_KEYWORD_SEMI[0].c_str());
   writelineend();
   for (unsigned int i=0; i<model.variables.size(); i++) {
      std::shared_ptr<Variable> var = model.variables[i];
      if (var->type == VariableType::SEMICONTINUOUS) {
         writetofile(" %s", var->name.c_str());
         writelineend();
      }
   }

   // TODO: write SOS section

   // write end
   writetofile("%s", LP_KEYWORD_END[0].c_str());
   writelineend();
}
