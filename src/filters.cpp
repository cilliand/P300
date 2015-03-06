//
//  filters.cpp
//  pamEEG
//
//  Created by Chris Kiefer on 03/10/2012.
//  Copyright (c) 2012 Goldsmiths, University of London. EAVI. All rights reserved.
//

#include <iostream>


/*
 Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
 Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 4 -a 1.7578125000e-02 0.0000000000e+00 -l
 
 You specified the following parameters:
 filtertype	 =	 Butterworth
 passtype	 =	 Lowpass
 ripple	 =	
 order	 =	 4
 samplerate	 =	 256
 corner1	 =	 4.5
 corner2	 =	
 adzero	 =	
 logmin	 =	
 
 #define NZEROS 4
 #define NPOLES 4
 #define GAIN   1.237233240e+05
 
 static float xv[NZEROS+1], yv[NPOLES+1];
 
 static void filterloop()
 { for (;;)
 { xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
 xv[4] = next input value / GAIN;
 yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
 yv[4] =   (xv[0] + xv[4]) + 4 * (xv[1] + xv[3]) + 6 * xv[2]
 + ( -0.7491747680 * yv[0]) + (  3.2128660223 * yv[1])
 + ( -5.1752804900 * yv[2]) + (  3.7114599149 * yv[3]);
 next output value = yv[4];
 }
 }
 */

#define NZEROS 4
#define NPOLES 4
#define GAIN   1.237233240e+05

static float xv[NZEROS+1], yv[NPOLES+1];

static inline float lpfilter45(float v) {
    xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
    xv[4] = v / GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
    yv[4] =   (xv[0] + xv[4]) + 4 * (xv[1] + xv[3]) + 6 * xv[2]
    + ( -0.7491747680 * yv[0]) + (  3.2128660223 * yv[1])
    + ( -5.1752804900 * yv[2]) + (  3.7114599149 * yv[3]);
    return yv[4];    
}


/*
You specified the following parameters:
filtertype	 =	 Butterworth
passtype	 =	 Bandstop
ripple	 =	
order	 =	 4
samplerate	 =	 256
corner1	 =	 48
corner2	 =	 52
adzero	 =	
logmin	 =	
Results



Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
 Command line: /www/usr/fisher/helpers/mkfilter -Bu -Bs -o 4 -a 1.8750000000e-01 2.0312500000e-01 -l 
*/
#define NZEROSNOTCH 8
#define NPOLESNOTCH 8
#define GAINNOTCH   1.136930320e+00

static float xvnotch[NZEROSNOTCH+1], yvnotch[NPOLESNOTCH+1];

static inline float filterNotch50Hz(float input) {
    xvnotch[0] = xvnotch[1]; xvnotch[1] = xvnotch[2]; xvnotch[2] = xvnotch[3]; xvnotch[3] = xvnotch[4]; xvnotch[4] = xvnotch[5]; xvnotch[5] = xvnotch[6]; xvnotch[6] = xvnotch[7]; xvnotch[7] = xvnotch[8]; 
    xvnotch[8] = input / GAINNOTCH;
    yvnotch[0] = yvnotch[1]; yvnotch[1] = yvnotch[2]; yvnotch[2] = yvnotch[3]; yvnotch[3] = yvnotch[4]; yvnotch[4] = yvnotch[5]; yvnotch[5] = yvnotch[6]; yvnotch[6] = yvnotch[7]; yvnotch[7] = yvnotch[8]; 
    yvnotch[8] =   (xvnotch[0] + xvnotch[8]) -   2.6983691309 * (xvnotch[1] + xvnotch[7]) +   6.7304484875 * (xvnotch[2] + xvnotch[6])
    -   9.3230670449 * (xvnotch[3] + xvnotch[5]) +  11.6679900010 * xvnotch[4]
    + ( -0.7736282195 * yvnotch[0]) + (  2.1544643168 * yvnotch[1])
    + ( -5.5436864413 * yvnotch[2]) + (  7.9278080943 * yvnotch[3])
    + (-10.2416524940 * yvnotch[4]) + (  8.4530545984 * yvnotch[5])
    + ( -6.3025540854 * yvnotch[6]) + (  2.6118549514 * yvnotch[7]);
    return yvnotch[8];
}