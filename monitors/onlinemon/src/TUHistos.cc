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

#include <iostream>
#include <TH1I.h>
#include <TFile.h>


TUHistos::TUHistos(){
    _CoincidenceCount = new TH1I("Concidence Count", "Coincidence Count",400,0,0.01);

    if((_CoincidenceCount==NULL)){
      std::cout<< "TUHistos:: Error allocating Histograms" <<std::endl;
      exit(-1);
    }
}

TUHistos::~TUHistos(){}


void TUHistos::Write(){
  _CoincidenceCount->Write();
}



void TUHistos::Fill(SimpleStandardEvent ev){
  //_CoincidenceCount->Fill(ev.)
  //implement SimpleStandardEvent for TU!
}



void TUHistos::Reset(){
  _CoincidenceCount->Reset();
}

