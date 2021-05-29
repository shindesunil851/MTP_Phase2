
#include <stdio.h>
//#include "mhead.h"
#include <string>     // std::string, std::to_string
#include <bits/stdc++.h>
//#include<map>
using namespace std;




#define maxPCISpeed 266
#define maxPCISpeedPerSlot 62
#define block_size 128
//define PacketSize 480
#define switch_distance 1000
#define brick_distance 1000
#define speed 200000000

//#define int long long int



int num_of_cores=1;
int network_bandwidth=1000000;
int bandwidth=1000000;
int switch_bandwidth=1000000;
int cyclenumber;
int numRequests;       
int number_of_instructions;
int frequency=100000000;
int presentcycle=-1;
int Q_limit=1000;
int PacketSize=128;




deque<pair<pair<int,int>,int>> Tx1,Tx2;
deque<pair<pair<int,int>,int>> Rx1,Rx2;



deque<pair<pair<int,int>,int>> switchTx1;
deque<pair<pair<int,int>,int>> switchRx1;


deque<pair<pair<int,int>,int>> pci_Tx1;
deque<pair<pair<int,int>,int>> pci_Rx1;


deque<pair<pair<int,int>,int>> memTx1,memTx2;
deque<pair<pair<int,int>,int>> memRx1,memRx2;








struct cmpByStringLength {
    bool operator()(const std::string& a, const std::string& b) const {
        
          istringstream ss(a);
  
          string word1; // for storing each word

          istringstream ss2(b);
  
          string word2;

          ss>>word1;
          ss2>>word2;

          //cout<<"word "<<word1<<" "<<word2<<endl;
          return stoi(word1)<stoi(word2);
  
    }
};

// ...
//std::map<std::string, std::string, cmpByStringLength> myMap;


map<string, vector<int> ,cmpByStringLength> dc;

deque<int> threadTx1,threadTx2,threadRx1,threadRx2,thread_switchTx1,thread_switchRx1,thread_memTx1,
thread_memTx2,thread_memRx1,thread_memRx2,thread_pci_Rx1,thread_pci_Tx1;

float CPUClock;
float l1_store_hit;
float l1_load_hit;
float l2_store_hit;
float l2_load_hit;
float llc_load_hit;
float llc_store_hit;



//https://www.youtube.com/watch?v=_Je8K-JcYms&t=1103s
//https://www.erg.abdn.ac.uk/users/gorry/course/lan-pages/nic.html
//https://medium.com/software-design/why-software-developers-should-care-about-cpu-caches-8da04355bb8a

//https://s3-eu-west-1.amazonaws.com/dredbox/IBM_samos2017.pdf




int    level_1_cache()
{
    //design specific--- direct mapped  -- Set Assosiative --- Fully Assosiative


    return 4;

}


int    level_2_cache()
{
    //
    return 11;

}



int    last_level_cache()
{

    return 39;
}



int    local_memory()
{
 return 105;
}



//This module1 used to simulate deleys by caches
// int module_cache()
// {


//     level_1_cache();
//     level_2_cache();
//     last_level_cache();
//     local_memory();

// }





int total_avg_pci_cycles=-1;

int y=-1;

int sub_module_pci_data_phase()
{


    float effectiveBw,d,extra_latency;

    //effectiveBw=bandwidth*(d/(d+extra_latency));

    effectiveBw=bandwidth;
    int data_size=256;


   double fraction=(PacketSize/(double)effectiveBw);

    //cout<<" fr "<< fraction<<" "<<effectiveBw<<"\n";    

    int num_of_cycles_required=abs(fraction)*frequency;

   // cout<<"num of cycles "<<num_of_cycles_required<<"\n";
 
    //remaining task:::::


    //end_phase
    int endphase=1; ///sending out frame down signal



    num_of_cycles_required+=endphase;
    
    if(total_avg_pci_cycles!=-1)
      {
          total_avg_pci_cycles=num_of_cycles_required+5;
          total_avg_pci_cycles*=2;


      }


    return  num_of_cycles_required;


}







int sub_module_pci_addr_phase()
{    
    

    // for medium speed device it takes 5-6 clcok cycles
    
    int activate_frame=1;
    int set_C_BE_line=1;
    int set_A0_A31_lines=1;
    
    //initiator sets IRDY (initiator ready )  and that of target sets TRDY ( target ready)

    int IRDY=1;
    int TRDY=1;
    


    //
    
    // set appropriate values to these vars according to device
    
    //
    //
    int t=activate_frame+set_C_BE_line+set_A0_A31_lines+IRDY+TRDY;



    return activate_frame+set_C_BE_line+set_A0_A31_lines+IRDY+TRDY;
}




int module_pci( int current ,int fl)
{
    // address phase ------> data phase

    int v= current+sub_module_pci_addr_phase()    + sub_module_pci_data_phase();
    
    

    return v;

}




//this function adds entries with apropriate deleys to Tx
int odd=1;

void push_entry(int cnum,int fl,int original_cnum,int thread)
{

    //int presentcycle=-1;
    if(presentcycle!=-1)
    {
          presentcycle=max(presentcycle+1,cnum+1);

    }
    else{

        presentcycle=cnum+1;

    }
    


    if(odd&1)
    {
        Tx1.push_back(make_pair(make_pair(original_cnum,presentcycle),fl));
        threadTx1.push_back(thread);
        odd-=1;
        //
       
     //
      //  cout<<"\n"<<presentcycle<<" "<<original_cnum<<endl;

    }
    else{
        Tx2.push_back(make_pair(make_pair(original_cnum,presentcycle),fl));
        threadTx2.push_back(thread);
        odd+=1;
    }

  


}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//odd=1;
//presentcycle=-1

