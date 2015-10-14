/*
 * WaveformSignalRegions.cpp
 *
 *  Created on: Oct 14, 2015
 *      Author: bachmair
 */

#include <eudaq/WaveformSignalRegions.hh>

//ClassImp(eudaq::WaveformSignalRegions);
void eudaq::WaveformSignalRegions::AddRegion(WaveformSignalRegion region) {
    region.SetPolarity(polarity);
    this->regions.push_back(region);
}

void eudaq::WaveformSignalRegions::Print(std::ostream& out) const {
    for (auto i: this->regions)
        out<<"\t"<<i<<"\n";
}

void eudaq::WaveformSignalRegions::CalculateIntegrals(const StandardWaveform* wf){
    for (size_t i=0; i<this->regions.size();i++){
        regions[i].calculateIntegrals(wf);
//        std::cout<<"calculated: "<<regions[i]<<std::endl;
    }
}

void eudaq::WaveformSignalRegions::Reset(){
    for (auto i: this->regions)
            i.ResetIntegrals();
}
