/*
	transpose: Convert a libfm-format file in a binary sparse matrix for x and a dense vector for the target y. 

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2013-07-03

	Copyright 2011-2013 Steffen Rendle, see license.txt for more information
*/

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include "../../util/util.h"
#include "../../util/cmdline.h"
#include "../src/Data.h"

/**
 * 
 * Version history:
 * 1.4.0:
 *	no differences, version numbers are kept in sync over all libfm tools
 * 1.3.6:
 *	binary mode for file access
 * 1.3.4:
 *	no differences, version numbers are kept in sync over all libfm tools
 * 1.3.2:
 *	reading without token reader class
 * 1.0:
 *	first version
 */
 


using namespace std;

int main(int argc, char **argv) { 
 	
 	srand ( time(NULL) );
	try {
		CMDLine cmdline(argc, argv);
		cout << "----------------------------------------------------------------------------" << endl;
		cout << "Convert" << endl;
		cout << "  Version: 1.4.0" << endl;
		cout << "  Author:  Steffen Rendle, steffen.rendle@uni-konstanz.de" << endl;
		cout << "  WWW:     http://www.libfm.org/" << endl;
		cout << "  License: Free for academic use. See license.txt." << endl;
		cout << "----------------------------------------------------------------------------" << endl;
		
		const string param_ifile	= cmdline.registerParameter("ifile", "input file name, file has to be in binary sparse format [MANDATORY]");
		const string param_ofilex	= cmdline.registerParameter("ofilex", "output file name for x [MANDATORY]");
		const string param_ofiley	= cmdline.registerParameter("ofiley", "output file name for y [MANDATORY]");
		const string param_help       = cmdline.registerParameter("help", "this screen");


		if (cmdline.hasParameter(param_help) || (argc == 1)) {
			cmdline.print_help();
			return 0;
		}
		cmdline.checkParameters();

		string ifile = cmdline.getValue(param_ifile);
		string ofilex = cmdline.getValue(param_ofilex);
		string ofiley = cmdline.getValue(param_ofiley);

		uint num_rows = 0;
		uint64 num_values = 0;
		uint num_feature = 0;
		bool has_feature = false;
		DATA_FLOAT min_target = +numeric_limits<DATA_FLOAT>::max();
		DATA_FLOAT max_target = -numeric_limits<DATA_FLOAT>::max();

		// (1) determine the number of rows and the maximum feature_id
		{
			ifstream fData(ifile.c_str());
			if (! fData.is_open()) {
				throw "unable to open " + ifile;
			}
			DATA_FLOAT _value;
			int nchar;
			uint _feature;
			while (!fData.eof()) {
				string line;
				getline(fData, line);
				const char *pline = line.c_str();
				while ((*pline == ' ')  || (*pline == 9)) { pline++; } // skip leading spaces
				if ((*pline == 0)  || (*pline == '#')) { continue; }  // skip empty rows
				if (sscanf(pline, "%f%n", &_value, &nchar) >=1) {
					pline += nchar;
					min_target = min(_value, min_target);
					max_target = max(_value, max_target);			
					num_rows++;
					while (sscanf(pline, "%d:%f%n", &_feature, &_value, &nchar) >= 2) {
						pline += nchar;	
						num_feature = max(_feature, num_feature);
						has_feature = true;
						num_values++;	
					}
					while ((*pline != 0) && ((*pline == ' ')  || (*pline == 9))) { pline++; } // skip trailing spaces
					if ((*pline != 0)  && (*pline != '#')) { 
						throw "cannot parse line \"" + line + "\" at character " + pline[0];
					}
				} else {
					throw "cannot parse line \"" + line + "\" at character " + pline[0];
				}
			} 
			fData.close();
		}
		if (has_feature) {	
			num_feature++; // number of feature is bigger (by one) than the largest value
		}
		cout << "num_rows=" << num_rows << "\tnum_values=" << num_values << "\tnum_features=" << num_feature << "\tmin_target=" << min_target << "\tmax_target=" << max_target << endl;
		
		sparse_row<DATA_FLOAT> row;
		row.data = new sparse_entry<DATA_FLOAT>[num_feature];

		// (2) read the data and write it back simultaneously
		{
			ifstream fData(ifile.c_str());
			if (! fData.is_open()) {
				throw "unable to open " + ifile;
			}
			ofstream out_x(ofilex.c_str(), ios_base::out | ios_base::binary);
			if (! out_x.is_open()) {
				throw "unable to open " + ofilex;
			} else {
				file_header fh;
				fh.id = FMATRIX_EXPECTED_FILE_ID;
				fh.num_values = num_values;
				fh.num_rows = num_rows;
				fh.num_cols = num_feature;
				fh.float_size = sizeof(DATA_FLOAT);
				out_x.write(reinterpret_cast<char*>(&fh), sizeof(fh));
			}
			ofstream out_y(ofiley.c_str(), ios_base::out | ios_base::binary);
			if (! out_y.is_open()) {
				throw "unable to open " + ofiley;
			} else {
				uint file_version = 1;
				uint data_size = sizeof(DATA_FLOAT);
				out_y.write(reinterpret_cast<char*>(&file_version), sizeof(file_version));
				out_y.write(reinterpret_cast<char*>(&data_size), sizeof(data_size));
				out_y.write(reinterpret_cast<char*>(&num_rows), sizeof(num_rows));
			}

			DATA_FLOAT _value;
			int nchar;
			uint _feature;
			while (!fData.eof()) {
				string line;
				getline(fData, line);
				const char *pline = line.c_str();
				while ((*pline == ' ')  || (*pline == 9)) { pline++; } // skip leading spaces
				if ((*pline == 0)  || (*pline == '#')) { continue; }  // skip empty rows
				if (sscanf(pline, "%f%n", &_value, &nchar) >=1) {
					pline += nchar;
					out_y.write(reinterpret_cast<char*>(&(_value)), sizeof(DATA_FLOAT));
					row.size = 0;
					while (sscanf(pline, "%d:%f%n", &_feature, &_value, &nchar) >= 2) {
						pline += nchar;	
						assert(row.size < num_feature);
						row.data[row.size].id = _feature;
						row.data[row.size].value = _value;
						row.size++;	
					}
					out_x.write(reinterpret_cast<char*>(&(row.size)), sizeof(uint));
					out_x.write(reinterpret_cast<char*>(row.data), sizeof(sparse_entry<DATA_FLOAT>)*row.size);
					while ((*pline != 0) && ((*pline == ' ')  || (*pline == 9))) { pline++; } // skip trailing spaces
					if ((*pline != 0)  && (*pline != '#')) { 
						throw "cannot parse line \"" + line + "\" at character " + pline[0];
					}
				} else {
					throw "cannot parse line \"" + line + "\" at character " + pline[0];
				}
			}	
			fData.close();
			out_x.close();
			out_y.close();

		}
	} catch (string &e) {
		cerr << e << endl;
	}

}
