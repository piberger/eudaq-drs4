#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Utils.hh"
#include <string.h>
#include <stdio.h>


namespace eudaq{

//event type for which the converter will be used
static const char* EVENT_TYPE = "V1730";


class V1730ConverterPlugin:public DataConverterPlugin {


public:
  virtual void Initialize(const Event & bore, const Configuration & cnf) {
	std::cout<<"V1730 Initialize"<<std::endl;}

  virtual bool GetStandardSubEvent(StandardEvent & sev, const Event & ev) const {
	std::cout << "\nV1730: GetStandardSubEvent" << std::endl;
	
	const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
	int nblocks = in_raw.NumBlocks();
	std::cout << "Number of Blocks: " << nblocks << std::endl;

	int id = 0;
	int ch = 0;
	RawDataEvent::data_t data = in_raw.GetBlock(id);


	//get waveform:
	int data_size = data.size(); //size in 32bit words
	std::cout << "Size of data (waveform): " << data_size << std::endl;
	int n_samples =  (data_size-4)*2; //2 samples per 32bit word, -4 wegen header
	std::cout << "Number of samples in waveform: " << n_samples << std::endl;



	uint16_t *raw_wave_array = (uint16_t*) &data[0];
	uint16_t wave_array[n_samples];

	for (int i = 0; i < n_samples; i++)
	  wave_array[i] = (raw_wave_array[i]);

	//for(int i=0; i<n_samples;i++){
	//  std::cout << "data @" << i << ": " << data[i] << std::endl;
	//  }



	StandardWaveform wf(ch, EVENT_TYPE, "test");
	wf.SetChannelName("CH0");
	wf.SetChannelNumber(ch);
	wf.SetNSamples(n_samples);
	wf.SetWaveform((float*) wave_array);
	sev.AddWaveform(wf);

	return true;}







private:
  V1730ConverterPlugin() : DataConverterPlugin(EVENT_TYPE), m_serialno(-1), m_firmware(-1){
	std::cout<<"V1730ConverterPlugin Constructor"<<std::endl;}

  uint32_t m_serialno;
  float m_firmware;
  static V1730ConverterPlugin m_instance;

}; // class V1730ConverterPlugin

V1730ConverterPlugin V1730ConverterPlugin::m_instance;
} // namespace eudaq
