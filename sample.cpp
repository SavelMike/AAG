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
#include <bitset>

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

using Combined_state = std::set<State>;
using Partition = std::set<Combined_state>;

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

#endif /* not __PROGTEST__ */

using Combined_state = std::set<State>;
using Patyition = std::set<Combined_state>;

/**
 * This is super class for automates whose states are Combined_state-s
 */
class FAx {
protected:
    std::set<Symbol> m_Alphabet;
    Partition m_States;
    Partition m_FinalStates;
    Combined_state m_InitialState;
public:
    void set_alphabet(const std::set<Symbol>& alphabet) { m_Alphabet = alphabet; }
    void set_alphabet(Symbol sym) { m_Alphabet.insert(sym); }
    void add_state(const Combined_state& state);
    void add_final_state(const Combined_state& state);
    void set_init_state(const Combined_state& state);
    const std::set<Symbol>& get_alphabet(void);
    const Partition& get_states(void);
#ifndef __PROGTEST__
    void print(const std::string& txt);
#endif
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

const Partition& FAx::get_states(void)
{
    return m_States;
}

#ifndef __PROGTEST__

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
#endif

/**
 * This is NFA where states are represented as std::set<State>
 */
class NFAx: public FAx {
private:
    std::map<std::pair<Combined_state, Symbol>, Partition> m_Transitions;
public:
    NFAx() {};

