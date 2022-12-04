#ifndef __PROGTEST__

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

using State = unsigned int;
using Symbol = uint8_t;

struct NFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;
};

struct DFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, State> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;
};

void print_nfa(std::string text, const NFA& a);
void print_dfa(std::string text, const DFA& a);

#endif /* not __PROGTEST__ */

typedef std::set<State> Combined_state;

/**
 * print \a state in form:
 * { 1, 2, 3 }
 */
void print_comb_state(const Combined_state& state) {
    std::cout << "{ ";
    for (auto it = state.begin(); it != state.end(); ++it) {
        std::cout << *it;
        if (std::next(it) != state.end())
            /* comma after not last */
            std::cout << ",";
        std::cout << " ";
    }
    std::cout << "}";
}

/*
 * This typedefs set of sets of States
 */
typedef std::set<Combined_state> Partition;

/**
 * print \a part in form:
 * { { 1, 2, 3 }, { 4, 5 } }
 */
void print_partition(const Partition& part) {
    std::cout << "{ ";
    for (auto it = part.begin(); it != part.end(); ++it) {
        print_comb_state(*it);
        if (std::next(it) != part.end())
            std::cout << ", ";
    }
    std::cout << " }\n";
}

/**
 * This is super class for automates whose states are Combined_state-s
 */
class FAx {
protected:
    std::set<Symbol> m_Alphabet;
    std::set<Combined_state> m_States;
    std::set<Combined_state> m_FinalStates;
    Combined_state m_InitialState;
public:
    void set_alphabet(const std::set<Symbol>& alphabet) { m_Alphabet = alphabet; }
    void set_alphabet(Symbol sym) { m_Alphabet.insert(sym); }
    void add_state(const Combined_state& state);
    void add_final_state(const Combined_state& state);
    void set_init_state(const Combined_state& state);
    const std::set<Symbol>& get_alphabet(void);
    const std::set<Combined_state>& get_states(void);
    void print(const std::string& txt);
};

void FAx::add_state(const Combined_state& state)
{
    m_States.insert(state);
}

void FAx::add_final_state(const Combined_state& state)
{
    m_FinalStates.insert(state);
}

void FAx::set_init_state(const Combined_state& state)
{
    m_InitialState = state;
}

const std::set<Symbol>& FAx::get_alphabet(void)
{
    return m_Alphabet;
}

const std::set<Combined_state>& FAx::get_states(void)
{
    return m_States;
}

void FAx::print(const std::string& txt)
{
    std::cout << txt << std::endl;
    std::cout << "Alphabet : {";
    for (auto i : m_Alphabet) {
        std::cout << " '" << i << "'";
    }
    std::cout << " }\n";
    
    std::cout << "States:\n";
    print_partition(m_States);
    
    std::cout << "Final States:\n";
    print_partition(m_FinalStates);
    
    std::cout << "Initial State : ";
    print_comb_state(m_InitialState);
    std::cout << "\n";
}


/**
 * This is NFA where states are represented as std::set<State>
 */
class NFAx: public FAx {
private:
    std::map<std::pair<Combined_state, Symbol>, std::set<Combined_state>> m_Transitions;
public:
    NFAx() {};
    NFAx(const NFA& nfa);
    void add_transition(const std::pair<Combined_state, Symbol>& key,
                        const std::set<Combined_state>& value);
    NFA nfax2nfa();
    void print(const std::string& txt);
};

NFAx::NFAx(const NFA& nfa)
{
    m_Alphabet = nfa.m_Alphabet;
    for (auto i : nfa.m_States) {
        m_States.insert({i});
    }
    for (auto i : nfa.m_Transitions) {
        Partition p;
        for (auto j : i.second) {
            p.insert({j});
        }
        m_Transitions.insert({{{i.first.first}, i.first.second}, p});
    }
    m_InitialState = {nfa.m_InitialState};
}

void NFAx::add_transition(const std::pair<Combined_state, Symbol>& key,
                          const std::set<Combined_state>& value)
{
    m_Transitions.insert({key, value});
}

NFA NFAx::nfax2nfa()
{
    NFA nfa;
    std::map<Combined_state, State> cs2c;
    State s = 1;
    
    nfa.m_Alphabet = get_alphabet();
    for (auto i : m_States) {
        cs2c.insert({i, s});
        nfa.m_States.insert(s++);
    }
    for (auto i : m_FinalStates) {
        auto rc = cs2c.find(i);
        nfa.m_FinalStates.insert(rc->second);
    }
    auto rc = cs2c.find(m_InitialState);
    nfa.m_InitialState = rc->second;
    for (auto i : m_Transitions) {
        /* i is { (Combined_state, Symbol), set<Combined_state> */
        Combined_state cs = i.first.first;
        Symbol sym = i.first.second;
        State state = cs2c.find(cs)->second;

        /* set<Combined_state> i.second -> set<States> state */
        Combined_state states;
        for (auto j : i.second) {
            /* j is Combined_state */
            states.insert(cs2c.find(j)->second);
        }
        /* (State, Symbol) -> Combined_state */
        nfa.m_Transitions.insert({{state, sym}, states});
    }
    return nfa;
}

