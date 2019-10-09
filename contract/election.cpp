#include <eosio/eosio.hpp>

using namespace eosio;

// Candidate table
struct [[eosio::table("candidate"), eosio::contract("election")]] candidate {
    uint64_t    _key;
    std::string _name;
    uint32_t    _count = 0;

    uint64_t primary_key() const { return _key; }
};
using candidate_table = eosio::multi_index<"candidate"_n, candidate>;

// Candidate table
struct [[eosio::table("voter"), eosio::contract("election")]] voter {
    uint64_t    _key;
    uint64_t _candidate_key;
    eosio::name _account;
    eosio::name _candidate;
    uint32_t    primary_key() const { return _key; };
    uint32_t    candidate_key() const { return _candidate.value; };
};
using voter_table = eosio::multi_index<"voter"_n, voter>;

// The contract
class election : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;

    /// @abi action
    [[eosio::action]] void version() { print("Election Smart Contract version 0.0.1\n"); };

    /// @abi action
    [[eosio::action]] void addc(std::string name) {

        candidate_table table{get_self(), 0};

        print("Adding candidate ", name, "\n");

        uint64_t key = table.available_primary_key();

        // update the table to include a new candidate
        table.emplace(get_self(), [&](auto& p) {
            p._key   = key;
            p._name  = name;
            p._count = 0;
        });

        print("Candidate added successfully. candidate_key = ", key, "\n");
    };

    /// @abi action
    [[eosio::action]] void reset() {
        candidate_table table_candidate{get_self(), 0};
        // Get all keys of _candidates
        std::vector<uint64_t> keysForDeletion;
        for (auto& itr : table_candidate) {
            keysForDeletion.push_back(itr.primary_key());
        }

        // now delete each item for that poll
        for (uint64_t key : keysForDeletion) {
            auto itr = table_candidate.find(key);
            if (itr != table_candidate.end()) {
                table_candidate.erase(itr);
            }
        }

        voter_table table_voter{get_self(), 0};
        // Get all keys of _voters
        keysForDeletion.empty();
        for (auto& itr : table_voter) {
            keysForDeletion.push_back(itr.primary_key());
        }

        // now delete each item for that poll
        for (uint64_t key : keysForDeletion) {
            auto itr = table_voter.find(key);
            if (itr != table_voter.end()) {
                table_voter.erase(itr);
            }
        }

        print("candidates and voters reset successfully.\n");
    };

    /// @abi action
    [[eosio::action]] void results() {
        candidate_table table_candidate{get_self(), 0};
        print("Start listing voted results\n");
        for (auto& item : table_candidate) {
            print("Candidate ", item._name, " has voted count: ", item._count, "\n");
        }
    };

    /// @abi action
    [[eosio::action]] void vote(name s, uint64_t _vote_key) {
        voter_table table_voter{get_self(), 0};

        require_auth(s);

        bool found = false;

        // Did the voter vote before?
        for (auto& item : table_voter) {
            if (item._account == s) {
                found = true;
                break;
            }
        }
        print(!found, "You're voted already!");
        candidate_table table_candidate{get_self(), 0};
        // Findout the candidate by id
        std::vector<uint64_t> keysForModify;
        for (auto& item : table_candidate) {
            if (item.primary_key() == _vote_key) {
                keysForModify.push_back(item.primary_key());
                break;
            }
        }

        if (keysForModify.size() == 0) {
            print(found, "Invalid candidate id!");
            return;
        }

        // Update the voted count inside the candidate
        for (uint64_t key : keysForModify) {
            auto itr       = table_candidate.find(key);
            auto candidate = table_candidate.get(key);
            if (itr != table_candidate.end()) {
                table_candidate.modify(itr, get_self(), [&](auto& p) { p._count++; });

                print("Voted candidate: ", candidate._name, " successfully\n");
            }
        }
        // Add this user to voters array
        table_voter.emplace(get_self(), [&](auto& p) {
            p._key           = table_voter.available_primary_key();
            p._candidate_key = _vote_key;
            p._account       = s;
        });
    };
};

