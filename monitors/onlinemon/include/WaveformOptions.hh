/*
 * WaveformOptions.hh
 *
 *  Created on: Apr 16, 2015
 *      Author: bachmair
 */

#ifndef WAVEFORMOPTIONS_HH_
#define WAVEFORMOPTIONS_HH_

#include <string>
#include <vector>
#include <map>

#include <fstream>
#include <iostream>
#include <sstream>

class WaveformOptions {
public:
	WaveformOptions();
	virtual ~WaveformOptions();
	void SetVariable(std::string key, std::string value);

public:
	//taken from http://www.cplusplus.com/forum/articles/9645/
	template <typename T> static T StringToNumber(std::string Text ) //Text not by constANT reference so that the function can be used with a character array as argument
	{
		std::stringstream ss(Text);
		T result;
		return ss >> result ? result : 0;
	}
};

#endif /* WAVEFORMOPTIONS_HH_ */
