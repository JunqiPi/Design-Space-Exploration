#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <math.h>
#include <fcntl.h>

#include "431project.h"

/* 
Configerations incoming:
const std::string GLOB_baseline="0 0 0 0 0 0 5 0 5 0 2 2 2 3 0 0 3 0";
const std::string GLOB_dimensionnames[18]={"width","fetchspeed","scheduling","RUUsize","LSQsize","Memports","dl1sets","dl1assoc","il1sets","il1assoc","ul2sets","ul2blocksize","ul2assoc","tlbsets","dl1lat","il1lat","ul2lat","bpred"};
const unsigned int GLOB_dimensioncardinality[18]={4,2,2,6,4,2,9,3,9,3,10,4,5,5,7,7,9,6};
const std::string GLOB_fields[7]={"sim_num_insn ", "sim_cycle ","il1.accesses ","dl1.accesses ","ul2.accesses ","ul2.misses ","ul2.writebacks "};
const std::string GLOB_prefixes[5]={"0.","1.","2.","3.","4."};
*/

/*
p
0  width=( "1" "2" "4" "8" )
2  fetchspeed=( "1" "2" )
1  scheduling=( "-issue:inorder true -issue:wrongpath false" "-issue:inorder false -issue:wrongpath true" )
5  ruusize=( "4" "8" "16" "32" "64" "128" )
3  lsqsize=( "4" "8" "16" "32" )
1  memport=( "1" "2" )
2  dl1sets=( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
0  dl1assoc=( "1" "2" "4" )
6  il1sets=( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
0  il1assoc=( "1" "2" "4" )
3  ul2sets=( "256" "512" "1024" "2048" "4096" "8192" "16384" "32768" "65536" "131072" )
0  ul2block=( "16" "32" "64" "128" )
3  ul2assoc=( "1" "2" "4" "8" "16" ) 
  tlbsets=( "4" "8" "16" "32" "64" )
  dl1lat=( "1" "2" "3" "4" "5" "6" "7" )
  il1lat=( "1" "2" "3" "4" "5" "6" "7" )
  ul2lat=( "5" "6" "7" "8" "9" "10" "11" "12" "13" )
5  branchsettings=( "-bpred perfect" "-bpred nottaken" "-bpred bimod -bpred:bimod 2048" "-bpred 2lev -bpred:2lev 1 1024 8 0" "-bpred 2lev -bpred:2lev 4 256 8 0" "-bpred comb -bpred:comb 1024" )

0  l1block=( "8" "16" "32" "64" ) 

1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192 
*/
using namespace std;

int width[]={1,2,4,8};
//scheduling=( "-issue:inorder true -issue:wrongpath false" "-issue:inorder false -issue:wrongpath true" )
int fetchspeed[]={1,2};
int ruusize[]={ 4,8,16,32,64,128};
int lsqsize[]={4,8,16,32};//( "4" "8" "16" "32" )
int memport[]={1,2};//( "1" "2" )
int dl1sets[]={32,64,128,256,512,1024,2048,4096,8192};//( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
int dl1assoc[]={1,2,4};//( "1" "2" "4" )
int il1sets[]={32,64,128,256,512,1024,2048,4096,8192};//( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
int il1assoc[]={1,2,4};//( "1" "2" "4" )
int ul2sets[]={256,512,1024,2048,4096,8192,16384,32768,65536,131072};//( "256" "512" "1024" "2048" "4096" "8192" "16384" "32768" "65536" "131072" )
int ul2block[]={16,32,64,128};//( "16" "32" "64" "128" )
int ul2assoc[]={1,2,4,8,16};//( "1" "2" "4" "8" "16" ) 
int tlbsets[]={4,8,16,32,64};//( "4" "8" "16" "32" "64" )
int dl1lat[]={1,2,3,4,5,6,7};//( "1" "2" "3" "4" "5" "6" "7" )
int il1lat[]={1,2,3,4,5,6,7};//( "1" "2" "3" "4" "5" "6" "7" )
int ul2lat[]={5,6,7,8,9,10,11,12,13};//( "5" "6" "7" "8" "9" "10" "11" "12" "13" )
//String branchsettings[]={"-bpred perfect" ,"-bpred nottaken" ,"-bpred bimod -bpred:bimod 2048" ,"-bpred 2lev -bpred:2lev 1 1024 8 0" ,"-bpred 2lev -bpred:2lev 4 256 8 0" ,"-bpred comb -bpred:comb 1024"};
//( "-bpred perfect" "-bpred nottaken" "-bpred bimod -bpred:bimod 2048" "-bpred 2lev -bpred:2lev 1 1024 8 0" "-bpred 2lev -bpred:2lev 4 256 8 0" "-bpred comb -bpred:comb 1024" )
int l1block[]={8,16,32,64};//( "8" "16" "32" "64" ) 

int get_dl1size(std::string configuration){
  unsigned int dl1sets = 32 << atoi((configuration.substr(2*6,1)).c_str());
  unsigned int dl1assoc = 1 << atoi((configuration.substr(2*7,1)).c_str());
  unsigned int dl1blocksize = 8 * (1 << (atoi((configuration.substr(0,1)).c_str())));
  return dl1assoc*dl1sets*dl1blocksize;
}

