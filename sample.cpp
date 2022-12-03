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

#endif

typedef std::set<State> Combined_state;
typedef std::set<Combined_state> Partition;

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

void print_comb_state(Combined_state state) {
    std::cout << "{ ";
    for (auto j : state) {
        std::cout << j << ", ";
    }
    std::cout << "}";
}

void print_partition(const Partition& partition) {
    std::cout << "{\n";
    for (auto i : partition) {
        std::cout << "  ";
        print_comb_state(i);
        std::cout << "\n";
    }
    std::cout << "}\n";
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
    int cnt = 1;
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

    std::cout << "No reachable states:\n";
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
    res.m_Alphabet = a.m_Alphabet;
    State st = 0;
    std::map<Combined_state, State> class2state;
    for (auto i : partition) {
        res.m_States.insert(st);
        class2state.insert({ i, st });
        // Determine initial state
        auto init_state = i.find(a.m_InitialState);
        if (init_state != i.end()) {
            res.m_InitialState = st;
        }
        // Determine final states
        for (auto j : a.m_FinalStates) {
            auto fin_state = i.find(j);
            if (fin_state != i.end()) {
                res.m_FinalStates.insert(st);
            }
        }
        st++;
    }

    // Determine transitions
    for (auto tr : a.m_Transitions) {
        // (state, sym) -> value
        auto state = tr.first.first;
        auto sym = tr.first.second;
        auto value = tr.second;
        Combined_state cs = partition_find(partition, state);
        State s = class2state.find(cs)->second;
        cs = partition_find(partition, value);
        State v = class2state.find(cs)->second;
        res.m_Transitions.insert({ {s, sym}, v });
    }

    return res;
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

NFA intersect_nfa(const NFA& a, const NFA& b) {
    NFA nfa;

    nfa.m_Alphabet = a.m_Alphabet;
    for (auto i : b.m_Alphabet) {
        nfa.m_Alphabet.insert(i);
    }

    // This is to gurantee NFAs don't have common states
    NFA b1 = b;
    increase_states_by_delta(b1, find_delta_state(a));

    // Determine states of NFA
    std::set<Combined_state> comb_states;
    for (auto state1 : a.m_States) {
        for (auto state2 : b1.m_States) {
            comb_states.insert({ state1, state2 });
            store_combined_state({ state1, state2 });
            auto pos = combined_states.find({ state1, state2 });
            if (pos == combined_states.end()) {
                std::cerr << "Error. Failed to find mumber";
                exit(1);
            }
            nfa.m_States.insert(pos->second);
        }
    }
    // Determine final states
    for (auto fin_state_a : a.m_FinalStates) {
        for (auto fin_state_b : b1.m_FinalStates) {
            auto res = combined_states.find({ fin_state_a, fin_state_b });
            if (res == combined_states.end()) {
                std::cerr << "Error. Did not find state";
                exit(1);
            }
            nfa.m_FinalStates.insert(res->second);
        }
    }

    // Determine initial state
    auto res = combined_states.find({ a.m_InitialState, b1.m_InitialState });
    if (res == combined_states.end()) {
        std::cerr << "Error. Did not find init state";
        exit(1);
    }
    nfa.m_InitialState = res->second;

    // Create transition function
    for (auto state : comb_states) {
        State a_state;
        State b_state;
        Combined_state::iterator it = state.begin();
        a_state = *it;
        advance(it, 1);
        b_state = *it;
        for (auto sym : nfa.m_Alphabet) {
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
            auto res = combined_states.find({ a_state, b_state });
            Combined_state val;
            for (auto p : part) {
                auto s = combined_states.find(p);
                val.insert(s->second);
            }
            nfa.m_Transitions.insert({{ res->second, sym }, val });
        }
    }
#if 0
    print_nfa("Automat a:\n", a);
    print_nfa("Automat b:\n", b1);
    print_nfa("Res automat nfa:\n", nfa);
    std::cout << "Map of state pairs to state\n";
    
    for (auto cs : comb_states) {
        auto pos = combined_states.find(cs);
        std::cout << "(";
        print_comb_state(cs);
        std::cout << ") -> " << pos->second << "\n";
    }
#endif

    return nfa;
}

// Intersection algorithm (Lecture 3, p. 17)
DFA intersect(const NFA& a, const NFA& b) {
    reset_combined_states();
    
    // 1. Intersect NFA
    NFA nfa = intersect_nfa(a, b);
    print_nfa("Intersected nfa:\n", nfa);

    // 2. NFA determinization (subset construction) (Lecture 3, p.3)
    DFA dfa = nfa_determinization(nfa);
    print_dfa("DFA", dfa);

    // 3. Remove unreachable states
    unreachable_states_removal(dfa);

    // 4. DFA minimization (Algorithm Moore) (Lecture 3, p. 31)
    dfa = dfa_minimization(dfa);
    print_dfa("Minimized DFA", dfa);
    
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

int main()
{
    // Automat from lecture 3, p. 4
    NFA test1 {
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

//    DFA dfa = nfa_determinization(test02);
//    print_dfa("Determinized dfa", dfa);

//    return 0;

    DFA test2 {
        {0, 1, 2, 3, 4, 5, 6, 7},
        {'a', 'b'},
        {
            {{0, 'a'}, 1},
            {{0, 'b'}, 4},
            {{1, 'a'}, 5},
            {{1, 'b'}, 2},
            {{3, 'a'}, 3},
            {{3, 'b'}, 3},
            {{4, 'a'}, 1},
            {{4, 'b'}, 4},
            {{5, 'a'}, 1},
            {{5, 'b'}, 4},
            {{6, 'a'}, 3},
            {{6, 'b'}, 7},
            {{2, 'a'}, 3},
            {{2, 'b'}, 6},
            {{7, 'a'}, 3},
            {{7, 'b'}, 6},
        },
        0,
        {2, 7}
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
    
    // Lecture 3, p. 32
    DFA test4 {
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

//    print_dfa("Minimized DFA:\n", dfa_minimization(test5));
    
//    return 0;

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

//    print_nfa("Orig\n", test);
//    print_nfa("Epsilon transition removal\n", e_transition_removal(test));
    
//    return 0;

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
}
#endif
