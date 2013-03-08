// ================================================================
// $Id: Environment.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $
//
// environ.cpp
//     Implementation of the environment operations, ie. reading
//     environment file, reading command line parameters etc.
//

//#define _DEBUG_PARAMS

// Standard C++ headers
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>

// Local headers
#include "gzstream.h"
#include "common.h"
#include "Environment.h"
#include "mmVector3.h"

using namespace std;
  


Environment*
Environment::sEnvironment = NULL;

void
Environment::SetSingleton(Environment *e)
{
  sEnvironment = e;
}

Environment*
Environment::GetSingleton()
{
  if (!sEnvironment) {
    cerr<<"Environment not allocated!!";
    exit(1);
  }
  
  return sEnvironment;
}


void
Environment::DelSingleton()
{
  DEL_PTR(sEnvironment);
}

// Destructor
Environment::~Environment()
{
  int i, j;

  // delete the params structure
  for (i = 0; i < numParams; i++) {
    for (j = 0; j < paramRows; j++)
      if (params[i][j] != NULL)
	delete[] params[i][j];
    if (params[i] != NULL)
      delete[] params[i];
  }

  if (params != NULL)
    delete[] params;
  
  // delete the options structure
  if (options != NULL)
    delete[] options;
  
  if (optionalParams != NULL)
    DEL_PTR(optionalParams);
}

bool
Environment::CheckForSwitch(const int argc,
			    char **argv,
			    const char swtch) const
{
  for (int i = 1; i < argc; i++)
    if ((argv[i][0] == '-') && (argv[i][1] == swtch))
      return true;
  return false;
}

bool
Environment::CheckType(const char *value,
		       const EOptType type) const
{
  char *s, *t, *u;

  switch (type) {
    case optInt: {
      strtol(value, &t, 10);
      if (value + strlen(value) != t)
        return false;
      else
        return true;
    }
    case optFloat: {
      strtod(value, &t);
      if (value + strlen(value) != t)
        return false;
      else
        return true;
    }
    case optBool: {
      if (!strcasecmp(value, "true") ||
          !strcasecmp(value, "false") ||
          !strcasecmp(value, "YES") ||
          !strcasecmp(value, "NO") ||
          !strcmp(value, "+") ||
          !strcmp(value, "-") ||
          !strcasecmp(value, "ON") ||
          !strcasecmp(value, "OFF"))
        return true;
      return false;
    }
    case optVector:{
      strtod(value, &s);
      if (*s == ' ' || *s == '\t') {
        while (*s == ' ' || *s == '\t')
          s++;
        if (*s != ',')
          s--;
      }
      if ((*s != ',' && *s != ' ' && *s != '\t') || value == s)
        return false;
      t = s;
      strtod(s + 1, &u);
      if (*u == ' ' || *u == '\t') {
        while (*u == ' ' || *u == '\t')
          u++;
        if (*u != ',')
          u--;
      }
      if ((*u != ',' && *s != ' ' && *s != '\t') || t == u)
        return false;
      t = u;
      strtod(u + 1, &s);
      if (t == s || value + strlen(value) != s)
        return false;
      return true;
    }
    case optString: {
      return true;
    }
    default: {
      Debug << "Internal error: Unknown type of option.\n" << flush;
      exit(1);
    }
  }
  return false;
}

