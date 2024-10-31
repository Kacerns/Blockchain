#include "einsteinhash.hpp"
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>

using namespace std;

class UTXO {
public:
    string tx_id;
    int amount;
    string owner;

    UTXO() : tx_id(""), amount(0), owner("") {}

    UTXO(const string &tx_id, int amount, const string &owner)
        : tx_id(tx_id), amount(amount), owner(owner) {}

    friend ostream &operator<<(ostream &os, const UTXO &utxo) {
        os << "UTXO(tx_id=" << utxo.tx_id << ", amount=" << utxo.amount << ", owner=" << utxo.owner << ")";
        return os;
    }
};

class Transaction {
public:
    vector<UTXO> inputs;
    vector<UTXO> outputs;
    string tx_id;

    Transaction(const vector<UTXO> &inputs, const vector<UTXO> &outputs)
        : inputs(inputs), outputs(outputs) {
        tx_id = calculate_hash();
    }

    string calculate_hash() {
        string tx_data;
        for (const auto &utxo : inputs)
            tx_data += utxo.tx_id;
        for (const auto &utxo : outputs)
            tx_data += to_string(utxo.amount);
        return HashFunction(tx_data);
    }

    void print_transaction() const {
        cout << "Transaction ID: " << tx_id << endl;
        cout << "Inputs:" << endl;
        for (const auto &utxo : inputs) {
            cout << "  - " << utxo << endl;
        }
        cout << "Outputs:" << endl;
        for (const auto &utxo : outputs) {
            cout << "  - " << utxo << endl;
        }
    }
};

class Block {
public:
    string prev_hash;
    string timestamp;
    vector<Transaction> transactions;
    string merkle_root;
    int nonce = 0;
    int difficulty;
    string block_hash;

    Block(const string &prev_hash, const vector<Transaction> &transactions, int difficulty)
        : prev_hash(prev_hash), transactions(transactions), difficulty(difficulty) {
        timestamp = get_timestamp();
        merkle_root = calculate_merkle_root();
        block_hash = mine_block();
    }

    string get_timestamp() {
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    string calculate_merkle_root() {
        vector<string> tx_ids;
        for (const auto &tx : transactions) {
            tx_ids.push_back(tx.tx_id);
        }
        if (tx_ids.empty()) return "";

        while (tx_ids.size() > 1) {
            if (tx_ids.size() % 2 != 0) tx_ids.push_back(tx_ids.back());
            vector<string> new_level;
            for (size_t i = 0; i < tx_ids.size(); i += 2) {
                string combined = tx_ids[i] + tx_ids[i + 1];
                new_level.push_back(HashFunction(combined));
            }
            tx_ids = new_level;
        }
        return tx_ids[0];
    }

    string calculate_hash() {
        string block_data = prev_hash + timestamp + merkle_root + to_string(nonce) + to_string(difficulty);
        return HashFunction(block_data);
    }

    string mine_block() {
        string target(difficulty, '0');
        while (true) {
            block_hash = calculate_hash();
            if (block_hash.substr(0, difficulty) == target)
                return block_hash;
            nonce++;
        }
    }
};

class Blockchain {
public:
    int difficulty;
    unordered_map<string, UTXO> utxo_pool;
    vector<Block> chain;
    vector<Transaction> pending_transactions;

    Blockchain(int difficulty) : difficulty(difficulty) {
        chain.push_back(create_genesis_block());
    }

    Block create_genesis_block() {
        UTXO genesis_utxo("genesis_tx", 1000000, "genesis_owner");
        Transaction genesis_tx({}, {genesis_utxo});
        utxo_pool[genesis_utxo.tx_id] = genesis_utxo;
        return Block("0", {genesis_tx}, difficulty);
    }

    void add_block(const vector<Transaction> &transactions) {
        string prev_hash = chain.back().block_hash;
        Block new_block(prev_hash, transactions, difficulty);
        chain.push_back(new_block);
        for (const auto &tx : transactions) {
            for (const auto &utxo : tx.inputs) {
                utxo_pool.erase(utxo.tx_id);
            }
            for (const auto &utxo : tx.outputs) {
                utxo_pool[utxo.tx_id] = utxo;
            }
        }
    }

