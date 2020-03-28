#include "reader.hpp"

#include <cstdio>
#include <vector>
#include <limits>
#include <memory>

#define LP_MAX_NAME_LENGTH 255
#define LP_MAX_LINE_LENGTH 560

const char* const LP_KEYWORD_MIN[] = {"minimize", "min", "minimum"};
const char* const LP_KEYWORD_MAX[] = {"maximize", "max", "maximum"};
const char* const LP_KEYWORD_ST[] = {"subject to", "such that", "st", "s.t."};
const char* const LP_KEYWORD_BOUNDS[] = {"bounds", "bound"};
const char* const LP_KEYWORD_INF[] = {"infinity", "inf"};
const char* const LP_KEYWORD_FREE[] = {"free"};
const char* const LP_KEYWORD_GEN[] = {"general", "generals", "gen"};
const char* const LP_KEYWORD_BIN[] = {"binary", "binaries", "bin"};
const char* const LP_KEYWORD_SEMI[] = {"semi-continuous", "semi", "semis"};
const char* const LP_KEYWORD_SOS[] = {"sos"};
const char* const LP_KEYWORD_END[] = {"end"};

const int LP_KEYWORD_MIN_N = 3;
const int LP_KEYWORD_MAX_N = 3;
const int LP_KEYWORD_ST_N = 4;
const int LP_KEYWORD_BOUNDS_N = 2;
const int LP_KEYWORD_INF_N = 2;
const int LP_KEYWORD_FREE_N = 1;
const int LP_KEYWORD_GEN_N = 3;
const int LP_KEYWORD_BIN_N = 3;
const int LP_KEYWORD_SEMI_N = 3;
const int LP_KEYWORD_SOS_N = 1;
const int LP_KEYWORD_END_N = 1;

enum class RawTokenType {
   NONE,
   STR,
   CONS,
   LESS,
   GREATER,
   EQUAL,
   COLON,
   LNEND,
   FLEND,
   BRKOP,
   BRKCL,
   PLUS,
   MINUS,
   HAT,
   SLASH,
   ASTERISK,
   WHITE
};

class RawToken {
public:
   RawTokenType type;
   inline bool istype(RawTokenType t) {
      return this->type == t;
   }
   RawToken(RawTokenType t) : type(t) {} ;
};

class RawStringToken : public RawToken {
public:
   std::string value;
   RawStringToken(std::string v) : RawToken(RawTokenType::STR), value(v) {};
};

class RawConstantToken : public RawToken {
public:
   double value;
   RawConstantToken(double v) : RawToken(RawTokenType::CONS), value(v) {};
};

enum class ProcessedTokenType {
   NONE,
   SECID,
   VARID,
   CONID,
   CONST,
   FREE,
   BRKOP,
   BRKCL,
   COMP,
   LNEND,
   FLEND,
   SLASH,
   ASTERISK,
   HAT
};

enum class LpSectionKeyword {
  NONE,
  OBJ,
  CON,
  BOUNDS,
  GEN,
  BIN,
  SEMI,
  SOS,
  END
};

enum class LpObjectiveSectionKeywordType { NONE, MIN, MAX };

enum class LpComparisonType { LEQ, L, EQ, G, GEQ };

class ProcessedToken {
public:
   ProcessedTokenType type;
   ProcessedToken(ProcessedTokenType t) : type(t) {};
};

class ProcessedTokenSectionKeyword : public ProcessedToken {
public:
   LpSectionKeyword keyword;
   ProcessedTokenSectionKeyword(LpSectionKeyword k) : ProcessedToken(ProcessedTokenType::SECID), keyword(k) {};
};

class ProcessedTokenObjectiveSectionKeyword : public ProcessedTokenSectionKeyword {
public:
   LpObjectiveSectionKeywordType objsense;
   ProcessedTokenObjectiveSectionKeyword(LpObjectiveSectionKeywordType os) : ProcessedTokenSectionKeyword(LpSectionKeyword::OBJ), objsense(os) {};
};

class ProcessedConsIdToken : public ProcessedToken {
public:
   std::string name;
   ProcessedConsIdToken(std::string n) : ProcessedToken(ProcessedTokenType::CONID), name(n) {};
};

class ProcessedVarIdToken : public ProcessedToken {
public:
   std::string name;
   ProcessedVarIdToken(std::string n) : ProcessedToken(ProcessedTokenType::VARID), name(n) {};
};

