#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Utils.hh"

// All LCIO-specific parts are put in conditional compilation blocks
// so that the other parts may still be used if LCIO is not available.
#if USE_LCIO
#  include "IMPL/LCEventImpl.h"
#  include "IMPL/TrackerRawDataImpl.h"
#  include "IMPL/LCCollectionVec.h"
#  include "lcio.h"
#endif

namespace eudaq {

// The event type for which this converter plugin will be registered
// Modify this to match your actual event type (from the Producer)
static const char* EVENT_TYPE = "DRS4";

// Declare a new class that inherits from DataConverterPlugin
class DRS4ConverterPlugin : public DataConverterPlugin {
private:
	int m_serial_no;
	int m_firmware;
	int m_board_type;
	int m_n_channels;
	std::string m_dut_name;
public:

	// This is called once at the beginning of each run.
	// You may extract information from the BORE and/or configuration
	// and store it in member variables to use during the decoding later.
	virtual void Initialize(const Event & bore,
			const Configuration & cnf) {
		std::cout<<"DRS4 Initialize"<<std::endl;
#ifndef WIN32  //some linux Stuff //$$change
		(void)cnf; // just to suppress a warning about unused parameter cnf
#endif
		m_serial_no = bore.GetTag("DRS4_serial_no", (int)-1);
		m_firmware = bore.GetTag("DRS4_FW",(int)-1);
		m_board_type = bore.GetTag("DRS4_Board_type", (int)-1);
		m_n_channels = bore.GetTag("DRS4_n_channels", (int)-1);
		std::cout<<"serial_no:  "<<m_serial_no<<std::endl;
		std::cout<<"firmware:   "<<m_firmware<<std::endl;
		std::cout<<"board_type: "<<m_board_type<<std::endl;
		std::cout<<"n_channels: "<<m_n_channels<<std::endl;
	}

	// Here, the data from the RawDataEvent is extracted into a StandardEvent.
	// The return value indicates whether the conversion was successful.
	// Again, this is just an example, adapted it for the actual data layout.
	virtual bool GetStandardSubEvent(StandardEvent & sev,
			const Event & ev) const {
//		std::cout<<"DRS4::GetStandardSubEvent"<<std::endl;
		const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
		int nblocks = in_raw.NumBlocks();
//		std::cout<<"Number of Blocks: "<<nblocks<<std::endl;
		// If the event type is used for different sensors
		// they can be differentiated here
		// Create a StandardPlane representing one sensor plane
		int id = 0;
		RawDataEvent::data_t trigger_cell_data = in_raw.GetBlock(0);
		int trigger_cell = static_cast<int>(trigger_cell_data[0]);
		float min_waves[m_n_channels];
		float max_waves[m_n_channels];
		std::cout<<nblocks<<" "<<m_n_channels<<std::endl;
		for (int ch = 0; ch < m_n_channels && ch < nblocks-1; ch++){
			StandardPlane plane(ch, EVENT_TYPE, m_dut_name);
			// Set the number of pixels
			int width = 1024, height = 1;
			plane.SetSizeRaw(width, height);
			RawDataEvent::data_t wave_array_data = in_raw.GetBlock(ch+1);
			int wave_size = wave_array_data.size();
			int n_samples =  wave_size/sizeof(float);
			std::cout<<"CH: "<<ch<<" with "<<trigger_cell_data.size()<<" "
					<<wave_array_data.size()<<" -> "<<n_samples<<"  .";//<<std::endl;
			std::cout<<"Trigger cell "<<trigger_cell<<", ";
			float * wave_array = (float*) &wave_array_data[0];
			min_waves[ch] = *std::min_element(wave_array,wave_array+n_samples);
			max_waves[ch] = *std::max_element(wave_array,wave_array+n_samples);
			std::cout<<"From: "<< min_waves[ch] << " mV to " << max_waves[ch] << " mV"<<std::endl;
			for (int sample = 0; sample < n_samples; sample++)
				plane.PushPixel(sample,0,wave_array[sample]);
			// Add the plane to the StandardEvent
			sev.AddPlane(plane);
			// Indicate that data was successfully converted
		}
		return true;
	}

#if USE_LCIO
	// This is where the conversion to LCIO is done
	virtual lcio::LCEvent * GetLCIOEvent(const Event * /*ev*/) const {
		return 0;
	}
#endif

private:
	static std::vector<uint16_t> TransformRawData(const std::vector<unsigned char> & block) {

		// Transform data of form char* to vector<int16_t>
		std::vector<uint16_t> rawData;

		int size = block.size();
		if(size < 2) { return rawData; }

		int i = 0;
		while(i < size-1) {
			uint16_t temp = ((uint16_t)block.data()[i+1] << 8) | block.data()[i];
			rawData.push_back(temp);
			i+=2;
		}
		return rawData;
	}
	// The constructor can be private, only one static instance is created
	// The DataConverterPlugin constructor must be passed the event type
	// in order to register this converter for the corresponding conversions
	// Member variables should also be initialized to default values here.
	DRS4ConverterPlugin(): DataConverterPlugin(EVENT_TYPE),
			m_serial_no(-1),
			m_firmware(-1),
			m_board_type(-1),
			m_n_channels(0),
			m_dut_name("")
	{
		std::cout<<"DRS4ConverterPlugin Constructor"<<std::endl;
	}


	// The single instance of this converter plugin
	static DRS4ConverterPlugin m_instance;

}; // class DRS4ConverterPlugin

// Instantiate the converter plugin instance
DRS4ConverterPlugin DRS4ConverterPlugin::m_instance;

} // namespace eudaq
