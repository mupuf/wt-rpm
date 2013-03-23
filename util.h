#ifndef UTIL_H
#define UTIL_H
	#include <string>
	#include <iostream>

	#include <Wt/WEnvironment>

	#define UNUSED(expr) do { (void)(expr); } while (0)

	std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu);
	unsigned int countOccurencies(const std::string &str, const std::string &substr);
	std::string getExeDirectory();
	std::string getValueFromEnv(const Wt::WEnvironment& env, const std::string &key, const std::string &defaultValue);
#endif // UTIL_H
