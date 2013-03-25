#include "util.h"

#include <limits.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <execinfo.h>

/* from http://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c */
std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu)
{
	size_t uiui = - neu.size();

	do
	{
		uiui = subj.find(old, uiui + neu.size());
		if (uiui != std::string::npos)
		{
			subj.erase(uiui, old.size());
			subj.insert(uiui, neu);
		}
	} while(uiui != std::string::npos);
	return subj;
}

unsigned int countOccurencies(const std::string &str, const std::string &substr)
{
	size_t  pos = 0, count = 0;
	while ((pos = str.find(substr, pos)) != std::string::npos) {
		count++;
		pos++;
	}
	return count;
}

/* WARNING: This function is not portable! Linux ONLY! */
std::string getExeDirectory()
{
	static std::string exeDir;

	if (exeDir != std::string())
		return exeDir;

	char path[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
	path[len] = '\0';

	if (len < 0)
		perror("readlink failed");

	std::string dbPath(path);
	std::size_t dir = dbPath.find_last_of('/');
	if (dir != std::string::npos)
		exeDir = dbPath.substr(0, dir);
	else
		exeDir = std::string();

	return exeDir;
}

std::string getValueFromEnv(const Wt::WEnvironment& env,
		const std::string &key,
		const std::string &defaultValue)
{
	std::vector<std::string> param = env.getParameterValues(key);
	if (param.size() > 0) {
		return param[0];
	} else
		return defaultValue;
}

std::string floatToString(double value, int precision)
{
	std::stringstream ss;
	ss.precision(precision);
	ss << value;
	return ss.str();
}

void segv_handler(int sig) {
	void *array[1000];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 1000);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, 2);
	exit(1);
}
