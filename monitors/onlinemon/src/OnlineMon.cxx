#ifdef WIN32
#include <Windows4Root.h>
#endif

//whoever wrote this file (especially the corresponding header file) should be beaten up
//number of wasted hours = 4 (increase counter)



// ROOT includes
#include "TROOT.h"
#include "TNamed.h"
#include "TApplication.h"
#include "TGClient.h"
#include "TGMenu.h"
#include "TGTab.h"
#include "TGButton.h"
#include "TGComboBox.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TRootEmbeddedCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TPaletteAxis.h"
#include "TThread.h"
#include "TFile.h"
#include "TColor.h"
#include "TString.h"
#include "TF1.h"
//#include "TSystem.h" // for TProcessEventTimer
// C++ INCLUDES
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define EUDAQ_SLEEP(x) Sleep(x*1000)
#else
#define EUDAQ_SLEEP(x) sleep(x)
#endif

#include "TUCollection.hh"


//ONLINE MONITOR Includes
#include "OnlineMon.hh"

using namespace std;

RootMonitor::RootMonitor(const std::string & runcontrol, const std::string & datafile, int /*x*/, int /*y*/, int /*w*/,
       int /*h*/, int argc, int offline, const unsigned lim, const unsigned skip_, const unsigned int skip_with_counter,
       const std::string & conffile):eudaq::Holder<int>(argc), eudaq::Monitor("OnlineMon", runcontrol, lim, skip_, skip_with_counter, datafile), _offline(offline), _planesInitialized(false), _fft_resets(0) {

  if (_offline <= 0){
    onlinemon = new OnlineMonWindow(gClient->GetRoot(),800,600);
    if (onlinemon==NULL){
      cerr<< "Error Allocationg OnlineMonWindow"<<endl;
      exit(-1);}
  }
/*
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetCanvasDefH(600); //Height of canvas
  gStyle->SetCanvasDefW(600); //Width of canvas
  gStyle->SetCanvasDefX(0);   //POsition on screen
  gStyle->SetCanvasDefY(0);

// For the Pad:
  gStyle->SetPadBorderMode(0);
  // gStyle->SetPadBorderSize(Width_t size = 1);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadGridX(false);
  gStyle->SetPadGridY(false);
  gStyle->SetGridColor(0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);

// For the frame:
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(1);
  gStyle->SetFrameFillColor(0);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetFrameLineColor(1);
  gStyle->SetFrameLineStyle(1);
  gStyle->SetFrameLineWidth(1);

  gStyle->SetEndErrorSize(2);
  //gStyle->SetErrorMarker(20);
  gStyle->SetErrorX(0.);

  gStyle->SetMarkerStyle(20);

//For the fit/function:
  gStyle->SetOptFit(1);
  gStyle->SetFitFormat("5.4g");
  gStyle->SetFuncColor(2);
  gStyle->SetFuncStyle(1);
  gStyle->SetFuncWidth(1);

//For the date:
  gStyle->SetOptDate(0);

// For the statistics box:
  gStyle->SetOptFile(0);
  //gStyle->SetOptStat(0);
  gStyle->SetOptStat("mr");
  gStyle->SetStatColor(kWhite);
  gStyle->SetStatFont(42);
  gStyle->SetStatFontSize(0.04);///---> gStyle->SetStatFontSize(0.025);
  gStyle->SetStatTextColor(1);
  gStyle->SetStatFormat("6.4g");
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatH(0.1);
  gStyle->SetStatW(0.2);///---> gStyle->SetStatW(0.15);

  // gStyle->SetStatStyle(Style_t style = 1001);
  // gStyle->SetStatX(Float_t x = 0);
  // gStyle->SetStatY(Float_t y = 0);

// Margins:
  gStyle->SetPadTopMargin(0.09);
  gStyle->SetPadBottomMargin(0.17);
  gStyle->SetPadLeftMargin(0.20);
  gStyle->SetPadRightMargin(0.04);

// For the Global title:

  gStyle->SetOptTitle(0);
  gStyle->SetTitleFont(42);
  gStyle->SetTitleColor(1);
  gStyle->SetTitleTextColor(1);
  gStyle->SetTitleFillColor(10);
  gStyle->SetTitleFontSize(0.05);
  // gStyle->SetTitleH(0); // Set the height of the title box
  // gStyle->SetTitleW(0); // Set the width of the title box
  // gStyle->SetTitleX(0); // Set the position of the title box
  // gStyle->SetTitleY(0.985); // Set the position of the title box
  // gStyle->SetTitleStyle(Style_t style = 1001);
  // gStyle->SetTitleBorderSize(2);

// For the axis titles:

  gStyle->SetTitleColor(1, "XYZ");
  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetTitleSize(0.06, "XYZ");
  // gStyle->SetTitleXSize(Float_t size = 0.02); // Another way to set the size?
  // gStyle->SetTitleYSize(Float_t size = 0.02);
  gStyle->SetTitleXOffset(0.9);
  gStyle->SetTitleYOffset(1.25);
  // gStyle->SetTitleOffset(1.1, "Y"); // Another way to set the Offset

// For the axis labels:

  gStyle->SetLabelColor(1, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetLabelOffset(0.007, "XYZ");
  gStyle->SetLabelSize(0.05, "XYZ");

// For the axis:

  gStyle->SetAxisColor(1, "XYZ");
  gStyle->SetStripDecimals(kTRUE);
  gStyle->SetTickLength(0.03, "XYZ");
  gStyle->SetNdivisions(510, "XYZ");
  gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
  gStyle->SetPadTickY(1);

// Change for log plots:
  gStyle->SetOptLogx(0);
  gStyle->SetOptLogy(0);
  gStyle->SetOptLogz(0);

// Postscript options:
  gStyle->SetPaperSize(20.,20.);
  // gStyle->SetLineScalePS(Float_t scale = 3);
  // gStyle->SetLineStyleString(Int_t i, const char* text);
  // gStyle->SetHeaderPS(const char* header);
  // gStyle->SetTitlePS(const char* pstitle);

  // gStyle->SetBarOffset(Float_t baroff = 0.5);
  // gStyle->SetBarWidth(Float_t barwidth = 0.5);
  // gStyle->SetPaintTextFormat(const char* format = "g");
  // gStyle->SetPalette(Int_t ncolors = 0, Int_t* colors = 0);
  // gStyle->SetTimeOffset(Double_t toffset);
  // gStyle->SetHistMinimumZero(kTRUE);

  gStyle->cd();
*/

  hmCollection = new HitmapCollection();
  corrCollection = new CorrelationCollection();
  wfCollection = new WaveformCollection();
  tuCollection = new TUCollection();
  MonitorPerformanceCollection *monCollection = new MonitorPerformanceCollection();
  eudaqCollection = new EUDAQMonitorCollection();


  // put collections into the vector
  _colls.push_back(hmCollection);
  _colls.push_back(corrCollection);
  _colls.push_back(wfCollection);
  _colls.push_back(tuCollection);
  _colls.push_back(monCollection);
  _colls.push_back(eudaqCollection);
  // set the root Monitor

  if (_offline <= 0) {
    hmCollection->setRootMonitor(this);
    corrCollection->setRootMonitor(this);
    monCollection->setRootMonitor(this);
    wfCollection->setRootMonitor(this);
    tuCollection->setRootMonitor(this);
    eudaqCollection->setRootMonitor(this);
    onlinemon->setCollections(_colls);
  }

  //initialize with default configuration
  mon_configdata.SetDefaults();
  configfilename.assign(conffile);

  cout << "Set OnlineMonitor Configfile: \""<<configfilename<<"\""<<endl;

  if (configfilename.length()>1){
    mon_configdata.setConfigurationFileName(configfilename);
    if (mon_configdata.ReadConfigurationFile()!=0){
      // reset defaults, as Config file is bad
      cerr <<" As Config file can't be found, re-applying hardcoded defaults"<<endl;
      mon_configdata.SetDefaults();}
  }


  // print the configuration
  mon_configdata.PrintConfiguration();


  cout << "Datafile: " << datafile << endl;
  if (datafile != "") {
    cout << "Calling program from file" << endl;
    size_t first = datafile.find_last_of("/")+1;
    //extract filename
    string filename=datafile.substr(first,datafile.length());
    char out[1024] = "";
    char num[1024] = "";

    if (string::npos!=filename.find_last_of(".raw"))
    {
      filename.copy(num,filename.length()-filename.find_last_of(".raw"),0);
    }
    else
    {
      filename.copy(num,filename.length(),0);
    }
    filename=filename+".root";
    filename.copy(out,filename.length(),0);
    int n=atoi(num);


    if (_offline <= 0)
    {
      onlinemon->setRunNumber(n);
    }

    cout << "ROOT output filename is: " << out << endl;
    if (_offline <= 0)
    {
      onlinemon->setRootFileName(out);
    }
    rootfilename = out;
    if (_offline > 0) {
      _checkEOF.setCollections(_colls);
      _checkEOF.setRootFileName(out);
      _checkEOF.startChecking(10000);
    }
  }

  cout << "End of Constructor" << endl;

  // construct the FFT stuff
  int n_samples = 1024;
  Int_t n_size = n_samples+1;
  fft_own = TVirtualFFT::FFT(1, &n_size, "R2C P K");
  if (!fft_own) {
      cout << "something went wrong with the fft creation" << endl;
      return;
  }


  //set a few defaults
  snapshotdir=mon_configdata.getSnapShotDir();
  previous_event_analysis_time=0;
  previous_event_fill_time=0;
  previous_event_clustering_time=0;
  previous_event_correlation_time=0;

  onlinemon->SetOnlineMon(this);

}


