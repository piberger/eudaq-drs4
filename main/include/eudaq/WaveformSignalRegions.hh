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
#include "WaveformSignalRegion.hh"

class WaveformSignalRegions:public TObject{
    public:
        WaveformSignalRegions(int channel=-1, signed char pol=0);
        virtual ~WaveformSignalRegions(){};
        void AddRegion(WaveformSignalRegion region);
        void Clear(){channel=-1;regions.clear();polarity=0;}
        void Print() const{Print(std::cout);}
        void Print(std::ostream& out) const;
//        void CalculateIntegrals(const StandardWaveform* wf);
        void Reset();
        size_t GetNRegions(){return regions.size();}
        signed char GetPolarity(){return polarity;}
        WaveformSignalRegion* GetRegion(UInt_t i);
    private:
        std::vector<WaveformSignalRegion> regions;
        int channel;
        signed char polarity;
//        ClassDef(WaveformSignalRegions,1);
};
inline std::ostream & operator << (std::ostream& os, const WaveformSignalRegions& regions) { regions.Print(os); return os;}

#endif /* WAVEFORMSIGNALREGIONS_HH_ */
