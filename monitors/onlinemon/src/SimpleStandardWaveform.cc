/*
 * SimpleStandardWaveform.cxx
 *
 *  Created on: Jun 9, 2011
 *      Author: stanitz
 */

#include "include/SimpleStandardWaveform.hh"

SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name,  const int id, OnlineMonConfiguration* mymon) : _name(name), _id(id), calculated(false),_nsamples(1024)
{

}

SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name, const int id) : _name(name), _id(id), calculated(false)
{

}

void SimpleStandardWaveform::addData(float* data) {
	this->_data=data;
}


struct myclass {
  bool operator() (int i,int j) { return i<j; }
} myobj;


void SimpleStandardWaveform::Calculate() {
	if (calculated)
		return;
	_min = *std::min_element(_data,_data+_nsamples);//,myobj);
	_max = *std::max_element(_data,_data+_nsamples);//,myobj);
	_integral = 0;
	for (int i = 0; i < _nsamples;i++)
		_integral += _data[i];
	_integral/=_nsamples;
//	std::cout<<" SimpleStandardWaveform::Calculate "<<_min<<"-"<<_max<<" "<<_integral<<std::endl;
	this->calculated = true;
}
