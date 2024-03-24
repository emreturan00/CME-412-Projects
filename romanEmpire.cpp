#include <iostream>
#include <random>
#include <vector>
#include <algorithm>

// Parameters
const int NUM_YEARS_SIMULATION = 50;
const int NUM_QUAESTORS = 20;
const int NUM_AEDILES = 10;
const int NUM_PRAETORS = 8;
const int NUM_CONSULS = 2;
const int MIN_AGE_QUAESTOR = 30;
const int MIN_AGE_AEDILE = 36;
const int MIN_AGE_PRAETOR = 39;
const int MIN_AGE_CONSUL = 42;
const int REELECTION_INTERVAL_CONSUL = 10;
const int LIFE_EXPECTANCY_MEAN = 55;
const int LIFE_EXPECTANCY_STDDEV = 10;
const int LIFE_EXPECTANCY_MIN = 25;
const int LIFE_EXPECTANCY_MAX = 80;
const int NEW_CANDIDATES_MEAN = 15;
const int NEW_CANDIDATES_STDDEV = 5;
const int INITIAL_PSI = 100;
const int PSI_PENALTY_UNFILLED = 5;
const int PSI_PENALTY_REELECTED = 10;

// Function to generate truncated normal distribution
int truncated_normal(int mean, int stddev, int min_val, int max_val) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(mean, stddev);

    while (true) {
        int val = static_cast<int>(dist(gen));
        if (val >= min_val && val <= max_val)
            return val;
    }
}

// Function to simulate elections
int simulate_elections(int available_positions, int num_candidates) {
    if (available_positions >= num_candidates)
        return num_candidates;
    else
        return available_positions;
}

int main() {
    std::vector<int> ages = { 0 }; // Ages of politicians
    int psi = INITIAL_PSI; // Political Stability Index
    std::vector<int> office_fill_rates = { 0, 0, 0, 0 }; // Quaestor, Aedile, Praetor, Consul

    // Simulation loop
    for (int year = 1; year <= NUM_YEARS_SIMULATION; ++year) {
        // Generate new candidates
        int num_new_candidates = truncated_normal(NEW_CANDIDATES_MEAN, NEW_CANDIDATES_STDDEV, 0, INT_MAX);
        int new_quaestors = simulate_elections(NUM_QUAESTORS, num_new_candidates);
        int new_aediles = simulate_elections(NUM_AEDILES, num_new_candidates - new_quaestors);
        int new_praetors = simulate_elections(NUM_PRAETORS, num_new_candidates - new_quaestors - new_aediles);
        int new_consuls = simulate_elections(NUM_CONSULS, num_new_candidates - new_quaestors - new_aediles - new_praetors);

        // Update office fill rates
        office_fill_rates[0] += new_quaestors;
        office_fill_rates[1] += new_aediles;
        office_fill_rates[2] += new_praetors;
        office_fill_rates[3] += new_consuls;

        // Age politicians and remove those surpassing life expectancy
        for (auto& age : ages) {
            age++;
            if (age > truncated_normal(LIFE_EXPECTANCY_MEAN, LIFE_EXPECTANCY_STDDEV, LIFE_EXPECTANCY_MIN, LIFE_EXPECTANCY_MAX)) {
                age = -1; // Mark for removal
            }
        }
        ages.erase(std::remove(ages.begin(), ages.end(), -1), ages.end());

        // Update PSI based on office fill rates and re-election penalties
        psi -= PSI_PENALTY_UNFILLED * (NUM_QUAESTORS - new_quaestors);
        psi -= PSI_PENALTY_UNFILLED * (NUM_AEDILES - new_aediles);
        psi -= PSI_PENALTY_UNFILLED * (NUM_PRAETORS - new_praetors);
        psi -= PSI_PENALTY_UNFILLED * (NUM_CONSULS - new_consuls);
        if (new_consuls > 0) {
            psi -= PSI_PENALTY_REELECTED * std::count(ages.begin(), ages.end(), MIN_AGE_CONSUL);
        }
    }

    // Calculate final office fill rates
    double total_positions = NUM_QUAESTORS + NUM_AEDILES + NUM_PRAETORS + NUM_CONSULS;
    std::vector<double> final_fill_rates;
    for (int rate : office_fill_rates) {
        final_fill_rates.push_back((rate / total_positions) * 100);
    }

    // Output results
    std::cout << "End-of-Simulation PSI: " << psi << std::endl;
    std::cout << "Annual Fill Rate:" << std::endl;
    std::cout << "  Quaestor: " << final_fill_rates[0] << "%" << std::endl;
    std::cout << "  Aedile: " << final_fill_rates[1] << "%" << std::endl;
    std::cout << "  Praetor: " << final_fill_rates[2] << "%" << std::endl;
    std::cout << "  Consul: " << final_fill_rates[3] << "%" << std::endl;

    return 0;
}
