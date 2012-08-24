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
 * @file Fsm.c
 * 
 * @brief  State Machine Engine's Finite State Machine API.
 * 
 * An API for a hierarchical state machine engine.
 * 
 * @note This API is NOT thread-safe
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-13  <vmk>  Initial creation
 * ****************************************************************************
 */

#include <string.h>
#include <stdio.h>  ///< for snprintf

#include "FsmBuildConfig.h"

#include "FsmAssert.h"

#include "PalmFsm.h"
#include "PalmFsmDbg.h"

#include "FsmPrv.h"


/**
 * This structure contains the compile-time checks for this
 * module
 */
typedef struct CompileAssert {
    /**
     * If FsmMachine and FsmMachineImpl structure sizes don't match,
     * the compiler should generate a "divide by zero" error.
     */
    char    FsmMachine_is_correct_size[1/(sizeof(FsmMachine) ==
                                          sizeof(FsmMachineImpl))];

    /**
     * If FsmState and FsmStateImpl structure sizes don't match, the
     * compiler should generate a "divide by zero" error.
     */
    char    FsmState_is_correct_size[1/(sizeof(FsmState) ==
                                        sizeof(FsmStateImpl))];
} CompileAssert;


/**
 * Reserved events
 */
static const FsmEvent g_beginEvt    = {kFsmEventBegin};
static const FsmEvent g_entryEvt    = {kFsmEventEnterScope};
static const FsmEvent g_exitEvt     = {kFsmEventExitScope};



/**
 * Forward function declarations
 */
static void
DoEntryActions(FsmMachineImpl* pFsm);

static void
RecordInitialEntryPath(FsmMachineImpl* pFsm, FsmStateImpl* pAncestor,
                       FsmStateImpl* pDescendant);

/**
 * Delivers the given event, and optionally logs it (logging
 * depends on the pFsm->logOutKind_ setting)
 * 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int true (non-zero) if the event was handled (by the
 *         given state or one of its parent states); false
 *         (zero) if the event was not handled.
 */
static int
DeliverEvent(const FsmStateImpl* pState, FsmMachineImpl* pFsm,
             const FsmEvent* pEvt);



/**
 * ****************************************************************************
 */
void
FsmInitMachine(FsmMachine* pOpaqueFsm, const char* pName)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;

    FSM_ASSERT(pFsm);

    memset(pFsm, 0, sizeof(*pFsm));
    pFsm->logThresh_ = kFsmDbgLogLevelInfo;
    FsmInitState(&pFsm->rootState_.pub, &RootStateHandler, "<ROOT>");
    pFsm->pName_ = (pName && *pName) ? pName : "<UNNAMED-FSM>";
}


/**
 * ****************************************************************************
 */
void
FsmInitState(FsmState* pOpaqueState,
             FsmStateHandlerFnType* pStateHandlerCbFunc, const char* pName)
{
    FsmStateImpl* pState = (FsmStateImpl*)pOpaqueState;

    FSM_ASSERT(pState);
    FSM_ASSERT(pStateHandlerCbFunc);

    pState->pParent_ = NULL;
    pState->pHandler_ = pStateHandlerCbFunc;
    pState->pName_ = (pName && *pName) ? pName : "<UNNAMED-STATE>";
}


/**
 * ****************************************************************************
 */
void
FsmInsertState(FsmMachine* pOpaqueFsm, FsmState* pOpaqueState, FsmState* pParent)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;
    FsmStateImpl*   pState = (FsmStateImpl*)pOpaqueState;
    int i;


    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);
    FSM_ASSERT(pState);
    FSM_ASSERT(pState->pHandler_);
    FSM_ASSERT(!pState->pParent_);

    pState->pParent_ = 
        pParent ? (FsmStateImpl*)pParent : &pFsm->rootState_.impl;

    /// Validate state nesting and path to root state
    pState = pState->pParent_;
    i = 1;
    while (pState != &pFsm->rootState_.impl) {
        FSM_ASSERT(i < kFsmMaxStateNestingDepth);
        ++i;
        pState = pState->pParent_;
    }
}


/**
 * ****************************************************************************
 */
