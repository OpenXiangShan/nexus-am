#include "../../types.h"
#include "configSys.h"

std::string cfgFile = "fceux.cfg";
/**
 * Add a given option.  The option is specified as a short command
 * line (-f), long command line (--foo), option name (Foo), its type
 * (integer or string).
 */
int
Config::_addOption(char shortArg,
                   const std::string &longArg,
                   const std::string &name,
                   int type)
{
    // make sure we have a valid type
    if(type != INTEGER && type != STRING &&
       type != DOUBLE && type != FUNCTION) {
        return -1;
    }

    // check if the option already exists
    if(_shortArgMap.find(shortArg) != _shortArgMap.end() ||
       _longArgMap.find(longArg) != _longArgMap.end() ||
       (type == INTEGER && _intOptMap.find(name) != _intOptMap.end()) ||
       (type == STRING  && _strOptMap.find(name) != _strOptMap.end()) ||
       (type == DOUBLE  && _dblOptMap.find(name) != _dblOptMap.end())) {
        return -1;
    }

    // add the option
    switch(type) {
    case(STRING):
        _strOptMap[name] = "";
        break;
    case(INTEGER):
        _intOptMap[name] = 0;
        break;
    case(DOUBLE):
        _dblOptMap[name] = 0.0;
        break;
    case(FUNCTION):
        _fnOptMap[name] = NULL;
        break;
    default:
        break;
    }
    _shortArgMap[shortArg] = name;
    _longArgMap[longArg] = name;

    return 0;
}

int
Config::_addOption(const std::string &longArg,
                   const std::string &name,
                   int type)
{
    // make sure we have a valid type
    if(type != STRING && type != INTEGER && type != DOUBLE) {
        return -1;
    }

    // check if the option already exists
    if(_longArgMap.find(longArg) != _longArgMap.end() ||
       (type == STRING  && _strOptMap.find(name) != _strOptMap.end()) ||
       (type == INTEGER && _intOptMap.find(name) != _intOptMap.end()) ||
       (type == DOUBLE  && _dblOptMap.find(name) != _dblOptMap.end())) {
        return -1;
    }

    // add the option
    switch(type) {
    case(STRING):
        _strOptMap[name] = "";
        break;
    case(INTEGER):
        _intOptMap[name] = 0;
        break;
    case(DOUBLE):
        _dblOptMap[name] = 0.0;
        break;
    default:
        break;
    }
    _longArgMap[longArg] = name;

    return 0;
}


/**
 * Add a given option and sets its default value.  The option is
 * specified as a short command line (-f), long command line (--foo),
 * option name (Foo), its type (integer or string), and its default
 * value.
 */
int
Config::addOption(char shortArg,
                  const std::string &longArg,
                  const std::string &name,
                  int defaultValue)
{
    int error;

    // add the option to the config system
    error = _addOption(shortArg, longArg, name, INTEGER);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultValue);
    if(error) {
        return error;
    }

    return 0;
}

/**
 * Add a given option and sets its default value.  The option is
 * specified as a short command line (-f), long command line (--foo),
 * option name (Foo), its type (integer or string), and its default
 * value.
 */
int
Config::addOption(char shortArg,
                  const std::string &longArg,
                  const std::string &name,
                  double defaultValue)
{
    int error;

    // add the option to the config system
    error = _addOption(shortArg, longArg, name, DOUBLE);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultValue);
    if(error) {
        return error;
    }

    return 0;
}


/**
 * Add a given option and sets its default value.  The option is
 * specified as a short command line (-f), long command line (--foo),
 * option name (Foo), its type (integer or string), and its default
 * value.
 */
int
Config::addOption(char shortArg,
                  const std::string &longArg,
                  const std::string &name,
                  const std::string &defaultValue)
{
    int error;

    // add the option to the config system
    error = _addOption(shortArg, longArg, name, STRING);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultValue);
    if(error) {
        return error;
    }

    return 0;
}

int
Config::addOption(char shortArg,
                  const std::string &longArg,
                  const std::string &name,
                  void (*defaultFn)(const std::string &))
{
    int error;

    // add the option to the config system
    error = _addOption(shortArg, longArg, name, FUNCTION);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultFn);
    if(error) {
        return error;
    }

    return 0;
}

int
Config::addOption(const std::string &longArg,
                  const std::string &name,
                  const std::string &defaultValue)
{
    int error;

    // add the option to the config system
    error = _addOption(longArg, name, STRING);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultValue);
    if(error) {
        return error;
    }

    return 0;
}

