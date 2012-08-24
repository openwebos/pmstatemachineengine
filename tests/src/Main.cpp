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
 * ****************************************************************************
 * @file Main.cpp
 * 
 * @brief  Test application for testing PmStateMachineEngine
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-15  <vmk>  Initial creation
 * ****************************************************************************
 */

#include <stdint.h>

#include <stdio.h>

#include <stdarg.h>

#include <string>

#include <PmStateMachineEngine/PalmFsm.h>
#include <PmStateMachineEngine/PalmFsmDbg.h>

#include "TestCommon.h"


/**
 * @note We're not using stdbool.h (bool type) and stddef.h
 *       (NULL definition) because we're sharing this source
 *       code with Palm's QCOM modem firmware builds, and their
 *       current toolchains do not support stdbool.h/stddef.h.
 *       Conditional include of stdbool.h for the non-Modem
 *       builds could result in different side-effects and alter
 *       results (due to the "active" nature of the standard
 *       bool data type) (2009-09-18)
 */
#undef TRUE
#define TRUE 1

#undef FALSE
#define FALSE 0

#ifndef NULL
    #define NULL 0
#endif


typedef struct Test1Fsm {
    FsmMachine      fsmRep; ///< MUST be first member for C "subclassing"

    enum Test1UserSignals {
        kTest1Sig_startRange = kFsmEventFirstUserEvent, ///< just a placeholder

        kSig_pressure,
        kSig_wind
    };

    struct Test1StateS {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS1 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS11 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS111 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS112 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS2 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS21 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };

    struct Test1StateS22 {
        FsmState    stateRep; ///< MUST be first member for C "subclassing"
    };


    Test1StateS         s;
    Test1StateS11       s1;
    Test1StateS11       s11;
    Test1StateS111      s111;
    Test1StateS112      s112;

    Test1StateS2        s2;
    Test1StateS21       s21;
    Test1StateS22       s22;
} Test1Fsm;



/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s(const FsmState* pState,
                    Test1Fsm* pFsm,
                    const FsmEvent* pEvt)
{
    switch (pEvt->evtId) {
    case kFsmEventEnterScope:
        break;
    case kFsmEventExitScope:
        break;

    case kFsmEventBegin: {
        /// Initial transition to s1
        FsmBeginTransition((FsmMachine*)pFsm, (FsmState*)&pFsm->s1);
        return TRUE;
    }
    break;

    case Test1Fsm::kSig_pressure: {
        /// Transition to s2
        FsmBeginTransition((FsmMachine*)pFsm, (FsmState*)&pFsm->s2);
        return TRUE;
    }
    break;

    }

    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s1(const FsmState* pState,
                     Test1Fsm* pFsm,
                     const FsmEvent* pEvt)
{
    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s11(const FsmState* pState,
                      Test1Fsm* pFsm,
                      const FsmEvent* pEvt)
{
    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s111(const FsmState* pState,
                       Test1Fsm* pFsm,
                       const FsmEvent* pEvt)
{
    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s112(const FsmState* pState,
                       Test1Fsm* pFsm,
                       const FsmEvent* pEvt)
{
    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s2(const FsmState* pState,
                     Test1Fsm* pFsm,
                     const FsmEvent* pEvt)
{
    switch (pEvt->evtId) {
    case kFsmEventEnterScope:
        break;
    case kFsmEventExitScope:
        break;

    case kFsmEventBegin: {
        /// Initial transition to s21
        FsmBeginTransition((FsmMachine*)pFsm, (FsmState*)&pFsm->s21);
        return TRUE;
    }
    break;

    } // end switch

    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s21(const FsmState* pState,
                      Test1Fsm* pFsm,
                      const FsmEvent* pEvt)
{
    switch (pEvt->evtId) {
    case kFsmEventEnterScope:
        break;
    case kFsmEventExitScope:
        break;

    case kFsmEventBegin: {
    }
    break;

    case Test1Fsm::kSig_wind: {
        /// Transition to s111
        FsmBeginTransition((FsmMachine*)pFsm, (FsmState*)&pFsm->s111);
        return TRUE;
    }
    break;

    }

    return FALSE;
}


/** 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
StateHandlerTest1_s22(const FsmState* pState,
                      Test1Fsm* pFsm,
                      const FsmEvent* pEvt)
{
    return FALSE;
}



static int
Test1()
{

    struct Test1Fsm    fsm;

    FsmInitMachine((FsmMachine*)&fsm, "Test1");

    FsmDbgEnableLogging((FsmMachine*)&fsm,
                        kFsmDbgLogOptEvents,
                        StateMachineLogCb,
                        NULL);

    FsmInitState((FsmState*)&fsm.s,
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s, "s");

    FsmInitState((FsmState*)&fsm.s1, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s1, "s1");
    FsmInitState((FsmState*)&fsm.s11, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s11, "s11");
    FsmInitState((FsmState*)&fsm.s111, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s111, "s111");
    FsmInitState((FsmState*)&fsm.s112, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s112, "s112");

    FsmInitState((FsmState*)&fsm.s2, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s2, "s2");
    FsmInitState((FsmState*)&fsm.s21, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s21, "s21");
    FsmInitState((FsmState*)&fsm.s22, 
                 (FsmStateHandlerFnType*)&StateHandlerTest1_s22, "s22");


    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s, NULL/*pParent*/);

    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s1, (FsmState*)&fsm.s);
    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s11, (FsmState*)&fsm.s1);
    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s111, (FsmState*)&fsm.s11);
    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s112, (FsmState*)&fsm.s11);

    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s2, (FsmState*)&fsm.s);
    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s21, (FsmState*)&fsm.s2);
    FsmInsertState((FsmMachine*)&fsm, (FsmState*)&fsm.s22, (FsmState*)&fsm.s2);


    FsmStart((FsmMachine*)&fsm, (FsmState*)&fsm.s);

    //for (int i=0; i < 10000; ++i) {
        FsmEvent    evtPressure = {Test1Fsm::kSig_pressure};
        FsmDispatchEvent((FsmMachine*)&fsm, &evtPressure);

        FsmEvent    evtWind = {Test1Fsm::kSig_wind};
        FsmDispatchEvent((FsmMachine*)&fsm, &evtWind);
    //}


    FsmStart((FsmMachine*)&fsm, (FsmState*)&fsm.s22);

    return 0;
}


int main (int argc, char *argv[])
{
    printf("Running Test1...\n");
    int result = Test1();
    printf("Test1 returned with result = %d\n", result);

    printf("Running CplusPlusTest...\n");
    result = CplusPlusTest();
    printf("CplusPlusTest returned with result = %d\n", result);

    return 0;
}