void
Environment::ReadCmdlineParams(const int argc,
			       char **argv,
			       const char *optParams)
{
  int i;

  // Make sure we are called for the first time
  if (optionalParams != NULL)
    return;

  numParams = (int)strlen(optParams) + 1;
  optionalParams = new char[numParams];
  strcpy(optionalParams, optParams);

  // First, count all non-optional parameters on the command line
  for (i = 1; i < argc; i++)
    if (argv[i][0] != '-')
      paramRows++;

  // if there is no non-optional parameter add a default one...
  if (paramRows == 0)
    paramRows = 1;
  
  // allocate and initialize the table for parameters
  params = new char **[numParams];
  for (i = 0; i < numParams; i++) {
    params[i] = new char *[paramRows];
    for (int j = 0; j < paramRows; j++)
      params[i][j] = NULL;
  }
  // Now read all non-optional and optional parameters into the table
  curRow = -1;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      // non-optional parameter encountered
      curRow++;
      params[0][curRow] = new char[strlen(argv[i]) + 1];
      strcpy(params[0][curRow], argv[i]);
    }
    else {
      // option encountered
      char *t = strchr(optionalParams, argv[i][1]);
      if (t != NULL) {
        // this option is optional parameter
        int index = t - optionalParams + 1;
        if (curRow < 0) {
          // it's a global parameter
          for (int j = 0; j < paramRows; j++) {
            params[index][j] = new char[strlen(argv[i] + 2) + 1];
            strcpy(params[index][j], argv[i] + 2);
          }
        }
        else {
          // it's a scene parameter
          if (params[index][curRow] != NULL) {
            delete[] params[index][curRow];
          }
          params[index][curRow] = new char[strlen(argv[i] + 2) + 1];
          strcpy(params[index][curRow], argv[i] + 2);
        }
      }
    }
  }
  curRow = 0;

#ifdef _DEBUG_PARAMS
  // write out the parameter table
  cerr << "Parameter table for " << numParams << " columns and "
       << paramRows << " rows:\n";
  for (int j = 0; j < paramRows; j++) {
    for (i = 0; i < numParams; i++) {
      if (params[i][j] != NULL)
        cerr << params[i][j];
      else
        cerr << "NULL";
      cerr << "\t";
    }
    cerr << "\n";
  }
  cerr << "Params done.\n" << flush;
#endif // _DEBUG_PARAMS
}

bool
Environment::GetParam(const char name,
		       const int index,
		       char *value) const
{
  int column;

  if (index >= paramRows || index < 0)
    return false;
  if (name == ' ')
    column = 0;
  else {
    char *t = strchr(optionalParams, name);

    if (t == NULL)
      return false;
    column = t - optionalParams + 1;
  }

  if (params[column][index] == NULL)
    return false;
  //  value = new char[strlen(params[column][index]) + 1];
  strcpy(value, params[column][index]);
  return true;
}

void
Environment::RegisterOption(const char *name,
			    const EOptType type,
			    const char *abbrev,
			    const char *defValue)
{
  int i;

  // make sure this option was not yet registered
  for (i = 0; i < numOptions; i++)
    if (!strcmp(name, options[i].name)) {
      Debug << "Error: Option " << name << " registered twice.\n";
      exit(1);
    }
  // make sure we have enough room in memory
  if (numOptions >= maxOptions) {
    Debug << "Error: Too many options. Try enlarge the maxOptions "
	  << "definition.\n";
    exit(1);
  }

  // make sure the abbreviation doesn't start with 'D'
  if (abbrev != NULL && (abbrev[0] == 'D' )) {
    Debug << "Internal error: reserved switch " << abbrev
         << " used as an abbreviation.\n";
    exit(1);
  }
  // new option
  options[numOptions].type = type;
  options[numOptions].name = ::strdup(name);
  // assign abbreviation, if requested
  if (abbrev != NULL) {
	  options[numOptions].abbrev = ::strdup(abbrev);
  }
  // assign default value, if requested
  if (defValue != NULL) {
	  options[numOptions].defaultValue = ::strdup(defValue);
    if (!CheckType(defValue, type)) {
      Debug << "Internal error: Inconsistent type and default value in option "
           << name << ".\n";
      exit(1);
    }
  }
  // new option registered
  numOptions++;
}

bool
Environment::OptionPresent(const char *name) const
{
  bool found = false;
  int i;

  for (i = 0; i < numOptions; i++)
    if (!strcmp(options[i].name, name)) {
      found = true;
      break;
    }
  if (!found) {
    Debug << "Internal error: Option " << name << " not registered.\n" << flush;
    exit(1);
  }
  if (options[i].value != NULL || options[i].defaultValue != NULL)
    return true;
  else
    return false;
}

