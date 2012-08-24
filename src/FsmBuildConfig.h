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
 * @file FsmBuildConfig.h
 * 
 * @brief  State Machine Engine's Build Configuration settings
 * 
 * @note Override the build settings via Makefile or equivalent
 *       mechanism in your build environment (BUT DO NOT ALTER
 *       THIS HEADER FILE ITSELF FOR THAT PURPOSE! THIS HEADER
 *       FILE MUST BE SHARED INTACT BETWEEN HOST, MODEM, AND
 *       TEST TOOLS.)
 * 
 * @author vitaly kruglikov
 * 
 * @version  1.0  2009-12-09  <vmk>  Initial creation
 * 
 *******************************************************************************
 */

#ifndef STATE_MACHINE_ENGINE_FSM_BUILD_CONFIG_H
#define STATE_MACHINE_ENGINE_FSM_BUILD_CONFIG_H

#ifdef __cplusplus 
extern "C" {
#endif


/**
 * Controls use of FsmCustomAssertFail by the state machine
 * engine's implemetnation; if zero, state machine engine will
 * use the standard assert
 */
#ifndef FSM_CONFIG_USE_CUSTOM_ASSERT
    #define FSM_CONFIG_USE_CUSTOM_ASSERT 0
#endif


/**
 * Define the appropriate inline attribute for inline functions
 */
#ifndef FSM_CONFIG_INLINE_FUNC
    #ifdef __GNUC__
        #define FSM_CONFIG_INLINE_FUNC  extern __inline
    #else
        #define FSM_CONFIG_INLINE_FUNC  extern __inline
    #endif
#endif


/**
 * FSM_CONFIG_WEBOS_FEATURES: Controls inclusion of 
 * WEBOS-specific features 
 *  
 * @note STATE_MACHINE_ENGINE_WEBOS_FEATURES is the public 
 *       API-equivalent of FSM_CONFIG_WEBOS_FEATURES
 */
#undef STATE_MACHINE_ENGINE_WEBOS_FEATURES

#ifndef FSM_CONFIG_WEBOS_FEATURES
    #define FSM_CONFIG_WEBOS_FEATURES 0
#else
    #if FSM_CONFIG_WEBOS_FEATURES
        #define STATE_MACHINE_ENGINE_WEBOS_FEATURES
    #endif
#endif


#ifdef __cplusplus
}
#endif



#endif // STATE_MACHINE_ENGINE_FSM_BUILD_CONFIG_H
