
// @@@LICENSE
//
//      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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
 *******************************************************************************
 * @file PalmFsm.hpp
 * 
 * @note Work In Progress: not tested yet!
 * 
 * @brief  State Machine Engine's C++ Finite State Machine
 *         adaptor.
 * 
 * A C++ adaptor for a hierarchical state machine engine.  The
 * following types of state machines may be expressed with this
 * API:
 * 
 *  * Hierachical FSM
 *  * Non-hieararchical (regular/flat) FSM
 * 
 * 
 * @note This API is NOT thread-safe
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-17  <vmk>  Initial creation
 *******************************************************************************
 */

#ifndef STATE_MACHINE_ENGINE_FSM_HPP
#define STATE_MACHINE_ENGINE_FSM_HPP

#include <stddef.h>  ///< defines NULL
#include <stdbool.h>
#include <stdint.h>

#include "PalmFsm.h"


namespace pmfsm {

/**
 * Usage Model
 * 
 * This header is a C++ adaptor for the native
 * C-language state machine engine implementation (see PalmFsm.h and
 * PalmFsmDbg.h in the same component as this header).
 * 
 * Create your FSM class by subclassing from the class
 * pmfsm::StateMachineBase
 * 
 * Create your state classes by subclassing from the class
 * template pmfsm::StateBase.
 * 
 * You then instantiate the state classes of your state machine
 * *instead* of calling FsmInitState().
 * 
 * For everything else, you use the native Fsm API.  Whenever
 * the API needs a FsmState pointer arg, you pass a pointer to
 * your State instance (StateBase is publically subclassed from
 * FsmState); and whenever the native API needs a pointer to
 * FsmMachine, you pass a pointer to your FSM class
 * (StateMachineBase is publically subclassed from FsmMachine)
 * 
 * Usage Example:
 * 
 * #include <PmStateMachineEngine/Cplusplus/PalmFsm.hpp>
 * #include <PmStateMachineEngine/PalmFsm.h>
 * #include <PmStateMachineEngine/PalmFsmDbg.h>
 * 
 * class MyWorldFsm : public pmfsm::StateMachineBase {
 * 
 *  public:
 * 
 *      MyWorldFsm()
 *      : StateMachineBase("MyWorldFsm"), outdoors("outdoors"),
 *        shelter("shelter")
 *      {
 *          FsmInsertState(this, &outdoors, NULL);
 *          FsmInsertState(this, &shelter, NULL);
 *      }
 * 
 *  public:
 * 
 *      enum MyEventIds {
 *          kMyEvtIdWind = kFsmEventFirstUserEvent,
 *          kMyEvtIdRain
 * 
 *      };
 * 
 *      class Event : public FsmEvent {
 *       public:
 *          explicit Event(int id)
 *          {
 *              evtId = id;
 *          }
 * 
 *          union {
 *              struct {
 *                  float mph; ///< Wind speed in miles per hour
 *              } wind;
 * 
 *              struct {
 *                  float in;  ///< Rain in inches
 *              } rain;
 *          };
 *      };
 * 
 *      typedef pmfsm::StateBase<MyWorldFsm, Event> MyStateBase;
 * 
 * 
 *      class OutdoorsState : public MyStateBase {
 * 
 *       public:
 *          explicit OutdoorsState(const char* pName)
 *          : MyStateBase(pName)
 *          {
 *          }
 * 
 *          bool OnFsmEvent(const Event* pEvt, MyWorldFsm* pFsm)
 *          {
 *              switch (pEvt->evtId) {
 *                case kMyEvtIdWind:
 *                    if (pEvt->wind.mph > 15) {
 *                       FsmBeginTransition(pFsm,
 *                                          &pFsm->shelter);
 *                    }
 *                    return true;
 *                    break;
 * 
 *                case kMyEvtIdRain:
 *                    if (pEvt->rain.in > 2) {
 *                       FsmBeginTransition(pFsm,
 *                                          &pFsm->shelter);
 *                    }
 *                    return true;
 *                    break;
 *              }
 * 
 *              return false;
 *          }
 *      }; /// class OutdoorsState
 * 
 * 
 *      class ShelterState : public MyStateBase {
 *       public:
 *          explicit ShelterState(const char* pName)
 *          : MyStateBase(pName)
 *          {
 *          }
 * 
 *          bool OnFsmEvent(const Event* pEvt, MyWorldFsm* pFsm)
 *          {
 *              switch (pEvt->evtId) {
 *              case kFsmEventEnterScope:
 *                  TurnLightOn();
 *                  return true;
 *                  break;
 *              case kFsmEventExitScope:
 *                  TurnLightOff();
 *                  return true;
 *                  break;
 *              }
 *              return false;
 *          }
 *      }; /// class ShelterState
 * 
 *  public:
 *      OutdoorsState       outdoors;
 * 
 *      ShelterState        shelter;
 * 
 * }; /// class MyWorldFsm
 * 
 * 
 * main()
 * {
 *      MyWorldFsm     world;
 * 
 *      FsmStart(&world, &world.outdoors);
 * 
 *      MyWorldFsm::Event evt(MyWorldFsm::kMyEvtIdWind);
 *      evt.wind.mph = 100;
 *      (void)FsmDispatchEvent(&world, &evt);
 * }
 */


/**
 * Create your FSM class by subclassing from the class
 * pmfsm::StateMachineBase.  Then pass a pointer to your FSM
 * class instance to the native FSM API (PalmFsm.h and PalmFsmDbg.h)
 * functions that need a pointer to FsmMachine.
 */
class StateMachineBase : public FsmMachine { 
public:

