/*
 * SimpleStandardWaveform.hh
 *
 *  Created on: Jun 9, 2011
 *      Author: stanitz
 */

#ifndef SimpleStandardWaveform_HH_
#define SimpleStandardWaveform_HH_

#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>

#include <stdio.h>
#include <string.h>

#include "include/OnlineMonConfiguration.hh"

//!Simple Standard Waveform Class
/*!

 */
class SimpleStandardWaveform {
protected:
	std::string _name;
	int _id;

	int _tlu_event;
public:
	SimpleStandardWaveform(const std::string & name, const int id, OnlineMonConfiguration* mymon);
	SimpleStandardWaveform(const std::string & name, const int id);
	virtual ~SimpleStandardWaveform() {};
	std::string getName() const {return _name;}
	void addSuffix( const std::string & suf ) { _name = _name + suf; }
	int getID() const {return _id;}
	void addData(float *data);
	void setMonitorConfiguration(OnlineMonConfiguration *mymon)
	{
		mon=mymon;
	}
	void Calculate();
	float getMax() const{return !calculated?-1e9:_max;};
	float getMin()const{return !calculated?-1e9:_min;};
	float getIntegral() const{return !calculated?-1e9:_integral;}
	void setNSamples(unsigned int n_samples){_nsamples = n_samples;}
	unsigned int getNSamples() const{return _nsamples;}
private:
	bool calculated;
	float *_data;
	float _max;
	float _min;
	float _integral;
	unsigned int _nsamples;
	OnlineMonConfiguration* mon;
};

#endif //ifndef SimpleStandardWaveform_HH_
