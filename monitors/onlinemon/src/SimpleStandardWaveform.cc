/*
 * SimpleStandardWaveform.cxx
 *
 *  Created on: Jun 9, 2011
 *      Author: stanitz
 */

#include "include/SimpleStandardWaveform.hh"

// marc SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name,  const int id, unsigned int nsamples, OnlineMonConfiguration* mymon ) :
SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name,  const int id, unsigned int nsamples, OnlineMonConfiguration* mymon ) :
	_name(name), _id(id), calculated(false),_nsamples(nsamples), _sign(-1),_channelnumber(-1)
{

}

// marc SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name, const int id, unsigned int nsamples) :
SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name, const int id,  unsigned int nsamples) :
		_name(name), _id(id), calculated(false),_nsamples(nsamples),_sign(1),_channelnumber(-1)
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

float SimpleStandardWaveform::getIntegral(float min, float max) const {
	float integral = 0;
	int i;
	for (i = min; i <= int(max+1) && i < _nsamples;i++){
		integral += _data[i];
	}
	return integral/(float)(i-(int)min);
}
float SimpleStandardWaveform::getMaximum(float min, float max) const {
    float maxVal = -999;
    int imax = min;
    for (int i = min; i <= int(max+1) && i < _nsamples ;i++){
        if (abs(_data[i]) > maxVal){ maxVal = abs(_data[i]); imax = i; }
    }
    maxVal = _data[imax];
    return maxVal;

}