void RootMonitor::setReduce(const unsigned int red) {
  if (_offline <= 0) onlinemon->setReduce(red);
  for (unsigned int i = 0 ; i < _colls.size(); ++i)
  {
    _colls.at(i)->setReduce(red);
  }
}


void RootMonitor::OnEvent(const eudaq::StandardEvent & ev) {
  #ifdef DEBUG
    cout << "Called onEvent " << ev.GetEventNumber()<< endl;
    cout << "Number of Planes " << ev.NumPlanes()<< endl;
  #endif

  _checkEOF.EventReceived();

  if (ev.IsBORE()){
    std::cout << "This is a BORE" << std::endl;
  }

  if (ev.GetEventNumber()<10 && ev.GetRunNumber()!=0)
   this->onlinemon->setRunNumber(ev.GetRunNumber());
  
  if (ev.GetEventNumber() < this->start_event){
      return;}


  //start timing to measure processing time
  my_event_processing_time.Start(true);

  bool reduce=false; //do we use Event reduction
  bool skip_dodgy_event=false; // do we skip this event because we consider it dodgy

  if (_offline > 0){
    if (_offline < (int)ev.GetEventNumber()){
      TFile *f = new TFile(rootfilename.c_str(),"RECREATE");
      if (f!=NULL){
        for (unsigned int i = 0 ; i < _colls.size(); ++i){
          _colls.at(i)->Write(f);}
        f->Close();
      }else{
        cout<< "Can't open "<<rootfilename<<endl;}
      exit(0);
    }
    reduce = true;

  }else{
    reduce = (ev.GetEventNumber() % onlinemon->getReduce() == 0);}

  if (reduce){
    unsigned int num = (unsigned int) ev.NumPlanes();
    unsigned int nwf = (unsigned int) ev.NumWaveforms();
    unsigned int ntu = (unsigned int) ev.NumTUEvents();
    //std::cout << "Number of TU events: " << ntu << std::endl;

    // Initialize the geometry with the first event received:
    if(!_planesInitialized){
      myevent.setNPlanes(num);
      myevent.setNWaveforms(nwf);

      std::cout << "Initialized geometry. Nplanes: " << num <<", NWaveforms: "<< nwf << std::endl;
    
    }else {
      if (myevent.getNPlanes()!=num) {
        cout << "Plane Mismatch on " <<ev.GetEventNumber()<<endl;
        cout << "Current/Previous " <<num<<"/"<<myevent.getNPlanes()<<endl;
        skip_dodgy_event=true; //we may want to skip this FIXME
        ostringstream eudaq_warn_message;
        eudaq_warn_message << "Plane Mismatch in Event "<<ev.GetEventNumber() <<" "<<num<<"/"<<myevent.getNPlanes();
        EUDAQ_LOG(WARN,(eudaq_warn_message.str()).c_str());

      }else{
        myevent.setNPlanes(num);}

      if (myevent.getNWaveforms()!=nwf){
        cout << "Waveform Mismatch on " <<ev.GetEventNumber()<<endl;
        cout << "Current/Previous " <<nwf<<"/"<<myevent.getNWaveforms()<<endl;
        skip_dodgy_event=true; //we may want to skip this FIXME
        ostringstream eudaq_warn_message;
        eudaq_warn_message << "Waveform Mismatch in Event "<<ev.GetEventNumber() <<" "<<nwf<<"/"<<myevent.getNWaveforms()<< ", Updating NWaveforms";
        EUDAQ_LOG(WARN,(eudaq_warn_message.str()).c_str());
      
      }else{
        myevent.setNWaveforms(nwf);}
    }//end else


    if ((ev.GetEventNumber() == 1) && (_offline <0)){ //only update Display, when GUI is active
      onlinemon->UpdateStatus("Getting data..");}


    SimpleStandardEvent simpEv;      
    // store the processing time of the previous EVENT, as we can't track this during the processing
    simpEv.setMonitor_eventanalysistime(previous_event_analysis_time);
    simpEv.setMonitor_eventfilltime(previous_event_fill_time);
    simpEv.setMonitor_eventclusteringtime(previous_event_clustering_time);
    simpEv.setMonitor_eventcorrelationtime(previous_event_correlation_time);
    simpEv.setEvent_number(ev.GetEventNumber());
    simpEv.setEvent_timestamp(ev.GetTimestamp());

    // Get Information wheater this event is an Pulser event
    // this is a hardcoded fix for setup at psi, think about a different option
    bool isPulserEvent = false;
    for (unsigned int i = 0; i < nwf;i++){
      const eudaq::StandardWaveform & waveform = ev.GetWaveform(i);
      if (waveform.GetChannelName() == "PULSER"){
        SimpleStandardWaveform simpWaveform(waveform.GetType(),waveform.ID(),waveform.GetNSamples(),&mon_configdata);
    simpWaveform.addData(&(*waveform.GetData())[0]);
    simpWaveform.Calculate();
    float integral = simpWaveform.getIntegral(700,900);
    float pulserMin = simpWaveform.getMinimum(700, 900);
    if( pulserMin < -100.)
        isPulserEvent = true;
      }
    }//end for

    if (_last_fft_min.size() <nwf)_last_fft_min.resize(nwf,-1);
    if (_last_fft_max.size() <nwf)_last_fft_max.resize(nwf,-1);
    if (_last_fft_mean.size() <nwf)_last_fft_mean.resize(nwf,-1);
    for (unsigned int i = 0; i < nwf;i++){
      const eudaq::StandardWaveform & waveform = ev.GetWaveform(i);


      #ifdef DEBUG
       cout << "Waveform ID          " << waveform.ID()<<endl;
       cout << "Waveform Size        " << sizeof(waveform) <<endl;
       //cout << "Waveform Frames      " << waveform.NumFrames() <<endl;
       cout << "Waveform Channel no. " << waveform.GetChannelNumber()<<endl;
       cout << "Waveform Channelname " << waveform.GetChannelName()<<endl;
       cout << "Waveform Sensor      " << waveform.GetSensor()<<endl;
       cout << "Waveform Type        " << waveform.GetType() << endl;
       cout << "Waveform NSamples    " << waveform.GetNSamples() <<endl; // gives 2560 for V1730
      #endif


      std::string sensorname;
      sensorname = waveform.GetType();
      SimpleStandardWaveform simpWaveform(sensorname,waveform.ID(),waveform.GetNSamples(),&mon_configdata);
      simpWaveform.setSign(mon_configdata.getSignalSign(waveform.GetChannelNumber()));
      simpWaveform.setNSamples(waveform.GetNSamples());
      simpWaveform.addData(&(*waveform.GetData())[0]);
            // perform the fft and put it into the simpleWF directly
            simpWaveform.performFFT(fft_own);
            int count = 0;
            if (simpWaveform.getMeanFFT() == _last_fft_mean.at(i)){
                //cout<<i<<"FFT Means disagree\t";
                count+=1;
            }
            if (simpWaveform.getMaxFFT() == _last_fft_max.at(i)){
                //cout<<i<<"FFT Max disagree\t";
                count+=1;
            }
            if (simpWaveform.getMinFFT() == _last_fft_min.at(i)){
                //cout<<i<<"FFT Min disagree\t";
                count+=1;
            }
            if (count!=0){
                if (fft_own){
                    delete fft_own;
                    _fft_resets+=1;
                    cout<<ev.GetEventNumber()<<" Reinitialized FFT "<<_fft_resets<<endl;
                    int n_samples = 1024;
                    Int_t n_size = n_samples+1;
                    fft_own = TVirtualFFT::FFT(1, &n_size, "R2C P K");
                }
                simpWaveform.performFFT(fft_own);
            }
            _last_fft_min[i] = simpWaveform.getMinFFT();
            _last_fft_max[i] = simpWaveform.getMaxFFT();
            _last_fft_mean[i] = simpWaveform.getMeanFFT();

      simpWaveform.Calculate();
      //simpWaveform.setTimestamp(ev.GetTimestamp());
      simpWaveform.setTimestamp(waveform.GetTimeStamp());
      simpWaveform.setEvent(ev.GetEventNumber());
      simpWaveform.setChannelName(waveform.GetChannelName());
      simpWaveform.setChannelNumber(waveform.GetChannelNumber());
      simpWaveform.setPulserEvent(isPulserEvent);
            //std::cout<<"isPulser Event: "<<isPulserEvent<<"/"<<simpWaveform.isPulserEvent()<<std::endl;
//      waveform.GetNSamples();
//      cout<<"simpWaveform no"<<i<<" name \""<<simpWaveform.getName()
//          <<"\" ID: "<<simpWaveform.getID()
//          <<" ch name \""<<simpWaveform.getChannelName()<<"\""<<endl;//<<"\" mon:"<<_mon<<endl;
      simpEv.addWaveform(simpWaveform);


/************************************** Start TU Event Stuff **************************************/
//don't blame me for this code ..

      unsigned int ntu = (unsigned int) ev.NumTUEvents();

      if(ntu > 0){

        if(ntu > 1){std::cout << "There is more than 1 TUEvent in the vector. Not good.." << std::endl;}
        const eudaq::StandardTUEvent &tuev = ev.GetTUEvent(0);
        SimpleStandardTUEvent simpleTUEvent(tuev.GetType());
 
        //just transfer data to SimpleStandardTUEvent for processing:
        bool valid = tuev.GetValid();
        if(valid){
          simpleTUEvent.SetValid(1);
          simpleTUEvent.SetTimeStamp(tuev.GetTimeStamp());
          simpleTUEvent.SetCoincCount(tuev.GetCoincCount());
          simpleTUEvent.SetCoincCountNoSin(tuev.GetCoincCountNoSin());
          simpleTUEvent.SetPrescalerCount(tuev.GetPrescalerCount());
          simpleTUEvent.SetPrescalerCountXorPulserCount(tuev.GetPrescalerCountXorPulserCount());
          simpleTUEvent.SetAcceptedPrescaledEvents(tuev.GetAcceptedPrescaledEvents());
          simpleTUEvent.SetAcceptedPulserCount(tuev.GetAcceptedPulserCount());
          simpleTUEvent.SetHandshakeCount(tuev.GetHandshakeCount());
          simpleTUEvent.SetBeamCurrent(tuev.GetBeamCurrent());
          for(int idx=0; idx<10; idx++){ //hard coded beause..., that's why
            simpleTUEvent.SetScalerValue(idx, tuev.GetScalerValue(idx));
          }
        }else{
          simpleTUEvent.SetValid(0);
        }

        simpEv.addTUEvent(simpleTUEvent);//send new

      }//if ntu > 0

/************************************** End TU Event Stuff **************************************/


    }


    if (skip_dodgy_event){
      return;} //don't process any further

    for (unsigned int i = 0; i < num;i++){
      const eudaq::StandardPlane & plane = ev.GetPlane(i);

      #ifdef DEBUG
      cout << "Plane ID         " << plane.ID()<<endl;
      cout << "Plane Size       " << sizeof(plane) <<endl;
      cout << "Plane Frames     " << plane.NumFrames() <<endl;
      for (unsigned int nframes=0; nframes<plane.NumFrames(); nframes++){
        cout << "Plane Pixels Hit Frame " << nframes <<" "<<plane.HitPixels(0) <<endl;}
        cout << i << " "<<plane.TLUEvent() << " "<< plane.PivotPixel() <<endl;
      #endif


      string sensorname;
      if ((plane.Type() == std::string("DEPFET")) &&(plane.Sensor().length()==0)){
        sensorname=plane.Type();
      }else{
        sensorname=plane.Sensor();}


      if (strcmp(plane.Sensor().c_str(), "FORTIS") == 0 ){
        continue;}


      SimpleStandardPlane simpPlane(sensorname,plane.ID(),plane.XSize(),plane.YSize(), plane.TLUEvent(),plane.PivotPixel(),&mon_configdata);
      simpPlane.m_ntrig = plane.m_ntrig;
      for (unsigned int lvl1 = 0; lvl1 < plane.NumFrames(); lvl1++){
        // if (lvl1 > 2 && plane.HitPixels(lvl1) > 0) std::cout << "LVLHits: " << lvl1 << ": " << plane.HitPixels(lvl1) << std::endl;
        for (unsigned int index = 0; index < plane.HitPixels(lvl1);index++){
          SimpleStandardHit hit((int)plane.GetX(index,lvl1),(int)plane.GetY(index,lvl1));
          hit.setTOT((int)plane.GetPixel(index,lvl1)); //this stores the analog information if existent, else it stores 1
          hit.setLVL1(lvl1);
          if (simpPlane.getAnalogPixelType()){ //this is analog pixel, apply threshold
            if (simpPlane.is_DEPFET){
              if ((hit.getTOT()< -20) || (hit.getTOT()>120)){
                continue;}
            }
            if (simpPlane.is_EXPLORER){
              if (lvl1!=0) continue;
              hit.setTOT((int)plane.GetPixel(index));
              if (hit.getTOT() < 20){
                continue;}
            }
            simpPlane.addHit(hit);
          }
          else{ //purely digital pixel
            simpPlane.addHit(hit);}

        }//inner for end
      }//outer for end

      if (simpPlane.is_CMSPIXEL){
          simpPlane.setTriggerPhase(plane.GetTrigPhase());}
      simpEv.addPlane(simpPlane);

      #ifdef DEBUG
        cout << "Type: " << plane.Type() << endl;
        cout << "StandardPlane: "<< plane.Sensor() <<  " " << plane.ID() << " " << plane.XSize() << " " << plane.YSize() << endl;
        cout << "PlaneAddress: " << &plane << endl;
      #endif
    }

    my_event_inner_operations_time.Start(true);
    simpEv.doClustering();
    my_event_inner_operations_time.Stop();
    previous_event_clustering_time = my_event_inner_operations_time.RealTime();

    if(!_planesInitialized){
      #ifdef DEBUG
      cout << "Waiting for booking of Histograms..." << endl;
      #endif
      EUDAQ_SLEEP(3);
      #ifdef DEBUG
      cout << "...long enough"<< endl;
      #endif
      _planesInitialized = true;}


    //stop the Stop watch
    my_event_processing_time.Stop();

    #ifdef DEBUG
      cout << "Analysing"<<   " "<< my_event_processing_time.RealTime()<<endl;
    #endif
    previous_event_analysis_time=my_event_processing_time.RealTime();


    //Filling
    my_event_processing_time.Start(true); //start the stopwatch again
    for (unsigned int i = 0 ; i < _colls.size(); ++i){
      if (_colls.at(i) == corrCollection){
        my_event_inner_operations_time.Start(true);
        if (getUseTrack_corr() == true){
          tracksPerEvent = corrCollection->FillWithTracks(simpEv);
          if (eudaqCollection->getEUDAQMonitorHistos() != NULL) //workaround because Correlation Collection is before EUDAQ Mon collection
            eudaqCollection->getEUDAQMonitorHistos()->Fill(simpEv.getEvent_number(), tracksPerEvent);
        }
        else
          _colls.at(i)->Fill(simpEv);
          my_event_inner_operations_time.Stop();
          previous_event_correlation_time = my_event_inner_operations_time.RealTime();
      }
      else
        _colls.at(i)->Fill(simpEv);

      // CollType is used to check which kind of Collection we are having
      if (_colls.at(i)->getCollectionType()==HITMAP_COLLECTION_TYPE){ // Calculate is only implemented for HitMapCollections
        _colls.at(i)->Calculate(ev.GetEventNumber());}

      if (_colls.at(i)->getCollectionType()==WAVEFORM_COLLECTION_TYPE){ // Calculate is only implemented for HitMapCollections
        _colls.at(i)->Calculate(ev.GetEventNumber());
      }
    }//end for

    if (_offline <= 0){
      onlinemon->setEventNumber(ev.GetEventNumber());
      onlinemon->increaseAnalysedEventsCounter();}
  } // end of reduce if



  my_event_processing_time.Stop();
  #ifdef DEBUG
    cout << "Filling " << " "<< my_event_processing_time.RealTime()<<endl;
    cout << "----------------------------------------"  <<endl<<endl;
  #endif
  previous_event_fill_time=my_event_processing_time.RealTime();

  if (ev.IsBORE()){
    std::cout << "This is a BORE" << std::endl;
    if (ev.GetRunNumber() != 0)
        this->onlinemon->setRunNumber(ev.GetRunNumber());}
}//end of the whole bloody method



