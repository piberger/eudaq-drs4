/*
 * WaveformHistos.hh
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#ifndef WaveformHISTOS_HH_
#define WaveformHISTOS_HH_
//ROOT
//#include <TGraph.h>
#include <TH1F.h>
#include <TFile.h>
#include <TString.h>
#include <TGraph.h>
//std
#include <map>
#include <algorithm>    // std::min_element, std::max_element
#include <cstdlib>
//eudaq
#include "SimpleStandardEvent.hh"
#include "TGraphSet.hh"


class RootMonitor;

class WaveformHistos {
  protected:
    std::string _sensor;
    int _id;
    bool _wait;
    std::vector<TGraph*> _Waveforms;
    int _n_wfs;
    TH1F* h_minVoltage;
    TH1F* h_maxVoltage;
    TH1F* h_deltaVoltage;
    TH1F* h_FullIntegral;
  public:
    WaveformHistos(SimpleStandardWaveform p, RootMonitor * mon);
    virtual ~WaveformHistos(){}
    void Fill(const SimpleStandardWaveform & wf);
    void Reset();

    void Calculate(const int currentEventNum);
    void Write();
    unsigned GetNWaveforms() const {return _n_wfs;};
    TGraph * getWaveformGraph(int i) { return _Waveforms[i%_n_wfs]; }
    void setRootMonitor(RootMonitor *mon)  {_mon = mon; };
    TH1F* getDeltaVoltageHisto() const { return h_deltaVoltage;};
    TH1F* getMinVoltageHisto() const { return h_minVoltage;};
    TH1F* getMaxVoltageHisto() const { return h_maxVoltage;};
    TH1F* getFullIntegralVoltageHisto() const { return h_FullIntegral;};

  private:
    unsigned int n_fills;
    void InitHistos();
    int SetHistoAxisLabelx(TH1* histo,std::string xlabel);
    int SetHistoAxisLabely(TH1* histo,std::string ylabel);
    int SetHistoAxisLabels(TH1* histo,std::string xlabel, std::string ylabel);
    RootMonitor * _mon;
};

#ifdef __CINT__
#pragma link C++ class WaveformHistos-;
#endif

#endif /* WaveformHISTOS_HH_ */