class ProcessedConstantToken : public ProcessedToken {
public:
   double value;
   ProcessedConstantToken(double v) : ProcessedToken(ProcessedTokenType::CONST), value(v) {};
};

class ProcessedComparisonToken : public ProcessedToken {
public:
   LpComparisonType dir;
   ProcessedComparisonToken(LpComparisonType d) : ProcessedToken(ProcessedTokenType::COMP), dir(d) {};
};

class Reader {
private:
   FILE* file;
   std::vector<std::unique_ptr<RawToken>> rawtokens;
   std::vector<std::unique_ptr<ProcessedToken>> processedtokens;
   
   char linebuffer[LP_MAX_LINE_LENGTH+1];
   bool linebufferrefill;
   char* linebufferpos;

public:
   Reader(FILE* f) : file(f) {};
   void tokenize();
   void readnexttoken(bool* done);
   void processtokens();
};

void readinstance(std::string filename) {
   FILE* file = fopen(filename.c_str(), "r");
   Reader reader(file);
   reader.tokenize();
   reader.processtokens();

   fclose(file);
}

bool isstrequalnocase(const std::string str1, const std::string str2) {
   unsigned int len = str1.size();
    if (str2.size() != len)
        return false;
    for (unsigned int i = 0; i < len; ++i)
        if (tolower(str1[i]) != tolower(str2[i]))
            return false;
    return true;
}

bool iskeyword(const std::string str, const char* const* keywords, const int nkeywords) {
   for (int i=0; i<nkeywords; i++) {
      if (isstrequalnocase(str, keywords[i])) {
         return true;
      }
   }
   return false;
}

LpObjectiveSectionKeywordType parseobjectivesectionkeyword(const std::string str) {
   if (iskeyword(str, LP_KEYWORD_MIN, LP_KEYWORD_MIN_N)) {
      return LpObjectiveSectionKeywordType::MIN;
   }

   if (iskeyword(str, LP_KEYWORD_MAX, LP_KEYWORD_MAX_N)) {
      return LpObjectiveSectionKeywordType::MAX;
   }
   
   return LpObjectiveSectionKeywordType::NONE;
}

LpSectionKeyword parsesectionkeyword(const std::string& str) {
   if (parseobjectivesectionkeyword(str) != LpObjectiveSectionKeywordType::NONE) {
      return LpSectionKeyword::OBJ;
   }

   if (iskeyword(str, LP_KEYWORD_ST, LP_KEYWORD_ST_N)) {
      return LpSectionKeyword::CON;
   }

   if (iskeyword(str, LP_KEYWORD_BOUNDS, LP_KEYWORD_BOUNDS_N)) {
      return LpSectionKeyword::BOUNDS;
   }

   if (iskeyword(str, LP_KEYWORD_END, LP_KEYWORD_END_N)) {
      return LpSectionKeyword::END;
   }

   return LpSectionKeyword::NONE;
}

