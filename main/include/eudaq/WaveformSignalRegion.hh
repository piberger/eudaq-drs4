/*
 * WaveformSignalRegion.h
 *
 *  Created on: Oct 13, 2015
 *      Author: bachmair
 */

#ifndef WAVEFORMSIGNALREGION_H_
#define WAVEFORMSIGNALREGION_H_

#include "StandardEvent.hh"
#include "WaveformIntegral.hh"
#include <limits>
#include <vector>
#include "TROOT.h"
namespace eudaq {

class WaveformSignalRegion:public TObject {
    public:
        WaveformSignalRegion(int low=-1, int high=-1, std::string name="");
        virtual ~WaveformSignalRegion();
        void calculateIntegrals(const StandardWaveform* wf);
        void ResetIntegrals();
        void Print() const {Print(std::cout);}
        void Print(std::ostream& out) const;
        void SetPolarity(unsigned char pol){polarity=pol;}
        void AddIntegral(WaveformIntegral integralDef){this->signals.push_back(integralDef);}
    private:
        std::string name;
        signed char polarity;
        int low_border;
        int high_border;
        int peak_pos;
        std::vector<WaveformIntegral> signals;
//        ClassDef(WaveformSignalRegion,1);
};

inline std::ostream & operator << (std::ostream & os, const WaveformSignalRegion & c){c.Print(os);return os;}
}
#endif /* WAVEFORMSIGNALREGION_H_ */
