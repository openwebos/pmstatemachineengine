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
 * @brief  Lightweight Finite State Machine Framework API.
 * 
 * An API for a hierarchical state machine framework.  The
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
 * @version  0.01  2009-07-10  <vmk>  Initial creation
 *******************************************************************************
 */

#ifndef STATE_MACHINE_ENGINE_FSM_H
#define STATE_MACHINE_ENGINE_FSM_H

#ifdef __cplusplus 
extern "C" {
#endif

/**
 * =============================================================================
 * Usage Info
 * =============================================================================
 * 
 * Introduction
 * ============
 * 
 * Palm State Machine Engine is a lightweight framework for
 * implementing "flat" and hierarchical finite state machines.
 * The framework's primary responsibilities are:
 * 
 *   1. Track the current state
 *   2. Dispatch user-defined events to the user-provided
 *      handler function of the current state, and propagate the
 *      event up the ancestry chain if needed
 *   3. Provide a consistent, orderly mechanism and events for
 *      state transitions.
 *   4. Error-checking and diagnostics
 * 
 * Each FSM is represented by an instance of the FsmMachine
 * structure, and each state is represented by an instance of
 * the FsmState structure.  User associates a state handler
 * callback function with each state instance.  The framework
 * dispatches framework-defined as well as user-defined events
 * by invoking such state handler callback function(s).
 * 
 * 
 * Portability
 * ===========
 * 
 * The framework's implementation makes every effort to limit
 * its use of the language to portable, broadly-supported
 * C-language syntax.  This was partially motivated by the lack
 * of support of certain C99 features (e.g., unnamed unions,
 * zero-length array members, stdbool.h) in compilers used for
 * building our modem firmware at the time of this writing.
 * 
 * 
 * Memory Management
 * =================
 * 
 * This state machine framework does not allocate any memory: it
 * relies exclusively on the user to provide all object
 * instances (FsmMachine and FsmState structures). This, along
 * with portability, should make it suitable for use in
 * user-space, kernel, as well as modem firmware, which uses a
 * non-standard memory-allocation API (non C-standard, that is).
 * 
 * 
 * C++ Support
 * ===========
 * 
 * The header PalmFsm.hpp defines a couple of thin "adapter"
 * classes that faciliate natural use of the framework in C++
 * applications.  These adapter classes enable events to be
 * dispatched to member functions of user-defined classes.
 * 
 * 
 * Thread-safety
 * =============
 * 
 * This API and its implementation are NOT thread-safe per given
 * FSM instance.  However, different FSM instances MAY "run" in
 * differrent threads.  A given FSM instance (or multiple FSM
 * instances) typically runs in a single thread (i.e., all State
 * Machine Engine API functions invoked on that FSM instance are
 * from the same thread)
 * 
 * 
 * RTC - Run to Completion
 * =======================
 * 
 * Users of the Finite State Machine MUST abide by the FSM
 * Run-to-Completion (RTC) rule: this means that the processing
 * of the current event by a given FSM intance MUST be allowed
 * to complete (i.e., FsmDispatchEvent returns) before
 * dispatching another event.  RTC is enforced by the
 * implementation via assertions.
 * 
 * 
 * Diagnostics
 * ===========
 * 
 * The Palm State Machine Engine provides a simple, yet
 * powerful, diagnostic and logging facility.  See PalmFsmDbg.h
 * for more details.
 * 
 * 
 * FSM Driver
 * ==========
 * 
 * Applications that have a need to dispatch events to an FSM
 * from contexts that would constitute a Run-to-Completion
 * Violation (e.g., from a different thread or from the scope of
 * the same state machine's event handler) may find it necessary
 * to implement a simple FSM "Driver" in order to facilitate
 * processing of asynchronous events by a given FSM instance
 * without violating the RTC requirement.  An FSM Driver would
 * thus consist of the following mechanisms:
 *  * Thread-safe Event Queue
 *  * Event Dispatcher (dispatches events from the Event Queue
 *    to the FSM in the context of a single thread)
 * 
 * The Palm State Machine Engine implementation presently does
 * NOT provide an FSM Driver.
 * 
 * 
 * Hierarchical Event Dispatch
 * ===========================
 * 
 * Whenever a user-defined event is dispatched to a given FSM
 * instance (see FsmDispatchEvent), it is first dispatched to
 * the handler function of the Current State; if the Current
 * State's event handler returns 0 (false), the event is
 * dispatched to the Current State's parent state (if any), and
 * so forth.  The event dispatch terminates once an event
 * handler in the chain returns non-zero (true) or all
 * user-defined ancestor states have been exhausted.
 * 
 * @note Palm State Machine Engine-defined events are NEVER
 *       forwarded to a state's ancestor (this includes
 *       kFsmEventEnterScope, kFsmEventExitScope, and
 *       kFsmEventBegin)
 * 
 * 
 * State Transitions
 * =================
 * 
 * The Palm State Machine Engine implements UML-2.0 Local state
 * transitions.  The following examples demonstrate how these
 * state transitions work by expressing them in terms of events
 * delivered to states during various state transition
 * scenrarios.
 * 
 * There are three types of state transitions within a given
 * instance of FSM:
 * 
 *      1. The Original State Transition: This is the very first
 *      state transition for any FSM instance.  It's initiated
 *      by the user via FsmStart() after initializing the FSM
 *      (FsmInitMachine), the FSM's states (FsmInitState), and
 *      adding the states to the FSM (FsmInsertState).
 * 
 *      2. Initial State Transition: This represents the typical
 *      initial transition that a state may perform ONLY in the
 *      scope of kFsmEventBegin dispatch.  The target state MUST
 *      be a proper descendant (a child or a child of a child,
 *      etc.) of the state receiving kFsmEventBegin.
 * 
 *      3. Regular State Transition (or simply Transition): This
 *      is the type of transition that may be requested by a
 *      state's handler ONLY from the scope of a user-defined
 *      event dispatch.
 * 
 * With the exception of the Original State Transition, which is
 * requested via FsmStart(), the other two types of state
 * transitions are initiated by calling FsmBeginTransition()
 * from the dispatch scope of permitted events as described
 * above.
 * 
 * @note It's a violation of the Run-to-Completion principle to
 *       request state transition more than once from the scope
 *       of the same event dispatch.
 * 
 * 
 * Examples of State Transitions
 * =============================
 * 
 * When describing events delivered to states in the examples
 * below, the following mappings apply:
 * 
 *  * ENTER    = kFsmEventEnterScope
 *  * EXIT     = kFsmEventExitScope
 *  * BEGIN    = kFsmEventBegin
 * 
 *  * "A <= ENTER" = Event ENTER dispatched to state A
 * 
 *  * "[ROOT]" is the implicit root state (highest-level parent
 *    state implicitly provided by the implementation).
 * 
 *  * "[ROOT] <-- A <-- B" = state hierarchy, where state A is
 *    the parent of state B 0i.e., state B is the child of state
 *    A)
 * 
 *  * "(E)" = the parentheses demarcate the "Current" State in
 *    the FSM.
 * 
 *  * "{B}" = the curly braces demarcate the state whose handler
 *    function initiated the state transition, if different from
 *    FSM's Current State.
 * 
 *  * "<K>" = the angle brackets demarcate the state that is the
 *    target of the state transition (i.e., the one that will
 *    become the new Current State upon completion of the state
 *    transition operation)
 * 
 *  * "<(D)>" = D is both the source and the target of the
 *    transition.
 * 
 * @note If in doubt about state transtions or event dispatches,
 *       enable logging of the given FSM instance and observe
 *       the output (see FsmDbgEnableLogging and
 *       FsmDbgSetLogLevelThreshold)
 * 
 * 
 * The Original Transition to state C via FsmStart():
 * --------------------------------------------------
 * 
 *                            D
 *                           /
 *                B <-- <C> <
 *               /           \
 * [ROOT] <-- A <             E
 *               \
 *                K <-- L
 * 
 * A <= ENTER
 * B <= ENTER
 * C <= ENTER
 * C <= BEGIN
 * 
 * 
 * Initial Transition from state C to state F:
 * -------------------------------------------
 * 
 * @note An Initial Transition MAY be initiated ONLY from the
 *       scope of the BEGIN event dispatch, and the target of
 *       the transition MUST be a proper descendant state.
 * 
 *                          D
 *                         /
 *                B <-- C <
 *               /         \
 * [ROOT] <-- A <           E <-- <F>
 *               \
 *                K <-- L
 * 
 * ...
 * C <= BEGIN
 * E <= ENTER
 * F <= ENTER
 * F <= BEGIN
 * 
 * 
 * Transition from C to itself:
 * ----------------------------
 * 
 *                              D
 *                             /
 *                B <-- <(C)> <
 *               /             \
 * [ROOT] <-- A <               E
 *               \
 *                K <-- L
 * 
 * C <= EXIT
 * C <= ENTER
 * C <= BEGIN
 * 
 * 
 * Transition from sibling D to sibling E:
 * ---------------------------------------
 * 
 *                          (D)
 *                         /
 *                B <-- C <
 *               /         \
 * [ROOT] <-- A <           <E>
 *               \
 *                K <-- L
 * 
 * @note State C is not exited (local transition rule).
 * 
 * D <= EXIT
 * E <= ENTER
 * E <= BEGIN
 * 
 * 
 * Transition from descendant D to anscestor B:
 * --------------------------------------------
 * 
 *                            (D)
 *                           /
 *                <B> <-- C <
 *               /           \
 * [ROOT] <-- A <             E
 *               \
 *                K <-- L
 * 
 * @note State B is not exited (local transition rule).
 * 
 * D <= EXIT
 * C <= EXIT
 * B <= BEGIN
 * 
 * 
 * Transition from anscestor B to descendant D:
 * --------------------------------------------
 * 
 *                            <D>
 *                           /
 *                (B) <-- C <
 *               /           \
 * [ROOT] <-- A <             E
 *               \
 *                K <-- L
 * 
 * @note State B is not exited (local transition rule).
 * 
 * C <= ENTER
 * D <= ENTER
 * D <= BEGIN
 * 
 * 
 * Transition from C to L:
 * -----------------------
 * 
 *                            D
 *                           /
 *                B <-- (C) <
 *               /           \
 * [ROOT] <-- A <             E
 *               \
 *                K <-- <L>
 * 
 * @note State A is not exited (local transition rule).
 * 
 * C <= EXIT
 * B <= EXIT
 * K <= ENTER
 * L <= ENTER
 * L <= BEGIN
 * 
 * 
 * Anscestor state B of Current State D requests transition to
 * state K:
 * -----------------------------------------------------------
 * 
 *                            (D)
 *                           /
 *                {B} <-- C <
 *               /           \
 * [ROOT] <-- A <             E
 *               \
 *                <K> <-- L
 * 
 * @note In this example, a user-defined event was propagated
 *       to state B (because event handlers of states D and C
 *       returned false), and the event handler of state B
 *       requested transition to state K.
 * 
 * @note First, all states in the active chain up to, but not
 *       including the requesting state B will be exited. Then,
 *       the normal transition rules will apply as if B was the
 *       Current State and it initiated the transition.
 * 
 * @note State A is not exited (local transition rule).
 * 
 * D <= EXIT
 * C <= EXIT
 * B <= EXIT
 * K <= ENTER
 * K <= BEGIN
 * 
 * 
 * Anscestor state A of Current State C requests transition to
 * state E (a descendant of C):
 * -------------------------------------------------------------
 * 
 *                              D
 *                             /
 *                  B <-- (C) <
 *                 /           \
 * [ROOT] <-- {A} <             <E>
 *                 \
 *                  K <-- L
 * 
 * @note First, all states in the active chain up to, but not
 *       including the requesting state A, will be exited first.
 *       Then, the normal transition rules will apply as if A
 *       was the Current State and it initiated the transition.
 * 
 * @note State A is not exited (local transition rule).
 * 
 * C <= EXIT
 * B <= EXIT
 * B <= ENTER
 * C <= ENTER
 * E <= ENTER
 * E <= BEGIN
 * 
 */

enum {
    /// Maximum supported state nesting levels in a hierarchical state machine 
    kFsmMaxStateNestingDepth = 10
};

/**
 * A Finite State Machine
 * 
 * @note WARNING: Changing the size or alignment of this
 *       structure will break binary API compatibility.
 * 
 * @note All fields ending in underscore are for internal use
 *       only and off-limits to users of the API
 */
typedef struct {
    void*                   opaque_[22];
} FsmMachine;

/**
 * A state in a state machine
 * 
 * @note WARNING: Changing the size or alignment of this
 *       structure will break binary API compatibility.
 * 
 * @note All fields ending in underscore are for internal use
 *       only and off-limits to users of the API
 */
typedef struct {
    void*                   opaque_[3];
} FsmState;


/// Reserved event identifiers
typedef enum {
    /**
     * kFsmEventEnterScope: Dispatched to each state being entered.
     * 
     * This is a good place to perform state-specific
     * initialization.
     * 
     * @note State transitions are NOT allowed from this event.
     */
    kFsmEventEnterScope     = -1,

    /**
     * kFsmEventExitScope: Dispatched to each state being exited.
     * 
     * This is a good place to perform state-specific clean-up.
     * 
     * @note Dispatched in the context of FsmBeginTransition() call.
     * 
     * @note State transitions are NOT allowed from this event.
     */
    kFsmEventExitScope      = -2,

    /**
     * kFsmEventBegin: Dispatched only to the target of the
     * transition after kFsmEventEnterScope, if any.
     * 
     * This is a good place for making an initial state transition,
     * if needed.  An Initial State Transition is a state transition
     * to a proper child (immediate or transitive) of the state
     * receiving the kFsmEventBegin event..
     * 
     * @note Only transitions to a proper child state (immediate or
     *       transitive) are allowed from this event.
     */
    kFsmEventBegin          = -3,

    /// To be used by users as the start of user-defined event id's
    kFsmEventFirstUserEvent = 0 
} FsmEventId;

/// Event identifier type
typedef int FsmEventIdType;

/// Base event structure
typedef struct {
    FsmEventIdType evtId;    ///< FsmEventId or user-defined event id
} FsmEvent;



/**
 * State event handler callback function type.  Called by SME to
 * dispatch an event to the given state.
 * 
 * @param pState Non-NULL pointer to the state that should
 *               handle the event.
 * @param pFsm Non-NULL pointer to the state's state machine.
 * @param pEvt Non-NULL pointer to the event structure
 * 
 * @return int true (non-zero) if the event was handled; false
 *         (zero) if not handled.  For user-defined events, if
 *         the event isn't handled, it is passed to the
 *         user-defined parent (if any) of the given state.
 * 
 * @note A state handler MUST always return true after calling
 *       FsmBeginTransition(), because FsmBeginTransition() may
 *       cause parent state(s) to be exited.
 */
typedef int FsmStateHandlerFnType(FsmState* pState,
                                  FsmMachine* pFsm,
                                  const FsmEvent* pEvt);

/**
 * Initializes an FSM instance
 * 
 * @param pFsm Non-NULL pointer to an instance of FsmMachine
 *             structure to be initialized.
 * @param pName FSM name to use for logging and debugging.  FSM
 *              saves the given pointer (i.e., doesn't copy the
 *              string).  The names "FSM" and "UNNAMED-FSM" are
 *              reserved.
 */
void
FsmInitMachine(FsmMachine* pFsm, const char* pName);


/**
 * Initializes an FSM state instance.
 * 
 * @param pState Non-NULL pointer to an instance of FsmState
 *               structure to be initialized.
 * @param pStateHandlerCbFunc Non-NULL state event handler
 *                            function
 * @param pName State name to use for logging and debugging.
 *              FSM saves the given pointer (i.e., doesn't copy
 *              the string).  The names "ROOT" and
 *              "UNNAMED-STATE" are reserved.
 */
void
FsmInitState(FsmState* pState, FsmStateHandlerFnType* pStateHandlerCbFunc,
             const char* pName);

/**
 * Inserts a state instance into an initialized state machine
 * 
 * @note WARNING: Do NOT insert states after calling FsmStart()!
 * 
 * @note Do NOT insert a given state instance more than once
 *       within a given state machine instance's lifetime.
 * 
 * @param pFsm Non-NULL pointer to an initialized state machine.
 * @param pState Non-NULL pointer to state being inserted. Note:
 *               It's the caller's responsibility to avoid
 *               cycles.
 * @param pParent NULL for a top super-state; or non-NULL
 *                pointer to parent state that was already
 *                inserted into this state machine.  Note: It's
 *                the caller's responsibility to avoid cycles.
 */
void
FsmInsertState(FsmMachine* pFsm, FsmState* pState, FsmState* pParent);


/**
 * Starts FSM at the given initial state.
 * 
 * @note WARNING: DO NOT call this from a state event handler or
 *       any other callback of the given state machine.
 * 
 * @note Will not cause any states to be exited.  If a
 *       previously-started state machine is being re-started,
 *       it's the caller's responsibility to ensure that states
 *       are properly cleaned up: this may be accomplished by
 *       designating a "final" state (a simple user-defined
 *       state) and dispatching a user-defined event to the
 *       state machine that causes an unconditional transition
 *       to the "final" state.
 * 
 * @param pFsm Properly initialized state machine instance
 * 
 * @param pInitialState Non-NULL pointer to the initial state in
 *                      the state machine.
 */
void
FsmStart(FsmMachine* pFsm, FsmState* pInitialState);


/**
 * Dispatches a user-defined event to the given state machine.
 * 
 * If the current state's handler doesn't handle the event
 * (returns false/zero), SME will iteratively pass the event to
 * parent of the current state, and so forth until one of the
 * ancestor states handles the event (returns true/non-zero) or
 * all user states in the hierarchy are exhausted.
 * 
 * @note SME is not thread-safe for any given state machine.
 * 
 * @note UML (and this implementation) imposes RTC (Run to
 *       Completion) semantics, meaning that processing of an
 *       event MUST BE completed before dispatching the next
 *       event to a given state machine (the only exception is
 *       kFsmEventExitScope, which is dispatched by
 *       FsmBeginTransition()). So, do NOT dispatch any event
 *       from the context of a state handler function (or a
 *       function called from it, and so forth) to the
 *       originating FSM, or BAD THINGs will happen.
 * 
 * @param pFsm Non-NULL pointer to an initialized/started state
 *             machine
 * 
 * @param pEvt Non-NULL pointer to user-defined event.  The
 *             event ID MUST NOT be one of the SME reserved
 *             events ( kFsmEventEnterScope, kFsmEventExitScope,
 *             kFsmEventBegin).
 * 
 * @note You may extend the event structure to include extra
 *       data by defining a new structure with FsmEvent as the
 *       first field, and your extra data below it. E.g.,
 * 
 *       in C:
 * 
 *       typedef struct {
 *         FsmEvent  base; ///< MUST BE FIRST
 * 
 *         int   myInt;
 *         void* myPtr;
 *       } MyEvent;
 * 
 *       MyEvent evt = {{...}, ...};
 *       FsmDispatchEvent(pFsm, &evt.base);
 * 
 *       Or in C++:
 * 
 *       struct MyEvent : public FsmEvent {
 *         int myInt;
 *         void* myPtr;
 *       }
 * 
 *       MyEvent evt;
 *       /// Init evt
 *       ...
 *       FsmDispatchEvent(pFsm, evt);
 * 
 * @return int true (non-zero) if the event was handled (by the
 *         given state or one of its parent states); false
 *         (zero) if the event was not handled.
 */
int
FsmDispatchEvent(FsmMachine* pFsm, const FsmEvent* pEvt);


/**
 * May be called only from the user's state event handler to
 * initiate a transition to another state of the same FSM.
 * 
 * @note 1. A state handler that calls FsmBeginTransition() MUST
 *       return true (non-zero), or bad things will happen.
 *       It's impossible to pass a user-defined event to parent
 *       state handler(s) after calling FsmBeginTransition().
 * 
 * @note 2. UML prohibits state transitions during state Entry
 *       and Exit; so don't call this function when handling
 *       kFsmEventEnterScope and kFsmEventExitScope, or bad
 *       things will happen.
 * 
 * @note 3. When handling kFsmEventBegin, this function may be
 *       called to initiate what's known as the "initial
 *       transition": in this case, the target state MUST be a
 *       descendant of the given state (a child or the
 *       child's child, etc.). Also, see note #1 above.
 * 
 * @note 4. For User-defined events, the target state may be any
 *       valid state in the given state machine. In this case,
 *       SME exits all the required states, but postpones the
 *       entry/begin transactions until control returns from the
 *       state handler function. WARNING: this implies that the
 *       calling state-handler may be re-entered by SME to
 *       deliver the kFsmEventExitScope event (if the state
 *       tranistion rules require that the requesting state be
 *       exited for the given transition). Also, see note #1
 *       above.
 * 
 * This function facilitates the following UML-prescribed state
 * transition ordering:
 * 
 *   * Perform all necessary Exit actions (FsmBeginTransition()
 *     triggers these before returning)
 *   * Perform actions associated with the given state
 *     transition, if needed (user's state handler code)
 *   * Perform all necessary Entry actions (after control
 *     returns from state handler to FsmDispatchEvent())
 * 
 * @param pFsm Non-NULL pointer to an initialized/started state
 *             machine
 * @param pTargetState Non-NULL pointer to target of the
 *                     transition in the given state machine.
 */
void
FsmBeginTransition(FsmMachine* pFsm, FsmState* pTargetState);




#ifdef __cplusplus
}
#endif



#endif // STATE_MACHINE_ENGINE_FSM_H