int
Environment::FindOption(const char *name, const bool isFatal) const
{
  int i;
  bool found = false;
  // is this option registered ?
  for (i = 0; i < numOptions; i++)
    if (!strcmp(options[i].name, name)) {
      found = true;
      break;
    }
  if (!found) {
    // no registration found
    Debug << "Internal error: Required option " << name
	  << " not registered.\n" << flush;
    exit(1);
  }
  if (options[i].value == NULL && options[i].defaultValue == NULL)
    // this option was not initialised to some value
    if (isFatal) {
      Debug << "Error: Required option " << name << " not found.\n" << flush;
      exit(1);
    }
    else {
      Debug << "Error: Required option " << name << " not found.\n" << flush;
      return -1;
    }
  return i;
}

bool
Environment::GetIntValue(const char *name,
			 int &value,
			 const bool isFatal) const
{
  int i = FindOption(name, isFatal);

  if (i<0)
    return false;

  if (options[i].value != NULL) {
    // option was explicitly specified
    value = strtol(options[i].value, NULL, 10);
  } else {
    // option was not read, so use the default
    value = strtol(options[i].defaultValue, NULL, 10);
  }

  return true;
}

bool
Environment::GetDoubleValue(const char *name,
			    double &value,
			    const bool isFatal) const
{
  int i = FindOption(name, isFatal);

  if (i<0)
    return false;

  if (options[i].value != NULL) {
    // option was explicitly specified
    value = strtod(options[i].value, NULL);
  }
  else {
    // option was not read, so use the default
    value = strtod(options[i].defaultValue, NULL);
  }
  return true;
}


bool
Environment::GetFloatValue(const char *name,
			   float &value,
			   const bool isFatal) const
{
  int i = FindOption(name, isFatal);

  if (i<0)
    return false;

  if (options[i].value != NULL) {
    // option was explicitly specified
    value = (float)strtod(options[i].value, NULL);
  } else {
    // option was not read, so use the default
    value = (float)strtod(options[i].defaultValue, NULL);
  }
  return true;
}

bool
Environment::GetBool(const char *name,
		     const bool isFatal) const
{
  bool ret;
  if (GetBoolValue(name, ret, isFatal))
    return ret;
  else
    return false;
}

bool
Environment::ParseBool(const char *name) const
{

  bool value = true;
  
  if (!strcasecmp(name, "false") ||
      !strcasecmp(name, "NO") ||
      !strcmp(name, "-") ||
      !strcasecmp(name, "OFF"))
    value = false;
  
  return value;
}

void
Environment::ParseVector(const char *name, mmVector3 &v) const
{
  // option was not read, so use the default
  char *s, *t;
  
  v.x = (float)strtod(name, &s);
  v.y = (float)strtod(s + 1, &t);
  v.z = (float)strtod(t + 1, NULL);

}

bool
Environment::GetBoolValue(const char *name,
			   bool &value,
			   const bool isFatal) const
{
  int i = FindOption(name, isFatal);

  if (i < 0)
    return false;
  
  if (options[i].value != NULL) 
    value = ParseBool(options[i].value);
  else
    value = ParseBool(options[i].defaultValue);

  return true;
}

bool
Environment::GetVectorValue(const char *name,
			    mmVector3 &v,
			    const bool isFatal) const
{
  int i = FindOption(name, isFatal);
  if (i < 0)
    return false;

  if (options[i].value != NULL) 

    
  if (options[i].value != NULL)
    ParseVector(options[i].value, v);
  else
    ParseVector(options[i].defaultValue, v);

  return true;
}

bool
Environment::GetStringValue(const char *name,
			    char *value,
			    const bool isFatal) const
{
  int i = FindOption(name, isFatal);

  if (i<0)
    return false;

  
  if (options[i].value != NULL)
    // option was not read, so use the default
    strcpy(value, options[i].value);
  else
    // option was explicitly specified
    strcpy(value, options[i].defaultValue);

  return true;
}

bool
Environment::GetStringValue(const char *name,
			    string &value,
			    const bool isFatal) const
{
  char buffer[MaxStringLength];
  bool result = GetStringValue(name, buffer, isFatal);
  if (result)
    value = buffer;
  return result;
}

