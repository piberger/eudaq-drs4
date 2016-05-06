/* ---------------------------------------------------------------------------------
** OSU Trigger Logic Unit EUDAQ Implementation
** 
**
** <TUHistos>.cc
** 
** Date: May 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

#include "TUHistos.hh"
#include "SimpleStandardEvent.hh"
#include "SimpleStandardTUEvent.hh"

#include <iostream>
#include <TH1I.h>
#include <TFile.h>


TUHistos::TUHistos(){

    _CoincidenceCount = new TH1I("Concidence Count", "Coincidence Count; Number of Events; Entries",1000, 0, 20000);
    _CoincidenceCountNoScint = new TH1I("Coincidence Count No Scintillator", "Coincidence Count No Scintillator; Number of Events; Entries",1000, 0, 20000);
    _PrescalerCount = new TH1I("Prescaler Count", "Prescaler Count; Number of Events; Entries",1000, 0, 20000);
    _PrescalerXPulser = new TH1I("Prescaler Xor Pulser Count", "Prescaler Xor Pulser Count; Number of Events; Entries",1000, 0, 20000);
    _AcceptedPrescaledEvents = new TH1I("Accepted Prescaled Events", "Accepted Prescaled Events; Number of Events; Entries",1000, 0, 20000);
    _AcceptedPulserEvents = new TH1I("Accepted Pulser Events", "Accepted Pulser Events; Number of Events; Entries",1000, 0, 20000);
    _EventCount = new TH1I("Event Count", "Event Count; Number of Events; Entries",1000, 0, 20000);
    _AvgBeamCurrent = new TH1I("Average Beam Current", "Average Beam Current; Number of Events; Beam Current [mA]",1000, 0, 20000);
    _Scaler1 = new TH1I("Plane Scaler 1", "Plane Scaler 1; Number of Events; Entries",1000, 0, 20000);
    _Scaler2 = new TH1I("Plane Scaler 2", "Plane Scaler 2; Number of Events; Entries",1000, 0, 20000);


 	_CoincidenceCount->SetBit(TH1::kCanRebin);
	_CoincidenceCountNoScint->SetBit(TH1::kCanRebin);
	_PrescalerCount->SetBit(TH1::kCanRebin);
	_PrescalerXPulser->SetBit(TH1::kCanRebin);
	_AcceptedPrescaledEvents->SetBit(TH1::kCanRebin);
	_AcceptedPulserEvents->SetBit(TH1::kCanRebin);
	_EventCount->SetBit(TH1::kCanRebin);
	_AvgBeamCurrent->SetBit(TH1::kCanRebin);
	_Scaler1->SetBit(TH1::kCanRebin);
	_Scaler2->SetBit(TH1::kCanRebin);


    if((_CoincidenceCount==NULL)){
      std::cout<< "TUHistos:: Error allocating Histograms" <<std::endl;
      exit(-1);
    }
}

TUHistos::~TUHistos(){}



void TUHistos::Write(){
  _CoincidenceCount->Write();
  _CoincidenceCountNoScint->Write();
  _PrescalerCount->Write();
  _PrescalerXPulser->Write();
  _AcceptedPrescaledEvents->Write();
  _AcceptedPulserEvents->Write();
  _EventCount->Write();
  _AvgBeamCurrent->Write();
  _Scaler1->Write();
  _Scaler2->Write();

}



void TUHistos::Fill(SimpleStandardTUEvent ev, unsigned int event_nr){
	bool valid = ev.GetValid();
	if(valid){
	  _CoincidenceCount->Fill(event_nr, ev.GetPrescalerCount());
      _CoincidenceCountNoScint->Fill(event_nr, ev.GetCoincCountNoSin());
      _PrescalerCount->Fill(event_nr, ev.GetPrescalerCount());
      _PrescalerXPulser->Fill(event_nr, ev.GetPrescalerCountXorPulserCount());
      _AcceptedPrescaledEvents->Fill(event_nr, ev.GetAcceptedPrescaledEvents());
      _AcceptedPulserEvents->Fill(event_nr, ev.GetAcceptedPulserCount());
      _EventCount->Fill(event_nr, ev.GetHandshakeCount());
      _AvgBeamCurrent->Fill(event_nr, ev.GetBeamCurrent());
      _Scaler1->Fill(event_nr, ev.GetScalerValue(0));
      _Scaler2->Fill(event_nr, ev.GetScalerValue(1));
	}
}



void TUHistos::Reset(){
  _CoincidenceCount->Reset();
  _CoincidenceCountNoScint->Reset();
  _PrescalerCount->Reset();
  _PrescalerXPulser->Reset();
  _AcceptedPrescaledEvents->Reset();
  _AcceptedPulserEvents->Reset();
  _EventCount->Reset();
  _AvgBeamCurrent->Reset();
  _Scaler1->Reset();
  _Scaler2->Reset();
}

