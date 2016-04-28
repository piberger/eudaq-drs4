#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Utils.hh"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>


namespace eudaq{

//event type for which the converter will be used
static const char* EVENT_TYPE = "VX1742";


class VX1742ConverterPlugin:public DataConverterPlugin {

public:
  virtual void Initialize(const Event & bore, const Configuration & cnf) {
	//std::cout<<"VX1742 Initialize"<<std::endl;
	//m_serialno = bore.GetTag("VX1742_serial_no", (int)-1);
	//m_firmware = bore.GetTag("VX1742_firmware_v", (int)-1);
	//m_timestamp = bore.GetTag("VX1742_timestamp", (int)-1);
	m_range = 0.5; //TODO

}

  virtual bool GetStandardSubEvent(StandardEvent & sev, const Event & ev) const {
	const RawDataEvent &in_raw = dynamic_cast<const RawDataEvent &>(ev);
	int nblocks = in_raw.NumBlocks();
	//get data:
	int id = 0;
	RawDataEvent::data_t data = in_raw.GetBlock(id);
	u_int event_size = static_cast<u_int>(data[0]);
	id++;

	data = in_raw.GetBlock(id);
	u_int group_mask = static_cast<u_int>(data[0]);
	id++;

	data = in_raw.GetBlock(id);
	u_int n_channels = static_cast<u_int>(data[0]);
	id++;

	data = in_raw.GetBlock(id);
	u_int samples_per_channel = static_cast<u_int>(data[0]);
	id++;

	data = in_raw.GetBlock(id);
	u_int trigger_time_tag = static_cast<u_int>(data[0]);
	id++;


	for(u_int ch = 0; ch < 8; ch++){ //FIXME!!!!
	  data = in_raw.GetBlock(id); // not there

	  int data_size = data.size(); 
	  samples_per_channel =  data_size/sizeof(uint16_t);
	  uint16_t wave_array[samples_per_channel];
	  uint16_t *raw_wave_array = (uint16_t*)(&data[0]);

	  for (int i = 0; i < samples_per_channel; i++){
	  	wave_array[i] = (uint16_t)(raw_wave_array[i]); //fixme: ranges etc
	  	//std::cout << wave_array[i] << std::endl;
	  	if(wave_array[i] == 0){
	  		std::cout << "data is zero at channel " << ch << "at sample " << i << std::endl;
	  	}
	  }
	  StandardWaveform wf(ch, EVENT_TYPE, " VX1742 CH" + std::to_string(ch));
	  wf.SetChannelName("CH" + std::to_string(ch));
	  wf.SetChannelNumber(ch);
	  wf.SetNSamples(samples_per_channel);
	  wf.SetWaveform((uint16_t*) wave_array); //cast (uint16_t*)  war drinnen
	  //wf.SetTimeStamp(timestamp);
	  //wf.SetTriggerCell(trigger_cell);
	  sev.AddWaveform(wf);
	  id++;
	}//end ch loop

	return true;
}




private:
  VX1742ConverterPlugin():DataConverterPlugin(EVENT_TYPE), m_serialno(-1), m_firmware(-1), m_range(0){

}

  uint64_t m_timestamp;
  uint32_t m_serialno;
  float m_firmware, m_range;
  static VX1742ConverterPlugin m_instance;

}; // class VX1742ConverterPlugin

VX1742ConverterPlugin VX1742ConverterPlugin::m_instance;
} // namespace eudaq