void
FsmStart(FsmMachine* pOpaqueFsm, FsmState* pInitialOpaqueState)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;
    FsmStateImpl*   pInitialState = (FsmStateImpl*)pInitialOpaqueState;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(&RootStateHandler == pFsm->rootState_.impl.pHandler_);
    FSM_ASSERT(pInitialState);
    FSM_ASSERT(pInitialState->pHandler_);
    FSM_ASSERT(pInitialState->pParent_);

    /// Reset the FSM runtime environment
    memset(&pFsm->rt_, 0, sizeof(pFsm->rt_));

    /// Record the initial state entry path
    RecordInitialEntryPath(pFsm, &pFsm->rootState_.impl, pInitialState);

    /// Enter ancestors and the initial state, and process initial transitions
    pFsm->rt_.pTranTarget = pInitialState; ///< DoEntryActions() expects it
    DoEntryActions(pFsm);
}


/**
 * ****************************************************************************
 */
int
FsmDispatchEvent(FsmMachine* pOpaqueFsm, const FsmEvent* pEvt)
{
    int             isHandled = FALSE;
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;
    FsmStateImpl*   pDisp = NULL;

    FSM_ASSERT(pFsm);
    FSM_ASSERT(pEvt);

    /// Check for stateless re-entry violation
    if (!pFsm->rt_.pCurrentState) {
        FSM_LOG_FATAL(pFsm,
                      "FSM.%s(%p/c=%p): ERROR: NULL-Target-Dispatch Violation " \
                      "while attempting to dispatch EVT.%d; " \
                      "probably re-entered from the scope of " \
                      "ENTER, EXIT, or BEGIN event handler",
                      pFsm->pName_, pFsm, pFsm->logCookie_, pEvt->evtId);

        FSM_ASSERT(FALSE && "FSM: NULL-Target-Dispatch Violation; " \
               "probably re-entered from ENTER, EXIT, or BEGIN event handler");
    }

    /// Check for RTC violation
    if (pFsm->rt_.pDispatchSrcState) {
        FSM_LOG_FATAL(pFsm,
                      "FSM.%s(%p/c=%p): ERROR: Run-to-Completion Violation " \
                      "while attempting to dispatch EVT.%d to %s " \
                      "from the scope of active dispatch",
                      pFsm->pName_, pFsm, pFsm->logCookie_, pEvt->evtId,
                      pFsm->rt_.pCurrentState->pName_);

        FSM_ASSERT(FALSE && "FSM: Run-to-Completion Violation");
    }

    FSM_ASSERT(!pFsm->rt_.pTranTarget);
    FSM_ASSERT(pEvt);
    FSM_ASSERT(pEvt->evtId >= kFsmEventFirstUserEvent);

    pFsm->rt_.pDispatchSrcState = pFsm->rt_.pCurrentState;
    do {
        pDisp = pFsm->rt_.pDispatchSrcState;

        isHandled = DeliverEvent(pDisp, pFsm, pEvt);

        if (pFsm->rt_.pTranTarget && !isHandled) {
            FSM_LOG_FATAL(pFsm,
                          "FSM.%s(%p/c=%p): ERROR: Can't pass EVT.%d to parent " \
                          "after transition request to state %s",
                          pFsm->pName_, pFsm, pFsm->logCookie_,
                          pEvt->evtId, pFsm->rt_.pTranTarget->pName_);
            FSM_ASSERT(FALSE && "FSM: Can't pass evt to parent after " \
                   "state transition request");
        }

    } while (!isHandled && (pFsm->rt_.pDispatchSrcState = pDisp->pParent_) != NULL);

    pFsm->rt_.pDispatchSrcState = NULL; ///< we're done with event dispatch


    /// Check if a transition was taken
    if (isHandled && pFsm->rt_.pTranTarget) {
        /// @note Exit actions were already processed in FsmBeginTransition()

        /// Handle entry actions and initial transition drill-down
        DoEntryActions(pFsm);
    }

    FSM_ASSERT(!pFsm->rt_.pTranTarget);

    return isHandled;
}



/**
 * ****************************************************************************
 */