    void add_transaction(const Transaction &transaction) {
        bool valid = true;
        for (const auto &utxo : transaction.inputs) {
            if (utxo_pool.find(utxo.tx_id) == utxo_pool.end()) {
                valid = false;
                break;
            }
        }
        if (valid) pending_transactions.push_back(transaction);
    }

    void mine_pending_transactions() {
        while (pending_transactions.size() >= 100) {
            vector<Transaction> transactions_to_mine(pending_transactions.begin(),
                                                     pending_transactions.begin() + 100);
            add_block(transactions_to_mine);
            pending_transactions.erase(pending_transactions.begin(), pending_transactions.begin() + 100);
        }
    }

    void print_block(int block_index) const {
        if (block_index >= 0 && block_index < chain.size()) {
            const Block &block = chain[block_index];
            cout << "Block Index: " << block_index << endl;
            cout << "Block Hash: " << block.block_hash << endl;
            cout << "Previous Hash: " << block.prev_hash << endl;
            cout << "Merkle Root: " << block.merkle_root << endl;
            cout << "Timestamp: " << block.timestamp << endl;
            cout << "Transactions: " << block.transactions.size() << endl;
        } else {
            cout << "Block index out of range." << endl;
        }
    }
};

class User {
public:
    string name;
    string public_key;
    vector<UTXO> utxos;

    User(const string &name, const string &public_key)
        : name(name), public_key(public_key) {}

    int balance() const {
        int total = 0;
        for (const auto &utxo : utxos) {
            total += utxo.amount;
        }
        return total;
    }
};

vector<User> generate_users(Blockchain &blockchain, int num_users) {
    vector<User> users;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(100, 1000000);

    for (int i = 0; i < num_users; ++i) {
        string name = "User" + to_string(i);
        string public_key = HashFunction(name);
        User user(name, public_key);

        int balance = dist(gen);
        UTXO initial_utxo("utxo_" + public_key.substr(0, 6), balance, public_key);
        user.utxos.push_back(initial_utxo);
        blockchain.utxo_pool.emplace(initial_utxo.tx_id, initial_utxo);

        users.push_back(user);
    }
    return users;
}

vector<Transaction> generate_transactions(const vector<User> &users, int target_num_transactions) {
    vector<Transaction> transactions;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> user_dist(0, users.size() - 1);

    while (transactions.size() < target_num_transactions) {
        int sender_index = user_dist(gen);
        User sender = users[sender_index];

        if (!sender.utxos.empty()) {
            UTXO utxo_to_spend = sender.utxos[rand() % sender.utxos.size()];

            int amount_to_send = rand() % utxo_to_spend.amount + 1;

            int receiver_index = user_dist(gen);
            while (receiver_index == sender_index) {
                receiver_index = user_dist(gen);
            }

            User receiver = users[receiver_index];

            UTXO receiver_utxo(utxo_to_spend.tx_id, amount_to_send, receiver.public_key);
            vector<UTXO> outputs = {receiver_utxo};

            UTXO change_utxo("", 0, sender.public_key);
            if (utxo_to_spend.amount > amount_to_send) {
                change_utxo = UTXO(utxo_to_spend.tx_id + "_change", utxo_to_spend.amount - amount_to_send,
                                   sender.public_key);
                outputs.push_back(change_utxo);
            }

            Transaction new_tx({utxo_to_spend}, outputs);
            transactions.push_back(new_tx);
        }
    }
    return transactions;
}

int main() {
    Blockchain blockchain(4);
    vector<User> users = generate_users(blockchain, 1000);

    cout << "Total Users Created: " << users.size() << endl;

    vector<Transaction> transactions = generate_transactions(users, 1000);
    cout << "Total Transactions Created: " << transactions.size() << endl;

    for (const auto &tx : transactions) {
        blockchain.add_transaction(tx);
    }

    while (!blockchain.pending_transactions.empty()) {
        blockchain.mine_pending_transactions();
    }

    blockchain.print_block(blockchain.chain.size() - 1);

    return 0;
}

