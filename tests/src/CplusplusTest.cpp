// @@@LICENSE
//
//      Copyright (c) 2009-2013 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

/** 
 * ****************************************************************************
 * @file CplusplusTest.cpp
 * 
 * @brief  Test application for testing C++ FSM adapter API
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-17  <vmk>  Initial creation
 * ****************************************************************************
 */


#include <PmStateMachineEngine/Cplusplus/PalmFsm.hpp>
#include <PmStateMachineEngine/PalmFsm.h>
#include <PmStateMachineEngine/PalmFsmDbg.h>

#include "TestCommon.h"


class MyWorldFsm : public pmfsm::StateMachineBase {

 public:

     MyWorldFsm()
     : StateMachineBase("MyWorldFsm"), outdoors("outdoors"),
       shelter("shelter")
     {         
         FsmDbgEnableLogging(this,
                             kFsmDbgLogOptEvents,
                             StateMachineLogCb,
                             NULL);

         FsmInsertState(this, &outdoors, NULL/*pParent*/);
         FsmInsertState(this, &shelter, NULL/*pParent*/);
     }

 public:

     enum MyEventIds {
         kMyEvtIdWind = kFsmEventFirstUserEvent,
         kMyEvtIdRain

     };

     class Event : public FsmEvent {
     public:
         explicit Event(int id)
         {
             evtId = id;
         }

         union {
             struct {
                 float mph; ///< Wind speed in miles per hour
             } wind;

             struct {
                 float in;  ///< Rain in inches
             } rain;
         };
     };

     typedef pmfsm::StateBase<MyWorldFsm, Event> MyStateBase;


     class OutdoorsState : public MyStateBase {

      public:
         explicit OutdoorsState(const char* pName)
         : MyStateBase(pName)
         {
         }

         bool OnFsmEvent(const Event* pEvt, MyWorldFsm* pFsm)
         {
             switch (pEvt->evtId) {
               case kMyEvtIdWind:
                   if (pEvt->wind.mph > 15) {
                      FsmBeginTransition(pFsm,
                                         &pFsm->shelter);
                   }
                   return true;
                   break;

               case kMyEvtIdRain:
                   if (pEvt->rain.in > 2) {
                      FsmBeginTransition(pFsm,
                                         &pFsm->shelter);
                   }
                   return true;
                   break;
             }

             return false;
         }
     }; /// class OutdoorsState


     class ShelterState : public MyStateBase {
      public:
         explicit ShelterState(const char* pName)
         : MyStateBase(pName)
         {
         }

         bool OnFsmEvent(const Event* pEvt, MyWorldFsm* pFsm)
         {
             return false;
         }
     }; /// class ShelterState

 public:
     OutdoorsState       outdoors;

     ShelterState        shelter;

}; /// class MyWorldFsm


int CplusPlusTest()
{
     MyWorldFsm     world;

     FsmStart(&world, &world.outdoors);

     MyWorldFsm::Event evt(MyWorldFsm::kMyEvtIdWind);
     evt.wind.mph = 100;
     FsmDispatchEvent(&world, &evt);

     return 0;
}