void
FsmBeginTransition(FsmMachine* pOpaqueFsm, FsmState* pOpaqueTarget)
{
    FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;
    FsmStateImpl*   pTarget = (FsmStateImpl*)pOpaqueTarget;
    FsmStateImpl*   pMainSrc = pFsm->rt_.pDispatchSrcState;
    FsmStateImpl*   pState = NULL;

    /**
     * @note pFsm->rt_.pCurrentState may be NULL on entry to this
     *       function if it's called from the scope of
     *       DoEntryActions() to process user's initial transition
     *       request.
     * 
     * @note This function also sets pFsm->rt_.pCurrentState to
     *       NULL.
     */

    FSM_ASSERT(pFsm);
    FSM_ASSERT(pTarget);

    FSM_LOG_DEBUG(pFsm,
                  "FSM.%s(%p/c=%p): requesting transition to %s",
                  pFsm->pName_, pFsm, pFsm->logCookie_, pTarget->pName_);

    FSM_ASSERT(!pFsm->rt_.pTranTarget);
    FSM_ASSERT(pTarget->pHandler_);
    FSM_ASSERT(pTarget->pParent_);




    pFsm->rt_.pTranTarget = pTarget; ///< required by DoEntryActions()

    if (pFsm->rt_.inInitialTrans) {
        FSM_ASSERT(!pFsm->rt_.pDispatchSrcState);
        FSM_ASSERT(!pFsm->rt_.pCurrentState);

        /// @note DoEntryActions() will record its own entry path in this case
        return;
    }

    /**
     * Handle a "normal" (non-initial) request by exiting the source
     * state configuration
     * 
     * The complete sequence is:
     * 
     *  * 1. Evaluate guard condition for the transition; peform the
     *    following states ONLY if it avaluated to TRUE:
     *  * 2. Exit Source state configuration
     *  * 3. Execute actions associated with the transition
     *  * 4. Enter the Target state configuration
     * 
     * @see UML Specification (OMG 07, Section 15.3.13)
     * 
     * Furthermore, we implement specifically the UML Local
     * Transition semantics (@see OMG 07, Section 15.3.15). Summary:
     * 
     *  * A Local Transition doesn't exit from the Main Source state
     *    if the Main Target state is a Descendant of the Main
     *    Source State.
     *  * A Local Transition doesn't Exit/Re-enter the Main Target
     *    state if the Main Target state is an Ancestor of the Main
     *    Source state.
     */
    FSM_ASSERT(pFsm->rt_.pCurrentState);
    FSM_ASSERT(pMainSrc);


    pState = pFsm->rt_.pCurrentState;

    /// We're entering the "no current state" twilight zone
    pFsm->rt_.pCurrentState = NULL;

    /// First, Exit the active configuration up to Main Source
    for (; pState != pMainSrc; pState = pState->pParent_) {
        (void)DeliverEvent(pState, pFsm, &g_exitEvt);
    }

    /**
     * Now, handle the exits and record the entry path in the
     * transition from Main Source to Main Target
     */

    pFsm->rt_.entryPath.size = 0;

    /// Handle Peer Source/Target states (including Main Source == Target)
    /// (exit source, enter target)
    if (pMainSrc->pParent_ == pTarget->pParent_) {
        (void)DeliverEvent(pMainSrc, pFsm, &g_exitEvt);
        pFsm->rt_.entryPath.states[0] = pTarget;
        pFsm->rt_.entryPath.size = 1;
        return;
    }

    /// Local Transition: Is Target a descendant of Main Source?
    /// (don't exit source; enter target)
    for (pState = pTarget;
          pState != &pFsm->rootState_.impl;
          pState = pState->pParent_) {

        if (pState == pMainSrc) {
            return; /// Target *IS* a descendant of Main Source
        }

        /// Record path from Target to Target's top user ancestor
        FSM_ASSERT(pFsm->rt_.entryPath.size < kFsmMaxStateNestingDepth);
        pFsm->rt_.entryPath.states[pFsm->rt_.entryPath.size++] = pState;
    }

    /**
     * Target is NOT a descendant of Main Source;
     * 
     * entryPath_ now contains the path from Target state to its top
     * user ancestor
     */

    /**
     * @note This might be a Local Transition
     * 
     * * Exit Main Source and its ancestors up to (but not
     *   including) the point of intersection, if any, with the path
     *   from Target to Target's top user ancestor.
     * * Don't enter the point of intersection.
     * 
     * This point of intersection, if any, may be either:
     *  * 1. The LCA (Least Common Ancestor) of Main Source and
     *    Target; or
     *  * 2. The Target state itself, in case Target is the ancestor
     *    of Main Source, which would make it a Local Transition
     */
    (void)DeliverEvent(pMainSrc, pFsm, &g_exitEvt);
    for (pState = pMainSrc->pParent_;
          pState != &pFsm->rootState_.impl;
          pState = pState->pParent_) {
        int pathIndex;

        /// Check for intersection with Target Entry Path
        for (pathIndex = 0; pathIndex < pFsm->rt_.entryPath.size; ++pathIndex) {
            if (pState == pFsm->rt_.entryPath.states[pathIndex]) {
                /// Found the intersection: either LCA or Target itself

                /// Entry path will beging just after the intersection
                /// @note If intesected with Target, pathIndex will be 0
                pFsm->rt_.entryPath.size = pathIndex;
                return;
            }
        }

        /// Exit the current ancestor of Main Source
        (void)DeliverEvent(pState, pFsm, &g_exitEvt);
    }

    /**
     * If we go here, then Main Source and Target did not share a
     * common user-defined ancestor.
     * 
     * In this case, pFsm->rt_.entryPath contains the entry path from
     * top user-defined ancestor of Target to Target
     */

} // FsmBeginTransition()


