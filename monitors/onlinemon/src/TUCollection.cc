/* ---------------------------------------------------------------------------------
** OSU Trigger Logic Unit EUDAQ Implementation
** 
**
** <TUCollection>.cc
** 
** Date: May 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/


#include "TUCollection.hh"
#include "OnlineMon.hh"

//ROOT Includes
#include <TH2I.h>
#include <TFile.h>

//STL includes
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <iostream>

//project includes
#include "SimpleStandardEvent.hh"
#include "TUHistos.hh"


TUCollection::TUCollection(): BaseCollection(){
  mymonhistos = new TUHistos();
  histos_init = false;
  std::cout << " Initialising TU Collection" << std::endl;
  //CollectionType = TU_COLLECTION_TYPE;
}

TUCollection::~TUCollection(){}


void TUCollection::Write(TFile *file)
{
//  if (file==NULL)
//  {
//    cout << "TUCollection::Write File pointer is NULL"<<endl;
//    exit(-1);
//  }
//  if (gDirectory!=NULL) //check if this pointer exists
//  {
//    gDirectory->mkdir("TU");
//    gDirectory->cd("TU");
//    mymonhistos->Write();
//    gDirectory->cd("..");
//  }
}

void TUCollection::Calculate(const unsigned int /*currentEventNumber*/)
{

}

void TUCollection::Reset()
{
  //mymonhistos->Reset();
}

void TUCollection::Fill(const SimpleStandardEvent &simpev)
{
//  if (histos_init==false)
//  {
//    bookHistograms(simpev);
//    histos_init=true;
//  }
//  mymonhistos->Fill(simpev);
}


void TUCollection::bookHistograms(const SimpleStandardEvent & /*simpev*/)
{
  if (_mon != NULL)
  {
//    string performance_folder_name="Monitor Performance";
//    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Data Analysis Time"));
//    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Data Analysis Time"),mymonhistos->getAnalysisTimeHisto());
//    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Histo Fill Time"));
//    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Histo Fill Time"),mymonhistos->getFillTimeHisto());
//    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Clustering Time"));
//    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Clustering Time"),mymonhistos->getClusteringTimeHisto());
//    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Correlation Time"));
//    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Correlation Time"),mymonhistos->getCorrelationTimeHisto());
//    _mon->getOnlineMon()->makeTreeItemSummary(performance_folder_name.c_str()); //make summary page
  }
}

TUHistos * TUCollection::getTUHistos()
{
  return mymonhistos;
}