void Reader::processtokens() {
   unsigned int i = 0;
   printf("Processing %d raw token\n", (int)rawtokens.size());
   while (i < this->rawtokens.size()) {
      fflush(stdout);

      // long section keyword semi-continuous
      if (rawtokens.size() - i >= 3 && rawtokens[i]->istype(RawTokenType::STR) && rawtokens[i+1]->istype(RawTokenType::MINUS) && rawtokens[i+2]->istype(RawTokenType::STR)) {
         std::string temp = ((RawStringToken*)&rawtokens[i])->value + "-" + ((RawStringToken*)&this->rawtokens[i+2])->value;
         LpSectionKeyword keyword = parsesectionkeyword(temp);
         if (keyword != LpSectionKeyword::NONE) {
            processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedTokenSectionKeyword(keyword)));
            i += 3;
            continue;
         }
      }

      // long section keyword subject to/such that
      if (rawtokens.size() - i >= 2 && rawtokens[i]->istype(RawTokenType::STR) && rawtokens[i+1]->istype(RawTokenType::STR)) {
         std::string temp = ((RawStringToken*)&rawtokens[i])->value + " " + ((RawStringToken*)&this->rawtokens[i+1])->value;
         LpSectionKeyword keyword = parsesectionkeyword(temp);
         if (keyword != LpSectionKeyword::NONE) {
            processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedTokenSectionKeyword(keyword)));
            i += 2;
            continue;
         }
      }

      // other section keyword
      if (rawtokens[i]->istype(RawTokenType::STR)) {
         LpSectionKeyword keyword = parsesectionkeyword(((RawStringToken*)rawtokens[i].get())->value);
         if (keyword != LpSectionKeyword::NONE) {
            if (keyword == LpSectionKeyword::OBJ) {
               LpObjectiveSectionKeywordType kw = parseobjectivesectionkeyword(((RawStringToken*)rawtokens[i].get())->value);
               processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedTokenObjectiveSectionKeyword(kw)));
            } else {
               processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedTokenSectionKeyword(keyword)));
            }
            i++;
            continue;
         }
      }

      // constraint identifier?
      if (rawtokens.size() - i >= 2 && rawtokens[i]->istype(RawTokenType::STR) && rawtokens[i+1]->istype(RawTokenType::COLON)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConsIdToken(((RawStringToken*)rawtokens[i].get())->value)));
         i += 2;
         continue;
      }

      // check if free
      if (rawtokens[i]->istype(RawTokenType::STR) && iskeyword(((RawStringToken*)rawtokens[i].get())->value, LP_KEYWORD_FREE, LP_KEYWORD_FREE_N)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::FREE)));
         i++;
         continue;
      }

      // check if infinty
      if (rawtokens[i]->istype(RawTokenType::STR) && iskeyword(((RawStringToken*)rawtokens[i].get())->value, LP_KEYWORD_INF, LP_KEYWORD_INF_N)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConstantToken(std::numeric_limits<double>::infinity())));
         i++;
         continue;
      }

      // assume var identifier
      if (rawtokens[i]->istype(RawTokenType::STR)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedVarIdToken(((RawStringToken*)rawtokens[i].get())->value)));
         i++;
         continue;
      }

      // + Constant
      if (rawtokens.size() - i >= 2 && rawtokens[i]->istype(RawTokenType::PLUS) && rawtokens[i+1]->istype(RawTokenType::CONS)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConstantToken(((RawConstantToken*)rawtokens[i+1].get())->value)));
         i += 2;
         continue;
      }

      // - constant
      if (rawtokens.size() - i >= 2 && rawtokens[i]->istype(RawTokenType::MINUS) && rawtokens[i+1]->istype(RawTokenType::CONS)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConstantToken(-((RawConstantToken*)rawtokens[i+1].get())->value)));
         i += 2;
         continue;
      }

      // +
      if (rawtokens[i]->istype(RawTokenType::PLUS)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConstantToken(1.0)));
         i++;
         continue;
      }

      // -
      if (rawtokens[i]->istype(RawTokenType::MINUS)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConstantToken(-1.0)));
         i++;
         continue;
      }

      // constant
      if (rawtokens[i]->istype(RawTokenType::CONS)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedConstantToken(((RawConstantToken*)rawtokens[i].get())->value)));
         i++;
         continue;
      }

      // lineend
      if (rawtokens[i]->istype(RawTokenType::LNEND)) {
         i++;
         continue;
      }

      // whitespace
      if (rawtokens[i]->istype(RawTokenType::WHITE)) {
         i++;
         continue;
      }

      // [
      if (rawtokens[i]->istype(RawTokenType::BRKOP)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::BRKOP)));
         i++;
         continue;
      }

      // ]
      if (rawtokens[i]->istype(RawTokenType::BRKCL)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::BRKCL)));
         i++;
         continue;
      }

      // /
      if (rawtokens[i]->istype(RawTokenType::SLASH)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::SLASH)));
         i++;
         continue;
      }

      // *
      if (rawtokens[i]->istype(RawTokenType::ASTERISK)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::ASTERISK)));
         i++;
         continue;
      }

      // ^
      if (rawtokens[i]->istype(RawTokenType::HAT)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::HAT)));
         i++;
         continue;
      }

      // <=
      if (rawtokens.size() - i >= 2 && rawtokens[i]->istype(RawTokenType::LESS) && rawtokens[i+1]->istype(RawTokenType::EQUAL)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedComparisonToken(LpComparisonType::LEQ)));
         i += 2;
         continue;
      }

      // <
      if (rawtokens[i]->istype(RawTokenType::LESS)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedComparisonToken(LpComparisonType::L)));
         i++;
         continue;
      }

      // >=
      if (rawtokens.size() - i >= 2 && rawtokens[i]->istype(RawTokenType::GREATER) && rawtokens[i+1]->istype(RawTokenType::EQUAL)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedComparisonToken(LpComparisonType::GEQ)));
         i += 2;
         continue;
      }

      // >
      if (rawtokens[i]->istype(RawTokenType::GREATER)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedComparisonToken(LpComparisonType::G)));
         i++;
         continue;
      }

      // =
      if (rawtokens[i]->istype(RawTokenType::EQUAL)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedComparisonToken(LpComparisonType::EQ)));
         i++;
         continue;
      }

      // FILEEND
      if (rawtokens[i]->istype(RawTokenType::FLEND)) {
         processedtokens.push_back(std::unique_ptr<ProcessedToken>(new ProcessedToken(ProcessedTokenType::FLEND)));
         i++;
         continue;
      }

      // catch all
      printf("\n\n stuck on %d: RawTokenType type %d \n", i, (int)rawtokens[i]->type);
      break;
   }
}

