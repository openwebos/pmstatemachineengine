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
 * @brief  Common routines for FSM test code.
 * 
 * @author vitaly
 * 
 * @version  0.01  2009-07-17  <vmk>  Initial creation
 * ****************************************************************************
 */

#include <stdio.h>

#include <stdarg.h>

#include <PmStateMachineEngine/PalmFsm.h>
#include <PmStateMachineEngine/PalmFsmDbg.h>
#include <PmStateMachineEngine/Cplusplus/PalmFsm.hpp>



void
StateMachineLogCb(FsmMachine* pFsm,
                  void* cookie,
                  const enum FsmDbgLogLevel level,
                  const char* pFmt, ...)
{
    FsmDbgLogLineFnType StateMachineLogCb;

    va_list args;
    va_start(args, pFmt);

    vprintf(pFmt, args);

    va_end(args);

    putchar('\n'); ///< Append End-Of-Line
}
