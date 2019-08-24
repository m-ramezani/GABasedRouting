/*
 * BaseMod.cc
 *
 *  Created on: ??? ?, ????
 *      Author: Mahlagha
 */




#include <GA.h>
#include <math.h>


Define_Module(GA);

void GA::initialize(int stage)
{
    if(stage == 0)
       {
        ev <<"*********** "<< POP_SIZE << endl;
        cModule *parent = getParentModule();
        len = parent->par("numNodes");
        McastDestPerSrc = par("McastDestPerSrc");
        McastSrcCnt = par("McastSrcCnt");
        for (int i = 0; i <len; ++i) {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           if(i == 26)
             netl->setParentType("Source");
           else if(i == 7 || i == 15 || i == 44)// Number of Destination=3

           netl->setParentType("Destination");
           else
               netl->setParentType("Intermediate");
           nodeList.push_back(node->getId());
         }

       }

       else if(stage == 1)
       {
        setMcastGroups();
       }
}

///////////////////////////////////////////////
void GA::setMcastGroups()
{
    LAddress::L3Type MulticastAdr = 10001;
    cModule *parent = getParentModule();
    EV << "Setting multicast group..." << endl;
    for(int k = 0; k < McastSrcCnt; ++k)
    {
       // EV << "Multicast source node:  " <<  *it1 << endl;
        for(int i=0; i<len; i++)
        {
            cModule *node = parent->getSubmodule("node",i);
            myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           // if(strcmp(netl->getParentType() , "Source") == 0)
            if(netl->getParentType() == "Source")
            {
               // netl->setParentType("Source");
                McastGroupArray[k].Source = netl->getId();
                McastGroupArray[k].McastAdr = MulticastAdr;
                MulticastAdr = MulticastAdr + 10;
                SourceIp = netl->getId();
                break;
            }
        }
       int l = 0;
      // for (int l = 0; l < McastDestPerSrc; ++l) {
            for(int i=0; i<len; i++)
               {
                   cModule *node = parent->getSubmodule("node",i);
                   myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
                  //if(strcmp(netl->getParentType() , "Destination") == 0)
                  if(netl->getParentType() == "Destination")
                   {
                       McastGroupArray[k].Dest[l] = netl->getId();
                       l++;
                   }
               }
         // }
   }
    ////////////////////////////////
    for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           for(int m =0; m<McastSrcCnt; m++){
               netl->McastGroupArray[m].Source = McastGroupArray[m].Source;
               netl->McastGroupArray[m].McastAdr = McastGroupArray[m].McastAdr;
               for(int n =0; n<McastDestPerSrc; n++){
                   netl->McastGroupArray[m].Dest[n] = McastGroupArray[m].Dest[n];

               }
           }
       }
}

///////////////////////////////////////////////
void GA::InitializePopulation()
{
    int cnt=0;
    bool FirstNiebOfSource = true;
    bool Iterative = false;
    LAddress::L3Type target;

    for(int i=0;i<POP_SIZE;i++)
    {
     Population[i].Source = SourceIp;
     Population[i].isSelected = false;//
     Population[i].fitness = 0;
     for(int j=0;j<McastDestPerSrc;j++)
     {
        Population[i].Dest[j] = 0;
        for(int k=0; k<100; k++)
        {
            Population[i].Path[j].NodeList[k] = 0;
            Population[i].Path[j].HasSharedNode = false;
            Population[i].Path[j].len = 0;
        }
     }
    }
    /////////////////////////
    cModule *parent = getParentModule();
       for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           netl->extractIP();
      }
    /////////////////////////
    for(int i=0;i<POP_SIZE;i++)
    {

        treeNodeList.push_back(Population[i].Source);
        for(int j=0;j<McastDestPerSrc;j++)
           {
             Population[i].Path[j].NodeList[0] = Population[i].Source;
             cnt=1;
             //Population[i].Source = McastGroupArray[j].McastAdr;
             Population[i].Dest[j] = McastGroupArray[j].Dest[j];
             target = Population[i].Source;
             while(target != Population[i].Dest[j])
             {
                target = getRandomNode(target);
              if(FirstNiebOfSource == true)
                {
                    if(IterativeNode(target))
                    {
                       for(int l=0; l<McastDestPerSrc; l++)
                       {
                           if(Population[i].Path[l].NodeList[1] == target)
                               Population[i].Path[l].HasSharedNode = true;
                       }
                    }
                    FirstNiebOfSource = false;
                }
                else if(IterativeNode(target))
                {
                    for(int l=0; l<McastDestPerSrc; l++)
                       {
                         if(Population[i].Path[l].HasSharedNode == true && IsMemOfPath(Population[i].Path[l].NodeList,target))
                          Iterative = false;
                       }
                    if(Iterative == true)
                      target = getRandomNode(target);
                }
                Population[i].Path[j].NodeList[cnt] = target;
                treeNodeList.push_back(target);
                cnt++;
             }
             Population[i].Path[j].NodeList[cnt] = Population[i].Dest[j];
             Population[i].Path[j].len = cnt + 1;
             FirstNiebOfSource = true;
           }
        treeNodeList.clear();
       // assignChannel(Population[i]);
       //setFitness(Population[i]);
    }
   // MainFunc();
}
///////////////////////////////////////////////////