void push_entry2(int cnum,int fl,int original_cnum,int thread)
{

        //int presentcycle=-1;
        if(presentcycle!=-1)
        {
              presentcycle=max(presentcycle+1,cnum+1);

        }
        else{

            presentcycle=cnum+1;

        }
        

        pci_Rx1.push_back(make_pair(make_pair(original_cnum,presentcycle),fl));
        thread_pci_Rx1.push_back(thread);
        

}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::map<int, int> mp;

int request_preperation_steering()
{



   //map<int, int> mapper; 

   // mapper.insert(make_pair (100,14));
   // mapper.insert(make_pair (200,12));
   
   // mapper.insert(make_pair (300,10));
   // mapper.insert(make_pair (400,8));
   // mapper.insert(make_pair (500,4));
   // mapper.insert(make_pair (1000,2));
   

    
 
    // insert elements in random order
      mp.insert({ 100, 24 });
      mp.insert({ 200, 19 });
      mp.insert({ 300, 17 });
      mp.insert({ 400, 15 });
      mp.insert({ 500, 13 });
      mp.insert({ 1000, 11 });
      mp.insert({ 100000, 7 });
      

      // when 2 is present
      auto it = mp.lower_bound(bandwidth/1000000);

  


    //auto it = lower_bound(bandwidth);
    return (*it).second;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int request_preperation_steering2()
{



   //map<int, int> mapper; 

   // mapper.insert(make_pair (100,14));
   // mapper.insert(make_pair (200,12));
   
   // mapper.insert(make_pair (300,10));
   // mapper.insert(make_pair (400,8));
   // mapper.insert(make_pair (500,4));
   // mapper.insert(make_pair (1000,2));
   

    
 
    // insert elements in random order
      mp.insert({ 100, 15 });
      mp.insert({ 200, 20 });
      mp.insert({ 300, 15 });
      mp.insert({ 400, 9 });
      mp.insert({ 500, 5 });
      mp.insert({ 1000, 4 });
      mp.insert({ 100000000, 3 });
      

      // when 2 is present
      auto it = mp.lower_bound(bandwidth/1000000);

  


    //auto it = lower_bound(bandwidth);
    return (*it).second;

}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void module_nic_at_compute_brick(int cnum,int fl,int original_cnum,int thread)
{

    int extra_cycles=request_preperation_steering();
    push_entry(cnum+extra_cycles,fl,original_cnum,thread);
  


    //as entries are pushed into deques 


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void module_pci_controller_at_compute_brick(int cnum,int fl,int original_cnum,int thread)
{

    int extra_cycles=request_preperation_steering2();

    
    push_entry2(cnum+extra_cycles,fl,original_cnum,thread);
  
  //as entries are pushed into deques 


}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





int br=-1,loop=1;
int nic_init_cost=0;

int check=1;

void process_entries_at_nic(int type)
{

    
    if(type==1)
    {

           int turn =1;  //used for round robin scheduling 
           int pcn=-1;

            while(true)
            {

                if(Tx1.empty() && Tx2.empty())
                {
                      break;
                }

                if(turn&1)
                {

                      if(!Tx1.empty())
                      {
                            
                            
                            int started=Tx1.front().first.first;
                            int curr=Tx1.front().first.second;
                            int fl=Tx1.front().second ;
                            
                            

                            pcn=max(pcn+1,curr+1);

                          

                            //int loading_cycles=(int)((long long)(PacketSize*frequency)/(double)network_bandwidth);//*frequency;

                            double c=PacketSize/(double)network_bandwidth;

                            int loading_cycles=abs(c)*frequency;


                            
                            if(loop==1)
                            {
                                nic_init_cost=curr-started;
                                //cout<<nic_init_cost<<" "<<loading_cycles<<" "<<endl;

                                loop++;
                            }

                            int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                            switchRx1.push_back(make_pair(make_pair(started,pcn+loading_cycles+reaching_switch_cycles),fl));

                        //

                            //cout<<" load cyc  "<<loading_cycles<<"  switch load "<<reaching_switch_cycles<<" "<<((float)PacketSize/network_bandwidth)<<" \n ";
                            

                            int thread=threadTx1.front();

                            thread_switchRx1.push_back(thread);

                            string str1= to_string(started); 

                            string str2= to_string(thread); 


                            string str3=str1+" "+str2;

                            dc[str3].push_back(pcn);

                            
                            threadTx1.pop_front();


                            Tx1.pop_front();
                           
                      }
                      else
                      {
                            
                            
                            int started=Tx2.front().first.first;
                            int curr=Tx2.front().first.second;
                            int fl=Tx2.front().second ;

                            pcn=max(pcn+1,curr+1);


                            double c=PacketSize/(double)network_bandwidth;

                            int loading_cycles=abs(c)*frequency;




                            int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                            switchRx1.push_back(make_pair(make_pair(started,pcn+loading_cycles+reaching_switch_cycles),fl));

                            int thread=threadTx2.front();

                         
                                                   

                            thread_switchRx1.push_back(thread);

                            string str1= to_string(started); 

                            string str2= to_string(thread); 


                            string str3=str1+" "+str2;

                            dc[str3].push_back(pcn);

                            
                            threadTx2.pop_front();





                           Tx2.pop_front();


                      }


                }
                else
                {

                      if(!Tx2.empty())
                       {
                            
                              int started=Tx2.front().first.first;
                              int curr=Tx2.front().first.second;
                              int fl=Tx2.front().second ;

                              pcn=max(pcn+1,curr+1);

                             
                              double c=PacketSize/(double)network_bandwidth;

                              int loading_cycles=abs(c)*frequency;

                              int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                              switchRx1.push_back(make_pair(make_pair(started,pcn+loading_cycles+reaching_switch_cycles),fl));

                              int thread=threadTx2.front();

                              thread_switchRx1.push_back(thread);

                                                       


                              string str1= to_string(started); 

                              string str2= to_string(thread); 


                              string str3=str1+" "+str2;

                              dc[str3].push_back(pcn);

                              
                              threadTx2.pop_front();

                              Tx2.pop_front();

                       } 
                       else
                       {
                            
                              int started=Tx1.front().first.first;
                              int curr=Tx1.front().first.second;
                              int fl=Tx1.front().second ;
                             

                              pcn=max(pcn+1,curr+1);

                              
                              double c=PacketSize/(double)network_bandwidth;

                              int loading_cycles=abs(c)*frequency;

                              int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                              switchRx1.push_back(make_pair(make_pair(started,pcn+loading_cycles+reaching_switch_cycles),fl));

                              
                              int thread=threadTx1.front();

                         


                              thread_switchRx1.push_back(thread);

                              string str1= to_string(started); 

                              string str2= to_string(thread); 


                              string str3=str1+" "+str2;

                              dc[str3].push_back(pcn);

                              
                              threadTx1.pop_front();

                              Tx1.pop_front();
                             

                       }
                }

                turn^=1;





            }
      }

      else
      {
    
        int turn =1;  //used for round robin scheduling 
        int present_cn=-1;


        while(true)
            {

                if(Rx1.empty() && Rx2.empty())
                {
                      break;
                }

                if(turn&1)
                {

                      if(!Rx1.empty())
                      {
                            
                            
                            int started=Rx1.front().first.first;
                            int curr=Rx1.front().first.second;
                            int fl=Rx1.front().second ;
                             
                            present_cn=max(present_cn+1,curr+1);

                         
                            double c=PacketSize/(double)network_bandwidth;

                            int loading_cycles=abs(c)*frequency;

                            //int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                            // switchRx1.push_back(make_pair(make_pair(started,present_cn+loading_cycles+reaching_switch_cycles),fl));

                                                          

                            int thread=threadRx1.front();

                           




                            
                            string str1= to_string(started); 

                            string str2= to_string(thread); 


                            string str3=str1+" "+str2;

                            dc[str3].push_back(present_cn+nic_init_cost);
                           // dc[str3].push_back_back(present_cn);

                            
                            threadRx1.pop_front();


                            Rx1.pop_front();
                           
                      }
                      else
                      {
                            
                            
                            int started=Rx2.front().first.first;
                            int curr=Rx2.front().first.second;
                            int fl=Rx2.front().second ;

                            present_cn=max(present_cn+1,curr+1);

                         
                            double c=PacketSize/(double)network_bandwidth;

                            int loading_cycles=abs(c)*frequency;

                            // int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                            // switchRx1.push_back(make_pair(make_pair(started,present_cn+loading_cycles+reaching_switch_cycles),fl));

                            int thread=threadRx2.front();

                            
                            string str1= to_string(started); 

                            string str2= to_string(thread); 

                            

                            string str3=str1+" "+str2;

                            dc[str3].push_back(present_cn+nic_init_cost);

                            
                            threadRx2.pop_front();

                            Rx2.pop_front();


                      }


                }
                else
                {

                      if(!Rx2.empty())
                       {
                            
                              int started=Rx2.front().first.first;
                              int curr=Rx2.front().first.second;
                              int fl=Rx2.front().second ;

                              present_cn=max(present_cn+1,curr+1);

                              
                            double c=PacketSize/(double)network_bandwidth;

                            int loading_cycles=abs(c)*frequency;

                              // int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                              // switchRx1.push(make_pair(make_pair(started,present_cn+loading_cycles+reaching_switch_cycles),fl));
                              int thread=threadRx2.front();

                              
                              string str1= to_string(started); 

                              string str2= to_string(thread); 


                              string str3=str1+" "+str2;
                              
                              dc[str3].push_back(present_cn+nic_init_cost);
                              

                           
                          
                              
                              threadRx2.pop_front();


                              Rx2.pop_front();

                       } 
                       else
                       {
                            
                              int started=Rx1.front().first.first;
                              int curr=Rx1.front().first.second;
                              int fl=Rx1.front().second ;
                             

                              present_cn=max(present_cn+1,curr+1);

                              int loading_cycles=((float)PacketSize/(float)network_bandwidth)*frequency;


                              // int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                              // switchRx1.push(make_pair(make_pair(started,present_cn+loading_cycles+reaching_switch_cycles),fl));

                              int thread=threadRx1.front();

                              
                              string str1= to_string(started); 

                              string str2= to_string(thread); 


                              string str3=str1+" "+str2;

                              
                              dc[str3].push_back(present_cn+nic_init_cost);

                                 

                           
                             
                          
                              threadRx1.pop_front();  

                              Rx1.pop_front();
                               

                       }
                }

                turn^=1;





            }
      }


}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void process_entries_at_module_switch(int type)
{

    int base_deley=2;   //to remove headers and to add appropriate headers 


    
  
    if(type==1)
    {
          int p_cn=-1;
          while(true)
          {

              
                if(switchRx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=switchRx1.front().first.first ;
                int curr=switchRx1.front().first.second ;
                int fl=switchRx1.front().second;
                
                int thread=thread_switchRx1.front();

                int q_count=0;
                for (int i=0;i<Tx1.size();i++) 
                   {
                         if(switchRx1[i].first.first<presentcycle)
                         {
                            q_count+=1;
                            if(q_count*128>Q_limit*1000)
                              switchRx1.erase(Tx1.begin()+i);


                         }


                    }


                p_cn=max(p_cn+1,curr+1);



                int reaching_memb_cycles=((float)switch_distance/(float)speed)*frequency;


              //  int transfer_cycles=((float)PacketSize/(float)network_bandwidth)*frequency;


                                
                double c=PacketSize/(double)switch_bandwidth;

                int transfer_cycles=abs(c)*frequency;

                
                if(turn&1)
                {
                      memRx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      thread_memRx1.push_back(thread);
                      turn^=1;
               
                }

                else
                {
                      memRx2.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      thread_memRx2.push_back(thread);
                      turn^=1;
               

                }

                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                // if(check==1)
                // {
                //   cout<<transfer_cycles<<endl;
                //   check++;
                // }
                dc[str3].push_back(p_cn);

              //  cout<<" \n switch "<<p_cn<<" "<<started<<"\n ";

                thread_switchRx1.pop_front();
                switchRx1.pop_front();


              
          }
       

   }
   else
   {

         int p_cn=-1;
         while(true)
          {

              
                if(switchTx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=switchTx1.front().first.first ;
                int curr=switchTx1.front().first.second ;
                int fl=switchTx1.front().second;
                
                int thread=thread_switchTx1.front();


                int q_count=0;
                for (int i=0;i<Tx1.size();i++) 
                   {
                         if(switchTx1[i].first.first<presentcycle)
                         {
                            q_count+=1;
                            if(q_count*128>Q_limit*1000)
                              switchTx1.erase(Tx1.begin()+i);


                         }


                    }


                
                p_cn=max(p_cn+1,curr+1);


                int reaching_memb_cycles=((float)switch_distance/(float)speed)*frequency;


                
                              
                double c=PacketSize/(double)switch_bandwidth;

                int transfer_cycles=abs(c)*frequency;
                
                if(turn&1)
                {
                      Rx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      turn^=1;
                      threadRx1.push_back(thread);
               
                }

                else
                {
                      Rx2.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      turn^=1;
                      threadRx2.push_back(thread);
               

                }

                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                dc[str3].push_back(p_cn);


                thread_switchTx1.pop_front();
                




                
                switchTx1.pop_front();


              
          }
       


   }


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////










void process_entries_at_module_switch2(int type)
{

    int base_deley=2;   //to remove headers and to add appropriate headers 


    
  
    if(type==1)
    {
          int p_cn=-1;
          while(true)
          {

              
                if(switchRx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=switchRx1.front().first.first ;
                int curr=switchRx1.front().first.second ;
                int fl=switchRx1.front().second;
                
                int thread=thread_switchRx1.front();

                int q_count=0;
                for (int i=0;i<Tx1.size();i++) 
                   {
                         if(switchRx1[i].first.first<presentcycle)
                         {
                            q_count+=1;
                            if(q_count*128>Q_limit*1000)
                              switchRx1.erase(Tx1.begin()+i);


                         }


                    }


                p_cn=max(p_cn+1,curr+1);



                int reaching_memb_cycles=((float)switch_distance/(float)speed)*frequency;
                
                  
                double c=PacketSize/(double)network_bandwidth;

                int transfer_cycles=abs(c)*frequency;
                if(turn&1)
                {
                      memRx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      thread_memRx1.push_back(thread);
                      turn^=1;
               
                }

                else
                {
                      memRx2.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      thread_memRx2.push_back(thread);
                      turn^=1;
               

                }

                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                dc[str3].push_back(p_cn);

              //  cout<<" \n switch "<<p_cn<<" "<<started<<"\n ";

                thread_switchRx1.pop_front();
                switchRx1.pop_front();


              
          }
       

   }
   else
   {

         int p_cn=-1;
         while(true)
          {

              
                if(switchTx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=switchTx1.front().first.first ;
                int curr=switchTx1.front().first.second ;
                int fl=switchTx1.front().second;
                
                int thread=thread_switchTx1.front();


                int q_count=0;
                for (int i=0;i<Tx1.size();i++) 
                   {
                         if(switchTx1[i].first.first<presentcycle)
                         {
                            q_count+=1;
                            if(q_count*128>Q_limit*1000)
                              switchTx1.erase(Tx1.begin()+i);


                         }


                    }


                
                p_cn=max(p_cn+1,curr+1);


                int reaching_memb_cycles=((float)switch_distance/(float)speed)*frequency;



                              
                double c=PacketSize/(double)network_bandwidth;

                int transfer_cycles=abs(c)*frequency;
                
                if(turn&1)
                {
                      pci_Tx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      turn^=1;
                      thread_pci_Tx1.push_back(thread);
               
                }

                else
                {
                      pci_Tx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      turn^=1;
                      thread_pci_Tx1.push_back(thread);
               

                }

                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                dc[str3].push_back(p_cn);


                thread_switchTx1.pop_front();
                




                
                switchTx1.pop_front();


              
          }
       


   }


}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void process_entries_at_module_switch3(int type)
{

    int base_deley=2;   //to remove headers and to add appropriate headers 


    
  
    if(type==1)
    {
          int p_cn=-1;
          while(true)
          {

              
                if(switchRx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=switchRx1.front().first.first ;
                int curr=switchRx1.front().first.second ;
                int fl=switchRx1.front().second;
                
                int thread=thread_switchRx1.front();

                int q_count=0;
                for (int i=0;i<Tx1.size();i++) 
                   {
                         if(switchRx1[i].first.first<presentcycle)
                         {
                            q_count+=1;
                            if(q_count*128>Q_limit*1000)
                              switchRx1.erase(Tx1.begin()+i);


                         }


                    }


                p_cn=max(p_cn+1,curr+1);



                int reaching_memb_cycles=((float)switch_distance/(float)speed)*frequency;



                              
                double c=PacketSize/(double)network_bandwidth;

                int transfer_cycles=abs(c)*frequency;
                
                if(turn&1)
                {
                      memRx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      thread_memRx1.push_back(thread);
                      turn^=1;
               
                }

                else
                {
                      memRx2.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      thread_memRx2.push_back(thread);
                      turn^=1;
               

                }

                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                dc[str3].push_back(p_cn);

              //  cout<<" \n switch "<<p_cn<<" "<<started<<"\n ";

                thread_switchRx1.pop_front();
                switchRx1.pop_front();


              
          }
       

   }
   else
   {

         int p_cn=-1;
         while(true)
          {

              
                if(switchTx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=switchTx1.front().first.first ;
                int curr=switchTx1.front().first.second ;
                int fl=switchTx1.front().second;
                
                int thread=thread_switchTx1.front();


                int q_count=0;
                for (int i=0;i<Tx1.size();i++) 
                   {
                         if(switchTx1[i].first.first<presentcycle)
                         {
                            q_count+=1;
                            if(q_count*128>Q_limit*1000)
                              switchTx1.erase(Tx1.begin()+i);


                         }


                    }


                
                p_cn=max(p_cn+1,curr+1);

                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                dc[str3].push_back(p_cn);



                int reaching_memb_cycles=((float)switch_distance/(float)speed)*frequency;



                  
                double c=PacketSize/(double)network_bandwidth;

                int loading_cycles=abs(c)*frequency;
                
                if(turn&1)
                {
                 //   Rx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      turn^=1;
                  //  threadRx1.push_back(thread);
                      dc[str3].push_back(p_cn+loading_cycles+reaching_memb_cycles);

                //
                }

                else
                {
                  //  Rx2.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_memb_cycles),fl));
                      turn^=1;
                   // threadRx2.push_back(thread);
                      dc[str3].push_back(p_cn+loading_cycles+reaching_memb_cycles);

                //

                }

                


                thread_switchTx1.pop_front();
                switchTx1.pop_front();



          }
       


   }


}













/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void process_entries_at_module_memory_brick()
{


    int turn =1;
    int p_cn=-1;
  

    while(true)
    {


          if(memRx1.empty() && memRx2.empty())
          {
                break;
          }
          

          if(turn&1)
          {
                
                if(!memRx1.empty())
                { 
                      
                      int started=memRx1.front().first.first ;
                      int curr=memRx1.front().first.second ;
                      int flag=memRx1.front().second;
                      p_cn=max(p_cn+1,curr+1);
                      int thread=thread_memRx1.front();

                      string str1= to_string(started); 

                      string str2= to_string(thread); 


                      string str3=str1+" "+str2;

                      dc[str3].push_back(p_cn);


                      memRx1.pop_front();

                      thread_memRx1.pop_front();

                      if(flag==0 || flag==1)
                      {

                          int load_data_cycles=abs((float)PacketSize/(float)bandwidth)*frequency;

                          int transfer_cycles=abs(PacketSize/(float)network_bandwidth)*frequency;
                          
                          int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                          switchTx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_switch_cycles+load_data_cycles),flag));

                          thread_switchTx1.push_back(thread);

                      }
                   
                      else
                      {

                          int load_instruction_cycles=abs(PacketSize/(float)bandwidth)*frequency;


                          string str1= to_string(started); 

                          string str2= to_string(thread); 


                          string str3=str1+" "+str2;

                          dc[str3].push_back(p_cn+load_instruction_cycles);




                      }
          

                }
                else
                {

                      int started=memRx2.front().first.first;
                      int curr=memRx2.front().first.second;
                      int flag=memRx2.front().second;

                      p_cn=max(p_cn+1,curr+1);
                      
                      int thread=thread_memRx2.front();

                      string str1= to_string(started); 

                      string str2= to_string(thread); 


                      string str3=str1+" "+str2;

                      dc[str3].push_back(p_cn);

                      memRx2.pop_front();

                      thread_memRx2.pop_front();

                      if(flag==0 || flag==1)
                      {

                          int load_data_cycles=abs(PacketSize/(float)bandwidth)*frequency;

                          int transfer_cycles=abs(PacketSize/(float)network_bandwidth)*frequency;
                          
                          int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                          switchTx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_switch_cycles+load_data_cycles),flag));

                          thread_switchTx1.push_back(thread);

                      }
                      
                      else
                      {

                          int load_instruction_cycles=abs(PacketSize/(float)bandwidth)*frequency;

                          string str1= to_string(started); 

                          string str2= to_string(thread); 


                          string str3=str1+" "+str2;

                          dc[str3].push_back(p_cn+load_instruction_cycles);


                      }


                }
                turn^=1;


          }
          else
          {
                
                if(!memRx2.empty())
                {
                    
                      int started=memRx2.front().first.first;
                      int curr=memRx2.front().first.second;
                      int flag=memRx2.front().second;
                      p_cn=max(p_cn+1,curr+1);

                      int thread=thread_memRx2.front();

                      string str1= to_string(started); 

                      string str2= to_string(thread);   


                      string str3=str1+" "+str2;

                      dc[str3].push_back(p_cn);


                      memRx2.pop_front();

                      thread_memRx2.pop_front();
                      
                      if(flag==0 || flag==1)
                      {

                          int load_data_cycles=abs(block_size/(float)bandwidth)*frequency;

                          int transfer_cycles=abs(block_size/(float)network_bandwidth)*frequency;
                          
                          int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                          switchTx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_switch_cycles+load_data_cycles),flag));

                          thread_switchTx1.push_back(thread);

                      }
                      else
                      {

                          int load_instruction_cycles=abs(block_size/(float)bandwidth)*frequency;

                          string str1= to_string(started); 

                          string str2= to_string(thread); 


                          string str3=str1+" "+str2;

                          dc[str3].push_back(p_cn+load_instruction_cycles);



                      }


                }
                else
                {


                      int started=memRx1.front().first.first;
                      int curr=memRx1.front().first.second;
                      int flag=memRx1.front().second;
                      p_cn=max(p_cn+1,curr+1);

                      int thread=thread_memRx1.front();

                      string str1= to_string(started); 

                      string str2= to_string(thread);   


                      string str3=str1+" "+str2;

                      dc[str3].push_back(p_cn);

                      memRx1.pop_front();

                      thread_memRx1.pop_front();

                      
                      if(flag==0 || flag==1)
                      {

                          int load_data_cycles=abs(PacketSize/(float)bandwidth)*frequency;

                          int transfer_cycles=abs(PacketSize/(float)network_bandwidth)*frequency;
                          
                          int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                          switchTx1.push_back(make_pair(make_pair(started,p_cn+transfer_cycles+reaching_switch_cycles+load_data_cycles),flag));

                          thread_switchTx1.push_back(thread);


                      }
                      else
                      {

                          int load_instruction_cycles=abs(block_size/(float)bandwidth)*frequency;

                          string str1= to_string(started); 

                          string str2= to_string(thread); 


                          string str3=str1+" "+str2;

                          dc[str3].push_back(p_cn+load_instruction_cycles);




                      }

                }


                turn^=1;



          }



    }


}














