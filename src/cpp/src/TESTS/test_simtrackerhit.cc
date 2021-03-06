////////////////////////////////////////
//  test lcio::SimTrackerHit
////////////////////////////////////////

#include "tutil.h"
#include "lcio.h"

#include "EVENT/LCIO.h"
#include "IO/LCReader.h"
#include "IO/LCWriter.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/SimTrackerHitImpl.h"
#include "IMPL/LCFlagImpl.h"
//#include "UTIL/Operators.h"

#include "UTIL/CellIDEncoder.h"
#include "UTIL/CellIDDecoder.h"
#include "UTIL/ILDConf.h"
#include "UTIL/LCTrackerConf.h"

//#include <iostream>

using namespace std ;
using namespace lcio ;

//static const int NRUN = 10 ;
static const int NEVENT = 10 ; // events
static const int NHITS = 1000 ;  // tracker hits per event

static string FILEN = "simtrackerhits.slcio" ;

// replace mytest with the name of your test
const static string testname="test_simtrackerhit";

//=============================================================================

int main(int /*argc*/, char** /*argv*/ ){

    // this should be the first line in your test
    TEST MYTEST=TEST( testname, std::cout );

    try{


	MYTEST.LOG( " writing SimTrackerHits " );

	// create sio writer
	LCWriter* lcWrt = LCFactory::getInstance()->createLCWriter()  ;

	lcWrt->open( FILEN , LCIO::WRITE_NEW ) ;

	// EventLoop - create some events and write them to the file
	for(int i=0;i<NEVENT;i++){

	    // we need to use the implementation classes here
	    LCEventImpl*  evt = new LCEventImpl() ;


	    evt->setRunNumber( 4711  ) ;
	    evt->setEventNumber( i ) ;

	    LCCollectionVec* trkHits = new LCCollectionVec( LCIO::SIMTRACKERHIT )  ;

	    ILDCellIDEncoder<SimTrackerHitImpl> idEnc( "readoutUnit:8,daqChannel:16,"  ,   trkHits ) ;
	    //   this is effectively the same as:
	    //	 CellIDEncoder<SimTrackerHitImpl> idEnc( LCTrackerCellID::encoding_string() + ",readoutUnit:8,daqChannel:16"  ,   trkHits ) ;



	    for(int j=0;j<NHITS;j++){

	      SimTrackerHitImpl* trkHit = new SimTrackerHitImpl ;

	      idEnc.reset() ;

	      idEnc[ LCTrackerCellID::subdet() ]  = ILDDetID::FTD ;
	      idEnc[ LCTrackerCellID::layer() ]  = j % 100 ;
	      idEnc[ LCTrackerCellID::side()   ]  = ILDDetID::bwd ;
	      idEnc[ LCTrackerCellID::module() ]  = j / 100 + 1 ;
	      idEnc[ LCTrackerCellID::sensor() ]  = j % 4  ;
	      idEnc["daqChannel"] =  j*8 ;

	      idEnc.setCellID( trkHit ) ;

	      trkHit->setEDep( i*j*117. ) ;

	      if( (i+j) % 3 == 0 ) {
		trkHit->setProducedBySecondary( true );
	      }

	      if( (i+j) % 5 == 0 ) {
		trkHit->setOverlay( true );
	      }

	      double pos[3] = { 1.*i, 1.*j, 1.*i*j } ;
	      trkHit->setPosition( pos ) ;

	      trkHits->addElement( trkHit ) ;
	    }
	    evt->addCollection( trkHits , "SimTrackerHits") ;

	    lcWrt->writeEvent(evt) ;

	    delete evt ;
	}


	lcWrt->close() ;

	MYTEST.LOG(" reading back SimTrackerHits from file " ) ;

	// create sio reader
	LCReader* lcRdr = LCFactory::getInstance()->createLCReader()  ;

	lcRdr->open( FILEN ) ;

	for(int i=0;i<NEVENT;i++){

	    //std::cout << " testing event " << i << std::endl ;

	    LCEvent*  evt =  lcRdr->readNextEvent() ;

	    MYTEST( evt->getRunNumber() ,  4711 , " run number " )  ;

	    MYTEST(  evt->getEventNumber() ,  i , " event number "  ) ;

	    LCCollection* trkHits = evt->getCollection( "SimTrackerHits") ;

	    CellIDDecoder<SimTrackerHit> idDec( trkHits ) ;

	    for(int j=0;j<NHITS;j++) {

		//std::cout << " testing hit " << j << std::endl ;

		SimTrackerHit* trkHit = dynamic_cast<SimTrackerHit*>(trkHits->getElementAt(j)) ;


		MYTEST( idDec(trkHit)[ LCTrackerCellID::subdet() ]  , ILDDetID::FTD , " cellID(trkHit) == ( ILDDetID::FTD  ) " ) ;
		MYTEST( idDec(trkHit)[ LCTrackerCellID::layer()  ]  , j % 100       , " cellID(trkHit) == ( j % 100        ) " ) ;
		MYTEST( idDec(trkHit)[ LCTrackerCellID::side()   ]  , ILDDetID::bwd , " cellID(trkHit) == ( ILDDetID::bwd  ) " ) ;
		MYTEST( idDec(trkHit)[ LCTrackerCellID::module() ]  , j / 100 + 1   , " cellID(trkHit) == ( j / 100 + 1    ) " ) ;
		MYTEST( idDec(trkHit)[ LCTrackerCellID::sensor() ]  , j % 4         , " cellID(trkHit) == ( j % 4          ) " ) ;
		MYTEST( idDec(trkHit)["daqChannel"]          , j*8           , " cellID(trkHit) == ( j*8            ) " ) ;

		MYTEST( trkHit->getEDep() ,  i*j*117. , "EDep" ) ;

		MYTEST( trkHit->isProducedBySecondary() , bool((i+j)%3==0) , "Secondary" ) ;
		MYTEST( trkHit->isOverlay() , bool((i+j)%5==0) , "Overlay" ) ;

		const double* pos = trkHit->getPosition() ;

		MYTEST( pos[0] , i , " pos[0] " ) ;
		MYTEST( pos[1] , j , " pos[1] " ) ;
		MYTEST( pos[2] , i*j , " pos[2] " ) ;

	    }
	}
	lcRdr->close() ;


    } catch( Exception &e ){
	MYTEST.FAILED( e.what() );
    }

    return 0;
}

//=============================================================================
