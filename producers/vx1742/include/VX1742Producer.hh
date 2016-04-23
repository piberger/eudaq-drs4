/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework
** 
**
** <VX1742Producer>.hh
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/


#ifndef VX1742PRODUCER_HH
#define VX1742PRODUCER_HH

// VME includes
#include "RCDVme/RCDVme.h"
#include "RCDVme/RCDCmemSegment.h"
#include "VX1742DEFS.hh"
#include "VX1742Interface.hh"


class Producer;
class Configuration;

class VX1742Producer: public eudaq::Producer{
public:
  VX1742Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity);
  void OnConfigure(const eudaq::Configuration & config);
  void OnStartRun(unsigned runnumber);
  void OnStopRun();
  void OnTerminate();
  void SetTimeStamp();
  void ReadoutLoop();

private:
  VX1742Interface *caen;
  eudaq::Configuration m_config;

  //config values
  u_int sampling_frequency;
  u_int post_trigger_samples;
  u_int trigger_source;
  u_int active_groups;
  u_int groups[4];
  u_int custom_size;




  std::string m_producerName, m_event_type;
  
  uint32_t m_serialno, m_active_channels, m_ev, m_trigger_source, m_post_trigger_samples;
  float m_firmware;
  uint64_t m_timestamp;
  int m_run;
  bool m_running, m_terminated;
  uint16_t m_trigger_threshold;
  std::map<uint8_t,float> m_dynamic_range;
  std::map<uint8_t,uint16_t> m_channel_gain;
  std::map<uint8_t,uint16_t> m_channel_dac;







};



int main(int /*argc*/, const char ** argv);


#endif