void RootMonitor::autoReset(const bool reset){
  //_autoReset = reset;
  if (_offline <= 0) onlinemon->setAutoReset(reset);

}

void RootMonitor::OnStopRun(){
  if (_writeRoot)
  {
    TFile *f = new TFile(rootfilename.c_str(),"RECREATE");
    for (unsigned int i = 0 ; i < _colls.size(); ++i)
    {
      _colls.at(i)->Write(f);
    }
    f->Close();
  }
  onlinemon->UpdateStatus("Run stopped");
}


void RootMonitor::OnStartRun(unsigned param){

  if (onlinemon->getAutoReset()){
    onlinemon->UpdateStatus("Resetting..");
    for (unsigned int i = 0 ; i < _colls.size(); ++i){
      if (_colls.at(i) != NULL)
        _colls.at(i)->Reset();
    }
  }

  Monitor::OnStartRun(param);
  std::cout << "Called on start run" << param <<std::endl;
  onlinemon->UpdateStatus("Starting run..");
  char out[255];
  sprintf(out, "run%d.root",param);
  rootfilename = std::string(out);
  runnumber = param;

  if (_offline <= 0){
    onlinemon->setRunNumber(runnumber);
    onlinemon->setRootFileName(rootfilename);
  }

  // Reset the planes initializer on new run start:
  _planesInitialized = false;

  SetStatus(eudaq::Status::LVL_OK);
}


