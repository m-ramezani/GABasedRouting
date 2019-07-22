/*
 * BaseMod.h
 *
 *  Created on: ??? ?, ????
 *      Author: persian co
 */

#ifndef BASEMOD_H_
#define BASEMOD_H_
#define MIXIM_INET
#include <cassert>
#include <omnetpp.h>
#include <sstream>


//#endif /* BASEMOD_H_ */
#include "myNetwL.h"
#include "cmessage.h"
#include "csimplemodule.h"
#include "SimpleAddress.h"
#include "State.h"
#include "CMAC.h"

#define CROSSOVER_RATE            0.8
#define MUTATION_RATE             0.05
#define POP_SIZE                  50           //must be an even number
#define CHROMO_LENGTH             100
#define GENE_LENGTH               4
#define MAX_ALLOWABLE_GENERATIONS   30

class GA: public BaseModule
{
private:
    int McastSrcCnt;
    int McastDestPerSrc;
    int len;
    typedef std::list<int> NodeList;
    NodeList nodeList;
    typedef std::list<LAddress::L3Type> TreeNodeList;
    TreeNodeList treeNodeList;
    std::list<int>::iterator it1, it2;
    LAddress::L3Type SourceIp;
    enum MSG_Type
   {
        CREATE_GROUP
    };

    struct Path_struct
    {
        LAddress::L3Type NodeList[100];
        bool HasSharedNode;
        int len;

    };
    struct chromosom_typ
    {
        LAddress::L3Type Source;
        LAddress::L3Type  Dest[50];
        Path_struct Path[50];
        double fitness;
        bool isSelected;
    };
    chromosom_typ Population[POP_SIZE];
    chromosom_typ Solution;
    LAddress::L3Type FirstNiebOfSource[];

public:
  struct McastGroup
  {
     LAddress::L3Type McastAdr;
     LAddress::L3Type Source;
     LAddress::L3Type Dest[100];
  };
  McastGroup McastGroupArray[10];
  virtual void initialize(int stage);
  virtual void handleMessage(cMessage *msg);
  virtual void setMcastGroups();
  virtual void InitializePopulation();
  void setFitness(chromosom_typ T);
  void assignChannel(chromosom_typ T);
  LAddress::L3Type getRandomNode(LAddress::L3Type ip);
  bool IterativeNode(LAddress::L3Type ip);
  bool IsMemOfPath(LAddress::L3Type path[],LAddress::L3Type ip);
  void assignChanneltoSenderRadio(LAddress::L3Type ip, int ch);
  void assignChanneltoReiciverRadio(LAddress::L3Type ip, int ch);
  int getConflict(LAddress::L3Type ip);
  int getNieb1Conflict(LAddress::L3Type ip, int dMacCh[], int uMacCh[]);
  int getNieb2Conflict(LAddress::L3Type ip, int dMacCh[], int uMacCh[]);
  void TournamentSelection(chromosom_typ pop[], int k, chromosom_typ &res);
  void crossover(chromosom_typ &T1, chromosom_typ &T2);
  void mutate(chromosom_typ &T);
  void MainFunc();
  void setForwarderNodes();
  LAddress::L3Type setForwarder(LAddress::L3Type ip);
  void testFunc();

 // virtual void finish();
};
#endif /* BASEMOD_H_ */