int cp=0;

void process_entries_at_pci_controller(int type)
{
    int base_deley=2;   

    //to remove headers and to add appropriate headers 

 
    if(type==1)
    {


          int p_cn=-1;
          while(true)
          {

              
                if(pci_Rx1.empty())
                {
                    break;
                }


                int turn =1;

                int started=pci_Rx1.front().first.first ;
                int curr=pci_Rx1.front().first.second ;
                int fl=pci_Rx1.front().second;
                
                int thread=thread_pci_Rx1.front();



                p_cn=max(p_cn+1,curr+1);




                int loading_cycles=abs(PacketSize/(float)network_bandwidth)*frequency;


                int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                switchRx1.push_back(make_pair(make_pair(started,p_cn+loading_cycles+reaching_switch_cycles),fl));

            //

                //cout<<" load cyc  "<<loading_cycles<<"  switch load "<<reaching_switch_cycles<<" "<<((float)PacketSize/network_bandwidth)<<" \n ";



                thread_switchRx1.push_back(thread);
                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;

                dc[str3].push_back(p_cn);

                
                thread_pci_Rx1.pop_front();


                pci_Rx1.pop_front();


              
          }
       

   }
   else
   {

         int p_cn=-1;
         while(true)
          {
               

                
                if(pci_Tx1.empty())
                {
                    break;
                }


                int turn =1;

               

                int started=pci_Tx1.front().first.first ;
                int curr=pci_Tx1.front().first.second ;
                int fl=pci_Tx1.front().second;
                
                int thread=thread_pci_Tx1.front();

                

                p_cn=max(p_cn+1,curr+1);


                int loading_cycles=abs(PacketSize/(float)network_bandwidth)*frequency;


                int reaching_switch_cycles=((float)switch_distance/(float)speed)*frequency;

                // switchRx1.push_back(make_pair(make_pair(started,p_cn+loading_cycles+reaching_switch_cycles),fl));

            //

                //cout<<" load cyc  "<<loading_cycles<<"  switch load "<<reaching_switch_cycles<<" "<<((float)PacketSize/network_bandwidth)<<" \n ";





                thread_switchRx1.push_back(thread);
                string str1= to_string(started); 

                string str2= to_string(thread); 


                string str3=str1+" "+str2;
                
                // cout<<"z\n";
                
                dc[str3].push_back(p_cn);
                
                
                thread_pci_Tx1.pop_front();


                pci_Tx1.pop_front();



           
              
          }
       


   }



}

















