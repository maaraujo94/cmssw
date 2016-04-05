#include <cassert>
#include <iostream>
#include <cmath>

#include "L1Trigger/L1TMuonOverlap/interface/OMTFinput.h"
#include "L1Trigger/L1TMuonOverlap/interface/OMTFConfiguration.h"
#include "L1Trigger/L1TMuonOverlap/interface/XMLConfigReader.h"

///////////////////////////////////////////////////
///////////////////////////////////////////////////
OMTFinput::OMTFinput(const OMTFConfiguration* omtfParams){

  clear();
  myOmtfParams = omtfParams;
  
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
const OMTFinput::vector1D & OMTFinput::getLayerData(unsigned int iLayer, bool giveEta) const{ 
  assert(iLayer<measurementsPhi.size());

  if(giveEta) return measurementsEta[iLayer];
  return measurementsPhi[iLayer];
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
std::bitset<128> OMTFinput::getRefHits(unsigned int iProcessor) const{
 
  std::bitset<128> refHits;

  unsigned int iRefHit = 0;
  for(auto iRefHitDef:myOmtfParams->refHitsDefs[iProcessor]){
    int iPhi = getLayerData(myOmtfParams->refToLogicNumber[iRefHitDef.iRefLayer])[iRefHitDef.iInput];    
    int iEta = getLayerData(myOmtfParams->refToLogicNumber[iRefHitDef.iRefLayer],true)[iRefHitDef.iInput];
    if(iPhi<(int)myOmtfParams->nPhiBins){
      refHits.set(iRefHit, iRefHitDef.fitsRange(iPhi));    
      refHitsEta[iRefHit] = iEta;
    }
    iRefHit++;
  }

  return refHits;
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
bool OMTFinput::addLayerHit(unsigned int iLayer,
			    unsigned int iInput,
			    int iPhi, int iEta){

  assert(iLayer<myOmtfParams->nLayers);
  assert(iInput<14);

  if(iPhi>=(int)myOmtfParams->nPhiBins) return true;

  if(measurementsPhi[iLayer][iInput]!=(int)myOmtfParams->nPhiBins) ++iInput;
  
  if(iInput>13) return false;
  
  measurementsPhi[iLayer][iInput] = iPhi;
  measurementsEta[iLayer][iInput] = iEta;

  return true;				      
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void OMTFinput::readData(XMLConfigReader *aReader, 
			 unsigned int iEvent,
			 unsigned int iProcessor){

  measurementsPhi = aReader->readEvent(iEvent, iProcessor);
  measurementsEta = aReader->readEvent(iEvent, iProcessor, true);
  
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void OMTFinput::mergeData(const OMTFinput *aInput){

  for(unsigned int iLayer=0;iLayer<myOmtfParams->nLayers;++iLayer){
    const OMTFinput::vector1D & aPhiVec = aInput->getLayerData(iLayer,false);
    const OMTFinput::vector1D & aEtaVec = aInput->getLayerData(iLayer,true);
    if(!aPhiVec.size()) continue;

    OMTFinput::vector1D layerData = getLayerData(iLayer, false);
    for(unsigned int iInput=0;iInput<14;++iInput){      
      addLayerHit(iLayer,iInput,aPhiVec[iInput],aEtaVec[iInput]);
    }
  }
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void OMTFinput::clear(){

  vector1D aLayer1D(14,myOmtfParams->nPhiBins);
  measurementsPhi.assign(myOmtfParams->nLayers,aLayer1D);
  measurementsEta.assign(myOmtfParams->nLayers,aLayer1D);
  refHitsEta.assign(128,myOmtfParams->nPhiBins);

}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
void  OMTFinput::shiftMyPhi(int phiShift){

  int lowScaleEnd = std::pow(2,myOmtfParams->nPhiBits-1);
  int highScaleEnd = lowScaleEnd-1;

for(unsigned int iLogicLayer=0;iLogicLayer<measurementsPhi.size();++iLogicLayer){
    for(unsigned int iHit=0;iHit<measurementsPhi[iLogicLayer].size();++iHit){
      if(!myOmtfParams->bendingLayers.count(iLogicLayer) &&
	 measurementsPhi[iLogicLayer][iHit]<(int)myOmtfParams->nPhiBins){
	if(measurementsPhi[iLogicLayer][iHit]<0) measurementsPhi[iLogicLayer][iHit]+=myOmtfParams->nPhiBins;
	measurementsPhi[iLogicLayer][iHit]-=phiShift;
	if(measurementsPhi[iLogicLayer][iHit]<0) measurementsPhi[iLogicLayer][iHit]+=myOmtfParams->nPhiBins;
	measurementsPhi[iLogicLayer][iHit]+=-lowScaleEnd;
	if(measurementsPhi[iLogicLayer][iHit]<-lowScaleEnd ||
	   measurementsPhi[iLogicLayer][iHit]>highScaleEnd) measurementsPhi[iLogicLayer][iHit] = (int)myOmtfParams->nPhiBins;	   
      }
    }
  }
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
std::ostream & operator << (std::ostream &out, const OMTFinput & aInput){
  
for(unsigned int iLogicLayer=0;iLogicLayer<aInput.measurementsPhi.size();++iLogicLayer){
    out<<"Logic layer: "<<iLogicLayer<<" Hits: ";
    for(unsigned int iHit=0;iHit<aInput.measurementsPhi[iLogicLayer].size();++iHit){
      out<<aInput.measurementsPhi[iLogicLayer][iHit]<<"\t";
    }
    out<<std::endl;
  }
  return out;
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
