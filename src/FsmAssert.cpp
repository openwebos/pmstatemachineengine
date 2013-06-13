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
 * @file FsmAssert.cpp
 * 
 * @brief  Implementation for State Machine Engine's custom
 *         assert handler.
 * 
 * @note This was necessitated because normal handling of failed
 *       assertions on the current Nova 1.x toolchain
 *       (2009/12/09) was not allowing a stack trace to be
 *       generated. However, unhandled C++ exception did not
 *       suffer from this problem.
 * 
 * @author vitaly kruglikov
 * 
 * @version  1.0  2009-12-09  <vmk>  Initial creation
 *******************************************************************************
 */

#include <stdexcept>
#include <sstream>

#include "FsmBuildConfig.h"

#include "FsmAssert.h"



/**
 * ****************************************************************************
 */
#if FSM_CONFIG_USE_CUSTOM_ASSERT
void FsmCustomAssertFail(const char* predicate_str, const char* file,
                         unsigned int line, const char* func)
{
    std::ostringstream oss;

    oss << file << ":" << line << ": "  << func << ": Assertion '"
        << predicate_str << "' FAILED.";


    throw std::runtime_error(oss.str());
}
#endif
