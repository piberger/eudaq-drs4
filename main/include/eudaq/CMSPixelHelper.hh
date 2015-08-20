#include "dictionaries.h"
#include "constants.h"
#include "datasource_evt.h"
#include "Utils.hh"
#include <exception>

#if USE_LCIO
#  include "IMPL/LCEventImpl.h"
#  include "IMPL/TrackerRawDataImpl.h"
#  include "IMPL/TrackerDataImpl.h"
#  include "IMPL/LCCollectionVec.h"
#  include "IMPL/LCGenericObjectImpl.h"
#  include "UTIL/CellIDEncoder.h"
#  include "lcio.h"
#endif

#if USE_EUTELESCOPE
#  include "EUTELESCOPE.h"
#  include "EUTelSetupDescription.h"
#  include "EUTelEventImpl.h"
#  include "EUTelTrackerDataInterfacerImpl.h"
#  include "EUTelGenericSparsePixel.h"
#  include "EUTelRunHeaderImpl.h"

#include "EUTelCMSPixelDetector.h"
using eutelescope::EUTELESCOPE;
#endif

#include <TMath.h>
#include <TString.h>
#include <TF1.h>

using namespace pxar;

namespace eudaq {

    struct VCALDict {
        int row;
        int col;
        float par0;
        float par1;
        float par2;
        float par3;
        float calibration_factor;
    };




  class CMSPixelHelper {
  public:
    std::map<std::string, float >roc_calibrations = {{"psi46v2",65},{"psi46digv21respin",47}};
    CMSPixelHelper(std::string event_type) : m_event_type(event_type) {};
    std::map< std::string, VCALDict> vcal_vals;
    TF1 * fFitfunction;
    void initializeFitfunction(){fFitfunction = new TF1("fitfunc", "[3]*(TMath::Erf((x-[0])/[1])+[2])",-4096,4096);}
    float getCharge(VCALDict d, float val,float factor = 65.) const{  
        fFitfunction->SetParameter(0, d.par0);
        fFitfunction->SetParameter(1, d.par1);
        fFitfunction->SetParameter(2, d.par2);
        fFitfunction->SetParameter(3, d.par3);

        float charge = d.calibration_factor * fFitfunction->GetX(val);
        //std::cout<<"get Charge: "<<val<<" "<<d.par0<<" "<<d.par1<<" "<<d.par2<<" "<<d.par3<<" "<<charge<<std::endl;
        return charge;
    }
    void Initialize(const Event & bore, const Configuration & cnf) {
      DeviceDictionary* devDict;
      std::string roctype = bore.GetTag("ROCTYPE", "");
      std::string tbmtype = bore.GetTag("TBMTYPE", "tbmemulator");

      m_detector = bore.GetTag("DETECTOR","");
      std::string pcbtype = bore.GetTag("PCBTYPE", "");
      m_rotated_pcb = (pcbtype.find("-rot") != std::string::npos ? true : false);

      // Get the number of planes:
      m_nplanes = bore.GetTag("PLANES", 1);

      m_roctype = devDict->getInstance()->getDevCode(roctype);
      m_tbmtype = devDict->getInstance()->getDevCode(tbmtype);

      if (m_roctype == 0x0)
	      EUDAQ_ERROR("Roctype" + to_string((int) m_roctype) + " not propagated correctly to CMSPixelConverterPlugin");
      read_PHCalibrationData(cnf);
      initializeFitfunction();

      std::cout<<"CMSPixel Converter initialized with detector " << m_detector << ", Event Type " << m_event_type 
	       << ", TBM type " << tbmtype << " (" << static_cast<int>(m_tbmtype) << ")"
	       << ", ROC type " << roctype << " (" << static_cast<int>(m_roctype) << ")" << std::endl;
    }

