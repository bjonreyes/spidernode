/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

static inline void check_eq_helper(const char* aFile, int aLine,
                                   int aExpected,
                                   int aActual)
{
  if (aExpected == aActual) {
    gPassedTests++;
  }
  else {
    std::ostringstream temp;
    temp << aFile << " | Expected '" << aExpected << "', got '";
    temp << aActual << "' at line " << aLine;
    fail(temp.str().c_str());
  }
}

static inline void check_eq_helper(const char* aFile, int aLine,
                                   JSInt64 aExpected,
                                   JSInt64 aActual)
{
  if (aExpected == aActual) {
    gPassedTests++;
  }
  else {
    std::ostringstream temp;
    temp << aFile << " | Expected '" << aExpected << "', got '";
    temp << aActual << "' at line " << aLine;
    fail(temp.str().c_str());
  }
}

static inline void check_eq_helper(const char* aFile, int aLine,
                                   double aExpected,
                                   double aActual)
{
  if (aExpected == aActual) {
    gPassedTests++;
  }
  else {
    std::ostringstream temp;
    temp << aFile << " | Expected '" << aExpected << "', got '";
    temp << aActual << "' at line " << aLine;
    fail(temp.str().c_str());
  }
}

static inline void check_eq_helper(const char* aFile, int aLine,
                                   bool aExpected,
                                   bool aActual)
{
  if (aExpected == aActual) {
    gPassedTests++;
  }
  else {
    std::ostringstream temp;
    temp << aFile << " | Expected '" << aExpected << "', got '";
    temp << aActual << "' at line " << aLine;
    fail(temp.str().c_str());
  }
}

static inline void check_eq_helper(const char* aFile, int aLine,
                                   const char* aExpected,
                                   const char* aActual)
{
  if ((aExpected == NULL && aActual == NULL) ||
      (aExpected != NULL && aActual != NULL &&
       strcmp(aExpected, aActual) == 0)) {
    gPassedTests++;
  }
  else {
    std::ostringstream temp;
    temp << aFile << " | Expected '" << aExpected << "', got '";
    temp << aActual << "' at line " << aLine;
    fail(temp.str().c_str());
  }
}

void check_eq_helper(const char* aFile, int aLine,
                     v8::Handle<v8::Value> aExpected,
                     v8::Handle<v8::Value> aActual)
{
  if (aExpected->Equals(aActual)) {
    gPassedTests++;
  }
  else {
    std::ostringstream temp;
    temp << aFile << " | Expected '" << aExpected << "', got '";
    temp << aActual << "' at line " << aLine;
    fail(temp.str().c_str());
  }
}


#define CHECK do_check_true
#define CHECK_EQ(expected, actual) \
  JS_BEGIN_MACRO \
    gTotalTests++; \
    check_eq_helper(__FILE__, __LINE__, expected, actual); \
  JS_END_MACRO