// reads the entire file and separates 
void Reader::tokenize() {
   this->linebufferrefill = true;
   bool done = false;
   while(true) {
      this->readnexttoken(&done);
      if (this->rawtokens.size() && this->rawtokens.back()->type == RawTokenType::FLEND) {
         break;
      }
   }
}

void Reader::readnexttoken(bool* done) {
   *done = false;
   if (this->linebufferrefill) {
      char* eof = fgets(this->linebuffer, LP_MAX_LINE_LENGTH+1, this->file);
      this->linebufferpos = this->linebuffer;
      this->linebufferrefill = false;

      // fgets returns NULL if end of file reached (EOF following a \n)
      if (eof == NULL) {
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::FLEND)));
         *done = true;
         return;
      }
   }

   // check single character tokens
   char nextchar = *this->linebufferpos;

   switch (nextchar) {
      // check for comment
      case '\\':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::LNEND)));
         this->linebufferrefill = true;
         return;
      
      // check for bracket opening
      case '[':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::BRKOP)));
         this->linebufferpos++;
         return;

      // check for bracket closing
      case ']':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::BRKCL)));
         this->linebufferpos++;
         return;

      // check for less sign
      case '<':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::LESS)));
         this->linebufferpos++;
         return;

      // check for greater sign
      case '>':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::GREATER)));
         this->linebufferpos++;
         return;

      // check for equal sign
      case '=':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::EQUAL)));
         this->linebufferpos++;
         return;
      
      // check for colon
      case ':':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::COLON)));
         this->linebufferpos++;
         return;

      // check for plus
      case '+':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::PLUS)));
         this->linebufferpos++;
         return;

      // check for hat
      case '^':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::HAT)));
         this->linebufferpos++;
         return;

      // check for hat
      case '/':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::SLASH)));
         this->linebufferpos++;
         return;

      // check for asterisk
      case '*':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::ASTERISK)));
         this->linebufferpos++;
         return;
      
      // check for minus
      case '-':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::MINUS)));
         this->linebufferpos++;
         return;

      // check for whitespace
      case ' ':
      case '\t':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::WHITE)));
         this->linebufferpos++;
         return;

      // check for line end
      case '\n':
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::LNEND)));
         this->linebufferrefill = true;
         return;

      // check for file end (EOF at end of some line)
      case '\0': 
         this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawToken(RawTokenType::FLEND)));
         *done = true;
         return;
   }
   
   // check for double value
   double constant;
   int ncharconsumed;
   int nread = sscanf(this->linebufferpos, "%lf%n", &constant, &ncharconsumed);
   if (nread == 1) {
      this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawConstantToken(constant)));
      this->linebufferpos += ncharconsumed;
      return;
   }

   // assume it's an (section/variable/constraint) idenifier
   char stringbuffer[LP_MAX_NAME_LENGTH];
   nread = sscanf(this->linebufferpos, "%[^][\t\n\\:+<>^= /-]%n",
                 stringbuffer, &ncharconsumed);
   if (nread == 1) {
      this->rawtokens.push_back(std::unique_ptr<RawToken>(new RawStringToken(stringbuffer)));
      this->linebufferpos += ncharconsumed;
      return;
   }
   
   printf("ERROR\n");
   exit(0);
}