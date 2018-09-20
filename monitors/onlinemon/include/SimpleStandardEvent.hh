#ifndef SIMPLE_STANDARD_EVENT_H
#define SIMPLE_STANDARD_EVENT_H

#ifdef __CINT__
#undef __GNUC__
typedef unsigned short int __char16_t;
typedef unsigned  int __char32_t;
typedef unsigned long long int uint64_t; 
typedef char __signed;
typedef char int8_t;
#endif

#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>

#if ((defined WIN32) && (defined __CINT__))
typedef unsigned long long uint64_t
#else
#ifdef WIN32
#include <cstdint>
#endif
#endif

#include "include/SimpleStandardPlane.hh"
#include "include/SimpleStandardWaveform.hh"
#include "include/SimpleStandardTUEvent.hh"

inline bool operator==(SimpleStandardPlane const &a, SimpleStandardPlane const &b) {
  return (a.getName()==b.getName() && a.getID() == b.getID());
}

inline bool operator<(SimpleStandardPlane const &a, SimpleStandardPlane const &b) { // Needed to use struct in a map
  return a.getName()<b.getName() || ( a.getName()==b.getName() && a.getID() < b.getID());
}

inline bool operator==(SimpleStandardWaveform const &a, SimpleStandardWaveform const &b) {
  return (a.getName()==b.getName() && a.getID() == b.getID());
}

inline bool operator<(SimpleStandardWaveform const &a, SimpleStandardWaveform const &b) {
  return a.getName()<b.getName() || ( a.getName()==b.getName() && a.getID() < b.getID());
}


class SimpleStandardEvent {
  protected:
    //int _nr;
    std::vector<SimpleStandardPlane> _planes;
    std::vector<SimpleStandardWaveform> _waveforms;
    std::vector<SimpleStandardTUEvent> _tuev;

  public:
    SimpleStandardEvent();

    void addPlane(SimpleStandardPlane &plane);
    void addWaveform(SimpleStandardWaveform &wf);
    void addTUEvent(SimpleStandardTUEvent &tuev);
    SimpleStandardPlane getPlane (const int i) const {return _planes.at(i);}
    SimpleStandardWaveform getWaveform (const int i) const {return _waveforms.at(i);}
    SimpleStandardTUEvent getTUEvent(const int i) const {return _tuev.at(i);}
    int getNPlanes() const {return _planes.size(); }
    int getNWaveforms() const {return _waveforms.size();}
    int getNTUEvent() const {return _tuev.size();}

    void doClustering();
    double getMonitor_eventanalysistime() const;
    double getMonitor_eventfilltime() const;
    double getMonitor_clusteringtime() const;
    double getMonitor_correlationtime() const;
    void setMonitor_eventanalysistime(double monitor_eventanalysistime);
    void setMonitor_eventfilltime(double monitor_eventfilltime);
    void setMonitor_eventclusteringtime(double monitor_eventclusteringtime);
    void setMonitor_eventcorrelationtime(double monitor_eventcorrelationtime);

    unsigned int getEvent_number() const;
    void setEvent_number(unsigned int event_number);
    uint64_t getEvent_timestamp() const;
    void setEvent_timestamp(uint64_t event_timestamp);
  private:
    double monitor_eventfilltime; //stores the time to fill the histogram
    double monitor_eventanalysistime;
    double monitor_clusteringtime; //stores the time to fill the histogram
    double monitor_correlationtime;
    unsigned int event_number;
    uint64_t event_timestamp;
};





#endif
