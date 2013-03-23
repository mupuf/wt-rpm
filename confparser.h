#ifndef CONFPARSER_H
#define CONFPARSER_H

#include <Wt/Json/Object>
#include <iostream>

template <class type_t>
	type_t readJSONValue(Wt::Json::Object result, const std::string &key, type_t defaultValue = type_t()) {
		try {
			return result.get(key);
		} catch (Wt::WException error) {
			std::cerr << "Attribute '" << key << "' is invalid: " << error.what() << std::endl;
			return defaultValue;
		}
	}

class ConfParser
{
private:
	Wt::WString _backend;
	Wt::Json::Object _backendConfig;

	std::string confFilePossibleLocation (int id);

public:
	ConfParser();

	Wt::WString rpmBackend() const { return _backend;};
	Wt::Json::Object rpmBackendConfiguration() const { return _backendConfig;};
};

#endif // CONFPARSER_H