void
Environment::SetInt(const char *name, const int value)
{

  int i = FindOption(name);
  if (i<0)
    return;

  if (options[i].type == optInt) {
    delete options[i].value;
    options[i].value = new char[16];
    sprintf(options[i].value, "%.15d", value);
  }
  else {
    Debug << "Internal error: Trying to set non-integer option " << name
	  << " to integral value.\n" << flush;
    exit(1);
  }
}

void
Environment::SetFloat(const char *name, const float value)
{
  int i = FindOption(name);
  if (i<0)
    return;

  if (options[i].type == optFloat) {
    delete options[i].value;
    options[i].value = new char[25];
    sprintf(options[i].value, "%.15e", value);
  }
  else {
    Debug << "Internal error: Trying to set non-float option " << name
	  << " to float value.\n" << flush;
    exit(1);
  }
}

void
Environment::SetBool(const char *name, const bool value)
{
  int i = FindOption(name);
  if (i<0)
    return;

  if (options[i].type == optBool) {
    delete options[i].value;
    options[i].value = new char[6];
    if (value)
      sprintf(options[i].value, "true");
    else
      sprintf(options[i].value, "false");
  }
  else {
    Debug << "Internal error: Trying to set non-bool option " << name
	  << " to boolean value.\n" << flush;
    exit(1);
  }
}

void
Environment::SetVector(const char *name,
		       const mmVector3 &v)
{
  int i = FindOption(name);
  if (i<0)
    return;

  if (options[i].type == optVector) {
    delete options[i].value;
    options[i].value = new char[128];
    sprintf(options[i].value, "%.15e,%.15e,%.15e", v.x, v.y, v.z);
  }
  else {
    Debug << "Internal error: Trying to set non-vector option " << name
	  << " to vector value.\n" << flush;
    exit(1);
  }
}

void
Environment::SetString(const char *name, const char *value)
{
  int i = FindOption(name);
  if (i<0)
    return;

  if (options[i].type == optString) {
    delete options[i].value;
    options[i].value = ::strdup(value);
  }
  else {
    Debug << "Internal error: Trying to set non-string option " << name
	  << " to string value.\n" << flush;
    exit(1);
  }
}

void
Environment::ParseCmdline(const int argc,
			  char **argv,
			  const int index)
{
  int curIndex = -1;

  for (int i = 1; i < argc; i++) {
    // if this parameter is non-optional, skip it and increment the counter
    if (argv[i][0] != '-') {
      curIndex++;
      continue;
    }
    // make sure to skip all non-optional parameters
    char *t = strchr(optionalParams, argv[i][1]);
    if (t != NULL)
      continue;

    // if we are in the scope of the current parameter, parse it
    if (curIndex == -1 || curIndex == index) {
      if (argv[i][1] == 'D') {
        // it's a full name definition
        bool found = false;
        int j;

        char *t = strchr(argv[i] + 2, '=');
        if (t == NULL) {
          Debug << "Error: Missing '=' in option. "
		<< "Syntax is -D<name>=<value>.\n" << flush;
          exit(1);
        }
        for (j = 0; j < numOptions; j++)
          if (!strncmp(options[j].name, argv[i] + 2, t - argv[i] - 2) &&
              (unsigned)(t - argv[i] - 2) == strlen(options[j].name)) {
            found = true;
            break;
          }
        if (!found) {
          Debug << "Warning: Unregistered option " << argv[i] << ".\n" << flush;
	  //  exit(1);
        }
	if (found) {
	  if (!CheckType(t + 1, options[j].type)) {
	    Debug << "Error: invalid type of value " << t + 1 << " in option "
		  << options[j].name << ".\n";
	    exit(1);
	  }
	  if (options[j].value != NULL)
	    delete options[j].value;
	  options[j].value = strdup(t + 1);
	}
      }
      else {
        // it's an abbreviation
        bool found = false;
        int j;
	
        for (j = 0; j < numOptions; j++)
          if (options[j].abbrev != NULL &&
              !strncmp(options[j].abbrev, argv[i] + 1, strlen(options[j].abbrev))) {
            found = true;
            break;
          }
        if (!found) {
          Debug << "Warning: Unregistered option " << argv[i] << ".\n" << flush;
	  //          exit(1);
        }
	if (found) {
	  if (!CheckType(argv[i] + 1 + strlen(options[j].abbrev), options[j].type)) {
	    Debug << "Error: invalid type of value "
		  << argv[i] + 1 + strlen(options[j].abbrev) << "in option "
		  << options[j].name << ".\n";
	    exit(1);
	  }
	  if (options[j].value != NULL)
	    delete options[j].value;
	  options[j].value = strdup(argv[i] + 1 + strlen(options[j].abbrev));
	}
      }
    }
  }
#ifdef _DEBUG_PARAMS
  // write out the options table
  cerr << "Options table for " << numOptions << " options:\n";
  for (int j = 0; j < numOptions; j++) {
    cerr << options[j];
    cerr << "\n";
  }
  cerr << "Options done.\n" << flush;
#endif // _DEBUG_PARAMS
}


