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
 *******************************************************************************
 * @file PalmFsm.h
 * 
 * @brief  Private declarations for State Machine Engine's
 *         Finite State Machine implementation.
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-13  <vmk>  Initial creation
 *******************************************************************************
 */

#ifndef STATE_MACHINE_ENGINE_FSM_PRV_H
#define STATE_MACHINE_ENGINE_FSM_PRV_H

#include "FsmBuildConfig.h"

#if FSM_CONFIG_WEBOS_FEATURES
#include <PmLogLib.h>
#endif

#include "PalmFsm.h"
#include "PalmFsmDbg.h"
#include "FsmAssert.h"

#ifdef __cplusplus 
extern "C" {
#endif

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


/**
 * A state in a state machine
 * 
 * @note All fields ending in underscore are for internal use
 *       only and off-limits to users of the API
 */
typedef struct FsmStateImpl_tag {
    FsmStateHandlerFnType*      pHandler_;
    struct FsmStateImpl_tag*    pParent_;
    const char*                 pName_;
} FsmStateImpl;


/**
 * Log output kind
 */
typedef enum FsmLogOutputKind_ {
    kFsmLogOutputKind_none          = 0,    ///< logging is disabled

    #if FSM_CONFIG_WEBOS_FEATURES
    kFsmLogOutputKind_pmloglib      = 1,    ///< via PmLogLib
    #endif

    kFsmLogOutputKind_cb            = 2     ///< via callback function
} FsmLogOutputKind;

/**
 * A Finite State Machine
 * 
 * @note All fields ending in underscore are for internal use
 *       only and off-limits to users of the API
 */
typedef struct FsmMachineImpl_ {
    /**
     * @note The union helps us work around the C99 aliasing rules,
     *       which is permitted by C99 (it helps us avoid compiler
     *       errors like this: "dereferencing type-punned pointer
     *       will break strict-aliasing rules")
     */
    union {
        FsmState        pub;    ///< public version
        FsmStateImpl    impl;   ///< private version
    } rootState_;

    /// FSM's name string proivided by user; _not_ copied
    const char*             pName_;

    /// Log level threshold
    unsigned int            logThresh_:3;   ///< enum FsmDbgLogLevel

    unsigned int            logOutKind_:2;  ///< FsmLogOutputKind
    union {
        FsmDbgLogLineFnType*    pLogFunc_;  ///< for kFsmLogOutputKind_cb

        #if FSM_CONFIG_WEBOS_FEATURES
        PmLogContext            pmlogCtx_;  ///< for kFsmLogOutputKind_pmloglib
        #endif
    } logOutput;

    const void*             logCookie_;

    /**
     * FsmRuntime contains FSM engine "runtime" information that
     * gets reset by FsmStart
     */
    struct FsmRuntime {
        /// Current state in the active configuration.
        /// Set/Reset by DoEntryActions(); also reset by FsmBeginTransition().
        /// NULL value indicates that the FSM is undergoing state transition.
        FsmStateImpl*           pCurrentState;

        /// State to which a messages is being dispatched. This may
        /// be pCurrentState or one of its ancestors, if any.
        /// Managed by FsmDispatchEvent().
        FsmStateImpl*           pDispatchSrcState;

        /// Set by FsmStart() and FsmBeginTransition() to mark the target
        /// state of the requested transition.  Used and reset by
        /// DoEntryActions().
        FsmStateImpl*           pTranTarget;


        struct FsmEntryPath {
            int                     size; ///< number of states in path

            /// States of the entry path in REVERSE order
            /// 
            /// @todo Use stack-based entry path to eliminate the extra
            ///       storage requirement in each state machine instance.
            FsmStateImpl*           states[kFsmMaxStateNestingDepth];
        } entryPath;

        /**
         * DoEntryActions() sets/clears inInitialTrans_ around the
         * dispatch of kFsmEventBegin.  FsmBeginTransition() tests this
         * flag to determine which type of transition to record.
         */
        int                     inInitialTrans:1;

    }                       rt_;    ///< FSM runtime environment

} FsmMachineImpl;


/**
 * @brief State Machine Engine's root state handler
 * 
 * A convenient handler for logging unhandled user events.
 * 
 * @param pState
 * @param pFsm
 * @param pEvt
 * 
 * @return int
 */
int
RootStateHandler(FsmState* pState, FsmMachine* pFsm,
                 const FsmEvent* pEvt);


FSM_CONFIG_INLINE_FUNC int
IsLogLevelEnabled(const FsmMachineImpl* const pImpl,
                  enum FsmDbgLogLevel   const fsmloglevel,
                  int                   const pmloglevel)
{
    if (!pImpl->logOutKind_) {
        return 0;
    }
    else if (kFsmLogOutputKind_cb == pImpl->logOutKind_) {
        return ((int)fsmloglevel >= (int)pImpl->logThresh_);
    }
    #if FSM_CONFIG_WEBOS_FEATURES
    else if (kFsmLogOutputKind_pmloglib == pImpl->logOutKind_) {
        return PmLogIsEnabled(pImpl->logOutput.pmlogCtx_,
                              (PmLogLevel)pmloglevel);
    }
    #endif
    else {
        FSM_ASSERT(0 && "UNEXPECTED logOutKind_");                  \
    }

}

#if FSM_CONFIG_WEBOS_FEATURES
    #define LOG_VIA_PMLOGLIB(pImpl__, pmlogLevel__, ...)                    \
        (                                                                   \
            kFsmLogOutputKind_pmloglib == (pImpl__)->logOutKind_            \
              ? PmLogPrint((pImpl__)->logOutput.pmlogCtx_,                  \
                           (pmlogLevel__),                                  \
                           __VA_ARGS__), 1                                  \
              : (0)                                                         \
        )
#else
    #define LOG_VIA_PMLOGLIB(pImpl__, level__, pmlogLevel__, ...)   (0)
#endif


#define FSM_LOG_HELPER(pImpl__, level__, pmlogLevel__, ...)                 \
    do {                                                                    \
        if ((pImpl__)->logOutKind_) {                                       \
            if (LOG_VIA_PMLOGLIB((pImpl__), (pmlogLevel__), __VA_ARGS__)) { \
            }                                                               \
            else if (kFsmLogOutputKind_cb == (pImpl__)->logOutKind_ &&      \
                     IsLogLevelEnabled((pImpl__), (level__), (pmlogLevel__))) { \
                (pImpl__)->logOutput.pLogFunc_((FsmMachine*)(pImpl__),      \
                                               (void*)(pImpl__)->logCookie_,\
                                               (level__), __VA_ARGS__);     \
            }                                                               \
            else {                                                          \
                FSM_ASSERT(0 && "UNEXPECTED logOutKind_");                  \
            }                                                               \
        }                                                                   \
    } while (0)

#if !(FSM_CONFIG_WEBOS_FEATURES)
    #define kPmLogLevel_Debug       0
    #define kPmLogLevel_Info        0
    #define kPmLogLevel_Notice      0
    #define kPmLogLevel_Warning     0
    #define kPmLogLevel_Error       0
    #define kPmLogLevel_Critical    0
#endif

#define FSM_LOG_DEBUG(pImpl__, ...) \
    FSM_LOG_HELPER((pImpl__), kFsmDbgLogLevelDebug, kPmLogLevel_Debug, __VA_ARGS__)


#define FSM_LOG_INFO(pImpl__, ...) \
    FSM_LOG_HELPER((pImpl__), kFsmDbgLogLevelInfo, kPmLogLevel_Info, __VA_ARGS__)

#define FSM_LOG_NOTICE(pImpl__, ...) \
    FSM_LOG_HELPER((pImpl__), kFsmDbgLogLevelNotice, kPmLogLevel_Notice, __VA_ARGS__)

#define FSM_LOG_WARN(pImpl__, ...) \
    FSM_LOG_HELPER((pImpl__), kFsmDbgLogLevelWarning, kPmLogLevel_Warning, __VA_ARGS__)

#define FSM_LOG_ERR(pImpl__, ...) \
    FSM_LOG_HELPER((pImpl__), kFsmDbgLogLevelError, kPmLogLevel_Error, __VA_ARGS__)

#define FSM_LOG_FATAL(pImpl__, ...) \
    FSM_LOG_HELPER((pImpl__), kFsmDbgLogLevelFatal, kPmLogLevel_Critical, __VA_ARGS__)


#define FSM_LOG_IS_DEBUG_ENABLED(pImpl__) \
    IsLogLevelEnabled((pImpl__), kFsmDbgLogLevelDebug, kPmLogLevel_Debug)


#define FSM_LOG_IS_INFO_ENABLED(pImpl__) \
    IsLogLevelEnabled((pImpl__), kFsmDbgLogLevelInfo, kPmLogLevel_Info)


#define FSM_LOG_IS_WARN_ENABLED(pImpl__) \
    IsLogLevelEnabled((pImpl__), kFsmDbgLogLevelWarning, kPmLogLevel_Warning)


#define FSM_LOG_IS_ERR_ENABLED(pImpl__) \
    IsLogLevelEnabled((pImpl__), kFsmDbgLogLevelError, kPmLogLevel_Error)


#ifdef __cplusplus
}
#endif



#endif // STATE_MACHINE_ENGINE_FSM_PRV_H
