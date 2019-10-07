
#include <eosio/eosio.hpp>

class [[eosio::contract("election")]] election : public eosio::contract {

  public:



  private:
    // Table Candidate
    struct [[eosio::table]] candidate {
        eosio::name _candidate; // primary key
        std::string _name;   // candidate name
        uint32_t _count = 0; // voted count

        auto primary_key() const { return _candidate.value;}
    };

    typedef eosio::multi_index<"candidates"_candidates, candidate> candidate_index;

    // Table Voter
    struct [[eosio::table]] voter {
        eosio::name _account;       // primary key
        eosio::name _candidate;     // candidate key
        auto primary_key() const { return _account.value;};
        uint32_t candidate_key() const { return _candidate.value;};
    };

    typedef eosio::multi_index<"voters"_voters, voter> voter_index;


};