void read_ints (const char* file_name)
{
          FILE* file = fopen (file_name, "r");
          int i = 0;

          fscanf (file, "%d", &i);    
          while (!feof (file))
          {  
              printf ("%d ", i);
              fscanf (file, "%d", &i);      
          }

          fclose (file);        
}



void printRandoms(int lower, int upper,  int count)
{
    int i;
    
    for(i = 0; i < count; i++)
    {
        int num = (rand() % (upper - lower + 1)) + lower;
        printf("%d ", num);

    }

}





// void access_remote_memory()
// {
// }






// void access_memory_load()
// {
//         int lower=0;
//         int upper=100;
//         srand(time(0));

//         int num = (rand() % (upper - lower + 1)) + lower;
//         if(num<=l1_load_hit)
//         {
//             level_1_cache();

//         }
//         else
//         {

//             int lower=0;
//             int upper=100;
//             srand(time(0));
//             int num = (rand() % (upper - lower + 1)) + lower;
            

//             if(num<=l2_load_hit)
//             {

//                 level_2_cache();

//             }
//             else
//             {
//                     int lower=0;
//                     int upper=100;
//                     srand(time(0));
//                     int num = (rand() % (upper - lower + 1)) + lower;
                    
//                     if(num<=llc_load_hit)
//                     {
//                         last_level_cache();