char *
Environment::ParseString(char *buffer, char *str) const
{
  char *s = buffer;
  char *t = str + strlen(str);

  // skip leading whitespaces
  while (*s == ' ' || *s == '\t' || *s == '\n' || *s == 0x0d)
    s++;
  if (*s == 0)
    return NULL;
  while ((*s >= 'a' && *s <= 'z') ||
         (*s >= 'A' && *s <= 'Z') ||
         (*s >= '0' && *s <= '9') ||
         *s == '_')
    *t++ = *s++;
  *t = 0;

  // skip trailing whitespaces
  while (*s == ' ' || *s == '\t' || *s == '\n' || *s == 0x0d)
    s++;
  return s;
}

const char code[] = "JIDHipewhfdhyd74387hHO&{WK:DOKQEIDKJPQ*H#@USX:#FWCQ*EJMQAHPQP(@G#RD";

void
Environment::DecodeString(char *buff, int max)
{
  buff[max] = 0;
  char *p = buff;
  const char *cp = code; 
  for (; *p; p++) {
    if (*p != '\n')
      *p = *p ^ *cp;
    ++cp;
    if (*cp == 0)
      cp = code;
  }
}

void
Environment::CodeString(char *buff, int max)
{
  buff[max] = 0;
  char *p = buff;
  const char *cp = code; 
  for (; *p; p++) {
    if (*p != '\n')
      *p = *p ^ *cp;
    ++cp;
    if (*cp == 0)
      cp = code;
  }
}

void
Environment::SaveCodedFile(char *filenameText,
			    char *filenameCoded)
{
  ifstream envStream(filenameText);
  
  // some error had occured
  if (envStream.fail()) {
    cerr << "Error: Can't open file " << filenameText << " for reading (err. "
         << envStream.rdstate() << ").\n";
    return;
  }
  char buff[256];
  envStream.getline(buff, 255);
  buff[8] = 0;
  if (strcmp(buff, "CGX_CF10") == 0)
    return;

  ofstream cStream(filenameCoded);
  cStream<<"CGX_CF10";
  
  // main loop
  for (;;) {
    // read in one line
    envStream.getline(buff, 255);
    if (!envStream)
      break;
    CodeString(buff, 255);
    cStream<<buff;
  }
  
}