void GA::setFitness(chromosom_typ T)
{
    int conflict = 0;
    for(int i=0; i<McastDestPerSrc; i++)
    {
        for(int j=0; j<T.Path[i].len; j++)
        {
          conflict = conflict + getConflict(T.Path[i].NodeList[j]);
        }
    }
    T.fitness = 1/double(1 + conflict);
    ev << "fitness= " << T.fitness << endl;

}
/////////////////////////////////////////////////////
int GA::getConflict(LAddress::L3Type ip)
{
    cModule *parent = getParentModule();
       for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           if(netl->getId() == ip)
           {
               for(int i = 0; i < netl->routingTable.size(); ++i)
               {
                  return getNieb1Conflict(netl->routingTable[i].ipAdr,netl->dMacChnl,netl->uMacChnl);
               }
               break;
          }
      }
}
/////////////////////////////////////////////////////
int GA::getNieb1Conflict(LAddress::L3Type ip, int dMacCh[], int uMacCh[])
{
    int value = 0;
    cModule *parent = getParentModule();
       for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           if(netl->getId() == ip)
           {
               for(int k=0; k<netl->nicCount;k++)
                 {
                   if(netl->dMacChnl[k] == dMacCh[k])
                      value++;
                   if(netl->uMacChnl[k] == uMacCh[k])
                     value++;
                 }
               /////////////////////////
               for(int n = 0; n < netl->routingTable.size(); ++n)
                 {
                     value = value + getNieb2Conflict(netl->routingTable[n].ipAdr,netl->dMacChnl,netl->uMacChnl);
                 }
               ///////////////////
           break;
           }

       }
 return value;
}
/////////////////////////////////////////////////////
int GA::getNieb2Conflict(LAddress::L3Type ip, int dMacCh[], int uMacCh[])
{
    int value = 0;
    cModule *parent = getParentModule();
       for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           if(netl->getId() == ip)
           {
               for(int k=0; k<netl->nicCount;k++)
                 {
                   if(netl->dMacChnl[k] == dMacCh[k])
                      value++;
                   if(netl->uMacChnl[k] == uMacCh[k])
                     value++;
                 }
             break;
           }
       }
 return value;
}
/////////////////////////////////////////////////////
void GA::assignChannel(chromosom_typ T)
{
   int channel = 0;
   for(int i=0; i<McastDestPerSrc; i++)
   {
       for(int j=0; j<T.Path[i].len - 2; j++)
       {
         channel = (j)%3;
         switch (channel) {
            case 0:
                channel = 1;
                break;
            case 1:
                channel = 6;
                break;
            case 2:
                channel = 11;
                break;
            default:
                break;
        }
         assignChanneltoSenderRadio(T.Path[i].NodeList[j],channel);
         assignChanneltoReiciverRadio(T.Path[i].NodeList[j+1],channel);
       }
   }
}
///////////////////////////////////////////////////
void GA::assignChanneltoSenderRadio(LAddress::L3Type ip, int ch)
{
    ev<<"assigning Channel to Sender Radios..."<<endl;
    simtime_t switchtime_channel;
    cModule *parent = getParentModule();
    for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           if(netl->getId() == ip)
           {
               if(netl->AssignedChannel == true)
                   return;
               for(int j=0; j<netl->nicCount;j++)
               {
                 cModule *dnic = netl->getSubmodule("dnic",j);
                 Mac80211MultiChannel *mac = (Mac80211MultiChannel*) dnic->getSubmodule("mac",0);
                 mac->switchChannel(ch);
                 netl->dMacChnl[j] = ch;
                 ev << "MAC Channel down = " << mac->getChannel() << endl;
               }
               netl->AssignedChannel = true;
               break;
           }
       }
}
///////////////////////////////////////////////////
void GA::assignChanneltoReiciverRadio(LAddress::L3Type ip, int ch)
{
    ev<<"assigning Channel to receiver Radios..."<<endl;
    simtime_t switchtime_channel;
    cModule *parent = getParentModule();
    for(int i=0; i<len; i++)
       {
           cModule *node = parent->getSubmodule("node",i);
           myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
           if(netl->getId() == ip)
           {
               if(netl->AssignedChannel == true)
                 return;
               for(int j=0; j<netl->nicCount;j++)
               {
                 cModule *unic = netl->getSubmodule("unic",j);
                 Mac80211MultiChannel *mac = (Mac80211MultiChannel*) unic->getSubmodule("mac",0);
                 mac->switchChannel(ch);
                 netl->uMacChnl[j] = ch;
                 ev << "MAC Channel up = " << mac->getChannel() << endl;
               }
               break;
           }
       }

}
///////////////////////////////////////////////////
bool GA::IsMemOfPath(LAddress::L3Type path[],LAddress::L3Type ip)
{
    for(int i=0; i<100;i++)
    {
        if(path[i] == ip)
            return true;
    }
  return false;
}
///////////////////////////////////////////////////
bool GA::IterativeNode(LAddress::L3Type ip)
{
    std::list<LAddress::L3Type>::iterator it;
    it = treeNodeList.begin();
    if(treeNodeList.size() != 0)
    {
        for(int i=0; i<treeNodeList.size(); ++i)
        {
            if(*it == ip)
                return true;
            ++it;
        }
    }
    return false;
}
//////////////////////////////////////////////////
LAddress::L3Type GA::getRandomNode(LAddress::L3Type ip)
{
    int r1 = 0;

    cModule *parent = getParentModule();
  /*  for(int i=0; i<len; i++)
    {
        cModule *node = parent->getSubmodule("node",i);
        myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
        if(netl->getId() == ip)
        {
            r1 = rand() % (netl->row);
            ev<<"my random node= "<<netl->NiebList[r1];
            return  netl->NiebList[r1];
            break;
       }
   }*/
}
//////////////////////////////////////////////////
void GA::MainFunc()
{
  int GenerationsRequiredToFindASolution = 0;
  int k = 2;  //k = 0.2*POP_SIZE;
  //int NEW_POP_SIZE = 20;
  int cnt = 0;
  chromosom_typ new_Population[POP_SIZE];
  chromosom_typ parent1, parent2;
  double fitness = 0;
  bool found = false;
  double crossoverProb = 0;
  double mutationProb = 0;

  while(!found)
   {

       cnt = 0;
       while(cnt < POP_SIZE)
       {
          TournamentSelection(Population, k, parent1);
          TournamentSelection(Population, k, parent2);
          crossoverProb =  ((double) rand() / (RAND_MAX +1));
          mutationProb =  ((double) rand() / (RAND_MAX + 1));

          if(crossoverProb < CROSSOVER_RATE)
              crossover(parent1, parent2);
          if(mutationProb < MUTATION_RATE)
            {
               mutate(parent1);
               mutate(parent2);
            }
          assignChannel(parent1);
          assignChannel(parent2);
          setFitness(parent1);
          setFitness(parent2);
          new_Population[cnt]=parent1;
          cnt++;
          new_Population[cnt]=parent2;
          cnt++;
       }
       ///////////////////////////coping
       for(int i=0;i<POP_SIZE;i++)
           {
            Population[i].isSelected = false;//
            Population[i].fitness = new_Population[i].fitness;
            for(int j=0;j<McastDestPerSrc;j++)
            {
              // Population[i].Dest[j] = 0;
               for(int k=0; k<Population[i].Path[j].len; k++)
               {
                   Population[i].Path[j].NodeList[k] = new_Population[i].Path[j].NodeList[k];
                   Population[i].Path[j].HasSharedNode = false;
                   Population[i].Path[j].len = new_Population[i].Path[j].len;
               }
            }
            if(Population[i].fitness > fitness)//search for solution
               {
                   fitness =  Population[i].fitness;
                   Solution = Population[i];
               }
           }
        ///////////////////////////////////////
       ++GenerationsRequiredToFindASolution;
       if (GenerationsRequiredToFindASolution > MAX_ALLOWABLE_GENERATIONS)
        {
            ev << "No solutions found this run!" << endl;
            found = true;
        }
       ///////////////
   }
}
////////////////////////////////////////////////////
void GA::TournamentSelection(chromosom_typ pop[], int k, chromosom_typ &res)
{
  int r1 = 0, r2 = 0;
  double fitness = 0;
  //chromosom_typ res;
  for(int i=0; i<k; i++)
  {
    r1 = rand() % (POP_SIZE);
    while(Population[r1].isSelected == true)
    {
        r1 = rand() % (POP_SIZE);
    }
    Population[r1].isSelected = true;
    if(Population[r1].fitness > fitness)
    {
        fitness = Population[r1].fitness;
        res = Population[r1];
    }
  }
 // return res;
}
///////////////////////////////////////////////////
void GA::crossover(chromosom_typ &T1, chromosom_typ &T2)
{
  int r = rand() % (McastDestPerSrc);
  int index1=0, index2=0;
  bool flag = false;
  LAddress::L3Type temp1[50];
  LAddress::L3Type temp2[50];
  for(int i=0; i<50; i++)
  {
      temp1[i] = temp2[i] = 0;
  }

  for(int i=1; i<T1.Path[r].len - 1; i++)
  {
      for(int j=1; j<T2.Path[r].len - 1; j++)
        {
          if(T1.Path[r].NodeList[i] == T2.Path[r].NodeList[j])
          {
             index1 = i;
             index2 = j;
            // LAddress::L3Type temp1[(T1.Path[r].len - 1) - i];
            // LAddress::L3Type temp2[(T2.Path[r].len - 1) - j];
             flag = true;
             break;
          }
        }
      if(flag == true)
        break;
  }
  /////////////////if tow path has shared node
  if(flag == true){
  for(int i=index1 + 1; i<T1.Path[r].len - 1; i++)
    {
       temp1[i] = T1.Path[r].NodeList[i];
       T1.Path[r].NodeList[i] = 0;
    }

  for(int j=index2 + 1; j<T2.Path[r].len - 1; j++)
   {
      temp2[j] = T2.Path[r].NodeList[j];
      T2.Path[r].NodeList[j] = 0;
   }

  /////////////////
  for(int i=index1 + 1; i<T2.Path[r].len - 1; i++)
      {
         T1.Path[r].NodeList[i] = temp2[i];
         T1.Path[r].len = T2.Path[r].len;
      }

    for(int j=index2 + 1; j<T1.Path[r].len - 1; j++)
     {
        T2.Path[r].NodeList[j] = temp1[j];
        T2.Path[r].len = T1.Path[r].len;
     }
  }

}
///////////////////////////////////////////////////
void GA::mutate(chromosom_typ &T)
{
    int r1 = rand() % (McastDestPerSrc);
    int r2 = rand() % (T.Path[r1].len - 3) + 1;

    while(T.Path[r1].NodeList[r2] != T.Path[r1].NodeList[len - 1])
    {
        T.Path[r1].NodeList[r2] = getRandomNode(T.Path[r1].NodeList[r2 -1]);
        r2++;
    }
    T.Path[r1].NodeList[r2] = T.Dest[r1];
    T.Path[r1].len = r2 + 1;
}
///////////////////////////////////////////////////
void GA::setForwarderNodes()
{
    for(int j=0;j<McastDestPerSrc;j++)
    {
       for(int k=1; k<Solution.Path[j].len - 1; k++)
       {
         setForwarder(Solution.Path[j].NodeList[k]);
       }
    }

}
///////////////////////////////////////////////////
LAddress::L3Type GA::setForwarder(LAddress::L3Type ip)
{
    cModule *parent = getParentModule();
    for(int i=0; i<len; i++)
    {
        cModule *node = parent->getSubmodule("node",i);
        myNetwL *netl = (myNetwL*)node->getSubmodule("NetwL",0);
        if(netl->getId() == ip)
        {
            for(int j = 0; j<McastSrcCnt;j++){
                if(netl->McastGroupArray[j].Source == Solution.Source){
                    netl->McastGroupArray[j].isForwarder = true;
                    ev<< "I am forwarder: " << ip <<endl;
                }
            }
            break;
        }
   }
}
///////////////////////////////////////////////////
void GA::handleMessage(cMessage *msg)
{
}