//                     }
//                     else
//                     {
//                         // if(valid_local_memory())
//                         // {

//                         // }
//                         // else
//                         // {
//                         //     access_remote_memory();
//                         // }


//                     }
        

//             }



//         }



// }






// void generate_requests()
// {
//     float load_probability=0.6;
//     srand(time(0));
//     for(int i=0;i<number_of_instructions;i++)
//     {    

//         int lower=0;
//         int upper=100;


//         int num = (rand() % (upper - lower + 1)) + lower;
        
//         if(num>load_probability*100)
//         {
//             //store();
//             //access_memory_store();

//         }
//         else
//         {
//             //load();
//             access_memory_load();
//         }




//     }


// }



void get_access_fetch( int current,int thread)
{

    int next_cur=module_pci(current,0);
    module_nic_at_compute_brick(next_cur,0,current,thread);


}




void get_access_read(int current,int thread)
{
    int next_cur=module_pci(current,1);
    module_nic_at_compute_brick(next_cur,1,current,thread);

}



void get_access_write(int current,int thread)
{

    int next_cur=module_pci(current,2);
    module_nic_at_compute_brick(next_cur,2,current,thread);

}










/////////////////////////////////////////////////////////////////////////////////////////////////////////














void get_access_fetch2( int current,int thread)
{

    int next_cur=module_pci(current,0);
    module_pci_controller_at_compute_brick(next_cur,0,current,thread);


}