void NFAx::print(const std::string& txt)
{
    std::cout << "NFA: ";
    FAx::print(txt);
    std::cout << "Transitions:\n";
    int counter = 0;
    for (auto j : m_Transitions) {
        std::cout << "\t"  << counter++ << " ( ";
        print_comb_state(j.first.first);
        std::cout << ", '" << j.first.second << "' ) -> ";
        print_partition(j.second);
    }
}

/**
 * This is DFA where states are represented as std::set<State>
 */
class DFAx: public FAx {
private:
    std::map<std::pair<Combined_state, Symbol>, Combined_state> m_Transitions;
public:
    DFAx() {};
    void add_transition(const std::pair<Combined_state, Symbol>& key,
                        const Combined_state& value);
    DFA dfax2dfa();
    void print(const std::string& txt);
};

void DFAx::add_transition(const std::pair<Combined_state, Symbol>& key,
                          const Combined_state& value)
{
    m_Transitions.insert({key, value});
}

DFA DFAx::dfax2dfa()
{
    DFA dfa;
    std::map<Combined_state, State> cs2c;
    State s = 1;
    
    dfa.m_Alphabet = get_alphabet();
    for (auto i : m_States) {
        cs2c.insert({i, s});
        dfa.m_States.insert(s++);
    }
    for (auto i : m_FinalStates) {
        auto rc = cs2c.find(i);
        dfa.m_FinalStates.insert(rc->second);
    }
    auto rc = cs2c.find(m_InitialState);
    dfa.m_InitialState = rc->second;
    for (auto i : m_Transitions) {
        /* i is { (Combined_state, Symbol), Combined_state */
        State state = cs2c.find(i.first.first)->second;
        Symbol sym = i.first.second;

        dfa.m_Transitions.insert({{state, sym}, cs2c.find(i.second)->second});
    }
    return dfa;
}

void DFAx::print(const std::string& txt)
{
    std::cout << "DFAx: ";
    FAx::print(txt);
    std::cout << "Transitions:\n";
    int counter = 0;
    for (auto j : m_Transitions) {
        std::cout << "\t"  << counter++ << " ( ";
        print_comb_state(j.first.first);
        std::cout << ", '" << j.first.second << "' ) -> ";
        print_comb_state(j.second);
        std::cout << std::endl;
    }
}


// Combined states needs to be mapped into states.
// This map is filled during determinization of combined states of DFA.
std::map<Combined_state, State> combined_states;
State dfa_state = 0;

void reset_combined_states() {
    combined_states.clear();
    dfa_state = 0;
}

// Return value:
//      true  if combined_state is added
//      false otherwise
bool store_combined_state(Combined_state state) {
    // Check  that \a states is in combined_states
    auto pos = combined_states.find(state);
    if (pos != combined_states.end()) {
        return false;
    }
    combined_states.insert({ state, dfa_state });
    dfa_state++;

    return true;
}

Partition new_partition(const Partition& P, const DFA& dfa) {
    Partition T;

    for (auto S : P) {
        std::set<State> S1;
        std::set<State> S2;
        for (auto a : dfa.m_Alphabet) {
            // Partition S by a
            std::set<State> S1Target;
            S1 = S1Target;
            S2 = S1Target;
            for (auto s : S) {
                auto m = dfa.m_Transitions.find({ s, a });
                if (m == dfa.m_Transitions.end()) {
                    std::cout << "{ " << s << ", " << a << " } -> empty\n";
                    continue;
                }
                // Look for set which contains m->second
                for (auto tmp : P) {
                    auto rc = tmp.find(m->second);
                    if (rc == tmp.end()) {
                        continue;
                    }
                    if (S1.empty()) {
                        S1Target = tmp;
                        S1.insert(s);
                    }
                    else {
                        if (tmp == S1Target) {
                            S1.insert(s);
                        }
                        else {
                            S2.insert(s);
                        }
                    }
                }
            }
            if (!S2.empty()) {
                break;
            }
        } // For alphabet
        T.insert(S1);
        if (!S2.empty()) {
            T.insert(S2);
        }
    }
    return T;
}

Combined_state partition_find(Partition partition, State state) {
    for (auto i : partition) {
        auto pos = i.find(state);
        if (pos != i.end()) {
            return i;
        }
    }

    return Combined_state();
}


// Find and return the biggest value of NFA states
State find_delta_state(const NFA& a) {
    State max_state = 0;
    for (auto i : a.m_States) {
        if (i > max_state) {
            max_state = i;
        }
    }

    return max_state + 1;
}