int
Config::addOption(const std::string &longArg,
                  const std::string &name,
                  int defaultValue)
{
    int error;

    // add the option to the config system
    error = _addOption(longArg, name, INTEGER);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultValue);
    if(error) {
        return error;
    }

    return 0;
}

int
Config::addOption(const std::string &longArg,
                  const std::string &name,
                  double defaultValue)
{
    int error;

    // add the option to the config system
    error = _addOption(longArg, name, DOUBLE);
    if(error) {
        return error;
    }

    // set the option to the default value
    error = setOption(name, defaultValue);
    if(error) {
        return error;
    }

    return 0;
}

int
Config::addOption(const std::string &name,
                  const std::string &defaultValue)
{
    if(_strOptMap.find(name) != _strOptMap.end()) {
        return -1;
    }

    // add the option
    _strOptMap[name] = defaultValue;
    return 0;
}

int
Config::addOption(const std::string &name,
                  int defaultValue)
{
    if(_intOptMap.find(name) != _intOptMap.end()) {
        return -1;
    }

    // add the option
    _intOptMap[name] = defaultValue;
    return 0;
}

int
Config::addOption(const std::string &name,
                  double defaultValue)
{
    if(_dblOptMap.find(name) != _dblOptMap.end()) {
        return -1;
    }

    // add the option
    _dblOptMap[name] = defaultValue;
    return 0;
}

/**
 * Sets the specified option to the given integer value.
 */
int
Config::setOption(const std::string &name,
                  int value)
{
    std::map<std::string, int>::iterator opt_i;

    // confirm that the option exists
    opt_i = _intOptMap.find(name);
    if(opt_i == _intOptMap.end()) {
        return -1;
    }

    // set the option
    opt_i->second = value;
    return 0;
}

/**
 * Sets the specified option to the given integer value.
 */
int
Config::setOption(const std::string &name,
                  double value)
{
    std::map<std::string, double>::iterator opt_i;

    // confirm that the option exists
    opt_i = _dblOptMap.find(name);
    if(opt_i == _dblOptMap.end()) {
        return -1;
    }

    // set the option
    opt_i->second = value;
    return 0;
}

/**
 * Sets the specified option to the given string value.
 */
int
Config::setOption(const std::string &name,
                  const std::string &value)
{
    std::map<std::string, std::string>::iterator opt_i;

    // confirm that the option exists
    opt_i = _strOptMap.find(name);
    if(opt_i == _strOptMap.end()) {
        return -1;
    }

    // set the option
    opt_i->second = value;
    return 0;
}

/**
 * Sets the specified option to the given function.
 */
int
Config::setOption(const std::string &name,
                  void (*value)(const std::string &))
{
    std::map<std::string, void (*)(const std::string &)>::iterator opt_i;

    // confirm that the option exists
    opt_i = _fnOptMap.find(name);
    if(opt_i == _fnOptMap.end()) {
        return -1;
    }

    // set the option
    opt_i->second = value;
    return 0;
}


int
Config::getOption(const std::string &name,
                  std::string *value) const
{
    std::map<std::string, std::string>::const_iterator opt_i;

    // confirm that the option exists
    opt_i = _strOptMap.find(name);
    if(opt_i == _strOptMap.end()) {
        return -1;
    }

    // get the option
    (*value) = opt_i->second;
    return 0;
}

int
Config::getOption(const std::string &name,
                  const char **value) const
{
    std::map<std::string, std::string>::const_iterator opt_i;

    // confirm that the option exists
    opt_i = _strOptMap.find(name);
    if(opt_i == _strOptMap.end()) {
        return -1;
    }

    // get the option
    (*value) = opt_i->second.c_str();
    return 0;
}

int
Config::getOption(const std::string &name,
                  int *value) const
{
    std::map<std::string, int>::const_iterator opt_i;

    // confirm that the option exists
    opt_i = _intOptMap.find(name);
    if(opt_i == _intOptMap.end()) {
        return -1;
    }

    // get the option
    (*value) = opt_i->second;
    return 0;
}

int
Config::getOption(const std::string &name,
                  double *value) const
{
    std::map<std::string, double>::const_iterator opt_i;

    // confirm that the option exists
    opt_i = _dblOptMap.find(name);
    if(opt_i == _dblOptMap.end()) {
        return -1;
    }

    // get the option
    (*value) = opt_i->second;
    return 0;
}

/**
 * Parses first the configuration file, and then overrides with any
 * command-line options that were specified.
 */
int
Config::parse(int argc,
              char **argv)
{
    // parse the arguments
  return 1;
}
