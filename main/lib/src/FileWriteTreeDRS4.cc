#ifdef ROOT_FOUND
 
#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/PluginManager.hh"
#include "eudaq/Logger.hh"
#include "eudaq/FileSerializer.hh"


#include "include/SimpleStandardEvent.hh"

//# include<inttypes.h>
# include "TFile.h"
# include "TTree.h"
# include "TRandom.h"
# include "TString.h"

using namespace std;

template <typename T>
inline T unpack_fh (vector <unsigned char >::iterator &src, T& data){ //unpack from host-byte-order
  data=0;
  for(unsigned int i=0;i<sizeof(T);i++){
    data+=((uint64_t)*src<<(8*i));
    src++;
  }
  return data;
}

template <typename T>
inline T unpack_fn(vector<unsigned char>::iterator &src, T& data){            //unpack from network-byte-order
  data=0;
  for(unsigned int i=0;i<sizeof(T);i++){
    data+=((uint64_t)*src<<(8*(sizeof(T)-1-i)));
    src++;
  }
  return data;
}

template <typename T>
inline T unpack_b(vector<unsigned char>::iterator &src, T& data, unsigned int nb){            //unpack number of bytes n-b-o only
  data=0;
  for(unsigned int i=0;i<nb;i++){
    data+=(uint64_t(*src)<<(8*(nb-1-i)));
    src++;
  }
  return data;
}

typedef pair<vector<unsigned char>::iterator,unsigned int> datablock_t;


namespace eudaq {

  class FileWriterTreeDRS4 : public FileWriter {
  public:
    FileWriterTreeDRS4(const std::string &);
    virtual void StartRun(unsigned);
    virtual void WriteEvent(const DetectorEvent &);
    virtual uint64_t FileBytes() const;
    float Calculate(std::vector<float> * data, int min, int max);
    float CalculatePeak(std::vector<float> * data, int min, int max);
    virtual ~FileWriterTreeDRS4();
  private:
    TFile * m_tfile; // book the pointer to a file (to store the otuput)
    TTree * m_ttree; // book the tree (to store the needed event info)
    // Book variables for the Event_to_TTree conversion
    unsigned m_noe;
    short chan;
    int n_pixels;
    int n_event;

    // Scalar Branches     
    int f_event_number;
    float f_time;
    float f_nwfs;
    float f_ped;
    float f_sig;
    float f_adc;
    float f_charge;  

    // Vector Branches     
    std::vector<float> * f_wf0;
    std::vector< std::vector<float>> * f_waveforms;
    
  };

  namespace {
    static RegisterFileWriter<FileWriterTreeDRS4> reg("drs4tree");
  }

  FileWriterTreeDRS4::FileWriterTreeDRS4(const std::string & /*param*/)
    : m_tfile(0), m_ttree(0),m_noe(0),chan(4),n_pixels(90*90+60*60)
  {

    n_event = -1;
    f_nwfs    = 0.;	  
    f_ped    = 0.;	  
    f_sig    = 0.;	  
    f_adc    = 0.;	  
    f_charge = 0.;  

    f_wf0 = new std::vector<float>;
    f_waveforms = new std::vector< std::vector<float> >;
  }

  float FileWriterTreeDRS4::Calculate(std::vector<float> * data, int min, int max) {
    float integral = 0;
    int i;
    for (i = min; i <= int(max+1) && i < data->size() ;i++){
        integral += data->at(i);
    }
    return integral/(float)(i-(int)min);
  }
  float FileWriterTreeDRS4::CalculateSig(std::vector<float> * data, int min, int max) {
    int mid = (int)( (max + min)/2 );
    int iext = mid;
    int n = 1;
    while( (data->at(mid-1)/data->at(mid) -1)*(data->at(mid+1)/data->at(mid) -1) < 0) {
      mid += (-2)**n;
      n++;
    }
  }