// Modify all states of NFA a by adding delta
void increase_states_by_delta(NFA& a, State delta) {
    // Modify set of states
    std::set<State> states;
    for (auto i : a.m_States) {
        states.insert(i + delta);
    }
    a.m_States = states;

    // Modify set of final states
    states.clear();
    for (auto i : a.m_FinalStates) {
        states.insert(i + delta);
    }
    a.m_FinalStates = states;

    // Modify initial state
    a.m_InitialState += delta;

    // Modify transition function
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    for (auto j : a.m_Transitions) {
        std::pair<State, Symbol> key = j.first;
        key.first += delta;
        std::set<State> value;
        for (auto k : j.second) {
            value.insert(k + delta);
        }
        transitions.insert({ key, value });
    }

    a.m_Transitions = transitions;
}

// Unify two NFAs with epsilon transition
// Input:
//      a, b NFAs to unify
// Output:
//      NFA - L(NFA) = L(a) U L(b)
// Lecture 3, page 12
NFA unify_nfa(const NFA& a, const NFA& b) {
    NFA res;
    res.m_Alphabet = a.m_Alphabet;
    for (auto i : b.m_Alphabet) {
        res.m_Alphabet.insert(i);
    }
    
    // This is to gurantee NFAs don't have common states
    NFA b1 = b;
    increase_states_by_delta(b1, find_delta_state(a));
    
    // 1. Q <- Q1 U Q2 U {q0}
    res.m_States = a.m_States;
    for (auto i : b1.m_States) {
        res.m_States.insert(i);
    }
    res.m_InitialState = find_delta_state(b1);
    res.m_States.insert(res.m_InitialState);

    // Compose transition function of NFA res
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    
    // 3. delta(q, a) <- delta1(q, a)
    transitions = a.m_Transitions;
    
    // 4. delta(q, a) <- delta2(q, a)
    for (auto i : b1.m_Transitions) {
        transitions.insert(i);
    }

    // 2.   delta(q0, epsilon) <- {q01, q02}
    std::pair<State, Symbol> key = {res.m_InitialState, '\0'};
    std::set<State> value = { a.m_InitialState, b1.m_InitialState };
    transitions.insert({ key, value });
    res.m_Transitions = transitions;

    // 5. F <- F1 U F2
    res.m_FinalStates = a.m_FinalStates;
    for (auto i : b1.m_FinalStates) {
        res.m_FinalStates.insert(i);
    }
        
    return res;
}


// Calculates epsilon closure for a state \s of NFA \a by definition lecture 3 p. 25
std::set<State> e_closure(const NFA& a, State s) {
    std::set<State> res = { s };
    unsigned long cnt = 1;
    while (1) {
        for (auto i : res) {
            std::pair<State, Symbol> key = { i, '\0' };
            auto pos = a.m_Transitions.find(key);
            if (pos == a.m_Transitions.end()) {
                // Epsilon transition for this state not found
                continue;
            }
            // pos->second is set of states
            for (auto j : pos->second) {
                res.insert(j);
            }
        }
        if (res.size() == cnt) {
            break;
        }

        cnt = res.size();
    }

#ifdef DEBUG
    std::cout << "Epsilon closure for state " << s << "\n";
    for (auto k : res) {
        std::cout << k << " ";
    }
    std::cout << "\n"; 
#endif
    return res;
}

bool is_set_intersect_empty(const std::set<State>& a, const std::set<State>& b) {
    for (auto i : a) {
        for (auto j : b) {
            if (i == j) {
                return false;
            }
        }
    }

    return true;
}

// Conversion NFA with epsilon transitions into NFA without epsilon transitions
// Lecture 2, p. 26
NFA e_transition_removal(const NFA& a) {
    NFA res;
    
    res.m_States = a.m_States;
    res.m_Alphabet = a.m_Alphabet;
    res.m_InitialState = a.m_InitialState;
    
    // Compose delta'(transition function on NFA res)
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    for (auto state : res.m_States) {
        std::set<State> e_clos = e_closure(a, state);
        for (auto symbol : res.m_Alphabet) {
            std::pair<State, Symbol> key = { state, symbol };
            std::set<State> value;
            for (auto clos_state : e_clos) {
                std::pair<State, Symbol> clos_key = { clos_state, symbol };
                auto pos = a.m_Transitions.find(clos_key);
                if (pos == a.m_Transitions.end()) {
                    // Not found
                    continue;
                }
                // pos->second is set of states
                for (auto j : pos->second) {
                    value.insert(j);
                }
            }
            transitions.insert({ key, value });
        }
    }
    res.m_Transitions = transitions;

    // Compose F'(final states of NFA res)
    std::set<State> fin_states;
    for (auto q : res.m_States) {
        std::set<State> e_clos = e_closure(a, q);
        if (!is_set_intersect_empty(e_clos, a.m_FinalStates)) {
            fin_states.insert(q);
        }
    }

    res.m_FinalStates = fin_states;

    return res;
}