void RootMonitor::setUpdate(const unsigned int up){
  if (_offline <= 0) onlinemon->setUpdate(up);
}


//sets the location for the snapshots
void RootMonitor::SetSnapShotDir(string s){
  snapshotdir=s;
}

//gets the location for the snapshots
string RootMonitor::GetSnapShotDir(){
  return snapshotdir;
}

int main(int argc, const char ** argv) {
  eudaq::OptionParser op("EUDAQ Root Monitor", "1.0", "A Monitor using root for gui and graphics");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol", "tcp://localhost:44000", "address",
      "The address of the RunControl application");
  eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level",
      "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> file (op, "f", "data-file", "", "filename",
      "A data file to load - setting this changes the default"
      " run control address to 'null://'");
  eudaq::Option<int>             x(op, "x", "left",    100, "pos");
  eudaq::Option<int>             y(op, "y", "top",       0, "pos");
  eudaq::Option<int>             w(op, "w", "width",  1400, "pos");
  eudaq::Option<int>             h(op, "g", "height",  700, "pos", "The initial position of the window");
  eudaq::Option<int>             reduce(op, "rd", "reduce",  1, "Reduce the number of events");
  eudaq::Option<unsigned>        limit(op, "n", "limit", 0, "Event number limit for analysis");
  eudaq::Option<unsigned>        skip_counter(op, "sc", "skip_count", 10, "Number of events to skip per every taken event");
  eudaq::Option<unsigned>        skipping(op, "s", "skip", 0, "Percentage of events to skip");
  eudaq::Option<unsigned>        corr_width(op, "cw", "corr_width",500, "Width of the track correlation window");
  eudaq::Option<unsigned>        corr_planes(op, "cp", "corr_planes",  5, "Minimum amount of planes for track reconstruction in the correlation");
  eudaq::Option<bool>            track_corr(op, "tc", "track_correlation", false, "Using (EXPERIMENTAL) track correlation(true) or cluster correlation(false)");
  eudaq::Option<int>             update(op, "u", "update",  1000, "update every ms");
  eudaq::Option<unsigned int>             start_event(op, "st", "start",  0, "starting at event <num>");
  eudaq::Option<int>             offline(op, "o", "offline",  0, "running is offlinemode - analyse until event <num>");
  eudaq::Option<std::string>     configfile(op, "c", "config_file"," ", "filename","Config file to use for onlinemon");
  eudaq::OptionFlag do_rootatend (op, "rf","root","Write out root-file after each run");
  eudaq::OptionFlag do_resetatend (op, "rs","reset","Reset Histograms when run stops");


  try {
    op.Parse(argv);
    EUDAQ_LOG_LEVEL(level.Value());
    if (file.IsSet() && !rctrl.IsSet()) rctrl.SetValue("null://");

    if (gROOT==NULL){
      cout<<"Global gROOT Object not found" <<endl;
      exit(-1);}

    if (gStyle!=NULL){
      gStyle->SetPalette(1);
      gStyle->SetNumberContours(99);
      gStyle->SetOptStat(1101);
      gStyle->SetStatH(0.07);
      gStyle->SetStatW(0.12);
      gStyle->SetStatX(0.20);
      gStyle->SetStatY(1.00);
      gStyle->SetPadRightMargin(0.12);}

    else{
      cout<<"Global gStyle Object not found" <<endl;
      exit(-1);}


    // start the GUI
    TApplication theApp("App", &argc, const_cast<char**>(argv),0,0);
    RootMonitor mon(rctrl.Value(), file.Value(), x.Value(), y.Value(),
        w.Value(), h.Value(), argc, offline.Value(), limit.Value(),
        skipping.Value(), skip_counter.Value(), configfile.Value());
    mon.setWriteRoot(do_rootatend.IsSet());
    mon.autoReset(do_resetatend.IsSet());
    mon.setReduce(reduce.Value());
    mon.setUpdate(update.Value());
    mon.setCorr_width(corr_width.Value());
    mon.setCorr_planes(corr_planes.Value());
    mon.setUseTrack_corr(track_corr.Value());
    mon.setStartEvent(start_event.Value());

    cout <<"Monitor Settings:" <<endl;
    cout <<"Update Interval :" <<update.Value() <<" ms" <<endl;
    cout <<"Reduce Events   :" <<reduce.Value() <<endl;
    
    if (offline.Value() > 0){
      cout <<"Offline Mode not supported"<<endl;
      exit(-1);}


    theApp.Run(); //execute

  } catch (...) {
    return op.HandleMainException();}

  return 0;
}