    void read_PHCalibrationData(const Configuration & cnf){
        std::cout<<"REAAD PH CALIBRATION"<<std::endl;
        for (auto i: cnf.GetSections()){
            if (i.find("Producer.")==-1)
                continue;
            cnf.SetSection(i);
            std::string roctype = cnf.Get("roctype","roctype","");
            if (roctype == "")
                continue;
            bool is_digital = true;
            if (roctype.find("dig")==-1)
                is_digital = false;

            std::string fname = cnf.Get("phCalibrationFile","");
            if (fname == ""){
                fname = cnf.Get("dacFile","");
                std::size_t found = fname.find_last_of("/");
                if (is_digital)
                    fname = fname.substr(0,found) + (std::string)"/phCalibrationFitErr";
                else
                    fname = fname.substr(0,found) + (std::string)"/phCalibrationGErfFit";
            }
            std::string i2c = cnf.Get("i2c","i2caddresses","0");
            if (i.find("REF") != -1)
                read_PH_CalibrationFile("REF",fname,i2c,roc_calibrations[roctype]);
            else if (i.find("ANA") != -1)
                read_PH_CalibrationFile("ANA",fname,i2c,roc_calibrations[roctype]);
            else if (i.find("DIG") != -1)
                read_PH_CalibrationFile("DIG",fname,i2c,roc_calibrations[roctype]);
            else if (i.find("TRP") != -1)
                read_PH_CalibrationFile("TRP",fname,i2c,roc_calibrations[roctype]);
            else
                read_PH_CalibrationFile("DUT",fname,i2c,roc_calibrations[roctype]);
        }
        char t;
        std::cin>>t;
    }

    void read_PH_CalibrationFile(std::string roc_type,std::string fname, std::string i2cs,float factor){
      std::vector<std::string> vec_i2c = split(i2cs," ");
      int nRocs  = vec_i2c.size();

      // getting vcal-charge translation from file
      //
      float par0, par1, par2, par3;
      int row, col;
      std::string dump;
      char trash[30];
      for (int iroc = 0; iroc<nRocs; iroc++){
        std::string i2c = vec_i2c.at(iroc);
        FILE * fp;
        char *line = NULL;
        size_t len= 0;
        ssize_t read;
        
        TString filename = fname;
        filename+=(std::string)"_C"+(std::string)i2c+(std::string)".dat";
        std::cout<<filename<< " with " << factor <<" e/ADC" << std::endl;
        fp = fopen (filename,"r");//String::Format("/home/testbeam/sdvlp/TrackingTelescope/Calibrations/telescope9/phCalibrationGErfFit_C%d.dat", iroc), "r");
        //std::cout << "this is the file: " << fp << std::endl;

        VCALDict tmp_vcaldict;
        if (!fp) {
          std::cout <<  " DID NOT FIND A FILE TO GO FROM ADC TO CHARGE!!!!" << std::endl;}
        else{
          std::cout <<  " FILLING THE VCAL - ADC TRANSLATION FACTORS!!!!" << std::endl;
          read = getline(&line,&len,fp);
          read = getline(&line,&len,fp);
          read = getline(&line,&len,fp);

          int q = 0;
          while (fscanf(fp, "%f %f %f %f %s %d %d", &par0, &par1, &par2, &par3, trash,&col, &row) == 7){
              // std::cout << "par0: " << par0 << "  par1: " << par1 << " row and col " << row << " " << col << std::endl;
              tmp_vcaldict.par0 = par0;
              tmp_vcaldict.par1 = par1;
              tmp_vcaldict.par2 = par2;
              tmp_vcaldict.par3 = par3;
              tmp_vcaldict.calibration_factor = factor;
              std::string identifier = roc_type + std::string( TString::Format("%01d%02d%02d",iroc,row,col));
              //if (q==0) std::cout<<"IDENTIFIER: "<<identifier<<std::endl;
              q++;
              vcal_vals[identifier] = tmp_vcaldict;
          }
          std::cout<<"Read "<<q <<" Pixels for "<<roc_type<<i2c<<std::endl;
        }
        delete fp;
      }
    }