bool
Environment::ReadEnvFile(const char *envFilename)
{
  char buff[MaxStringLength], name[MaxStringLength];
  char *s, *t;
  int i, line = 0;
  bool found;
  //  igzstream envStream(envFilename);
  ifstream envStream(envFilename);

  // some error had occured
  if (envStream.fail()) {
    cerr << "Error: Can't open file " << envFilename << " for reading (err. "
         << envStream.rdstate() << ").\n";
    return false;
  }

  name[0] = '\0';

//    bool coded;
//    envStream.getline(buff, 255);
//    buff[8] = 0;
//    if (strcmp(buff, "CGX_CF10") == 0) 
//      coded = true;
//    else {
//      coded = false;
//      envStream.Rewind();
//    }
  
  // main loop
  for (;;) {
    // read in one line
    envStream.getline(buff, MaxStringLength-1);
    
    if (!envStream)
      break;
	
//      if (coded)
//        DecodeString(buff, 255);

    line++;
    // get rid of comments
    s = strchr(buff, '#');
    if (s != NULL)
      *s = '\0';

    // get one identifier
    s = ParseString(buff, name);
	
#ifdef _DEBUG_PARAMS
    cout<<line<<" : ";
    if ( s == NULL)
      cout<<"NULL\n";
    else
      cout<<name<<" "<<s<<endl;
#endif
	
    // parse line
    while (s != NULL) {
      // it's a group name - make the full name
      if (*s == '{') {
        strcat(name, ".");
        s++;
        s = ParseString(s, name);
        continue;
      }
      // end of group
      if (*s == '}') {
        if (strlen(name) == 0) {
          cerr << "Error: unpaired } in " << envFilename << " (line "
               << line << ").\n";
          envStream.close();
          return false;
        }
        name[strlen(name) - 1] = '\0';
        t = strrchr(name, '.');
        if (t == NULL)
          name[0] = '\0';
        else
          *(t + 1) = '\0';
        s++;
        s = ParseString(s, name);
        continue;
      }
      // find variable name in the table
      found = false;
      for (i = 0; i < numOptions; i++)
        if (!strcmp(name, options[i].name)) {
          found = true;
          break;
        }
	  
      if (!found) {
        cerr << "Warning: unknown option " << name << " in environment file "
             << envFilename << " (line " << line << ").\n";
      }
      else
	switch (options[i].type) {
        case optInt: {
          strtol(s, &t, 10);
          if (t == s || (*t != ' ' && *t != '\t' &&
                         *t != '\0' && *t != '}')) {
            cerr << "Error: Mismatch in int variable " << name << " in "
                 << "environment file " << envFilename << " (line "
                 << line << ").\n";
            envStream.close();
            return false;
          }
          if (options[i].value != NULL)
            delete options[i].value;
          options[i].value = new char[t - s + 1];
          strncpy(options[i].value, s, t - s);
          options[i].value[t - s] = '\0';
          s = t;
          break;
        }
        case optFloat: {
          strtod(s, &t);
          if (t == s || (*t != ' ' && *t != '\t' &&
                         *t != '\0' && *t != '}')) {
            cerr << "Error: Mismatch in float variable " << name << " in "
                 << "environment file " << envFilename << " (line "
                 << line << ").\n";
            envStream.close();
            return false;
          }
          if (options[i].value != NULL)
            delete options[i].value;
          options[i].value = new char[t - s + 1];
          strncpy(options[i].value, s, t - s);
          options[i].value[t - s] = '\0';
          s = t;
          break;
        }
        case optBool: {
          t = s;
          while ((*t >= 'a' && *t <= 'z') ||
                 (*t >= 'A' && *t <= 'Z') ||
                 *t == '+' || *t == '-')
            t++;
          if (((!strncasecmp(s, "true", t - s)  && t - s == 4) ||
               (!strncasecmp(s, "false", t - s) && t - s == 5) ||
               (!strncasecmp(s, "YES", t -s)    && t - s == 3) ||
               (!strncasecmp(s, "NO", t - s)    && t - s == 2) ||
               (!strncasecmp(s, "ON", t - s)    && t - s == 2) ||
               (!strncasecmp(s, "OFF", t - s)   && t - s == 3) ||
               (t - s == 1 && (*s == '+' || *s == '-'))) &&
              (*t == ' ' || *t == '\t' || *t == '\0' || *t == '}')) {
            if (options[i].value != NULL)
              delete options[i].value;
            options[i].value = new char[t - s + 1];
            strncpy(options[i].value, s, t - s);
            options[i].value[t - s] = '\0';
            s = t;
          }
          else {
            cerr << "Error: Mismatch in bool variable " << name << " in "
                 << "environment file " << envFilename << " (line "
                 << line << ").\n";
            envStream.close();
            return false;
          }
          break;
        }
        case optVector:{
          strtod(s, &t);
          if (*t == ' ' || *t == '\t') {
            while (*t == ' ' || *t == '\t')
              t++;
            if (*t != ',')
              t--;
          }
          if (t == s || (*t != ' ' && *t != '\t' && *t != ',')) {
            cerr << "Error: Mismatch in vector variable " << name << " in "
                 << "environment file " << envFilename << " (line "
                 << line << ").\n";
            envStream.close();
            return false;
          }
          char *u;
          strtod(t, &u);
          t = u;
          if (*t == ' ' || *t == '\t') {
            while (*t == ' ' || *t == '\t')
              t++;
            if (*t != ',')
              t--;
          }
          if (t == s || (*t != ' ' && *t != '\t' && *t != ',')) {
            cerr << "Error: Mismatch in vector variable " << name << " in "
                 << "environment file " << envFilename << " (line "
                 << line << ").\n";
            envStream.close();
            return false;
          }
          strtod(t, &u);
          t = u;
          if (t == s || (*t != ' ' && *t != '\t' &&
                         *t != '\0' && *t != '}')) {
            cerr << "Error: Mismatch in vector variable " << name << " in "
                 << "environment file " << envFilename << " (line "
                 << line << ").\n";
            envStream.close();
            return false;
          }
          if (options[i].value != NULL)
            delete options[i].value;
          options[i].value = new char[t - s + 1];
          strncpy(options[i].value, s, t - s);
          options[i].value[t - s] = '\0';
          s = t;
          break;
        }
        case optString: {
          if (options[i].value != NULL)
            delete options[i].value;
          options[i].value = new char[strlen(s) + 1];
          strcpy(options[i].value, s);
          s += strlen(s);
		  int last = strlen(options[i].value)-1;
		  if (options[i].value[last] == 0x0a ||
			  options[i].value[last] == 0x0d)
			options[i].value[last] = 0;		  
          break;
        }
        default: {
          Debug << "Internal error: Unknown type of option.\n" << flush;
          exit(1);
        }
      }
      // prepare the variable name for next pass
      t = strrchr(name, '.');
      if (t == NULL)
        name[0] = '\0';
      else
        *(t + 1) = '\0';
      // get next identifier
      s = ParseString(s, name);
    }
  }
  envStream.close();
  return true;
}

