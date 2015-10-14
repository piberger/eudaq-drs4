/*
 * WaveformSignalRegions.hh
 *
 *  Created on: Oct 14, 2015
 *      Author: bachmair
 */

#ifndef WAVEFORMSIGNALREGIONS_HH_
#define WAVEFORMSIGNALREGIONS_HH_


#include "StandardEvent.hh"
#include "WaveformIntegral.hh"
#include <limits>
#include <vector>
#include "TROOT.h"
#include "WaveformSignalRegion.h"
namespace eudaq {

class WaveformSignalRegions:public TObject{
    public:
        WaveformSignalRegions(int channel=-1, signed char pol=0){this->channel=channel;polarity=pol;}
        virtual ~WaveformSignalRegions(){};
        void AddRegion(WaveformSignalRegion region);
        void Clear(){channel=-1;regions.clear();polarity=0;}
        void Print() const{Print(std::cout);}
        void Print(std::ostream& out) const;
        void CalculateIntegrals(const StandardWaveform* wf);
        void Reset();
    private:
        std::vector<WaveformSignalRegion> regions;
        int channel;
        unsigned char polarity;
//        ClassDef(WaveformSignalRegions,1);
};
inline std::ostream & operator << (std::ostream& os, const WaveformSignalRegions& region) { region.Print(os); return os;}
} /* namespace eudaq */

#endif /* WAVEFORMSIGNALREGIONS_HH_ */