    bool GetStandardSubEvent(StandardEvent & out, const Event & in) const {

      // If we receive the EORE print the collected statistics:
      if (in.IsEORE()) {
	// Set decoder to INFO level for statistics printout:
	std::cout << "Decoding statistics for detector " << m_detector << std::endl;
	pxar::Log::ReportingLevel() = pxar::Log::FromString("INFO");
	decoding_stats.dump();
      }
          
      // Check if we have BORE or EORE:
      if (in.IsBORE() || in.IsEORE()) { return true; }

      // Check ROC type from event tags:
      if(m_roctype == 0x0){
	EUDAQ_ERROR("Invalid ROC type\n");
	return false;
      }

      const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(in);
      // Check of we have more than one data block:
      if(in_raw.NumBlocks() > 1) {
	EUDAQ_ERROR("Only one data block is expected!");
	return false;
      }

      unsigned plane_id;
      if(m_detector == "TRP") { plane_id = 6; }      // TRP
      else if(m_detector == "DUT") { plane_id = 7; } // DUT
      else { plane_id = 8; }                         // REF

      // Set decoder to reasonable verbosity (still informing about problems:
      pxar::Log::ReportingLevel() = pxar::Log::FromString("WARNING");
      //pxar::Log::ReportingLevel() = pxar::Log::FromString("DEBUGPIPES");

      // The pipeworks:
      evtSource src;
      passthroughSplitter splitter;
      dtbEventDecoder decoder;
      dataSink<pxar::Event*> Eventpump;
      pxar::Event* evt ;
      try{
          // Connect the data source and set up the pipe:
          src = evtSource(0, m_nplanes, m_tbmtype, m_roctype);
          src >> splitter >> decoder >> Eventpump;

          // Transform from EUDAQ data, add it to the datasource:
          src.AddData(TransformRawData(in_raw.GetBlock(0)));
          // ...and pull it out at the other end:
          evt = Eventpump.Get();
          decoding_stats += decoder.getStatistics();
      }
      catch (std::exception& e){
          EUDAQ_WARN("Decoding crashed");
//cout << e.what() << '\n';
          return false;
      }

      // Iterate over all planes and check for pixel hits:
      for(size_t roc = 0; roc < m_nplanes; roc++) {

	// We are using the event's "sensor" (m_detector) to distinguish DUT and REF:
	StandardPlane plane(plane_id + roc, m_event_type, m_detector);
	plane.SetTrigCount(evt->triggerCount());
	plane.SetTrigPhase(evt->triggerPhase());

	// Initialize the plane size (zero suppressed), set the number of pixels
	// Check which carrier PCB has been used and book planes accordingly:
	if(m_rotated_pcb) { plane.SetSizeZS(ROC_NUMROWS, ROC_NUMCOLS, 0); }
	else { plane.SetSizeZS(ROC_NUMCOLS, ROC_NUMROWS, 0); }
	plane.SetTLUEvent(0);

	// Store all decoded pixels belonging to this plane:
	for(std::vector<pxar::pixel>::iterator it = evt->pixels.begin(); it != evt->pixels.end(); ++it){
	  // Check if current pixel belongs on this plane:
	  if(it->roc() == roc) {
          float factor;
          if (m_detector == "DUT")
              factor = 65;
          else
              factor = 47.;
      std::string identifier = (std::string)m_detector+(std::string)TString::Format("%01zu%02d%02d",roc,it->row(),it->column());
      float charge = getCharge(vcal_vals.find(identifier)->second, it->value());
      if (charge < 0){
        EUDAQ_WARN(std::string("Invalid cluster charge -" + to_string(charge) +  "/" + to_string(it->value())));
        charge = 0;
      }

      //std::cout << "filling charge " <<it->value()<<" "<< charge << " "<<factor<<" "<<identifier<<std::endl;
	    if(m_rotated_pcb) { plane.PushPixel(it->row(), it->column(), charge /*it->value()*/); }
	    else { plane.PushPixel(it->column(), it->row(), charge /*it->value()*/); }
	  }
	}

	// Add plane to the output event:
	out.AddPlane(plane);
      }
      return true;
    }

    #if USE_LCIO && USE_EUTELESCOPE
    virtual void GetLCIORunHeader(lcio::LCRunHeader & header, eudaq::Event const & /*bore*/, eudaq::Configuration const & conf) const {
      eutelescope::EUTelRunHeaderImpl runHeader(&header);
      // Type of data: real.
      runHeader.setDAQHWName(EUTELESCOPE::DAQDATA);
    }