int get_il1size(std::string configuration){
  unsigned int il1sets = 32 << atoi((configuration.substr(2*8,1)).c_str());
  unsigned int il1assoc = 1 << atoi((configuration.substr(2*9,1)).c_str());
  unsigned int il1blocksize = 8 * (1 <<(atoi((configuration.substr(0,1)).c_str())));
  return il1assoc*il1sets*il1blocksize;
}

int get_l2size(std::string configuration){
  unsigned int l2sets = 256 << atoi((configuration.substr(2*10,1)).c_str());
  unsigned int l2blocksize =  16 << (atoi((configuration.substr(2*11,1)).c_str()));
  unsigned int l2assoc = 1 << atoi((configuration.substr(2*12,1)).c_str());
  return l2assoc*l2sets*l2blocksize;
}

/*
 * Returns 1 if valid, else 0
 */
int validateConfiguration(std::string configuration){
  int configurationDimsAsInts[18];
  int returnValue=1; // assume true, set to zero otherwise
  if(isan18dimconfiguration(configuration)){ // necessary, but insufficient
    extractConfiguration(configuration, configurationDimsAsInts); // Configuration parameters now available in array
    int Width=width[configurationDimsAsInts[0]];
    int fetchSpeed = fetchspeed[configurationDimsAsInts[1]];

		int ruuSize = ruusize[configurationDimsAsInts[3]];   
		int lsqSize = lsqsize[configurationDimsAsInts[4]];
		int memPorts = configurationDimsAsInts[5]+1;

    int dl1Sets=dl1sets[configurationDimsAsInts[6]];
    int dl1Assoc=dl1assoc[configurationDimsAsInts[7]];
    int il1Sets=il1sets[configurationDimsAsInts[8]];
    int il1Assoc=il1assoc[configurationDimsAsInts[9]];

    int ul2Sets = ul2sets[configurationDimsAsInts[10]];
		int ul2BlockSize = ul2block[configurationDimsAsInts[11]];
		int ul2Assoc = ul2assoc[configurationDimsAsInts[12]];
		int tlbSets = tlbsets[configurationDimsAsInts[13]];
		int dl1Lat = configurationDimsAsInts[14]+1;
		int il1Lat = configurationDimsAsInts[15]+1;
		int ul2Lat = ul2lat[configurationDimsAsInts[16]];
    

    int il1Size = (int)get_il1size(configuration);
    int dl1Size = (int)get_dl1size(configuration);
    int ul2Size = (int)get_l2size(configuration);			
            
    int il1BlockSize = (int)(il1Size/(il1Sets*il1Assoc));	
    int dl1BlockSize = (int)(dl1Size/(dl1Sets*dl1Assoc));
    
    if (il1BlockSize != Width*8) { //The il1 block size must match the ifq size
      return 0; 	
      if (il1BlockSize != dl1BlockSize) { //The dl1 should have the same block size as your il1
        return 0;
	    }
  	}

    if (ul2BlockSize < 2*il1BlockSize||ul2BlockSize>128*8) { //The ul2 block size must be at least twice your il1 (dl1) block size with a maximum block size of 128B
      if (ul2Size < (il1Size+dl1Size)) { //ul2 must be at least as large as il1+dl1 in order to be inclusive.
        return 0; 	
      }
		  return 0;
	  }
    /*
    The il1 sizes and il1 latencies are linked as follows (the same linkages hold for the dl1 size and dl1 latency):
    il1 = 8 KB (baseline, minimum size) means il1lat = 1
    il1 = 16 KB means il1lat = 2
    il1 = 32 KB means il1lat = 3
    il1 = 64 KB (maximum size) means il1lat = 4
    The above are for direct mapped caches. 
    */

    int additioncycle=0;
    if (il1Assoc == 2) { //For 2-way set associative add an additional cycle of latency to each of the above; 
      additioncycle = 1;
    }
    else if (il1Assoc == 4) {//for 4-way (maximum) add two additional cycles.
      additioncycle = 2;
    }

    if ((il1Size == 8*1024 && il1Lat != 1 + additioncycle)||(il1Size == 16*1024 && il1Lat != 2 + additioncycle)
              ||(il1Size == 32*1024 && il1Lat != 3 + additioncycle)||(il1Size == 64*1024 && il1Lat != 4 + additioncycle)||(il1Size > 64*1024)) {
      return 0;
    }

    additioncycle=0;
    if (dl1Assoc == 2) { //For 2-way set associative add an additional cycle of latency to each of the above; 
      additioncycle = 1;
    }
    else if (dl1Assoc == 4) {//for 4-way (maximum) add two additional cycles.
      additioncycle = 2;
    }

    if ((dl1Size == 8*1024 && il1Lat != 1 + additioncycle)||(dl1Size == 16*1024 && il1Lat != 2 + additioncycle)
              ||(dl1Size == 32*1024 && il1Lat != 3 + additioncycle)||(dl1Size == 64*1024 && il1Lat != 4 + additioncycle)||(dl1Size > 64*1024)) {
      return 0;
    }

    /* The ul2 sizes and ul2 latencies are linked as follows:
    ul2 = 128 KB (minimum size) means ul2lat = 7
    ul2 = 256 KB (baseline) means ul2lat = 8
    ul2 = 512 KB means ul2 lat = 9
    ul2 = 1024 KB (1 MB) means ul2lat = 10
    ul2 = 2 MB (maximum size) means ul2lat = 11
    The above are for 4-way set associative caches. For 8-way set associative add one additional cycle of latency to each of the above; for 16 way (maximum) add two additional cycles; for 2-way set associative subtract 1 cycle; for direct mapped subtract 2 cycles.
    */

    additioncycle = 0;
    if(ul2Assoc==1){
      additioncycle=-2;
    }
    if(ul2Assoc==4){
      additioncycle=0;
    }
    if (ul2Assoc == 8) {
      additioncycle = 1;
    }
    else if (ul2Assoc == 16) {
      additioncycle = 2;
    }
    else if (ul2Assoc == 2) {
      additioncycle = -1;
    }
    
    if ((ul2Size == 128*1024 && ul2Lat != 7 + additioncycle)
              || (ul2Size == 256*1024 && ul2Lat != 8 + additioncycle)
              || (ul2Size == 512*1024 && ul2Lat != 9 + additioncycle)
              || (ul2Size == 1024*1024 && ul2Lat != 10 + additioncycle)
        || (ul2Size == 2048*1024 and ul2Lat != 11 + additioncycle)||(il1Size > 64*1024)) {
      return 0;
    }

    /*
        mplat is fixed at 3
        fetch:speed ratio of no more than 2
        ifqsize can be set to a maximum of 8 words (64B)
        decode:width and issue:width equal to your fetch:ifqsize??
        mem:width is fixed at 8B (memory bus width)??
        memport can be set to a maximum of 2
        mem:lat is fixed at 51 + 7 cycles for 8 word
        tlb:lat is fixed at 30, maximum tlb size of 512 entries for a 4-way set associative tlb
        ruu:size maximum of 128 (assume must be a power of two)
        lsq:size maximum of 32 (assume must be a power of two)
    */
    if ((fetchSpeed > 2)||(Width > 8)||(memPorts > 2)||(ruuSize > 128)||(lsqSize > 32)) { 
        return 0;
    }

  } else {
    returnValue=0;
  }

  return returnValue;
}



