#include "RecCdMuonAlg/PmtProp.h"
#include "RecCdMuonAlg/Params.h"
#include"TVector3.h"

struct FourDCoor {
	TVector3 P;
	double T;
};

double* firstHitTimeExpected(TVector3/*injecting point*/,
    double/*injecting time*/,
    TVector3/*track direction*/,
    double/*track length in lS*/,
    const PmtProp&/*pmt info*/,
    const Params& pars/*transmit in the env parameters:
                        Params pars;
                        pars.set("LightSpeed",299.) //speed of light, eg. 299.0
                        pars.set("MuonSpeed",299.) //speed of muon, eg. 299.0
                        pars.set("LSRefraction",1.485) // refraction index of light in the LS, eg. 1.485
                        */
    );