    virtual bool GetLCIOSubEvent(lcio::LCEvent & result, const Event & source) const {

      if(source.IsBORE()) {
	std::cout << "CMSPixelConverterPlugin::GetLCIOSubEvent BORE " << source << std::endl;
	return true;
      } else if(source.IsEORE()) {
	std::cout << "CMSPixelConverterPlugin::GetLCIOSubEvent EORE " << source << std::endl;
	return true;
      }

      // Set event type Data Event (kDE):
      result.parameters().setValue(eutelescope::EUTELESCOPE::EVENTTYPE, eutelescope::kDE);

      // Prepare the data collection and check for its existence:
      LCCollectionVec * zsDataCollection;
      bool zsDataCollectionExists = false;
      try {
	/// FIXME choose another name for the collection!
	zsDataCollection = static_cast<LCCollectionVec*>(result.getCollection(m_event_type));
	zsDataCollectionExists = true;
      } catch(lcio::DataNotAvailableException& e) {
	zsDataCollection = new LCCollectionVec(lcio::LCIO::TRACKERDATA);
      }

      // Set the proper cell encoder
      CellIDEncoder<TrackerDataImpl> zsDataEncoder(eutelescope::EUTELESCOPE::ZSDATADEFAULTENCODING, zsDataCollection);

      // Prepare a description of the setup
      std::vector<eutelescope::EUTelSetupDescription*> setupDescription;

      // Decode the raw data and retrieve the eudaq StandardEvent:
      StandardEvent tmp_evt;
      GetStandardSubEvent(tmp_evt, source);

      // Loop over all planes available in the data stream:
      for (size_t iPlane = 0; iPlane < tmp_evt.NumPlanes(); ++iPlane) {
	StandardPlane plane = static_cast<StandardPlane>(tmp_evt.GetPlane(iPlane));

	// The current detector is ...
	eutelescope::EUTelPixelDetector * currentDetector = 0x0;
	if(plane.Sensor() == "DUT" || plane.Sensor() == "REF" || plane.Sensor() == "TRP") {

	  currentDetector = new eutelescope::EUTelCMSPixelDetector;
	  // FIXME what is that mode used for?
	  std::string mode = "ZS";
	  currentDetector->setMode(mode);
	  if(result.getEventNumber() == 0) {
	    setupDescription.push_back(new eutelescope::EUTelSetupDescription(currentDetector));
	  }
	} else {
	  EUDAQ_ERROR("Unrecognised sensor type in LCIO converter: " + plane.Sensor());
	  return true;
	}

	zsDataEncoder["sensorID"] = plane.ID();
	zsDataEncoder["sparsePixelType"] = eutelescope::kEUTelGenericSparsePixel;

	// Get the total number of pixels
	size_t nPixel = plane.HitPixels();

	// Prepare a new TrackerData for the ZS data
	std::auto_ptr<lcio::TrackerDataImpl>zsFrame(new lcio::TrackerDataImpl);
	zsDataEncoder.setCellID(zsFrame.get());

	// This is the structure that will host the sparse pixels
	std::auto_ptr<eutelescope::EUTelTrackerDataInterfacerImpl<eutelescope::EUTelGenericSparsePixel> >
	  sparseFrame(new eutelescope::EUTelTrackerDataInterfacerImpl<eutelescope::EUTelGenericSparsePixel>(zsFrame.get()));

	// Prepare a sparse pixel to be added to the sparse data:
	std::auto_ptr<eutelescope::EUTelGenericSparsePixel> sparsePixel(new eutelescope::EUTelGenericSparsePixel);
	for(size_t iPixel = 0; iPixel < nPixel; ++iPixel) {

	  // Fill the sparse pixel coordinates with decoded data:
	  sparsePixel->setXCoord((size_t)plane.GetX(iPixel));
	  sparsePixel->setYCoord((size_t)plane.GetY(iPixel));
	  // Fill the pixel charge:

	  sparsePixel->setSignal((int32_t)plane.GetPixel(iPixel));
	  
	  // Add the pixel to the readout frame:
	  sparseFrame->addSparsePixel(sparsePixel.get());
	}

	// Now add the TrackerData to the collection
	zsDataCollection->push_back(zsFrame.release());
	delete currentDetector;

      } // loop over all planes

      // Add the collection to the event only if not empty and not yet there
      if(!zsDataCollectionExists){
	if(zsDataCollection->size() != 0) {
	  result.addCollection(zsDataCollection, m_event_type);
	} else {
	  delete zsDataCollection; // clean up if not storing the collection here
	}
      }

      return true;
    }
#endif

  private:
    uint8_t m_roctype, m_tbmtype;
    size_t m_planeid;
    size_t m_nplanes;
    std::string m_detector;
    bool m_rotated_pcb;
    std::string m_event_type;
    mutable pxar::statistics decoding_stats;

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
  };

}