// Identification and removal unreachable states (Lecture 2, p. 17)
// Input: DFA
// Output: DFA without unreachable states
DFA unreachable_states_removal(const DFA& a) {
    Combined_state Q_cur = { a.m_InitialState };
    Combined_state Q_next = Q_cur;
    while (1) {
        for (auto state : Q_cur) {
            for (auto sym : a.m_Alphabet) {
                auto pos = a.m_Transitions.find({ state, sym });
                if (pos == a.m_Transitions.end()) {
                    continue;
                }
                Q_next.insert(pos->second);
            }
        }
        if (Q_cur == Q_next) {
            break;
        }
        Q_cur = Q_next;
    }

    std::cout << "Reachable states:\n";
    print_comb_state(Q_cur);
    std::cout << "\n";

    return DFA();
}

// NFA to DFA conversion using subset construction
// Input:
//      NFA    
//  Output:
//      DFA
//  Lecture 3, p. 3 (NFA determinization)
DFA nfa_determinization(const NFA& a) {
    DFA res;
    std::set<Combined_state> Q;
    std::map<std::pair<Combined_state, Symbol>, Combined_state> transitions;
    std::set<Combined_state> tmp;
    tmp.insert({ a.m_InitialState });
    store_combined_state({ a.m_InitialState });
    // Determine set of states and transition function of DFA
    while (1) {
        if (tmp.empty()) {
            break;
        }
        std::set<Combined_state> tmp2;
        for (auto comb_state : tmp) {
            for (auto symbol : a.m_Alphabet) {
                Combined_state uni;
                for (auto state : comb_state) {
                    auto pos = a.m_Transitions.find({ state, symbol });
                    if (pos != a.m_Transitions.end()) {
                        // Add all states from pos.second
                        for (auto s : pos->second) {
                            uni.insert(s);
                        }

                    }
                }
                // Try to save new combined state
                if (store_combined_state(uni)) {
                    tmp2.insert(uni);
                }
                transitions.insert({ { comb_state, symbol }, uni });
            }
            Q.insert(comb_state);
        }

        tmp = tmp2;
#ifdef DEBUG        
        // Print transitions
        std::cout << "{\n";
        for (auto i : transitions) {
            auto value = i.second;
            auto cs = i.first.first;
            auto sym = i.first.second;
            std::cout << "{ ";
            print_comb_state(cs);
            std::cout << ", " << sym << "} -> ";
            print_comb_state(value);
            std::cout << "\n";
        }
        std::cout << "}\n";

        // Print states of DFA
        std::cout << "{\n";
        for (auto i : Q) {
            std::cout << "  ";
            print_comb_state(i);
            std::cout << "\n";
        }
        std::cout << "}\n";
#endif
    }
    // Determine set of final states
    std::set<Combined_state> F;
    for (auto comb_state : Q) {
        for (auto state : comb_state) {
            auto pos = a.m_FinalStates.find(state);
            if (pos != a.m_FinalStates.end()) {
                F.insert(comb_state);
                break;
            }
        }
    }

    // Build DFA
    // res.m_States
    for (auto comb_state : Q) {
        auto pos = combined_states.find(comb_state);
        if (pos == combined_states.end()) {
            std::cerr << "Combined state not found";
            exit(1);
        }
        res.m_States.insert(pos->second);
    }

    // res.m_Alphabet
    res.m_Alphabet = a.m_Alphabet;

    // res.m_InitialState
    auto pos = combined_states.find({ a.m_InitialState });
    if (pos == combined_states.end()) {
        std::cerr << "Initial combined state not found";
        exit(1);
    }
    res.m_InitialState = pos->second;

    // res.m_FinalStates
    for (auto comb_state : F) {
        auto pos = combined_states.find(comb_state);
        if (pos == combined_states.end()) {
            std::cerr << "Combined state not found";
            exit(1);
        }
        res.m_FinalStates.insert(pos->second);
    }

    // res.m_Transition
    for (auto i : transitions) {
        auto key = i.first;
        auto value = i.second;
        //  TODO: pos must be found!
        auto pos = combined_states.find(key.first);
        auto pos2 = combined_states.find(value);
        res.m_Transitions.insert({ { pos->second, key.second }, pos2->second });
    }

    return res;
    
}

/**
 * Convert NFA \a a to DFA
 * Subset construction algorithm (Lecture 3, p. 3)
 */