  void FileWriterTreeDRS4::StartRun(unsigned runnumber) {
    EUDAQ_INFO("Converting the inputfile into a DRS4 TTree " );
    std::string foutput(FileNamer(m_filepattern).Set('X', "_drs4.root").Set('R', runnumber));
    EUDAQ_INFO("Preparing the outputfile: " + foutput);
    m_tfile = new TFile(foutput.c_str(), "RECREATE");
    m_ttree = new TTree("tree", "a simple Tree with simple variables");

    // Set Branch Addresses
    m_ttree->Branch("event_number",&f_event_number, "event_number/I");

    m_ttree->Branch("time",&f_time, "time/F");

    m_ttree->Branch("nwfs", &f_nwfs,"n_waveforms/I");
    m_ttree->Branch("ped", &f_ped, "pedestal/F");
    m_ttree->Branch("sig", &f_sig, "signal/F");
    m_ttree->Branch("adc", &f_adc, "signal/F");
    m_ttree->Branch("charge", &f_charge, "charge/F");
    m_ttree->Branch("wf0", &f_wf0);

  }

  void FileWriterTreeDRS4::WriteEvent(const DetectorEvent & ev) {

    if (ev.IsBORE()) {
      eudaq::PluginManager::Initialize(ev);
      //firstEvent =true;
      return;
    } else if (ev.IsEORE()) {
      return;
    }

    StandardEvent sev = eudaq::PluginManager::ConvertToStandard(ev);

    f_event_number = sev.GetEventNumber();
    f_time = sev.GetTimestamp()/384066.;

    unsigned int nwf = (unsigned int) sev.NumWaveforms();
    // assuming the DRS4 waveform is the first
    // int len_drs4_wf = sev.GetWaveform(0);

    f_wf0->clear();

    // std::cout << "number of wf in my event" << nwf << std::endl;

    // SimpleStandardEvent simpEv;
    // simpEv.setEvent_number(ev.GetEventNumber());
    // simpEv.setEvent_timestamp(ev.GetTimestamp());

    //initialize with default configuration
//    mon_configdata.SetDefaults();
    // print the configuration
//    mon_configdata.PrintConfiguration();


    std::vector<float> * data;
    


    // assuming exactly one waveform for (unsigned int i = 0; i < nwf;i++){
        const eudaq::StandardWaveform & waveform = sev.GetWaveform(0);
            std::string sensorname;
            sensorname = waveform.GetType();
            // SimpleStandardWaveform simpWaveform(sensorname,waveform.ID());
            // simpWaveform.setNSamples(waveform.GetNSamples());
            // std::cout << "number of samples in my wf" << waveform.GetNSamples() << std::endl;
            data = waveform.GetData();
            // std::cout << "this is data " << data << endl;
            // std::cout << "this is data->at(3) " << data->at(3) << endl;
            
            float sig = Calculate(data, 1075, 1150);
            float ped = Calculate(data,    1,  900);
            f_sig= sig;
            f_ped= ped;

            f_wf0 = data;

            data->clear();

            // simpWaveform.addData(&(*waveform.GetData())[0]);
            // // simpWaveform.Calculate();
            // simpWaveform.setTimestamp(ev.GetTimestamp());
            // simpWaveform.setEvent(ev.GetEventNumber());
            // simpWaveform.setChannelName(waveform.GetChannelName());
            // simpWaveform.setChannelNumber(waveform.GetChannelNumber());
            // // simpEv.addWaveform(simpWaveform);


    // for (size_t iplane = 0; iplane < sev.NumPlanes(); ++iplane) {
    //   
    //   const eudaq::StandardPlane & plane = sev.GetPlane(iplane);
    //   std::vector<double> cds = plane.GetPixels<double>();
    //   
    //   for (size_t ipix = 0; ipix < cds.size(); ++ipix) {
	// 
	// f_plane->push_back(iplane);
	// f_col->push_back(plane.GetX(ipix));
	// f_row->push_back(plane.GetY(ipix));
	// f_adc->push_back((int)plane.GetPixel(ipix));
	// f_charge->push_back(42);	
    //   }
    // }

    m_ttree->Fill();
    
  }


  FileWriterTreeDRS4::~FileWriterTreeDRS4() {
    std::cout<<"Tree has " << m_ttree->GetEntries() << " entries" << std::endl;
    m_ttree->Write();
    if(m_tfile->IsOpen()) m_tfile->Close();
  }

  uint64_t FileWriterTreeDRS4::FileBytes() const { return 0; }

}
#endif // ROOT_FOUND