    /**
     * Constructor
     * 
     * @param pName FSM name to use for logging and debugging.  FSM
     *              saves the given pointer (i.e., doesn't copy the
     *              string).  The names "FSM" and "UNNAMED-FSM" are
     *              reserved.
     */
    explicit StateMachineBase(const char* pName)
    {
        FsmInitMachine(this, pName);
    }
};


/**
 * This class template is a C++ adaptor for the native
 * C-language state machine engine implementation (see PalmFsm.h and
 * PalmFsmDbg.h in the same component as this header).
 * 
 * StateBase implements the simple mapping from the native C
 * event handler callback to the virtual C++ method OnFsmEvent().
 * 
 * Define your state classes for a given state machine by
 * subclassing from this class template instead of calling
 * FsmInitState(). Then, pass a pointer to your class instance
 * whenever calling a native FSM API function (PalmFsm.h and
 * PalmFsmDbg.h) that needs a pointer to FsmState.
 */
template<
    class FsmType_ = FsmMachine,
    typename FsmEvtType_ = FsmEvent
>
class StateBase : public FsmState {
public:
    /**
     * Constructor.
     * 
     * @param pName FSM name to use for logging and debugging.  FSM
     *              saves the given pointer (i.e., doesn't copy the
     *              string).  The names "ROOT" and "UNNAMED-STATE"
     *              are reserved.
     */
    explicit StateBase(const char* pName)
    {
        FsmInitState(this, &GenericStateHandler, pName);
    }

    virtual bool OnFsmEvent(const FsmEvtType_* pEvt, FsmType_* pFsm) = 0;

private:
    /**
     * Handles event handler callbacks from the state machine engine
     * by invoking the OnFsmEvent virtual method of the corresponding
     * state instance.
     * 
     * @param pState @see FsmStateHandlerFnType
     * @param pFsm @see FsmStateHandlerFnType
     * @param pEvt @see FsmStateHandlerFnType
     * 
     * @return int @see FsmStateHandlerFnType
     */
    static int
    GenericStateHandler(FsmState* pState,
                        FsmMachine* pFsm,
                        const FsmEvent* pEvt)
    {
        StateBase* pBase = static_cast<StateBase*>(pState);

        return pBase->OnFsmEvent(static_cast<const FsmEvtType_*>(pEvt),
                              static_cast<FsmType_*>(pFsm));
    }
};


} // end namespace



#endif // STATE_MACHINE_ENGINE_FSM_HPP
