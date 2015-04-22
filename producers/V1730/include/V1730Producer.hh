#ifndef V1730PRODUCER_HH
#define V1730PRODUCER_HH


class Producer;
class Configuration;
class caen_v1730;
class VP717;
class vme_interface;


class V1730Producer: public eudaq::Producer{
public:
  V1730Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity);
  void OnConfigure(const eudaq::Configuration & config);
  void OnStartRun(unsigned runnumber);
  void OnStopRun();
  void OnTerminate();
  void SetTimeStamp();
  void ReadoutLoop();


private:
  VP717 *vp717_interface;
  vme_interface *interface;
  caen_v1730 *V1730_handle;


  std::string m_producerName, m_event_type;
  eudaq::Configuration m_config;
  unsigned int m_ev;
  uint32_t m_serialno, m_active_channels;
  float m_firmware;
  uint64_t m_timestamp;
  int m_run, m_trigger_source;
  bool m_running, m_terminated;

  //to be implemented: int n_channels;


};


int main(int /*argc*/, const char ** argv);


#endif 