void
Environment::PrintUsage(ostream &s) const
{
  // Print out all environment variable names
  s << "Registered options:\n";
  for (int j = 0; j < numOptions; j++)
    s << options[j] << "\n";
  s << flush;
}

/**
   Input scene filename. Currently simplified X3D (.x3d), Unigraphics (.dat),
   and UNC (.ply) formats are supported.
*/

Environment::Environment()
{
  optionalParams = NULL;
  paramRows = 0;
  numParams = 0;
  params = NULL;
  maxOptions = 500;

  
// this is maximal nuber of options.
  numOptions = 0;

  options = new COption[maxOptions];

  if (options == NULL ) {
    Debug << "Error: Memory allocation failed.\n";
    exit(1);
  }

  return;
}

void
Environment::SetStaticOptions()
{
  // get Global option values
  GetFloatValue("Limits.threshold", Limits::Threshold);
  GetFloatValue("Limits.small", Limits::Small);
  GetFloatValue("Limits.infinity", Limits::Infinity);
}

bool
Environment::Parse(const int argc, char **argv, bool useExePath)
{
  bool result = true;
  // Read the names of the scene, environment and output files
  ReadCmdlineParams(argc, argv, "");

  char *envFilename = new char[128];
  char filename[64];

  // Get the environment file name
  if (!GetParam(' ', 0, filename)) {
    // user didn't specified environment file explicitly, so
    strcpy(filename, "default.env");
  }
  
  if (useExePath) {
    char *path = GetPath(argv[0]);
    if (*path != 0)
      sprintf(envFilename, "%s/%s", path, filename);
    else
      strcpy(envFilename, filename);
    
    delete path;
  }
  else
    strcpy(envFilename, filename);

  
  // Now it's time to read in environment file.
  if (!ReadEnvFile(envFilename)) {
    // error - bad input file name specified ?
    cerr<<"Error parsing environment file "<<envFilename<<endl;
	result = false;
  }
  delete envFilename;

  // Parse the command line; options given on the command line subsume
  // stuff specified in the input environment file.
  ParseCmdline(argc, argv, 0);

  SetStaticOptions();

  // Check for request for help
  if (CheckForSwitch(argc, argv, '?')) {
    PrintUsage(cout);
    exit(0);
  }
  
  return true;
}

// End of file Environment.cpp