void get_access_read2(int current,int thread)
{
    int next_cur=module_pci(current,1);
    module_pci_controller_at_compute_brick(next_cur,1,current,thread);

}



void get_access_write2(int current,int thread)
{

    int next_cur=module_pci(current,2);
    module_pci_controller_at_compute_brick(next_cur,2,current,thread);

}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int check_valid_remote(string addr)
{
	//return 1<<32&atoi(addr[0]);
	//check cache remote	

	//CHECK Higher bit set or not 


	return 1; // constant number 

}

int presentcycleNumber=-1;


int brr=1;

void push_to_switch_using_on_chip(int current_cycle_number,int thread,int instr_type)
{

      
      string adrress;// assumed all are remote addresses

      int req_cycles=check_valid_remote(adrress);
    	
      int loading_cycles=((float)PacketSize/bandwidth)*frequency;
    	
      int transfer_cycles=(float)switch_distance/(float)speed*frequency/8;
      
      int request_prep=request_preperation_steering2();
      
      request_prep=0;

    	presentcycleNumber=max(presentcycleNumber+1,current_cycle_number+1);

      // if(brr==1)
      // {
      // //  cout<<loading_cycles<<" "<<transfer_cycles<<" "<<request_prep<<endl;
      //   brr++;
      // }

    	string str1= to_string(current_cycle_number); 
      string str2= to_string(thread); 

      string str3=str1+" "+str2;

      dc[str3].push_back(presentcycleNumber);

      switchRx1.push_back(make_pair(make_pair(current_cycle_number,presentcycleNumber+req_cycles+loading_cycles+transfer_cycles+request_prep),instr_type));

      thread_switchRx1.push_back(thread);

    
    


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
    //simulating memory requests....
    //
    //=2.6Ghz.;MemDelay=20cycles;pkPrep=10cy-cles;MdequeTimeIn=MdequeTimeOut=15cycles;PacketSize=64Bytes(CacheLineSize);NwBandwidth=102.4Gbps(DDR3bandwidth);andNwDelay=26cycles(10ns.).


    //scanf("%d",&cyclenumber);
    //scanf("%d",&bandwidth);
    
    //module1();
    //add(3,5);



    srand(time(0));
    int lower,upper,count;

      lower=0;
      upper=100;
      count=100;

    //printRandoms(lower, upper, count);






    //read_ints("input.txt");
    FILE* file = fopen ("source", "r");
    
    // fscanf (file, "%d", &cyclenumber);
     
     fscanf (file, "%d", &num_of_cores);
     
     fscanf (file, "%d", &bandwidth);

     fscanf (file, "%d", &switch_bandwidth);
     
     fscanf (file, "%d", &network_bandwidth);

     fscanf (file, "%d", &PacketSize);
     

     bandwidth*=1000000;
     switch_bandwidth*=1000000;
     network_bandwidth*=1000000;
     presentcycle=-1;
     //cout<<" band " <<bandwidth;
    // exit(1);
    
    // fscanf (file, "%f", &l1_load_hit);
    // fscanf (file, "%f", &l1_store_hit);
    
    // fscanf (file, "%f", &l2_load_hit);
    // fscanf (file, "%f", &l2_store_hit);
    
    // fscanf (file, "%f", &llc_load_hit);
    // fscanf (file, "%f", &llc_store_hit);



    // printf ("%d  %d %d", cyclenumber,bandwidth,numRequests);   


    

    // //fscanf (file, "%d", &);        
    // while (!feof (file))
    //     {  
     //                  printf ("%d ", i);
     //                  fscanf (file, "%d", &i);      
    //     }




    


    // generate_requests();


    

    fclose (file);

    int choice;

    cout<<"\n.....................Enter module you want the Simulation.......................\n\n>>>>> Simulation 1] Compute Brick->PCI Bus-> NIC->Switch->Mem Brick";


    cout<<"\n\n>>>>> Simulation 2] Compute Brick->PCI controller->Switch->Mem Brick \n\n";


    cout<<">>>>> Simulation 3] Compute Brick-> System On Chip \n";



    cin>>choice;

   

    if(choice==1)
    {
          //Using NIC


          FILE* file2 = fopen ("sample", "r");
          char addr[11]="";
          char instr_type[11]="";
          int current_cycle_num;
          int thread;


          while (!feof (file2))
            {  

                      fscanf (file2, "%s", addr);
                      fscanf (file2, "%s", instr_type);
                      fscanf (file2, "%d", &current_cycle_num);
                      fscanf (file2, "%d", &thread);

                   //   printf("%s  %s %d \n",addr,instr_type,current_cycle_num );

                      //break;
                      int result = strcmp(instr_type,"IFETCH");

                      int result2 = strcmp(instr_type,"READ");
                      
                      int result3 = strcmp(instr_type,"WRITE");
                      


                      if(!result)
                      {
                              get_access_fetch(current_cycle_num,thread);        

                      }    

                      
                      if(!result2)
                      {
                              get_access_read(current_cycle_num,thread);

                      }

                      
                      if(!result3)
                      {
                              get_access_write(current_cycle_num,thread);        
                          //printf("FOOOR\n");

                      }    
        
                       
                    


            }


                     dc.clear();
                     
                     process_entries_at_nic(1);
                     process_entries_at_module_switch(1);
                     process_entries_at_module_memory_brick();
                     process_entries_at_module_switch(0);
                     process_entries_at_nic(0);
                   
                     auto it =dc.begin();
                     ofstream outfile;
                     outfile.open("output");

                     vector<string> op;


                      while(it!=dc.end())
                      {

                         // cout<<"\n "<<it->first<<" first \n";
                          string line="";

                          //cout<<it->first<<" ";
                          outfile <<it->first<< " ";
                          line+=it->first;
                          line+=" ";
                          int counter=0;

                          for(auto it2=it->second.begin();it2!=it->second.end();it2++)
                          {
                        //    cout<<*it2<<" ";
                              line+=to_string(*it2);
                              line+=" ";
                              outfile <<*it2<< " ";
                              counter++;
                              if(counter==5)
                                break;
                          }
                         // cout<<"\n";

                          op.push_back(line);
                          outfile <<endl;
                           it++;
                        }

                     // cout<<op[0];


                      outfile.close();

                   
    }
    else if(choice==2)
    {


          // Using PCI e

          FILE* file2 = fopen ("sample", "r");

          char addr[11]="";

          char instr_type[11]="";

          int current_cycle_num;

          int thread;



          while (!feof (file2))
            {  

                
                      fscanf (file2, "%s", addr);
                      fscanf (file2, "%s", instr_type);
                      fscanf (file2, "%d", &current_cycle_num);
                      fscanf (file2, "%d", &thread);

                      //   printf("%s  %s %d \n",addr,instr_type,current_cycle_num );

                      //break;
                      int result = strcmp(instr_type,"IFETCH");

                      int result2 = strcmp(instr_type,"READ");
                      
                      int result3 = strcmp(instr_type,"WRITE");
                      


                      if(!result)
                      {
                              get_access_fetch2(current_cycle_num,thread);        

                      }    

                      
                      if(!result2)
                      {
                              get_access_read2(current_cycle_num,thread);

                      }

                      
                      if(!result3)
                      {
                              get_access_write2(current_cycle_num,thread);        
                          //printf("FOOOR\n");

                      }    
                      //cout<<"Hello";
        
                       
                    


            }


                   dc.clear();
                   process_entries_at_pci_controller(1);
                   process_entries_at_module_switch2(1);
                   process_entries_at_module_memory_brick();
                   process_entries_at_module_switch2(0);
                   process_entries_at_pci_controller(0);
                   
                   auto it =dc.begin();
                   ofstream outfile;
                   outfile.open("output");

                   vector<string> op;

                   while(it!=dc.end())
                    {

                     // cout<<"\n "<<it->first<<" first \n";
                      string line="";

                      //cout<<it->first<<" ";
                      outfile <<it->first<< " ";
                      line+=it->first;
                      line+=" ";
                      int counter=0;
                      for(auto it2=it->second.begin();it2!=it->second.end();it2++)
                      {
                    //    cout<<*it2<<" ";
                        line+=to_string(*it2);
                        line+=" ";
                        outfile <<*it2<< " ";
                        counter++;
                        if(counter==5)
                          break;
                      }
                     // cout<<"\n";
                      op.push_back(line);
                      outfile <<endl;
                      it++;

                    }

                   // cout<<op[0];
                    outfile.close();

                    



    }
    else if(choice==3)
    {
        //Using System On Chip





          FILE* file2 = fopen ("sample", "r");
          char addr[11]="";
          char instr_type[11]="";
          int current_cycle_num;
          int thread;


          while (!feof (file2))
            {  

                      fscanf (file2, "%s", addr);
                      fscanf (file2, "%s", instr_type);
                      fscanf (file2, "%d", &current_cycle_num);
                      fscanf (file2, "%d", &thread);

                      //   printf("%s  %s %d \n",addr,instr_type,current_cycle_num );

                      //break;
                      int result = strcmp(instr_type,"IFETCH");

                      int result2 = strcmp(instr_type,"READ");
                      
                      int result3 = strcmp(instr_type,"WRITE");
                      


                      if(!result)
                      {
                              push_to_switch_using_on_chip(current_cycle_num,thread,0);        

                      }    

                      
                      if(!result2)
                      {
                              push_to_switch_using_on_chip(current_cycle_num,thread,1);

                      }

                      
                      if(!result3)
                      {
                              push_to_switch_using_on_chip(current_cycle_num,thread,2);        
                          

                      }    
        
                       
                    


            }



            
               process_entries_at_module_switch3(1);
               process_entries_at_module_memory_brick();
               process_entries_at_module_switch3(0);
               
               auto it =dc.begin();
               ofstream outfile;
               outfile.open("output");

               vector<string> op;

               while(it!=dc.end())
                {

                 // cout<<"\n "<<it->first<<" first \n";
                  string line="";

                  //cout<<it->first<<" ";
                  outfile <<it->first<< " ";
                  line+=it->first;
                  line+=" ";
                  int counter=0;
                  for(auto it2=it->second.begin();it2!=it->second.end();it2++)
                  {
                //
                  //    cout<<*it2<<" ";
                      line+=to_string(*it2);
                      line+=" ";
                      outfile <<*it2<< " ";
                      counter++;
                      if(counter==5)
                        break;
                  
                  }
                 // cout<<"\n";
                  op.push_back(line);
                  outfile <<endl;
                  it++;
                }

               // cout<<op[0];
                outfile.close();

                


    }
    else
    {

    }






    







    return 0;
}






