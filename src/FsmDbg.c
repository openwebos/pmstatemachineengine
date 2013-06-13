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
 * @file FsmDbg.c
 * 
 * @brief  State Machine Engine's Finite State Machine Debugging
 *         API.
 * 
 * A debugging API for the hierarchical state machine engine.
 * 
 * @note This API is NOT thread-safe
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-14  <vmk>  Initial creation
 * ****************************************************************************
 */

#include <string.h>

#include "FsmBuildConfig.h"

#if FSM_CONFIG_WEBOS_FEATURES
#include <PmLogLib.h>
#endif

#include "FsmAssert.h"

#include "PalmFsm.h"
#include "PalmFsmDbg.h"

#include "FsmPrv.h"

/**
 * Reset all logging flags in the given FSM instance
 * 
 * @param pFsm
 */
static void
ResetLoggingOptions(FsmMachineImpl* pFsm);

/**
 * Apply log options 
 *  
 * @note Assumes that the logging function/cookie or 
 *       PmLogContext has already been set-up
 * 
 * @param pFsm 
 * @param logOptions 
 * 
 * @return FSM_CONFIG_INLINE_FUNC void 
 */
static void
ApplyLoggingOptions(FsmMachineImpl* pFsm, unsigned int logOptions);



/**
 * ****************************************************************************
 */
void
FsmDbgEnableLogging(FsmMachine*                   pOpaqueFsm,
                    unsigned int            const logOptions,
                    FsmDbgLogLineFnType*    const pLogCbFunc,
                    const void*             const cookie)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);
    FSM_ASSERT(pLogCbFunc);

    ResetLoggingOptions(pFsm);

    pFsm->logOutKind_ = kFsmLogOutputKind_cb;
    pFsm->logOutput.pLogFunc_ = pLogCbFunc;
    pFsm->logCookie_ = cookie;

    ApplyLoggingOptions(pFsm, logOptions);

}


/**
 * ****************************************************************************
 */
#if FSM_CONFIG_WEBOS_FEATURES
void
FsmDbgEnableLoggingViaPmLogLib(FsmMachine*  const pOpaqueFsm,
                               unsigned int const logOptions,
                               PmLogContext const pmlogContext,
                               const void*  const cookie)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);

    ResetLoggingOptions(pFsm);

    pFsm->logOutKind_ = kFsmLogOutputKind_pmloglib;
    pFsm->logOutput.pmlogCtx_ = pmlogContext;
    pFsm->logCookie_ = cookie;

    ApplyLoggingOptions(pFsm, logOptions);
}
#endif // #if FSM_CONFIG_WEBOS_FEATURES


/**
 * ****************************************************************************
 */
void
FsmDbgDisableLogging(FsmMachine* pOpaqueFsm)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);

    ResetLoggingOptions(pFsm);
}


/**
 * ****************************************************************************
 */
void
FsmDbgSetLogLevelThreshold(FsmMachine* const pOpaqueFsm,
                           enum FsmDbgLogLevel const level)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);

    pFsm->logThresh_ = level;
}


/**
 * ****************************************************************************
 */
const char*
FsmDbgPeekMachineName(FsmMachine* pOpaqueFsm)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);

    return pFsm->pName_;
}


/**
 * ****************************************************************************
 */
const FsmState*
FsmDbgPeekCurrentState(FsmMachine* pOpaqueFsm)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);

    return (FsmState*)pFsm->rt_.pCurrentState;
}


/**
 * ****************************************************************************
 */
const char*
FsmDbgPeekStateName(const FsmState* pOpaqueState)
{
    FsmStateImpl* pState = (FsmStateImpl*)pOpaqueState;

    FSM_ASSERT(pState);
    FSM_ASSERT(pState->pHandler_);

    return pState->pName_;
}


/**
 * ****************************************************************************
 */
const FsmState*
FsmDbgPeekParentState(FsmMachine* pOpaqueFsm, const FsmState* pOpaqueState)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;
    FsmStateImpl* pState = (FsmStateImpl*)pOpaqueState;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);
    FSM_ASSERT(pState);
    FSM_ASSERT(pState->pHandler_);
    FSM_ASSERT(pState->pParent_);

    if (pState->pParent_ == (FsmStateImpl*)&pFsm->rootState_) {
        return NULL;
    }
    else {
        return (FsmState*)pState->pParent_;
    }
}


/**
 * ****************************************************************************
 */
static void
ResetLoggingOptions(FsmMachineImpl* const pFsm)
{
    /// Reset all logging flags
    pFsm->logOutKind_ = kFsmLogOutputKind_none;
    pFsm->logOutput.pLogFunc_ = NULL;
    #if FSM_CONFIG_WEBOS_FEATURES
        pFsm->logOutput.pmlogCtx_ = NULL;
    #endif
    pFsm->logCookie_ = NULL;
}


/**
 * ****************************************************************************
 */
static void
ApplyLoggingOptions(FsmMachineImpl* const pFsm, unsigned int logOptions)
{
    FSM_ASSERT(logOptions);

    if ((logOptions & kFsmDbgLogOptEvents) != 0) {
        logOptions &= ~kFsmDbgLogOptEvents;
    }

    if (logOptions != 0) {
        FSM_LOG_ERR(pFsm,
                    "FSM.%s(%p/c=%p): ERROR: unexpected logging options: 0x%X",
                    pFsm->pName_, pFsm, pFsm->logCookie_, logOptions);
    }
}