DFA nfa2dfa(const NFA& a)
{
    DFAx dfax;
    
    dfax.set_alphabet(a.m_Alphabet);
    dfax.add_state({ a.m_InitialState });
    dfax.set_init_state({a.m_InitialState});
    
    std::set<Combined_state> states = dfax.get_states();
    while (!states.empty()) {
        std::set<Combined_state> added_states;
        for (auto cs : states) {
            for (auto sym : a.m_Alphabet) {
                /* compose new (potentially) state */
                Combined_state uni;
                for (auto state : cs) {
                    auto pos = a.m_Transitions.find({ state, sym });
                    if (pos != a.m_Transitions.end()) {
                        // Add all states from pos.second
                        for (auto s : pos->second) {
                            uni.insert(s);
                        }
                    }
                }
                if (uni.size() == 0) {
                    /* (cs, sym) -> nowhere. add dead state */
                    uni.insert(666);
                }
                if (dfax.get_states().find(uni) == dfax.get_states().end()) {
                    /* new state is built, add to DFAx */
                    added_states.insert(uni);
                    dfax.add_state(uni);
                }
                dfax.add_transition({ cs, sym }, uni);
            }
        }
        
        states = added_states;
    }
    
    /* final states */
    for (auto i : dfax.get_states()) {
        /* i is Combined_state */
        for (auto j : i) {
            if (a.m_FinalStates.find(j) != a.m_FinalStates.end()) {
                dfax.add_final_state(i);
                break;
            }
        }
    }
    
    dfax.print("determinized");
    return dfax.dfax2dfa();
}

// DFA minimization using partinioning method
// Lecture 3. p. 31
DFA dfa_minimization(const DFA& a) {
    DFA res;
    Partition partition;
    Partition partition2;

    // Initial partition
    //  { a.FinalStates, a.States \ a.FinalStates }
    partition.insert(a.m_FinalStates);
    Combined_state nonfinal_states;
    std::set_difference(a.m_States.begin(), a.m_States.end(),
        a.m_FinalStates.begin(), a.m_FinalStates.end(),
        std::inserter(nonfinal_states, nonfinal_states.end()));
    partition.insert(nonfinal_states);

    while (1) {
        std::cout << "********\n";
        print_partition(partition);
        partition2 = new_partition(partition, a);
        if (partition == partition2) {
            break;
        }
        partition = partition2;
    }

    // Build minimized DFA
    DFAx dfax;
    dfax.set_alphabet(a.m_Alphabet);
    std::map<Combined_state, State> class2state;
    for (auto i : partition) {
        dfax.add_state(i);
        // Determine initial state
        auto init_state = i.find(a.m_InitialState);
        if (init_state != i.end()) {
            dfax.set_init_state(i);
        }
        // Determine final states
        for (auto j : a.m_FinalStates) {
            auto fin_state = i.find(j);
            if (fin_state != i.end()) {
                dfax.add_final_state(i);
            }
        }
    }

    // Determine transitions
    for (auto tr : a.m_Transitions) {
        // (state, sym) -> value
        auto state = tr.first.first;
        auto sym = tr.first.second;
        auto value = tr.second;
        Combined_state cs_state;
        Combined_state cs_value;
        for (auto cs : dfax.get_states()) {
            auto pos = cs.find(state);
            if (pos != cs.end()) {
                cs_state = cs;
                break;
            }
        }
        for (auto cs : dfax.get_states()) {
            auto pos = cs.find(value);
            if (pos != cs.end()) {
                cs_value = cs;
                break;
            }
        }

        dfax.add_transition({ cs_state, sym }, cs_value);
    }
    dfax.print("Min");

    return dfax.dfax2dfa();
}

DFA unify(const NFA& a, const NFA& b) {
    reset_combined_states();
    
    // 1. Calculate union NFA with epsilon transition (Lecture 3, p. 12)
    NFA nfa = unify_nfa(a, b);
    print_nfa("NFA with epsilon transition", nfa);

    // 2. Convert res into NFA without epsilon transition (Lecture 2, p. 26)
    nfa = e_transition_removal(nfa);
    print_nfa("NFA without epsilon transition", nfa);
    
    // 3. NFA determinization (subset construction) (Lecture 3, p.3)
    DFA dfa = nfa_determinization(nfa);
    print_dfa("DFA", dfa);

    // 4. Remove unreachable states
    unreachable_states_removal(dfa);
    
    // 5. DFA minimization (Algorithm Moore) (Lecture 3, p. 31)
    dfa = dfa_minimization(dfa);
    print_dfa("Minimized DFA", dfa);

    return dfa;
}

/**
 * Intersection two NFAs using parallel run algorithm (Lecture 3, p. 17)
 */