/**
 * Delivers the given event, and optionally logs it (logging
 * depends on the pFsm->logOutKind_ setting)
 * 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
static int
DeliverEvent(const FsmStateImpl* pState, FsmMachineImpl* pFsm,
             const FsmEvent* pEvt)
{
    int isHandled = FALSE;
    char evtBuf[100] = "";

    if (pFsm->logOutKind_ && FSM_LOG_IS_INFO_ENABLED(pFsm)) {
        enum FsmDbgLogLevel logLevel = kFsmDbgLogLevelInfo;
        int logDelivery = 1;

        char numString[20];
        const char* pEvtName = NULL;

        switch (pEvt->evtId) {
        case kFsmEventEnterScope:
            pEvtName = "ENTER"; ///< this one gets logged @ INFO level
            break;
        case kFsmEventExitScope:
            pEvtName = "EXIT";
            logLevel = kFsmDbgLogLevelDebug;
            logDelivery = FSM_LOG_IS_DEBUG_ENABLED(pFsm);
            break;
        case kFsmEventBegin:
            pEvtName = "BEGIN";
            logLevel = kFsmDbgLogLevelDebug;
            logDelivery = FSM_LOG_IS_DEBUG_ENABLED(pFsm);
            break;

        default:
            pEvtName = numString;
            logLevel = kFsmDbgLogLevelDebug;
            logDelivery = FSM_LOG_IS_DEBUG_ENABLED(pFsm);
            break;
        }

        if (logDelivery) {
            if (pEvtName == numString) {
                snprintf(numString, sizeof(numString), "%d", (int)pEvt->evtId);
            }

            snprintf(evtBuf, sizeof(evtBuf), "EVT.%s ==> %s",
                     pEvtName, pState->pName_);

            if (kFsmDbgLogLevelInfo == logLevel) {
                FSM_LOG_INFO(pFsm, "FSM.%s(%p/c=%p): %s",
                             pFsm->pName_, pFsm, pFsm->logCookie_, evtBuf);
            }
            else {
                FSM_LOG_DEBUG(pFsm, "FSM.%s(%p/c=%p): %s",
                              pFsm->pName_, pFsm, pFsm->logCookie_, evtBuf);
            }
        }
    }


    /// Deliver the event
    isHandled = pState->pHandler_((FsmState*)pState, (FsmMachine*)pFsm, pEvt);

    FSM_LOG_DEBUG(pFsm,
                  "FSM.%s(%p/c=%p): <-- %s (%s)",
                  pFsm->pName_,
                  pFsm,
                  pFsm->logCookie_,
                  isHandled ? "<HANDLED>" : "<NOT HANDLED>",
                  evtBuf);

    return isHandled;
}


/**
 * Enter states on the current entry path, and process initial
 * transitions
 * 
 * @param pFsm
 */
