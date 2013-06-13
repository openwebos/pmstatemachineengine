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
 * @file FsmAssert.h
 * 
 * @brief  Private declarations for State Machine Engine's
 *         FSM_ASSERT macro.
 * 
 * @author vitaly kruglikov
 * 
 * @version  1.0  2009-12-09  <vmk>  Initial creation
 *******************************************************************************
 */

#ifndef STATE_MACHINE_ENGINE_FSM_ASSERT_H
#define STATE_MACHINE_ENGINE_FSM_ASSERT_H

#include "FsmBuildConfig.h"

#if (FSM_CONFIG_USE_CUSTOM_ASSERT)

    #ifdef	NDEBUG
        # define FSM_ASSERT(pred__)     (void (0))
    #else
    
        #ifdef __cplusplus 
        extern "C" {
        #endif
        
        void FsmCustomAssertFail(const char* predicate_str, const char* file,
                                  unsigned int line, const char* func);
        
        #define FSM_ASSERT(pred__) \
        ((pred__) \
         ? ((void)0) : FsmCustomAssertFail(#pred__, __FILE__, __LINE__, __func__))
        
        #ifdef __cplusplus
        }
        #endif
    #endif

#else
    #include <assert.h>

    #define FSM_ASSERT  assert
#endif





#endif // STATE_MACHINE_ENGINE_FSM_ASSERT_H