NFA intersect_nfa(const NFA& a, const NFA& b)
{
    NFAx nfax;

    nfax.set_alphabet(a.m_Alphabet);
    for (auto i : b.m_Alphabet) {
        nfax.set_alphabet(i);
    }

    // This is to gurantee NFAs don't have common states
    NFA b1 = b;
    increase_states_by_delta(b1, find_delta_state(a));

    // Determine states of NFA
    std::set<Combined_state> comb_states;
    for (auto a_state : a.m_States) {
        for (auto b_state : b1.m_States) {
            nfax.add_state({a_state, b_state});
        }
    }
    // Determine final states
    for (auto a_fin_state : a.m_FinalStates) {
        for (auto b_fin_state : b1.m_FinalStates) {
            nfax.add_final_state({ a_fin_state, b_fin_state });
        }
    }

    nfax.set_init_state({ a.m_InitialState, b1.m_InitialState });

    // Create transition function
    for (auto state : nfax.get_states()) {
        State a_state;
        State b_state;
        Combined_state::iterator it = state.begin();
        a_state = *it;
        advance(it, 1);
        b_state = *it;
        for (auto sym : nfax.get_alphabet()) {
            auto pos_a = a.m_Transitions.find({ a_state, sym });
            auto pos_b = b1.m_Transitions.find({ b_state, sym });
            if ((pos_a == a.m_Transitions.end()) || (pos_b == b1.m_Transitions.end())) {
                continue;
            }
            Partition part;
            for (auto a : pos_a->second) {
                for (auto b : pos_b->second) {
                    part.insert({ a, b });
                }
            }
            nfax.add_transition({{ a_state, b_state }, sym}, part);
        }
    }

    print_nfa("Automat a:\n", a);
    print_nfa("Automat b:\n", b1);
    nfax.print("a, b intersect");

    return nfax.nfax2nfa();
}

// Intersection algorithm (Lecture 3, p. 17)
DFA intersect(const NFA& a, const NFA& b) {
    reset_combined_states();
    
    NFA nfa = intersect_nfa(a, b);
    print_nfa("result of intersect", nfa);
    
    DFA dfa = nfa2dfa(nfa);
    print_dfa("result of determinization", dfa);
    
//    dfa = unreachable_states_removal(dfa);
    
    dfa = dfa_minimization(dfa);
    print_dfa("result of minimization", dfa);
    return dfa;
}

#ifndef __PROGTEST__

// You may need to update this function or the sample data if your state naming strategy differs.
bool operator==(const DFA& a, const DFA& b)
{
    return std::tie(a.m_States, a.m_Alphabet, a.m_Transitions, a.m_InitialState, a.m_FinalStates) == std::tie(b.m_States, b.m_Alphabet, b.m_Transitions, b.m_InitialState, b.m_FinalStates);
}

void print_nfa(std::string text, const NFA& a) {
    std::cout << "NFA: " << text << "\n";
    std::cout << "\tAlphabet [";
    for (auto i : a.m_Alphabet) {
        std::cout << " '" << i << "'";
    }
    std::cout << " ]\n";

    std::cout << "\tStates [";
    for (auto i : a.m_States) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tTransitions : \n";
    int counter = 0;
    for (auto j : a.m_Transitions) {
        std::cout << "\t"  << counter++ << ":\tkey [ " << j.first.first << ", '" << j.first.second << "' ], value [";
        for (auto k : j.second) {
            std::cout << " " << k;
        }
        std::cout << " ]\n";
    }

    std::cout << "\tFinal States [";
    for (auto i : a.m_FinalStates) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tInitial State [ " << a.m_InitialState << " ]\n";
}

void print_dfa(std::string text, const DFA& a) {
    std::cout << "DFA: " << text << "\n";
    std::cout << "\tAlphabet [";
    for (auto i : a.m_Alphabet) {
        std::cout << " '" << i << "'";
    }
    std::cout << " ]\n";

    std::cout << "\tStates [";
    for (auto i : a.m_States) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    int counter = 0;
    std::cout << "\tTransitions : \n";
    for (auto j : a.m_Transitions) {
        std::cout << "\t" << counter++ << ":\tkey [ " << j.first.first << ", '" << j.first.second <<
            "' ], value [ " << j.second << " ]\n";
    }

    std::cout << "\tFinal States [";
    for (auto i : a.m_FinalStates) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tInitial State [ " << a.m_InitialState << " ]\n";
}

/**
 * These are examples from lecture 3
 */
void test_nfa2dfa(void)
{
    /* lecture 3, p. 4 */
    NFA m {
        {1, 2, 3, 4}, // q, q0, q1, qf
        {'0', '1'},
        {
            {{1, '0'}, {1, 2}},
            {{1, '1'}, {1, 3}},
            {{2, '0'}, {2, 4}},
            {{2, '1'}, {2}},
            {{3, '0'}, {3}},
            {{3, '1'}, {3, 4}},
        },
        1,
        {4},
    };
    
    print_dfa("test on page 4", nfa2dfa(m));
    std::cout << std::endl;

    /* lecture 3, p. 6 */
    NFA m2 {
        {1, 2}, // z, f
        {'a', 'b'},
        {
            {{1, 'a'}, {1, 2}},
            {{2, 'b'}, {2}}
        },
        1,
        {2},
    };
    print_dfa("test on page 6", nfa2dfa(m2));
    std::cout << std::endl; 
}

