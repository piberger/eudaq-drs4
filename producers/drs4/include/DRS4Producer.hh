#ifndef DRS4PRODUCER_HH
#define DRS4PRODUCER_HH

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "strlcpy.h"
#include "DRS.h"

// EUDAQ includes:
#include "eudaq/Producer.hh"
#include "eudaq/Timer.hh"
#include "eudaq/Configuration.hh"

#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include<sys/time.h>
#include <vector>
#include <time.h>


class DRS4Producer: public eudaq::Producer  {
public:
	DRS4Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity);
	virtual void OnConfigure(const eudaq::Configuration & config);
	virtual void OnStartRun(unsigned runnumber);
	virtual void OnStopRun();
	virtual void OnTerminate();
	void ReadoutLoop();
	//  virtual ~DRS4Producer();
private:
	virtual void SendRawEvent();
	unsigned m_run, m_ev;
	std::string m_verbosity, m_producerNamem,m_event_type, m_producerName;
	bool m_terminated, m_running, triggering,m_self_triggering;;
	int m_n_self_trigger;
	unsigned NumOfChan, NumOfSil, NumOfADC;
	eudaq::Configuration m_config;
	eudaq::Timer* m_t;
	DRS *m_drs;
	int m_serialno;
	DRSBoard *m_b;
	float time_array[8][1024];
	unsigned short raw_wave_array[8][1024];
	float wave_array[8][1024];
	int n_channels;
	bool     m_chnOn[4];
};
int main(int /*argc*/, const char ** argv);
#endif /*DRS4PRODUCER_HH*/