static void
DoEntryActions(FsmMachineImpl* pFsm)
{
    FsmStateImpl*   pTarget = pFsm->rt_.pTranTarget;

    /// ASSUMPTIONS ON ENTRY:
    /// * the initial state entry path (if any) is already set-up
    /// * pFsm->rt_.pTranTarget is the destination state

    FSM_ASSERT(pFsm->rt_.entryPath.size <= kFsmMaxStateNestingDepth);
    FSM_ASSERT(pFsm->rt_.pTranTarget);

    pFsm->rt_.pCurrentState = NULL;     ///< we're in-between states

    /// Enter states in the path and drill down initial transitions
    do {
        /// Enter all the states in the current entry path, if any
        while (pFsm->rt_.entryPath.size > 0) {
            FsmStateImpl* pState =
                pFsm->rt_.entryPath.states[--pFsm->rt_.entryPath.size];

            (void)DeliverEvent(pState, pFsm, &g_entryEvt);
        }

        /// Mark initial transition to destination state
        {
            pTarget = pFsm->rt_.pTranTarget;
            pFsm->rt_.pTranTarget = NULL;   ///< reset destination holding register

            pFsm->rt_.inInitialTrans = TRUE;
            (void)DeliverEvent(pTarget, pFsm, &g_beginEvt);
            pFsm->rt_.inInitialTrans = FALSE;
        }

        /// Process new initial transition request, if any
        if (pFsm->rt_.pTranTarget) {
            RecordInitialEntryPath(pFsm, pTarget, pFsm->rt_.pTranTarget);
            /// New destination MUST be a PROPER descendant of current state
            FSM_ASSERT(pFsm->rt_.entryPath.size > 0);
        }         
         
    } while (pFsm->rt_.pTranTarget);

    /// State transitions settled down, and we now have a "current" state
    pFsm->rt_.pCurrentState = pTarget;

    FSM_LOG_DEBUG(pFsm,
                  "FSM.%s(%p/c=%p): Entry completed; current state is %s",
                  pFsm->pName_, pFsm, pFsm->logCookie_, pTarget->pName_);
} // DoEntryActions()


/**
 * Record entry path for an initial transition
 * 
 * @param pFsm
 * @param pAncestor
 * @param pDescendant
 */
static void
RecordInitialEntryPath(FsmMachineImpl* pFsm, FsmStateImpl* pAncestor,
                       FsmStateImpl* pDescendant)
{
    FsmStateImpl* pState;
    pFsm->rt_.entryPath.size = 0;
    for (pState = pDescendant;
          pState != pAncestor;
          pState = pState->pParent_) {

        FSM_ASSERT(pState);
        FSM_ASSERT(pState != &pFsm->rootState_.impl &&
               "Ancestor MUST be reachable from Descendant!");
        FSM_ASSERT(pFsm->rt_.entryPath.size < kFsmMaxStateNestingDepth);

        pFsm->rt_.entryPath.states[pFsm->rt_.entryPath.size++] = pState;
    }
}

/**
 * State handler for a state machine's root state provided by
 * the FSM implementation.
 * 
 * @see FsmStateHandlerFnType
 * 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int always returns false (zero) (i.e., not handled)
 */
int
RootStateHandler(FsmState* pOpaqueState, FsmMachine* pOpaqueFsm,
                 const FsmEvent* pEvt)
{
    //FsmMachineImpl* pFsm = (FsmMachineImpl*)pOpaqueFsm;
    //FsmStateImpl* pState = (FsmStateImpl*)pOpaqueState;

    /// ASSUMPTION: We should only see user events here
    FSM_ASSERT(pEvt->evtId >= kFsmEventFirstUserEvent);

    return FALSE;
}