    void add_transition(const std::pair<Combined_state, Symbol>& key,
                        const Partition& value);
    NFA nfax2nfa();
#ifndef __PROGTEST__
    void print(const std::string& txt);
#endif
};

void NFAx::add_transition(const std::pair<Combined_state, Symbol>& key,
                          const Partition& value)
{
    m_Transitions.insert({key, value});
}

/**
 * Builds NFA from NFAx and return it 
 * Assigns state for every combined state of NFAx
 * and uses those states for returned NFA
 */
NFA NFAx::nfax2nfa()
{
    NFA nfa;
    std::map<Combined_state, State> cs2s;
    State s = 1;
    
    nfa.m_Alphabet = get_alphabet();
    for (auto i : m_States) {
        cs2s.insert({i, s});
        nfa.m_States.insert(s++);
    }
    for (auto i : m_FinalStates) {
        auto rc = cs2s.find(i);
        nfa.m_FinalStates.insert(rc->second);
    }
    auto rc = cs2s.find(m_InitialState);
    nfa.m_InitialState = rc->second;
    for (auto i : m_Transitions) {
        /* i is { (Combined_state, Symbol), Partition */
        Combined_state cs = i.first.first;
        Symbol sym = i.first.second;
        State state = cs2s.find(cs)->second;

        /* set<Combined_state> i.second -> set<States> state */
        Combined_state states;
        for (auto j : i.second) {
            /* j is Combined_state */
            states.insert(cs2s.find(j)->second);
        }
        /* (State, Symbol) -> Combined_state */
        nfa.m_Transitions.insert({{state, sym}, states});
    }
    return nfa;
}

#ifndef __PROGTEST__
void NFAx::print(const std::string& txt)
{
    std::cout << "NFAx: ";
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
#endif

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
#ifndef __PROGTEST__
    void print(const std::string& txt);
#endif
};

void DFAx::add_transition(const std::pair<Combined_state, Symbol>& key,
                          const Combined_state& value)
{
    m_Transitions.insert({key, value});
}

DFA DFAx::dfax2dfa()
{
    DFA dfa;
    std::map<Combined_state, State> cs2s;
    State s = 1;
    
    dfa.m_Alphabet = get_alphabet();
    for (auto i : m_States) {
        cs2s.insert({i, s});
        dfa.m_States.insert(s++);
    }
    for (auto i : m_FinalStates) {
        auto rc = cs2s.find(i);
        dfa.m_FinalStates.insert(rc->second);
    }
    auto rc = cs2s.find(m_InitialState);
    dfa.m_InitialState = rc->second;
    for (auto i : m_Transitions) {
        /* i is { (Combined_state, Symbol), Combined_state */
        State state = cs2s.find(i.first.first)->second;
        Symbol sym = i.first.second;
        State val = cs2s.find(i.second)->second;

        dfa.m_Transitions.insert({ {state, sym}, val });
    }
    return dfa;
}

#ifndef __PROGTEST__
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
#endif

/**
 *  Return the biggest value of NFA states + 1.
 *  Used to gurantee uniqueness of states of automates which are to be intersected or unified
 *  and to calculate value for deadstate
 */
State find_delta_state(const NFA& a) {
    return *a.m_States.rbegin() + 1;
}

/**
 *  Modify all states of NFA \a a by adding \a delta
 *  It makes NFA \a a suitable for intersection or unify 
 */
void increase_states_by_delta(NFA& a, State delta) {
    // Modify set of states
    Combined_state states;

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
    std::map<std::pair<State, Symbol>, Combined_state> transitions;
    for (auto j : a.m_Transitions) {
        std::pair<State, Symbol> key = j.first;
        key.first += delta;
        Combined_state value;
        for (auto k : j.second) {
            value.insert(k + delta);
        }
        transitions.insert({ key, value });
    }

    a.m_Transitions = transitions;
}

/** 
 * Unify two NFAs with epsilon transition
 * Input:
 *      a, b NFAs to unify
 * Output:
 *      NFA : L(NFA) = L(a) U L(b)
 * Algorithm from Lecture 3, page 12.
 */
NFA unify_nfa_eps(const NFA& a, const NFA& b) {
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

    // Add new initial state
    res.m_InitialState = find_delta_state(b1);
    res.m_States.insert(res.m_InitialState);

    // Compose transition function of NFA res
    std::map<std::pair<State, Symbol>, Combined_state> transitions;
    
    // 3. delta(q, a) <- delta1(q, a)
    transitions = a.m_Transitions;
    
    // 4. delta(q, a) <- delta2(q, a)
    for (auto i : b1.m_Transitions) {
        transitions.insert(i);
    }

    // 2.   delta(q0, epsilon) <- {q01, q02}
    std::pair<State, Symbol> key = {res.m_InitialState, '\0'};
    Combined_state value = { a.m_InitialState, b1.m_InitialState };
    transitions.insert({ key, value });
    res.m_Transitions = transitions;

    // 5. F <- F1 U F2
    res.m_FinalStates = a.m_FinalStates;
    for (auto i : b1.m_FinalStates) {
        res.m_FinalStates.insert(i);
    }
        
    return res;
}

/**
 * Calculates epsilon closure for a state \a s of NFA \a a.
 * Algorithm from lecture 3 p. 25
 */
Combined_state e_closure(const NFA& a, State s) {
    Combined_state res = { s };
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

    return res;
}

/**
 * Return true if there is no common state in \a a and \a b
 * Used in function e_transition_removal 
 */
bool is_set_intersect_empty(const Combined_state& a, const Combined_state& b) {
    for (auto i : a) {
        for (auto j : b) {
            if (i == j) {
                return false;
            }
        }
    }

    return true;
}

/** 
 *  Conversion of NFA with epsilon transitions into NFA without epsilon transitions
 *  Algorithm from lecture 2, p. 26
 */
NFA e_transition_removal(const NFA& a) {
    NFA res;

    res.m_States = a.m_States;
    res.m_Alphabet = a.m_Alphabet;
    res.m_InitialState = a.m_InitialState;

    // Compose delta'(transition function on NFA res)
    std::map<std::pair<State, Symbol>, Combined_state> transitions;
    for (auto state : res.m_States) {
        Combined_state e_clos = e_closure(a, state);
        for (auto symbol : res.m_Alphabet) {
            std::pair<State, Symbol> key = { state, symbol };
            Combined_state value;
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
    Combined_state fin_states;
    for (auto q : res.m_States) {
        Combined_state e_clos = e_closure(a, q);
        if (!is_set_intersect_empty(e_clos, a.m_FinalStates)) {
            fin_states.insert(q);
        }
    }
    res.m_FinalStates = fin_states;
    return res;
}


/**
 * Unify two NFAs with parallel run
 * Input:
 *      a, b total NFAs to unify
 * Output:
 *      NFA : L(NFA) = L(a) U L(b)
 * Algorithm from Lecture 3, page 14.
 */
NFA unify_nfa_parallel(const NFA &a, const NFA &b) {
    NFAx nfax;

    // 1. Alphabet
    nfax.set_alphabet(a.m_Alphabet);
    for (auto i : b.m_Alphabet) {
        nfax.set_alphabet(i);
    }

    // This is to gurantee NFAs don't have common states
    NFA b1 = b;
    increase_states_by_delta(b1, find_delta_state(a));

    // 2. States: a.m_States x b1.m_States
    for (auto a_state : a.m_States) {
        for (auto b_state : b1.m_States) {
            nfax.add_state({ a_state, b_state });
        }
    }

    // 2. Initial state
    nfax.set_init_state({ a.m_InitialState, b1.m_InitialState });

    // 3. Final states: a.m_FinalStates x b1.m_States
    for (auto a_final_state : a.m_FinalStates) {
        for (auto b_state : b1.m_States) {
            nfax.add_final_state({ a_final_state, b_state });
        }
    }
    //  Final states: a.m_States x b1.m_FinalStates 
    for (auto a_state : a.m_States) {
        for (auto b_final_state : b1.m_FinalStates) {
            nfax.add_final_state({ a_state, b_final_state });
        }
    }
    
    // 4. Transition function
    for (auto st : nfax.get_states()) {
        // Split st into states of a and b1
        Combined_state::iterator it = st.begin();
        auto a_state = *it;
        advance(it, 1);
        auto b_state = *it;
        for (auto sym : nfax.get_alphabet()) {
            auto a_pos = a.m_Transitions.find({ a_state, sym });
            auto b_pos = b1.m_Transitions.find({b_state, sym});
            if (a_pos == a.m_Transitions.end() || b_pos == b1.m_Transitions.end()) {
                continue;
            }
            // a_pos->second and b_pos->second are of type Combined_state
            Partition value;
            for (auto i : a_pos->second) {
                for (auto j : b_pos->second) {
                    value.insert({ i, j });
                }
            }
            nfax.add_transition({ st, sym }, value);
        }
    }
    
    return nfax.nfax2nfa();
}


/**
 * identification of useful states and removal of redundant states
 * DFA \a dfa has not empty accepted language
 * Algorithm from lecture 2, p. 20
 */
DFA remove_redundant_states(const DFA& dfa)
{
	DFA res;

	res.m_States = dfa.m_FinalStates;
	Combined_state Q = res.m_States;
	while (1) {
		for (auto p : res.m_States) {
			for (auto t : dfa.m_Transitions) {
				if (t.second == p)
					Q.insert(t.first.first);
			}
		}
		if (Q == res.m_States)
			break;
		res.m_States = Q;
	}
    // Initial state must be already in res.m_State because L(DFA) is not empty
	res.m_Alphabet = dfa.m_Alphabet;
	res.m_InitialState = dfa.m_InitialState;
	res.m_FinalStates = dfa.m_FinalStates;
    // Calculating transition function
	for (auto q: res.m_States) {
		for (auto sym: res.m_Alphabet) {
			auto pos = dfa.m_Transitions.find({q, sym});
			if (pos == dfa.m_Transitions.end())
				continue;
			if (res.m_States.find(pos->second) != res.m_States.end())
				res.m_Transitions.insert({{q, sym}, pos->second});
		}
	}

	return res;
}

/**
 * Convert NFA \a a to DFA
 * Subset construction algorithm from Lecture 3, p. 3
 */
DFA nfa2dfa(const NFA& a)
{
    DFAx dfax;
    
    dfax.set_alphabet(a.m_Alphabet);
    dfax.add_state({ a.m_InitialState });
    dfax.set_init_state({a.m_InitialState});
    
    Partition states = dfax.get_states();
    while (!states.empty()) {
        Partition added_states;
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
                    uni.insert(0xffffffff);
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
    
    return dfax.dfax2dfa();
}

/**
 * Partition \a P to class of equivalence
 * Algorithm of minimization from Lecture 3, p. 37 
 */
Partition new_partition(const Partition& P, const DFA& dfa) {
    Partition T;

    for (auto S : P) {
        Combined_state S1;
        Combined_state S2;
        for (auto a : dfa.m_Alphabet) {
            // Partition S by a
            Combined_state S1Target;
            S1 = S1Target;
            S2 = S1Target;
            for (auto s : S) {
                auto m = dfa.m_Transitions.find({ s, a });
                if (m == dfa.m_Transitions.end()) {
                    // Should not happen for DFA
                    std::cout << "{ " << s << ", " << a << " } -> empty\n";
                    continue;
                }
                // Look for set which contains m->second
                for (auto tmp : P) {
                    auto rc = tmp.find(m->second);
                    if (rc == tmp.end()) {
                        // Should not happen
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
        // S1 U S2 = S
        T.insert(S1);
        if (!S2.empty()) {
            T.insert(S2);
        }
    }
    return T;
}


/**
 * DFA minimization using partinioning method
 * Algorithm from lecture 3. p. 31
 */
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

    // Repeat partitioning until no new set of states gets created 
    while (1) {
        partition2 = new_partition(partition, a);
        if (partition == partition2) {
            break;
        }
        partition = partition2;
    }

    // Build minimized DFA
    DFAx dfax;

    dfax.set_alphabet(a.m_Alphabet);
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

    return dfax.dfax2dfa();
}

 /**
  * Creating total NFA
  * Algorithm for total DFA is used from Lecture 2 p.9 
 */
NFA total_nfa(const NFA& nfa)
{
    NFA tnfa;

    tnfa.m_Alphabet = nfa.m_Alphabet;
    tnfa.m_InitialState = nfa.m_InitialState;
    tnfa.m_FinalStates = nfa.m_FinalStates;
    tnfa.m_States = nfa.m_States;
   
    State dead_state = find_delta_state(tnfa);
    tnfa.m_States.insert(dead_state);

    for (auto s : tnfa.m_States) {
        for (auto a : nfa.m_Alphabet) {
            auto pos = nfa.m_Transitions.find({ s, a });
            if (pos == nfa.m_Transitions.end()) { 
                tnfa.m_Transitions.insert({ {s, a}, {dead_state} });
            }
            else {
                tnfa.m_Transitions.insert({ {s, a}, pos->second });
            }
        }
    }

    return tnfa;
}

/**
 * Convert NFA \a a to optimal DFA
 * Determinization, minimization, redundant states removal
 */
DFA nfa_2min_dfa(const NFA& a) {
    DFA dfa = nfa2dfa(a);

    dfa = dfa_minimization(dfa);

    return remove_redundant_states(dfa);
}

/**
 * Unify implementation using parallel run algorithm 
 */
DFA unify_parallel(const NFA& a, const NFA& b) {
    // 0. Convert a and b to total NFAs
    NFA total_a = total_nfa(a);
    NFA total_b = total_nfa(b);

    // 1. Calculate union NFA with parallel run(Lecture 3, p. 14)
    NFA nfa = unify_nfa_parallel(total_a, total_b);
    
    return nfa_2min_dfa(nfa);
}

/**
 * Unify implementation using union with epsilon transition algorithm from Lecture 3, p. 12
*/
DFA unify_eps(const NFA& a, const NFA& b) {    
    // 1. Calculate union NFA with epsilon transition (Lecture 3, p. 12)
    NFA nfa = unify_nfa_eps(a, b);

    // 2. Convert res into NFA without epsilon transition (Lecture 2, p. 26)
    nfa = e_transition_removal(nfa);
    
    return nfa_2min_dfa(nfa);
}

DFA unify(const NFA& a, const NFA& b) {
    //    return unify_parallel(a, b);
    return unify_eps(a, b);
}

/**
 * Intersection two NFAs using parallel run algorithm (Lecture 3, p. 17)
 */
NFA intersect_nfa(const NFA& a, const NFA& b)
{
    NFAx nfax;
    NFA b1 = b;

    nfax.set_alphabet(a.m_Alphabet);
    for (auto i : b.m_Alphabet) {
        nfax.set_alphabet(i);
    }

    // This is to gurantee NFAs don't have common states
    increase_states_by_delta(b1, find_delta_state(a));

    // Determine states of NFA
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
            nfax.add_transition({ state, sym }, part);
        }
    }

    return nfax.nfax2nfa();
}

/**
 * Intersection implementation of two NFAs
 */
 DFA intersect(const NFA& a, const NFA& b) {    
    NFA nfa = intersect_nfa(a, b);
    
    return nfa_2min_dfa(nfa);
 }

#ifndef __PROGTEST__

// Set of strings to test
std::set<std::string> data;

#define MAXLEN 8
/*
 * Generate all possible strings of length from 1 to \a maxlen of alphabet {'a', 'b'}.
 * For instance: if \a maxlen = 3 generated strings are: a, b, aa, bb, ab, ba, aaa, bbb ... 
 */
std::set<std::string> test_strings(int maxlen)
{
    std::set<std::string> strings;
    std::stringstream s;

    if (maxlen > MAXLEN) {
        std::cerr << "too long strings, 10 is max";
        exit(1);
    }

    int n = 1 << maxlen;

    for (int i = 1; i <= maxlen; i++) {
        n = 1 << i;
        for (int j = 0; j < n; j++) {
            std::bitset<MAXLEN> b(j);
            s.str(std::string());
            s << b;
            std::string str = s.str();
            str = str.substr(str.size() - i, i);

            std::replace(str.begin(), str.end(), '0', 'a');
            std::replace(str.begin(), str.end(), '1', 'b');
            strings.insert(str);
        }
    }
    return strings;
}

/**
 * Return true if \a dfa accepts string \a str, false otherwise 
 */
bool accept(const DFA& dfa, const std::string str)
{
    State s = dfa.m_InitialState;
    for (auto i : str) {
        auto pos = dfa.m_Transitions.find({ s, i });
        if (pos == dfa.m_Transitions.end())
            return false;
        s = pos->second;
    }
    auto pos = dfa.m_FinalStates.find(s);
    if (pos == dfa.m_FinalStates.end())
        return false;
    return true;
}

/**
 * Return subset of string set data accepted by DFA \a a
 */
std::set<std::string> accepted_strings(const DFA& a) {
    std::set<std::string> res;

    for (auto st : data) {
        if (accept(a, st)) {
            res.insert(st);
        }
    }

    return res;
}

// You may need to update this function or the sample data if your state naming strategy differs.
bool operator==(const DFA& a, const DFA& b)
{
    return std::tie(accepted_strings(a)) == std::tie(accepted_strings(b));
}

void print_fa(const std::set<Symbol>& alphabet, const Combined_state& states,
              State init_state, const Combined_state& fin_states) {
    std::cout << "\tAlphabet [";
    for (auto i : alphabet) {
        std::cout << " '" << i << "'";
    }
    std::cout << " ]\n";

    std::cout << "\tStates [";
    for (auto i : states) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tFinal States [";
    for (auto i : fin_states) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tInitial State [ " << init_state << " ]\n";

}

/**
 * Debug function to print NFA
 */
void print_nfa(std::string text, const NFA& a) {
    std::cout << "NFA: " << text << "\n";
    
    print_fa(a.m_Alphabet, a.m_States, a.m_InitialState, a.m_FinalStates);

    std::cout << "\tTransitions : \n";
    int counter = 0;
    for (auto j : a.m_Transitions) {
        std::cout << "\t"  << counter++ << ":\tkey [ " << j.first.first << ", '" << j.first.second << "' ], value [";
        for (auto k : j.second) {
            std::cout << " " << k;
        }
        std::cout << " ]\n";
    }
}

/**
 * Debug function to print DFA
 */
void print_dfa(std::string text, const DFA& a) {
    std::cout << "DFA: " << text << "\n";

    print_fa(a.m_Alphabet, a.m_States, a.m_InitialState, a.m_FinalStates);
    
    int counter = 0;
    std::cout << "\tTransitions : \n";
    for (auto j : a.m_Transitions) {
        std::cout << "\t" << counter++ << ":\tkey [ " << j.first.first << ", '" << j.first.second <<
            "' ], value [ " << j.second << " ]\n";
    }
}

int main()
{
    data = test_strings(6);
 
    /*
     * two last chars are 'a'
     */
    NFA a1 {
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

    /*
     * two first chars are 'a'
     */
    NFA a2 {
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

    /*
     * two first chars and two last chars are 'a'
     */
    DFA a {
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
 
    assert(intersect(a1, a2) == a);

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

    assert(intersect(c1, c2) == c);

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
    
    assert(intersect(d1, d2) == d);
}
#endif