void test_dfa2mindfa(void)
{
    /* test from https://www.youtube.com/watch?v=0XaGAkY09Wc */
    DFA m {
        {0, 1, 2, 3, 4}, // A, B, C, D, E
        {'0', '1'},
        {
            {{0, '0'}, 1}, // (A, 0) -> B
            {{0, '1'}, 2}, // (A, 1) -> C
            {{1, '0'}, 1}, // (B, 0) -> B
            {{1, '1'}, 3}, // (B, 1) -> D
            {{2, '0'}, 1}, // (C, 0) -> B
            {{2, '1'}, 2}, // (C, 1) -> C
            {{3, '0'}, 1}, // (D, 0) -> B
            {{3, '1'}, 4}, // (D, 1) -> E
            {{4, '0'}, 1}, // (E, 0) -> B
            {{4, '1'}, 2}, // (E, 1) -> C
        },
        0,
        {4},
    };
    print_dfa("min", dfa_minimization(m));
    std::cout << std::endl;

    DFA m2{
      {0, 1, 2, 3, 4, 5, 6, 7},
      {'0', '1'},
      {
          {{0, '0'}, 1},
          {{0, '1'}, 5},
          {{1, '0'}, 6},
          {{1, '1'}, 2},
          {{2, '0'}, 0},
          {{2, '1'}, 2},
          {{3, '0'}, 2},
          {{3, '1'}, 6},
          {{4, '0'}, 7},
          {{4, '1'}, 5},
          {{5, '0'}, 2},
          {{5, '1'}, 6},
          {{6, '0'}, 6},
          {{6, '1'}, 4},
          {{7, '0'}, 6},
          {{7, '1'}, 2},
      },
      0,
      {2},
    };

    print_dfa("min2", dfa_minimization(m2));
    std::cout << std::endl;

    // Lecture 3, p. 32
    DFA m3{
        {0, 1, 2, 3, 4, 5},
        {'a', 'b'},
        {
            {{0, 'a'}, 5},
            {{0, 'b'}, 1},
            {{1, 'a'}, 4},
            {{1, 'b'}, 3},
            {{2, 'a'}, 2},
            {{2, 'b'}, 5},
            {{3, 'a'}, 3},
            {{3, 'b'}, 0},
            {{4, 'a'}, 1},
            {{4, 'b'}, 2},
            {{5, 'a'}, 0},
            {{5, 'b'}, 4},
        },
        0,
        {0, 5},
    };

    print_dfa("DFA m3:", m3);
    print_dfa("min3", dfa_minimization(m3));
    std::cout << std::endl;
}

