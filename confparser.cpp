#include "confparser.h"

#include <Wt/Json/Parser>
#include <Wt/Json/Array>
#include <Wt/Json/Value>

#include <fstream>
#include <string>

std::string ConfParser::confFilePossibleLocation (int id)
{
	switch (id) {
	case 0:
		return "wt-rpm.json";
	case 1:
		return "../wt-rpm.json";
	case 2:
		return getenv("HOME") + std::string("/.wt-rpm.json");
	case 3:
		return "/etc/wt-rpm.json";
	default:
		return std::string();
	}
}

ConfParser::ConfParser()
{
	std::string path, file;
	int id = 0;

	/* read the file */
	while ((path = confFilePossibleLocation(id)) != std::string()) {
		std::ifstream db(path);
		if (db.is_open())
		{
			std::string line;
			while (db.good()) {
				getline(db, line);
				file += line;
			}
			db.close();

			break;
		}
		else {
			std::cerr << "The configuration file '" + path + "' cannot be opened" << std::endl;
			id++;
		}
	}

	if (file.size() == 0)
		return;

	/* parse the configuration file */
	try {
		Wt::Json::Object parsed_file;
		Wt::WString backend;

		Wt::Json::parse(file, parsed_file);

		backend = readJSONValue<Wt::WString>(parsed_file, "backend");
		if (backend == std::string()) {
			std::cerr << "No backend found in configuration file '" << path << "'. Abort." << std::endl;
			return;
		}
		this->_backend = backend;

		_backendConfig = readJSONValue<Wt::Json::Object>(parsed_file, backend.toUTF8());
	}
	catch (Wt::WException error)
	{
		std::cerr << "Error while parsing file '" << path << "': " << error.what() << "\n" << std::endl;
	}
}
