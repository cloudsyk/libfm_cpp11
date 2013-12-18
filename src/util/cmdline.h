/*
	Command line parser

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2012-01-04

	Copyright 2010-2012 Steffen Rendle, see license.txt for more information
*/

#ifndef CMDLINE_H_
#define CMDLINE_H_

#include <map>
#include <vector>
#include "util.h"

using namespace std;

class CMDLine {
	protected:
		map< string, string > help;
		map< string, string > value;
		bool parse_name(string& s) {
			if ((s.length() > 0) && (s[0] == '-')) {
				if ((s.length() > 1) && (s[1] == '-')) {
					s = s.substr(2);
				} else {
					s = s.substr(1);
				}
				return true;
			} else {
				return false;
			}		
		}

	public:
		string delimiter;

		CMDLine(int argc, char **argv) {
			delimiter = ";,";
			int i = 1;
			while (i < argc) {
				string s(argv[i]);
				if (parse_name(s)) {
					if (value.find(s) != value.end()) {
						throw "the parameter " + s + " is already specified"; 							
					}
					if ((i+1) < argc) {
						string s_next(argv[i+1]);
						if (! parse_name(s_next)) {
							value[s] = s_next;
							i++;
						} else {
							value[s] = "";
						}
					} else {
						value[s] = "";
					}
				} else {
					throw "cannot parse " + s;
				}
				i++;
			}
		}

		void setValue(string parameter, string value) {
			this->value[parameter] = value;
		}

		bool hasParameter(string parameter) {
			return (value.find(parameter) != value.end());
		}

		
		void print_help() {
			for (map< string, string >::const_iterator pv = help.begin(); pv != help.end(); ++pv) {
			 	cout << "-" << pv->first;
				for (int i=pv->first.size()+1; i < 16; i++) { cout << " "; } 
				string s_out = pv->second;
				while (s_out.size() > 0) {
					if (s_out.size() > (72-16)) {
						size_t p = s_out.substr(0, 72-16).find_last_of(" \t");
						if (p == 0) {
							p = 72-16;
						}
						cout << s_out.substr(0, p) << endl;
						s_out = s_out.substr(p+1, s_out.length()-p);            
					} else {
						cout << s_out << endl;
						s_out = "";  
					}
					if (s_out.size() > 0) {
						for (int i=0; i < 16; i++) { cout << " "; }
					}
				}
			}
		}
		const string& registerParameter(const string& parameter, const string& help) {
			this->help[parameter] = help;
			return parameter;
		}

		void checkParameters() {
			// make sure there is no parameter specified on the cmdline that is not registered:
			for (map< string, string >::const_iterator pv = value.begin(); pv != value.end(); ++pv) {
				if (help.find(pv->first) == help.end()) {
					throw "the parameter " + pv->first + " does not exist";
				}
			}
		}

		const string& getValue(const string& parameter) {
			return value[parameter];
		}

		const string& getValue(const string& parameter, const string& default_value) {
			if (hasParameter(parameter)) {
				return value[parameter];
			} else {
				return default_value;
			}
		}

		const double getValue(const string& parameter, const double& default_value) {
			if (hasParameter(parameter)) {
				return atof(value[parameter].c_str());
			} else {
				return default_value;
			}
		}

		const int getValue(const string& parameter, const int& default_value) {
			if (hasParameter(parameter)) {
				return atoi(value[parameter].c_str());
			} else {
				return default_value;
			}
		}

		const uint getValue(const string& parameter, const uint& default_value) {
			if (hasParameter(parameter)) {
				return atoi(value[parameter].c_str());
			} else {
				return default_value;
			}
		}

		vector<string> getStrValues(const string& parameter) {
			vector<string> result = tokenize(value[parameter], delimiter);
			return result;
		}
		vector<int> getIntValues(const string& parameter) {
			vector<int> result; 
			vector<string> result_str = getStrValues(parameter);
			result.resize(result_str.size());
			for (uint i = 0; i < result.size(); i++) {
				result[i] = atoi(result_str[i].c_str());
			}
			return result;
		}
		vector<double> getDblValues(const string& parameter) {
			vector<double> result; 
			vector<string> result_str = getStrValues(parameter);
			result.resize(result_str.size());
			for (uint i = 0; i < result.size(); i++) {
				result[i] = atof(result_str[i].c_str());
			}
			return result;
		}
		vector<uint> getUIntValues(const string& parameter) {
			vector<uint> result; 
			vector<string> result_str = getStrValues(parameter);
			result.resize(result_str.size());
			for (uint i = 0; i < result.size(); i++) {
				result[i] = atoi(result_str[i].c_str());
			}
			return result;
		}
};


#endif /*CMDLINE_H_*/
