/*
 * WaveformSignalRegion.cc
 *
 *  Created on: Oct 13, 2015
 *      Author: bachmair
 */

#include <eudaq/WaveformSignalRegion.h>

//ClassImp(eudaq::WaveformSignalRegion);
eudaq::WaveformSignalRegion::WaveformSignalRegion(int low, int high, std::string name):peak_pos(-1) {
    // TODO Auto-generated constructor stub
   this->low_border = low;
   this->high_border = high;
   this->polarity = 0;
   this->name=name;
}

eudaq::WaveformSignalRegion::~WaveformSignalRegion() {
    // TODO Auto-generated destructor stub
}

void eudaq::WaveformSignalRegion::calculateIntegrals(const StandardWaveform* wf) {
    if (polarity>0)
        this->peak_pos = wf->getIndexMax(low_border,high_border);
    else
        this->peak_pos = wf->getIndexMin(low_border,high_border);
    for (size_t i = 0; i<this->signals.size();i++){
        signals[i].SetPeakPosition(peak_pos,wf->GetNSamples());
        float integral = wf->getIntegral(signals[i].GetIntegralStart(),signals[i].GetIntegralStop());
//        signals[i].calculateIntegral(peak_pos,wf);
        signals[i].SetIntegral(integral);
    }
//    std::cout<<"BLA"<<std::endl;
//    for (auto s: this->signals)
//        std::cout<<"\n\t\t* "<<s;
//    std::cout<<"BLA"<<std::endl;
//    this->Print(std::cout);
//    std::cout<<*this<<std::endl;
}

void eudaq::WaveformSignalRegion::ResetIntegrals() {
    for (auto s: this->signals)
        s.Reset();
    peak_pos = -1;
}

void eudaq::WaveformSignalRegion::Print(std::ostream& out) const {
    out<<"WaveformSignalRegion: \""<<name<<"\": "<<low_border<<"-"<<high_border<<" for Polarity: "<<(int)polarity;
    out<<" with "<<signals.size()<<" Integrals";
    for (auto s: this->signals)
        out<<"\n\t\t* "<<s;
}