int main()
{
#if 0
    /* test from https://www.youtube.com/watch?v=i-fk9o46oVY */
    NFA x {
        {1, 2, 3}, /* A, B, C */
        {'a', 'b'},
        {
            {{1, 'a'}, {1, 2}}, /* (A, a) -> A,B */
            {{1, 'b'}, {3}},    /* (A, b) -> C */
            {{2, 'a'}, {1}},    /* (B, a) -> A */
            {{2, 'b'}, {2}},    /* (B, b) -> B */
            {{3, 'b'}, {1, 2}}, /* (C, b) -> A,B */
        },
        1,
        {3},
    };
    DFA dfa = nfa_determinization_new(x);
    print_dfa("result of determinization: ", dfa);
    return 0;
#endif
    test_nfa2dfa();
    
    test_dfa2mindfa();
 //   return 0;

#if 1
    NFA test02{
        {0, 1, 2},
        {'0', '1'},
        {
            {{0, '0'}, {0}},
            {{0, '1'}, {1}},
            {{1, '0'}, {1,2}},
            {{1, '1'}, {1}},
            {{2, '0'}, {2}},
            {{2, '1'}, {1,2}},
        },
        0,
        {2},
    };
  
    DFA test3{
        {0, 1, 2, 3, 4, 5, 6, 7},
        {'0', '1'},
        {
            {{0, '0'}, 1},
            {{0, '1'}, 5},
            {{1, '0'}, 6},
            {{1, '1'}, 2},
            {{2, '0'}, 0},
            {{2, '1'}, 2},
            {{3, '0'}, 2},
            {{3, '1'}, 6},
            {{4, '0'}, 7},
            {{4, '1'}, 5},
            {{5, '0'}, 2},
            {{5, '1'}, 6},
            {{6, '0'}, 6},
            {{6, '1'}, 4},
            {{7, '0'}, 6},
            {{7, '1'}, 2},
        },
        0,
        {2},
    };

    DFA test5{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, 1},
            {{0, 'b'}, 4},
            {{1, 'a'}, 1},
            {{1, 'b'}, 2},
            {{2, 'a'}, 1},
            {{2, 'b'}, 3},
            {{3, 'a'}, 1},
            {{3, 'b'}, 4},
            {{4, 'a'}, 1},
            {{4, 'b'}, 4},
        },
        0,
        {3},
    };

    // Automat from lecture 3, p. 23
    NFA test {
        {0, 1, 2},
        {'a', 'b', 'c'},
        {
            {{0, 'a'}, {0}},
            {{0, '\0'}, {1}},
            {{1, 'b'}, {1}},
            {{1, '\0'}, {2}},
            {{2, 'c'}, {2}},
        },
        0,
        {2},
    };

    NFA a1{
        {0, 1, 2},
        {'a', 'b'},
        {
            {{0, 'a'}, {0, 1}},
            {{0, 'b'}, {0}},
            {{1, 'a'}, {2}},
        },
        0,
        {2},
    };
    NFA a2{
        {0, 1, 2}, // 3,4,5
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{1, 'a'}, {2}},
            {{2, 'a'}, {2}},
            {{2, 'b'}, {2}},
        },
        0,
        {2},
    };
    DFA a{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{1, 'a'}, {2}},
            {{2, 'a'}, {2}},
            {{2, 'b'}, {3}},
            {{3, 'a'}, {4}},
            {{3, 'b'}, {3}},
            {{4, 'a'}, {2}},
            {{4, 'b'}, {3}},
        },
        0,
        {2},
    };
    std::cout << "Test a\n";
    assert(intersect(a1, a2) == a);
    return 0;

    NFA b1{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{2, 'a'}, {2, 3}},
            {{2, 'b'}, {2}},
            {{3, 'a'}, {4}},
        },
        0,
        {1, 4},
    };
    NFA b2{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'b'}, {1}},
            {{1, 'a'}, {2}},
            {{2, 'b'}, {3}},
            {{3, 'a'}, {4}},
            {{4, 'a'}, {4}},
            {{4, 'b'}, {4}},
        },
        0,
        {4},
    };
    DFA b {
        {0, 1, 2, 3, 4, 5, 6, 7, 8},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{2, 'a'}, {3}},
            {{2, 'b'}, {4}},
            {{3, 'a'}, {5}},
            {{3, 'b'}, {6}},
            {{4, 'a'}, {7}},
            {{4, 'b'}, {4}},
            {{5, 'a'}, {5}},
            {{5, 'b'}, {4}},
            {{6, 'a'}, {8}},
            {{6, 'b'}, {4}},
            {{7, 'a'}, {5}},
            {{7, 'b'}, {4}},
            {{8, 'a'}, {8}},
            {{8, 'b'}, {8}},
        },
        0,
        {1, 5, 8},
    };
    assert(unify(b1, b2) == b);

    return 0;

    NFA c1{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{2, 'a'}, {2, 3}},
            {{2, 'b'}, {2}},
            {{3, 'a'}, {4}},
        },
        0,
        {1, 4},
    };
    NFA c2{
        {0, 1, 2},
        {'a', 'b'},
        {
            {{0, 'a'}, {0}},
            {{0, 'b'}, {0, 1}},
            {{1, 'b'}, {2}},
        },
        0,
        {2},
    };
    DFA c{
        {0},
        {'a', 'b'},
        {},
        0,
        {},
    };
//    assert(intersect(c1, c2) == c);
    unify(c1, c2);

    NFA d1{
        {0, 1, 2, 3},
        {'i', 'k', 'q'},
        {
            {{0, 'i'}, {2}},
            {{0, 'k'}, {1, 2, 3}},
            {{0, 'q'}, {0, 3}},
            {{1, 'i'}, {1}},
            {{1, 'k'}, {0}},
            {{1, 'q'}, {1, 2, 3}},
            {{2, 'i'}, {0, 2}},
            {{3, 'i'}, {3}},
            {{3, 'k'}, {1, 2}},
        },
        0,
        {2, 3},
    };
    NFA d2{
        {0, 1, 2, 3},
        {'i', 'k'},
        {
            {{0, 'i'}, {3}},
            {{0, 'k'}, {1, 2, 3}},
            {{1, 'k'}, {2}},
            {{2, 'i'}, {0, 1, 3}},
            {{2, 'k'}, {0, 1}},
        },
        0,
        {2, 3},
    };
    DFA d{
        {0, 1, 2, 3},
        {'i', 'k', 'q'},
        {
            {{0, 'i'}, {1}},
            {{0, 'k'}, {2}},
            {{2, 'i'}, {3}},
            {{2, 'k'}, {2}},
            {{3, 'i'}, {1}},
            {{3, 'k'}, {2}},
        },
        0,
        {1, 2, 3},
    };
//    assert(intersect(d1, d2) == d);
    unify(d1, d2);
#endif
}
#endif
