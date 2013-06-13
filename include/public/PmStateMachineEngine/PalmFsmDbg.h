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
 * @file PalmFsmDbg.h
 * 
 * @brief  State Machine Engine's Finite State Machine Debugging
 *         API.
 * 
 * @note This API is NOT thread-safe
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-13  <vmk>  Initial creation
 *******************************************************************************
 */

#ifndef STATE_MACHINE_ENGINE_FSM_DBG_H
#define STATE_MACHINE_ENGINE_FSM_DBG_H


#ifdef STATE_MACHINE_ENGINE_WEBOS_FEATURES
#include <PmLogLib.h>
#endif

#include "PalmFsm.h"


#ifdef __cplusplus 
extern "C" {
#endif


#ifdef __GNUC__
/// In FsmDbgLogLineFnType, the format string is arg #2;
/// var-args begin at arg #3
#define FSM_DBG_LOGFN_FMT_CHK __attribute__((__format__(__printf__, 4, 5)))
#else
#define FSM_DBG_LOGFN_FMT_CHK
#endif

enum FsmDbgLogLevel {
    kFsmDbgLogLevelDebug        = 0,
    kFsmDbgLogLevelInfo         = 1,
    kFsmDbgLogLevelNotice       = 2,
    kFsmDbgLogLevelWarning      = 3,
    kFsmDbgLogLevelError        = 4,
    kFsmDbgLogLevelFatal        = 5,
    kFsmDbgLogLevelNone         = 6
};

/**
 * Logging callback function type; @see FsmDbgEnableLogging().
 * 
 * format (pFmt) and variable args are same as for the standard
 * printf() function.
 */
typedef void FsmDbgLogLineFnType(FsmMachine* pFsm,
                                 void* cookie,
                                 enum FsmDbgLogLevel level,
                                 const char* pFmt, ...) FSM_DBG_LOGFN_FMT_CHK;


/// FSM logging options.  Multiple logging options may be
/// bitwise OR'ed together.
/// 
/// @note Used as argument for FsmDbgEnableLogging().
enum FsmDbgLogOptions {
    /// Turns on logging of reserved and user-defined events
    kFsmDbgLogOptEvents         = 0x01
};


#ifdef STATE_MACHINE_ENGINE_WEBOS_FEATURES
/**
 * FsmDbgEnableLoggingViaPmLogLib(): Enable FSM logging by 
 * sharing your PmLogContext. This is the preferred (more 
 * efficient) logging mechanism when building for WebOS 
 * userspace. 
 *  
 * @note WEBOS user-space-specific feature set; to access:
 *  
 *   #define STATE_MACHINE_ENGINE_WEBOS_FEATURES
 *   #include <PmStateMachineEngine/PalmFsmDbg.h>
 *  
 * @note FsmDbgEnableLoggingViaPmLogLib and FsmDbgEnableLogging 
 *       are mutually-exclusive.  FsmDbgEnableLoggingViaPmLogLib
 *       provides better performance than
 *       FsmDbgEnableLogging-based logging on WEBOS because
 *       calls outside the logging level may be optimized out
 *       efficiently at runtime _before_ the logging function's
 *       arguments are evaluated.
 *  
 * @note PmLogLib-based logging is not gated by 
 *       FsmDbgSetLogLevelThreshold().  PmLogLib provides its
 *       own mechanism for controlling output.  Refer to
 *       PmLogLib documentation for details.
 *  
 * @param pFsm Non-NULL pointer to a properly-initialized state
 *             machine.
 * @param logOptions Non-zero log options.  Pass one or more
 *                   enum FsmDbgLogOptions constants bitwise
 *                   OR'ed together.
 * @param logContext A pointer to a valid PmLogContext context 
 *                   instance for the FSM to use.  See PmLogLib
 *                   documentation/header for more info about
 *                   the context.
 * @param cookie A value that the FSM may display in its log 
 *               output to help with identification of the FSM
 *               in the log file.
 *  
 * @see FsmDbgDisableLogging 
 */
void
FsmDbgEnableLoggingViaPmLogLib(FsmMachine*  pFsm,
                               unsigned int logOptions,
                               PmLogContext pmlogContext,
                               const void*  cookie);
#endif ///STATE_MACHINE_ENGINE_WEBOS_FEATURES


/**
 * FsmDbgEnableLogging(): Enables FSM logging via platform-independendt
 * callback-based mechanism.
 * 
 * @note On WebOS, use FsmDbgEnableLoggingViaPmLogLib() instead.
 * 
 * @param pFsm Non-NULL pointer to a properly-initialized state
 *             machine.
 * @param logOptions Non-zero log options.  Pass one or more 
 *                   enum FsmDbgLogOptions constants bitwise
 *                   OR'ed together.
 * @param pLogCbFunc Non-NULL pointer to the logging callback
 *                   function.
 * @param cbCookie Cookie value to pass to FsmDbgLogLineFnType
 *                 callback.
 *
 * @example Example line-logger callback function:
 *
 * * #include <stdio.h>
 * * #include <stdarg.h>
 * *
 * * void
 * * StateMachineLogCb(FsmMachine* pFsm,
 * *                   void* cookie,
 * *                   const enum FsmDbgLogLevel level,
 * *                   const char* pFmt, ...)
 * * {
 * *     FsmDbgLogLineFnType StateMachineLogCb;
 * * 
 * *     va_list args;
 * *     va_start(args, pFmt);
 * * 
 * *     vprintf(pFmt, args);
 * * 
 * *     va_end(args);
 * * 
 * *     putchar('\n'); ///< Append End-Of-Line
 * * } 
 *  
 * @see FsmDbgEnableLoggingViaPmLogLib (preferred on WebOS) 
 * @see FsmDbgDisableLogging 
 * @see FsmDbgSetLogLevelThreshold 
 */
void
FsmDbgEnableLogging(FsmMachine*             pFsm,
                    unsigned int            logOptions,
                    FsmDbgLogLineFnType*    pLogCbFunc,
                    const void*             cbCookie);


/**
 * Set the minimum log-level at which output may be generated. 
 *  
 * @note FsmDbgSetLogLevelThreshold() applies _only_ to 
 *       callback-function-based logging (FsmDbgEnableLogging).
 *       If using FsmDbgEnableLoggingViaPmLogLib, then log
 *       output is gated by PmLogLib's own mechanism.
 * 
 * @param pFsm Non-NULL pointer to a properly-initialized state
 *             machine.
 * @param level A valid log level value or kFsmDbgLogLevelNone
 *              to suppress all log output.
 */
void
FsmDbgSetLogLevelThreshold(FsmMachine* pFsm, enum FsmDbgLogLevel level);


/**
 * Disables logging.
 * 
 * @param pFsm Non-NULL pointer to a properly-initialized state
 *             machine.
 * 
 */
void
FsmDbgDisableLogging(FsmMachine* pFsm);


/**
 * For debugging only;
 * 
 * @param pFsm Non-NULL, properly initialized state machine
 *             instance
 * 
 * @return const char* Pointer to name of the state machine
 *         supplied by user via FsmInitMachine().
 */
const char*
FsmDbgPeekMachineName(FsmMachine* pFsm);


/**
 * For debugging only: Returns a pointer to the current state in
 * the given FSM
 * 
 * @param pFsm Non-NULL, properly initialized state machine
 *             instance
 * 
 * @return const FsmState* Pointer to the current state in the
 *         given FSM; NULL if the FSM hasn't been started yet or
 *         is in the midst of a state transition.
 */
const FsmState*
FsmDbgPeekCurrentState(FsmMachine* pFsm);

/**
 * For debugging only: Returns the name of the given state.
 * 
 * @param pFsm Non-NULL, properly initialized state machine
 *             instance
 * @param pState Non-NULL, properly initialize state structure
 *               (@see FsmInitState()).
 * 
 * @return const char* Pointer to name of the state as supplied
 *         by user via FsmInitState().
 */
const char*
FsmDbgPeekStateName(const FsmState* pState);

/**
 * For debugging only: Returns a pointer to the parent state of
 * the given state in the given state machine.
 * 
 * @param pFsm Non-NULL, properly initialized state machine
 *             instance
 * @param pState Non-NULL pointer to state that has been
 *               inserted into the given state machine instance.
 * 
 * @return const FsmState* Pointer to the parent of the given
 *         state, or NULL if the given state is a top-level user
 *         state.
 */
const FsmState*
FsmDbgPeekParentState(FsmMachine* pFsm, const FsmState* pState);





#ifdef __cplusplus
}
#endif



#endif // STATE_MACHINE_ENGINE_FSM_DBG_H
