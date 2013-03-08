// ===================================================================
// $Id: Environment.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
//
// environ.h
//     Header file for environment operations, i.e.. reading
//     environment file, reading command line parameters etc.
//
// Class: CEnvironment
//
/// Initial coding by
/// @author             Tomas Kopal, 1996

#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

// forward declarations

#include "common.h"

// Forward declarations
class Vector3;

/// Enumeration type used to distinguish the type of the environment variable
enum EOptType {
  /// the integer variable
  optInt,
  /// the Real variable
  optFloat,
  /// the bool variable
  optBool,
  /// the vector variable
  optVector,
  /// the string variable
  optString
};

class COption
{
public:
  COption() {
    name = NULL;
    value = NULL;
    abbrev = NULL;
    defaultValue = NULL;
    description = NULL;
    pointer = NULL;
  }

  virtual ~COption() {
    if (name)
      delete name;
    if (value)
      delete value;
    if (abbrev)
      delete abbrev;
    if (defaultValue)
      delete defaultValue;
    if (description)
      delete description;
    
  }

  EOptType type;
  char *name;
  char *value;
  char *abbrev;
  char *defaultValue;

  char *description;
  void *pointer;
  friend ostream &operator <<(ostream &s, const COption &o) {
    s<<o.name<<"\t";
    if (o.abbrev)
      s<<o.abbrev;
    else
      s<<"no abbrev";
    s<<"\t";
    if (o.value)
      s<<o.value;
    else
      s<<"not set";
    s<<"\t";
    if (o.defaultValue)
      s<<"["<<o.defaultValue<<"]";
    else
      s<<"[not set]";
    s<<"\t";
    if (o.description)
      s<<endl<<o.description;
    return s;
  }
  
};

/** CEnvironment class.
  This class is used for reading command line, reading environment file and
  managing options. Each recognizable option must be registered in the
  constructor of this class.
 */
class Environment {
  /**@name Variables */
  //@{
private:
  /// Maximal number of options
  int maxOptions;
  /// Number of columns in the parameter table.
  int numParams;
  /// Number of rows in the parameter table.
  int paramRows;
  /// String with prefixes with non-optional parameters.
  char *optionalParams;
  /// Current row in the parameter table.
  int curRow;

  /** Parameter table itself.
    The organization of this table is two dimensional array of character
    strings. First column are parameters, all the other columns are options
    prefixed by char passed to function GetCmdlineParams and paired with
    corresponding parameters.
   */
  char ***params;

  /// Number of registered options.
  int numOptions;
  /** Options table itself.
    This table is two dimensional array of character strings with four
    columns. First column contains name of the option, second it's value,
    third eventual abbreviation, and finally the fourth contains the default
    value specified when registering the option.
    */
  COption *options;
  //  char ***options;
  /** Options types.
    This is auxiliary table to the table "options". It contains the type of
    each option as specified when registering.
    */
  //  EOptType *optTypes;
  //@}

  /**@name Methods */
  //@{
  /** Auxiliary method for parsing environment file.
    Function gets one string from buffer, skipping leading whitespaces, and
    appends it at the end of the string parameter. Returns pointer to the next
    string in the buffer with trailing whitespaces skipped. If no string in
    the buffer can't be found, returns NULL.
    @param buffer Input line for parsing.
    @param string Buffer to append gathered string to.
    @return Pointer to next string or NULL if not succesful.
    */
  char* ParseString(char *buffer, char *string) const; 
  /** Method for checking variable type.
    This method is used to check if the value of the variable can be taken
    as the specified type.
    @param value Tested value of the variable.
    @param type The type of the variable.
    @return true if the variable is of the specified type, false elsewhere.
    */
  bool CheckType(const char *value, const EOptType type) const;
  //@}

  int
  FindOption(const char *name,
	     const bool isFatal=false) const;

  bool
  ParseBool(const char *name) const;
  void
  ParseVector(const char *name, Vector3 &v) const;


public:

  /** Returns the resource manager as a singleton.
   */
  static Environment *GetSingleton();
  static void SetSingleton(Environment *e);
  
  static void DelSingleton();

    /// This method is used as a help on available command line options.
  virtual void PrintUsage(ostream &s) const;

  virtual void SetStaticOptions();


  /** Method for registering new option.
      Using this method is possible to register new option with it's name, type,
      abbreviation and default value.
      @param name Name of the option.
      @param type The type of the variable.
      @param abbrev If not NULL, alias usable as a command line option.
      @param defValue Implicit value used if not specified explicitly.
  */
  void RegisterOption(const char *name, const EOptType type,
		      const char *abbrev, const char *defValue = NULL);
  