/*
 * Given the current best known configuration, the current configuration, and the globally visible map of all previously
  investigated configurations, suggest a previously unexplored design point. 
 You will only be allowed to investigate 1000 design points in a particular run, so choose wisely.
 */
 int currentDim=0;
std::string YourProposalFunction(std::string currentconfiguration, std::string bestEXECconfiguration, std::string bestEDPconfiguration, int optimizeforEXEC, int optimizeforEDP){
  std::string nextconfiguration=GLOB_baseline;
  /*
   * REPLACE THE BELOW CODE WITH YOUR PROPOSAL FUNCTION
   *
   * The proposal function below is extremely unintelligent and
   * will produce configurations that, while properly formatted, violate specified project constraints
   */    
  
  // produces an essentially random proposal
  std::stringstream ss;

	int currentConfig[18]; //current configuration extract
  extractConfiguration(currentconfiguration, currentConfig);  
	int bestEXECConfig[18]; //the best exc design
  extractConfiguration(bestEXECconfiguration, bestEXECConfig);
	int bestEDPConfig[18]; //the best energy design
  extractConfiguration(bestEDPconfiguration, bestEDPConfig);
	int nextConfig[18];

  std::string tempconfig;
  if(optimizeforEXEC){ //“best” performance-oriented design
      for (int i = 0; i < currentDim; ++i) {
        nextConfig[i]=bestEXECConfig[i];
      }
  }else if(optimizeforEDP){ //most energy-efficient design
      for (int i = 0; i < currentDim; ++i) {
        nextConfig[i]=bestEXECConfig[i];
      }
  }

  int next_trial=currentConfig[currentDim]+1; 
  if (next_trial >= GLOB_dimensioncardinality[currentDim]) {
			next_trial = GLOB_dimensioncardinality[currentDim] - 1;
      currentDim++;
	}

  nextConfig[currentDim]=next_trial;
  for (int i = (currentDim+ 1); i < 18; ++i) {
		nextConfig[i]=0;
  }
  
  nextConfig[1]=1; //fetch speed should always be optimized
  nextConfig[5]=1; //mem port should be maximized for speed
  nextConfig[13]=4; //Tlb sets need to be as many as posible
  if(currentDim==1||currentDim==5||currentDim==13){
    currentDim++;
  }

	// produces an essentially random proposal
  	for(int dim = 0; dim<17; ++dim){
    		ss << nextConfig[dim] << " ";
  	} 
  	ss << nextConfig[17];
  	nextconfiguration=ss.str();
  	ss.str("");    
  	return nextconfiguration;
}

