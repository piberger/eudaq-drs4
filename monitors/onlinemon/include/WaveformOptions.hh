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

namespace eudaq {



class WaveformOptions {
public:
	//taken from http://www.cplusplus.com/forum/articles/9645/
	template <typename T> static T StringToNumber(std::string Text ) //Text not by constANT reference so that the function can be used with a character array as argument
	{
		std::stringstream ss(Text);
		T result;
		return ss >> result ? result : 0;
	}
	// taken from http://stackoverflow.com/questions/2333728/stdmap-default-value
	template <typename K, typename V>
	V GetWithDef(const  std::map <K,V> & m, const K & key, const V & defval ) {
	   typename std::map<K,V>::const_iterator it = m.find( key );
	   if ( it == m.end() ) {
	      return defval;
	   }
	   else {
	      return it->second;
	   }
	}
private:
    std::map<std::string, std::string> KeyValueMap;
    void SetDefaults();
public:
    WaveformOptions();
    virtual ~WaveformOptions();
    void SetVariable(std::string key, std::string value);
    std::string GetStringVariable(std::string key) { return GetWithDef(KeyValueMap,key,(std::string)"");}
    int GetIntVariable(std::string key){return from_string(GetWithDef(KeyValueMap,key,(std::string){"0"}),0);}
    int GetFloatVariable(std::string key){return from_string(GetWithDef(KeyValueMap,key,(std::string){"0"}),0.0);}

};

#endif /* WAVEFORMOPTIONS_HH_ */
