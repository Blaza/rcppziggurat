// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// ZigguratMZ.h:  Marsaglia and Tsang Ziggurat generator
//
// Copyright (C) 2013  Dirk Eddelbuettel
//
// This file is part of RcppZiggurat.
//
// RcppZiggurat is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// RcppZiggurat is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RcppZiggurat.  If not, see <http://www.gnu.org/licenses/>.

// This file is derived from the file 'rnorrexp.c' which accompanies this article
//    George Marsaglia and Wai Wan Tsang 
//    The Ziggurat Method for Generating Random Variables,
//    Journal of Statistical Software, Vol 5, Iss 8, Oct 2000
//    http://www.jstatsoft.org/v05/i08 
//
// The code by Marsaglia and Tsang has been modified 
// a) by reorganising it as a C++ class,
// b) by switching from use of (unsigned) long variable (which implied 32 bit
//    at the time) to use of (u)int32_t from the stdint.h header file
// c) removing the exponential generator
//
// Per the article of Leong, Zhang et al (see file ZigguratLZLLV.h here), this
// original Marsaglia and Tsang article should NOT be used as originally published.
//
// This implementation serves primarily for comparison.  


// An original comment follows

/* The ziggurat method for RNOR and REXP
Combine the code below with the main program in which you want
normal or exponential variates.   Then use of RNOR in any expression
will provide a standard normal variate with mean zero, variance 1,
while use of REXP in any expression will provide an exponential variate
with density exp(-x),x>0.
Before using RNOR or REXP in your main, insert a command such as
zigset(86947731 );
with your own choice of seed value>0, rather than 86947731.
(If you do not invoke zigset(...) you will get all zeros for RNOR and REXP.)
For details of the method, see Marsaglia and Tsang, "The ziggurat method
for generating random variables", Journ. Statistical Software.
*/

#ifndef RcppZiggurat__ZigguratMT_h
#define RcppZiggurat__ZigguratMT_h

#include <cmath>
#include <stdint.h>             // not allowed to use cstdint as it need C++11

class ZigguratMT {
private:
    uint32_t jz, jsr;
    long hz;
    uint32_t iz, kn[128]; /*, ke[256];*/
    float wn[128],fn[128]; /*, we[256],fe[256];*/
    
#define SHR3 (jz=jsr, jsr^=(jsr<<13), jsr^=(jsr>>17), jsr^=(jsr<<5),jz+jsr)
#define UNI (.5 + (signed) SHR3*.2328306e-9)
#define IUNI SHR3

#define RNOR (hz=SHR3, iz=hz&127, (fabs(hz)<kn[iz])? hz*wn[iz] : nfix())
//#define REXP (jz=SHR3, iz=jz&255, (    jz <ke[iz])? jz*we[iz] : efix())

public: 
    ZigguratMT(uint32_t seed=42) : jsr(123456789) {
        setup();
        setSeed(seed);
    }

    inline double norm(void) { 
        return RNOR; 
    }

    void setSeed(uint32_t jsrseed) {
        jsr = 123456789;
        jsr^=jsrseed;
    }

private:
    // setup internal tables
    void setup(void) {
        const double m1 = 2147483648.0; /*, m2 = 4294967296.;*/
        double dn=3.442619855899,tn=dn,vn=9.91256303526217e-3, q;
        //double de=7.697117470131487, te=de, ve=3.949659822581572e-3;
        int i;

        /* Set up tables for RNOR */
        q=vn/exp(-.5*dn*dn);
        kn[0]=(dn/q)*m1;
        kn[1]=0;

        wn[0]=q/m1;
        wn[127]=dn/m1;

        fn[0]=1.;
        fn[127]=exp(-.5*dn*dn);

        for(i=126;i>=1;i--) {
            dn=sqrt(-2.*log(vn/dn+exp(-.5*dn*dn)));
            kn[i+1]=(dn/tn)*m1;
            tn=dn;
            fn[i]=exp(-.5*dn*dn);
            wn[i]=dn/m1;
        }
    }

    // nfix() generates variates from the residue when rejection in RNOR occurs. 
    inline float nfix(void) {
        const float r = 3.442620f;      // The start of the right tail 
        float x, y;
        for(;;) {
            x=hz*wn[iz];                // iz==0, handles the base strip 
            if(iz==0) {
                do { x=-log(UNI)*0.2904764; y=-log(UNI); }      /* .2904764 is 1/r */
                while(y+y<x*x);
                return (hz>0)? r+x : -r-x;
            }
            // iz>0, handle the wedges of other strips 
            if( fn[iz]+UNI*(fn[iz-1]-fn[iz]) < exp(-.5*x*x) ) return x;

            // initiate, try to exit for(;;) for loop*/
            hz=SHR3;
            iz=hz&127;
            if(fabs(hz)<kn[iz]) return (hz*wn[iz]);
        }
    }
};

#undef SHR3 
#undef UNI 
#undef IUNI 
#undef RNOR 

#endif