  /** Check if the specified switch is present in the command line.
    Primary use of this function is to check for presence of some special
    switch, e.g. -h for help. It can be used anytime.
    @param argc Number of command line arguments.
    @param argv Array of command line arguments.
    @param swtch Switch we are checking for.
    @return true if found, false elsewhere.
   */
  bool CheckForSwitch(const int argc,
		      char **argv,
		      const char swtch) const;
  /** First pass of parsing command line.
    This function parses command line and gets from there all non-optional
    parameters (i.e. not prefixed by the dash) and builds a table of
    parameters. According to param optParams, it also writes some optional
    parameters to this table and pair them with corresponding non-optional
    parameters.
    @param argc Number of command line arguments.
    @param argv Array of command line arguments.
    @param optParams String consisting of prefixes to non-optional parameters.
                     All prefixes must be only one character wide !
   */
  void ReadCmdlineParams(const int argc,
			 char **argv,
			 const char *optParams);
  /** Reading of the environment file.
    This function reads the environment file.
    @param filename The name of the environment file to read.
    @return true if OK, false in case of error.
    */
  bool ReadEnvFile(const char *filename);
  /** Second pass of the command line parsing.
    Performs parsing of the command line ignoring all parameters and options
    read in the first pass. Builds table of options.
    @param argc Number of command line arguments.
    @param argv Array of command line arguments.
    @param index Index of non-optional parameter, of which options should
                 be read in.
   */
  void ParseCmdline(const int argc, char **argv, const int index);

  /** Clears the environment data structures.
    This function is intended to clear all data structures corresponding to
    particular non-optional parameter to allow options for the next
    non-optional parameter to be read. After this clearing, function
    ReadEnvFile() can be called again to obtain next load of options.
    */
  //  void ClearEnvironment();
  
  /** Parameters number query function.
    This function returns number of non-optional parameters specified on
    the command line.
    */
  int GetParamNum() const { return paramRows; }
  /** Returns the indexed parameter or corresponding optional parameter.
   This function is used for queries on individual non-optional parameters.
   The table must be constructed to allow this function to operate (i.e. the
   first pass of command line parsing must be done).
   @param name If space (' '), returns the non-optional parameter, else returns
               the corresponding optional parameter.
   @param index Index of the non-optional parameter to which all other options
                corresponds.
   @param value Return value of the queried parameter.
   @return true if OK, false in case of error or if the parameter wasn't
           specified.
   */
  bool GetParam(const char name, const int index, char *value) const;

  /**@name Options query functions.
   These functions are used for queries on individual options. The table must
   be constructed to allow these functions to operate (i.e. the environment
   file must be read and the second pass of command line parsing be done).
   @param name Name of the option to retrieve.
   @param value Return value of the queried option.
   @param isFatal Boolean value specifying, if the function should exit or
                  return false in case of error.
   @return true if OK, false in case of error.
   */
  //@{
  /// returns true, if the specified option has been specified
  bool OptionPresent(const char *name) const;
  /// returns named option as a integral value.
  bool GetIntValue(const char *name,
		   int &value,
		   const bool isFatal = false) const;
  
  /// returns named option as a floating point value.
  bool GetDoubleValue(const char *name,
		      double &value,
		      const bool isFatal = false) const;

  /// returns named option as a floating point value.
  bool GetFloatValue(const char *name,
		     float &value,
		     const bool isFatal = false) const;

  /// returns named option as a boolean value.
  bool GetBoolValue(const char *name,
		    bool &value,
		    const bool isFatal = false) const;

  bool GetBool(const char *name, const bool isFatal = false) const;
  
  /// returns named option as a 3D vector.
  bool GetVectorValue(const char *name,
		      Vector3 &v,
		      const bool isFatal = false) const;

  /// returns named option as a character string.
  bool GetStringValue(const char *name,
		      char *value,
		      const bool isFatal = false) const;

  bool GetStringValue(const char *name,
		      string &value,
		      const bool isFatal = false) const;

  //@}
  /**@name Options set functions.
   These functions are used for setting individual options.
   @param name Name of the option to set.
   @param value Value to set the option to.
   */
  //@{
  /// sets named option as a integral value.
  void SetInt(const char *name, const int value);
  /// sets named option as a floating point value.
  void SetFloat(const char *name, const float value);
  /// sets named option as a boolean value.
  void SetBool(const char *name, const bool value);
  /// sets named option as a 3D vector.
  void SetVector(const char *name,
		 const Vector3 &v);
  /// sets named option as a character string.
  void SetString(const char *name, const char *value);
  //@}
  //@}

  bool Parse(const int argc, char **argv, bool useExePath);

  void
  CodeString(char *buff, int max);

  void
  DecodeString(char *buff, int max);

  void
  SaveCodedFile(char *filenameText,
		char *filenameCoded);

  
  virtual void RegisterOptions() = 0;
  
protected:

  /**@name Methods */  //@{
  /// Constructor of the CEnvironment class.
  Environment();

  /// Destructor of the CEnvironment class.

  virtual ~Environment();

private:
  static Environment *sEnvironment;
};

// global environment value
//extern Environment *environment;


#endif // __ENVIRON_H__

