#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>

enum class Office { QUAESTOR, AEDILE, PRAETOR, CONSUL };

class Politician {
public:
    int age;
    int yearsInOffice;
    Office heldOffice;

    Politician(int initialAge, Office office) : age(initialAge), yearsInOffice(0), heldOffice(office) {}
};

class Simulation {
private:
    std::vector<Politician> politicians;
    int PSI; 
    int year;

     int MIN_QUAESTOR_AGE = 30;
     int MIN_AEDILE_AGE = 36;
     int MIN_PRAETOR_AGE = 39;
     int MIN_CONSUL_AGE = 42;
     int MIN_REELECTION_INTERVAL = 10;

     int NUM_QUAESTORS = 20;
     int NUM_AEDILES = 10;
     int NUM_PRAETORS = 8;
     int NUM_CONSULS = 2;

     int STARTING_PSI = 100;
     int UNFILLED_POSITION_PENALTY = 5;
     int REELECTION_PENALTY = 10;

    std::default_random_engine generator;
    std::normal_distribution<double> ageDistribution;

public:
    Simulation() : PSI(STARTING_PSI), year(0), generator(std::random_device{}()),
        ageDistribution(55.0, 10.0) {}

    void run(int numYears) {
        initialize();

        for (int i = 0; i < numYears; ++i) {
            simulateYear();
            updatePSI();
            ++year;
        }
    }

    void initialize() {
        politicians.clear();
        year = 0;
        PSI = STARTING_PSI;

        for (int i = 0; i < NUM_QUAESTORS; ++i)
            politicians.emplace_back(MIN_QUAESTOR_AGE, Office::QUAESTOR);
    }

    void simulateYear() {
        for (auto& politician : politicians) {
            ++politician.age;
            if (politician.age > calculateLifeExpectancy()) {
                politicians.erase(std::remove_if(politicians.begin(), politicians.end(),
                    [&](Politician& p) { return p.age > calculateLifeExpectancy(); }),
                    politicians.end());
                break;
            }
        }

        electOfficials(Office::CONSUL);
        electOfficials(Office::PRAETOR);
        electOfficials(Office::AEDILE);
        electOfficials(Office::QUAESTOR);

        int numNewCandidates = static_cast<int>(ageDistribution(generator));
        politicians.reserve(politicians.size() + numNewCandidates);
        for (int i = 0; i < numNewCandidates; ++i)
            politicians.emplace_back(MIN_QUAESTOR_AGE, Office::QUAESTOR);
    }
    int minimumAgeForOffice(Office office) {
        switch (office) {
        case Office::CONSUL:
            return MIN_CONSUL_AGE;
        case Office::PRAETOR:
            return MIN_PRAETOR_AGE;
        case Office::AEDILE:
            return MIN_AEDILE_AGE;
        case Office::QUAESTOR:
            return MIN_QUAESTOR_AGE;
        }
        return 0;
    }


    void electOfficials(Office office) {
        int numPositions = 0;
        switch (office) {
        case Office::CONSUL:
            numPositions = NUM_CONSULS;
            break;
        case Office::PRAETOR:
            numPositions = NUM_PRAETORS;
            break;
        case Office::AEDILE:
            numPositions = NUM_AEDILES;
            break;
        case Office::QUAESTOR:
            numPositions = NUM_QUAESTORS;
            break;
        }

        int numFilledPositions = 0;
        for (auto& politician : politicians) {
            if (politician.heldOffice == Office::QUAESTOR && politician.yearsInOffice == 0 &&
                politician.age >= minimumAgeForOffice(office)) {
                politician.heldOffice = office;
                politician.yearsInOffice = 1;
                ++numFilledPositions;
            }

            if (numFilledPositions >= numPositions)
                break;
        }

        if (numFilledPositions < numPositions)
            PSI -= (numPositions - numFilledPositions) * UNFILLED_POSITION_PENALTY;
    }


