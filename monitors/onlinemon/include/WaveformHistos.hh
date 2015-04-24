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
#include <TH2F.h>
#include <TFile.h>
#include <TString.h>
#include <THStack.h>
#include <TGraph.h>
//std
#include <map>
#include <algorithm>    // std::min_element, std::max_element
#include <cstdlib>
//eudaq
#include "SimpleStandardEvent.hh"
#include "TGraphSet.hh"
#include "WaveformOptions.hh"


class RootMonitor;

class WaveformHistos {
  protected:
    std::string _sensor;
    int _id;
    bool _wait;
    std::vector<TH1F*> _Waveforms;
    THStack* h_wf_stack;
    int _n_wfs;
    unsigned int _n_samples;
    std::map<std::string, TH1*> profiles;
    std::map<std::string, TH1*> histos;
    std::map<std::string, std::pair<float,float> > rangesX;
    std::map<std::string, std::pair<float,float> > rangesY;
  public:
    WaveformHistos(SimpleStandardWaveform p, RootMonitor * mon);
    std::string getName() const {return (std::string)TString::Format("%s_%d",_sensor.c_str(),_id);};
    virtual ~WaveformHistos(){}
    void Fill(const SimpleStandardWaveform & wf);
    unsigned int getNSamples() const {return _n_samples;}
    void Reset();
    void SetOptions(WaveformOptions* options){std::cout<<"Setting Options for "<<getName()<<std::endl;};

    void Calculate(const int currentEventNum);
    void Write();
    unsigned GetNWaveforms() const {return _n_wfs;};
    TH1F * getWaveformGraph(int i) { return _Waveforms[i%_n_wfs]; }
    THStack* getWaveformStack(){return h_wf_stack;}
    void setRootMonitor(RootMonitor *mon)  {_mon = mon; };
    TH1F* getDeltaVoltageHisto() const { return (TH1F*)histos.at("DeltaVoltage");};
    TH1F* getMinVoltageHisto() const { return (TH1F*)histos.at("MinVoltage");};
    TH1F* getMaxVoltageHisto() const { return (TH1F*)histos.at("MaxVoltage");};
    TH1F* getFullIntegralVoltageHisto() const { return (TH1F*)histos.at("FullIntegral");};
    TH1F* getSignalIntegralVoltageHisto() const{ return (TH1F*)histos.at("SignalIntegral");};
    TH1F* getPedestalIntegralVoltageHisto() const { return (TH1F*)histos.at("PedestalIntegral");};
    TH1F* getDeltaIntegralVoltageHisto() const { return (TH1F*)histos.at("DeltaIntegral");};
    TProfile* getProfileDeltaVoltage() const { return (TProfile*)profiles.at("DeltaVoltage");};
    TProfile* getProfileDeltaIntegral() const { return (TProfile*)profiles.at("DeltaIntegral");};
    TProfile* getProfileSignalIntegral() const { return (TProfile*)profiles.at("SignalIntegral");};
    TProfile* getProfilePedestalIntegral() const { return (TProfile*)profiles.at("PedestalIntegral");};
    void SetMaxRangeX(std::string,float minx, float maxx);
    void SetMaxRangeY(std::string,float min, float max);
    void SetPedestalIntegralRange(float min, float max);
    void SetSignalIntegralRange(float min, float max);

  private:
    std::pair<float,float> pedestal_integral_range;
    std::pair<float,float> signal_integral_range;
    unsigned int n_fills;
    void InitHistos();
    void Reinitialize_Waveforms();
    void UpdateRanges();
    void UpdateRange(TH1* histo);
    int SetHistoAxisLabelx(TH1* histo,std::string xlabel);
    int SetHistoAxisLabely(TH1* histo,std::string ylabel);
    int SetHistoAxisLabels(TH1* histo,std::string xlabel, std::string ylabel);
    RootMonitor * _mon;
    float min_wf;
    float max_wf;
};

#ifdef __CINT__
#pragma link C++ class WaveformHistos-;
#endif


#endif /* WaveformHISTOS_HH_ */

