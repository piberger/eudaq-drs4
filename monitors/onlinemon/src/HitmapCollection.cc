/*
 * HitmapCollection.cc
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#include "HitmapCollection.hh"
#include "OnlineMon.hh"

static int counting = 0;
static int events = 0;

bool HitmapCollection::isPlaneRegistered(SimpleStandardPlane p)
{
  std::map<SimpleStandardPlane,HitmapHistos*>::iterator it;
  it = _map.find(p);
  return (it != _map.end());
}

void HitmapCollection::fillHistograms(const SimpleStandardPlane &simpPlane, unsigned event_no, unsigned time_stamp)
{
    this->_current_timestamp = time_stamp;
    this->_current_eventnumber = event_no;
  if (!isPlaneRegistered(simpPlane))
  {
    registerPlane(simpPlane);
    isOnePlaneRegistered = true;
  }
  current_hitmap = _map[simpPlane];
  current_hitmap->Fill(simpPlane,_current_eventnumber,_current_timestamp);
  ++counting;
  events += simpPlane.getNHits();
}

void HitmapCollection::bookHistograms(const SimpleStandardEvent &simpev)
{
  for (int plane = 0; plane < simpev.getNPlanes(); plane++)
  {
    SimpleStandardPlane simpPlane = simpev.getPlane(plane);
    if (!isPlaneRegistered(simpPlane))
    {
      registerPlane(simpPlane);

    }

  }
}

void HitmapCollection::Write(TFile *file)
{
  if (file==NULL)
  {
    //cout << "HitmapCollection::Write File pointer is NULL"<<endl;
    exit(-1);
  }
  if (gDirectory!=NULL) //check if this pointer exists
  {
    gDirectory->mkdir("Hitmaps");
    gDirectory->cd("Hitmaps");


    std::map<SimpleStandardPlane,HitmapHistos*>::iterator it;
    for (it = _map.begin(); it != _map.end(); ++it) {

      char sensorfolder[255] = "";
      sprintf(sensorfolder,"%s_%d",it->first.getName().c_str(), it->first.getID());
      //cout << "Making new subfolder " << sensorfolder << endl;
      gDirectory->mkdir(sensorfolder);
      gDirectory->cd(sensorfolder);
      it->second->Write();

      //gDirectory->ls();
      gDirectory->cd("..");
    }
    gDirectory->cd("..");
  }
}

void HitmapCollection::Calculate(const unsigned int currentEventNumber)
{
  if ((currentEventNumber > 10 && currentEventNumber % 1000*_reduce == 0))
  {
    std::map<SimpleStandardPlane,HitmapHistos*>::iterator it;
    for (it = _map.begin(); it != _map.end(); ++it)
    {
      //std::cout << "Calculating ..." << std::endl;
      it->second->Calculate(currentEventNumber/_reduce);
    }
  }
}

void HitmapCollection::Reset()
{
  std::map<SimpleStandardPlane,HitmapHistos*>::iterator it;
  for (it = _map.begin(); it != _map.end(); ++it)
  {
    (*it).second->Reset();
  }

}

void HitmapCollection::Fill(const SimpleStandardEvent &simpev)
{

  for (int plane = 0; plane < simpev.getNPlanes(); plane++) {
    const SimpleStandardPlane&  simpPlane = simpev.getPlane(plane);
    fillHistograms(simpPlane,simpev.getEvent_number(),simpev.getEvent_timestamp());
  }
}
HitmapHistos * HitmapCollection::getHitmapHistos(std::string sensor, int id)
{
  SimpleStandardPlane sp(sensor,id);
  return _map[sp];
}


void HitmapCollection::registerPlane(const SimpleStandardPlane &p) {
  HitmapHistos *tmphisto = new HitmapHistos(p,_mon);
  _map[p] = tmphisto;
  //std::cout << "Registered Plane: " << p.getName() << " " << p.getID() << std::endl;
  //PlaneRegistered(p.getName(),p.getID());
  if (_mon != NULL)
  {
    if (_mon->getOnlineMon()==NULL)
    {
      return; // don't register items
    }
    //cout << "HitmapCollection:: Monitor running in online-mode" << endl;
    char tree[1024], folder[1024];
    sprintf(tree,"%s/Sensor %i/RawHitmap",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getHitmapHisto(), "COLZ",0);

    sprintf(tree,"%s/Sensor %i/RawChargemap",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getChargemapHisto(), "COLZ",0);


    sprintf(folder,"%s",p.getName().c_str());
#ifdef DEBUG
    cout << "DEBUG "<< p.getName().c_str() <<endl;
    cout << "DEBUG "<< folder << " "<<tree<<  endl;
#endif
    _mon->getOnlineMon()->addTreeItemSummary(folder,tree);

    sprintf(tree,"%s/Sensor %i/Hitmap X Projection",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getHitXmapHisto());

    sprintf(tree,"%s/Sensor %i/Hitmap Y Projection",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getHitYmapHisto());

    sprintf(tree,"%s/Sensor %i/Clustermap",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getClusterMapHisto(), "COLZ",0);
    if (p.is_CMSPIXEL){
        sprintf(tree,"%s/Sensor %i/SinglePixelCharge",p.getName().c_str(),p.getID());
        _mon->getOnlineMon()->registerTreeItem(tree);
        _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getTOTSingleHisto());

    sprintf(tree,"%s/Sensor %i/Calmap",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getCalMapHisto(), "COLZ",0);

    sprintf(tree,"%s/Sensor %i/Bgmap",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getBgMapHisto(), "COLZ",0);

    sprintf(tree,"%s/Sensor %i/CalEff",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getCalEff());

    sprintf(tree,"%s/Sensor %i/BG rate",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getBgRate());

		sprintf(tree,"%s/Sensor %i/PixelChargeProfile",p.getName().c_str(),p.getID());
		_mon->getOnlineMon()->registerTreeItem(tree);
		_mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getPixelChargeProfile());

		sprintf(tree,"%s/Sensor %i/ClusterCharge",p.getName().c_str(),p.getID());
		_mon->getOnlineMon()->registerTreeItem(tree);
		_mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getTOTClusterHisto());

		sprintf(tree,"%s/Sensor %i/ClusterChargeProfile",p.getName().c_str(),p.getID());
		_mon->getOnlineMon()->registerTreeItem(tree);
		_mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getClusterChargeProfile());

		//sprintf(tree,"%s/Sensor %i/ClusterChargeTimeProfile",p.getName().c_str(),p.getID());
		//        _mon->getOnlineMon()->registerTreeItem(tree);
		//        _mon->getOnlineMon()->registerHisto(tree,(TProfile*)getHitmapHistos(p.getName(),p.getID())->getHisto("ClusterChargeTimeProfile"));
    }
    if ((p.is_APIX) || (p.is_USBPIX) || (p.is_USBPIXI4) )
    {
      sprintf(tree,"%s/Sensor %i/LVL1Distr",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getLVL1Histo());

      sprintf(tree,"%s/Sensor %i/LVL1Cluster",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getLVL1ClusterHisto());

      sprintf(tree,"%s/Sensor %i/LVL1Width",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getLVL1WidthHisto());

      sprintf(tree,"%s/Sensor %i/SingleTOT",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getTOTSingleHisto());

      sprintf(tree,"%s/Sensor %i/ClusterTOT",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getTOTClusterHisto());

      sprintf(tree,"%s/Sensor %i/ClusterWidthX",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getClusterWidthXHisto());

      sprintf(tree,"%s/Sensor %i/ClusterWidthY",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getClusterWidthYHisto());
    }
    if (p.is_DEPFET)
    {
      sprintf(tree,"%s/Sensor %i/SingleTOT",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getTOTSingleHisto());
    }

    sprintf(tree,"%s/Sensor %i/Clustersize",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getClusterSizeHisto());

      sprintf(tree,"%s/Sensor %i/ClustersizeBG",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getClusterSizeBgHisto());


    sprintf(tree,"%s/Sensor %i/NumHits",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getNHitsHisto());

    sprintf(tree,"%s/Sensor %i/NumBadHits",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getNbadHitsHisto());
    sprintf(tree,"%s/Sensor %i/NumHotPixels",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getNHotPixelsHisto());

    sprintf(tree,"%s/Sensor %i/NumClusters",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getNClustersHisto());

    sprintf(tree,"%s/Sensor %i/Efficency",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getEfficencyPerEvent());

    sprintf(tree,"%s/Sensor %i/HitOcc",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getHitOccHisto(), "",1);

    sprintf(tree,"%s/Sensor %i/Hot Pixel Map",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->registerTreeItem(tree);
    _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getHotPixelMapHisto(), "COLZ",0);

    if (p.is_MIMOSA26)
    {
      // setup histogram showing the number of hits per section of a Mimosa26
      sprintf(tree,"%s/Sensor %i/Hitmap Sections",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getHitmapSectionsHisto());
      sprintf(tree,"%s/Sensor %i/Pivot Pixel",p.getName().c_str(),p.getID());
      _mon->getOnlineMon()->registerTreeItem(tree);
      _mon->getOnlineMon()->registerHisto(tree,getHitmapHistos(p.getName(),p.getID())->getNPivotPixelHisto());

    }

    sprintf(tree,"%s/Sensor %i",p.getName().c_str(),p.getID());
    _mon->getOnlineMon()->makeTreeItemSummary(tree);

    if (p.is_MIMOSA26)
    {
      char mytree[4][1024];// holds the number of histogramms for each section, hardcoded, will be moved to vectors
      TH1I * myhistos[4];//FIXME hardcoded
      //loop over all sections
      for (unsigned int section =0; section<_mon->mon_configdata.getMimosa26_max_sections(); section ++ )
      {
        sprintf(mytree[0],"%s/Sensor %i/Section %i/NumHits",p.getName().c_str(),p.getID(),section);
        sprintf(mytree[1],"%s/Sensor %i/Section %i/NumClusters",p.getName().c_str(),p.getID(),section);
        sprintf(mytree[2],"%s/Sensor %i/Section %i/ClusterSize",p.getName().c_str(),p.getID(),section);
        sprintf(mytree[3],"%s/Sensor %i/Section %i/HotPixels",p.getName().c_str(),p.getID(),section);
        myhistos[0]=getHitmapHistos(p.getName(),p.getID())->getSectionsNHitsHisto(section);
        myhistos[1]=getHitmapHistos(p.getName(),p.getID())->getSectionsNClusterHisto(section);
        myhistos[2]=getHitmapHistos(p.getName(),p.getID())->getSectionsNClusterSizeHisto(section);
        myhistos[3]=getHitmapHistos(p.getName(),p.getID())->getSectionsNHotPixelsHisto(section);
        //loop over all histograms
        for (unsigned int nhistos=0; nhistos<4; nhistos++)
        {
          if (myhistos[nhistos]==NULL)
          {
            //cout << section << " " << "is null" << endl;
          }
          else
          {
            _mon->getOnlineMon()->registerTreeItem(mytree[nhistos]);
            _mon->getOnlineMon()->registerHisto(mytree[nhistos],myhistos[nhistos]);
          }
        }


        sprintf(tree,"%s/Sensor %i/Section %i",p.getName().c_str(),p.getID(),section);
        _mon->getOnlineMon()->makeTreeItemSummary(tree); //make summary page
      }
    }

  }
}