    void updatePSI() {
        if (year > MIN_REELECTION_INTERVAL) {
            int numConsuls = 0;
            for ( auto& politician : politicians) {
                if (politician.heldOffice == Office::CONSUL)
                    ++numConsuls;
            }
            if (numConsuls > 2) 
                PSI -= (numConsuls - 2) * REELECTION_PENALTY;
        }
    }

    int calculateLifeExpectancy() {
        double mu = 55.0, sigma = 10.0, a = 25.0, b = 80.0;
        double Z = (a - mu) / sigma;
        double Phi_a = 0.5 * (1 + erf(Z / sqrt(2)));
        Z = (b - mu) / sigma;
        double Phi_b = 0.5 * (1 + erf(Z / sqrt(2)));

        std::uniform_real_distribution<double> uniform(Phi_a, Phi_b);
        double U = uniform(generator);

        double X = mu + sigma * sqrt(2) * erfinv(2 * U - 1);

        return static_cast<int>(X);
    }

    double erfinv(double x) {
        double sgn = (x < 0) ? -1 : 1;
        x = (1 - x) * (1 + x);
        double lnx = log(x);
        double tt1 = 2 / (3.14159265359 * 0.147) + 0.5 * lnx;
        double tt2 = 1 / 0.147 * lnx;

        return sgn * sqrt(-tt1 + sqrt(tt1 * tt1 - tt2));
    }

    void printResults() {
        int numQuaestors = 0, numAediles = 0, numPraetors = 0, numConsuls = 0;
        for (auto& politician : politicians) {
            switch (politician.heldOffice) {
            case Office::QUAESTOR:
                ++numQuaestors;
                break;
            case Office::AEDILE:
                ++numAediles;
                break;
            case Office::PRAETOR:
                ++numPraetors;
                break;
            case Office::CONSUL:
                ++numConsuls;
                break;
            }
        }

        double totalPoliticians = politicians.size();
        double quaestorFillRate = (numQuaestors / totalPoliticians) * 100;
        double aedileFillRate = (numAediles / totalPoliticians) * 100;
        double praetorFillRate = (numPraetors / totalPoliticians) * 100;
        double consulFillRate = (numConsuls / totalPoliticians) * 100;

        std::vector<int> quaestorAges, aedileAges, praetorAges, consulAges;
        for (auto& politician : politicians) {
            switch (politician.heldOffice) {
            case Office::QUAESTOR:
                quaestorAges.push_back(politician.age);
                break;
            case Office::AEDILE:
                aedileAges.push_back(politician.age);
                break;
            case Office::PRAETOR:
                praetorAges.push_back(politician.age);
                break;
            case Office::CONSUL:
                consulAges.push_back(politician.age);
                break;
            }
        }

        std::cout << "End-of-Simulation PSI: " << PSI << std::endl;
        std::cout << "Annual Fill Rate:\n";
        std::cout << "Quaestor: " << quaestorFillRate << "%\n";
        std::cout << "Aedile: " << aedileFillRate << "%\n";
        std::cout << "Praetor: " << praetorFillRate << "%\n";
        std::cout << "Consul: " << consulFillRate << "%\n";
        std::cout << "Age Distribution:\n";
        std::cout << "Quaestor: ";
        printAgeDistribution(quaestorAges);
        std::cout << "Aedile: ";
        printAgeDistribution(aedileAges);
        std::cout << "Praetor: ";
        printAgeDistribution(praetorAges);
        std::cout << "Consul: ";
        printAgeDistribution(consulAges);
       
    }

    void printAgeDistribution( std::vector<int>& ages) {
        if (ages.empty()) {
            std::cout << "No politicians\n";
            return;
        }

        std::sort(ages.begin(), ages.end());
        int minAge = ages.front();
        int maxAge = ages.back();
        int medianAge = ages[ages.size() / 2];
        double meanAge = static_cast<double>(std::accumulate(ages.begin(), ages.end(), 0)) / ages.size();

        std::cout << "Min: " << minAge << ", Max: " << maxAge << ", Median: " << medianAge << ", Mean: " << meanAge
            << std::endl;
    }
};

int main() {
    Simulation simulation;
    simulation.run(200);
    simulation.printResults();

    return 0;
}
