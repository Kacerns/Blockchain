#include "einsteinhash.hpp"

class UTXO {
public:
    int amount;
    string owner;
    string tx_id;
    bool spent = false;

    UTXO() : tx_id(""), amount(0), owner("") {}


    UTXO(int amount, const string &owner): tx_id(HashFunction(to_string(amount)+owner)), amount(amount), owner(owner) {}
    UTXO(const string &tx_id, int amount, const string &owner)
        : tx_id(tx_id), amount(amount), owner(owner) {}
     friend ostream &operator<<(ostream &os, const UTXO &utxo) {
        os << "UTXO(tx_id=" << utxo.tx_id << ", amount=" << utxo.amount << ", owner=" << utxo.owner << ")";
        return os;
    }

    void set_as_spent(){
        this->spent = true;
    }
};

class Transaction {
public:
    vector<UTXO> inputs;
    vector<UTXO> outputs;
    bool valid = true;
    string tx_id;

    Transaction(const vector<UTXO> &inputs, const vector<UTXO> &outputs)
        : inputs(inputs), outputs(outputs){
        int sender_coin = 0;
        for (int i = 0; i< inputs.size(); i++){
            sender_coin += inputs.at(i).amount;
            this -> inputs.at(i).set_as_spent();
        }
        int amount = 0;
        for (int i = 0; i< outputs.size(); i++){
            amount += outputs.at(i).amount;
        }
        switch (sender_coin > amount){
        case false:
            if(sender_coin == amount){
                break;
            }
            valid = false;
            break;
        default:
            UTXO change_utxo(sender_coin - amount, inputs.at(0).owner);
            this->outputs.push_back(change_utxo);
            break;
        }
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
    string merkle_root;
    int nonce = 0;
    string prev_hash;
    string timestamp;
    vector<Transaction> transactions;
    bool hash_found = false;
    int difficulty;
    string block_hash;

    Block(const string &prev_hash, const vector<Transaction> &transactions, int difficulty)
        : prev_hash(prev_hash), transactions(transactions), difficulty(difficulty) {
        timestamp = get_timestamp();
        merkle_root = calculate_merkle_root();
        if(prev_hash == "0"){
            block_hash = "genesis";
            hash_found = true;
        }
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

class Blockchain{
private:
    unordered_map<string, vector<UTXO>> utxo_pool;
    vector<Block> chain;
public:
    int difficulty;
    vector<Block> block_candidates;
    vector<Transaction> pending_transactions;

    Blockchain(int difficulty) : difficulty(difficulty) {
        chain.push_back(create_genesis_block());
    }

    Block create_genesis_block() {
        UTXO genesis_utxo("genesis_tx", 1000000, "genesis_owner");
        Transaction genesis_tx({}, {genesis_utxo});
        genesis_tx.valid = true;
        utxo_pool[genesis_utxo.tx_id].push_back(genesis_utxo);
        return Block("0", {genesis_tx}, difficulty);
    }
    void add_transaction(const Transaction &transaction) {
        bool validator = true;
        for (const auto &input : transaction.inputs) {
            if (utxo_pool.find(input.tx_id) == utxo_pool.end()) {
                validator = false;
                break;
            }
        }
        if (validator) {
            if(transaction.valid == true){
                cout<<transaction.valid<<endl;
                pending_transactions.push_back(transaction);
            }
        }
    }
    void confirm_transactions(const vector<Transaction> &transactions_to_confirm){
        for(const auto &transaction : transactions_to_confirm){
            for (const auto &input : transaction.inputs) {
                utxo_pool.erase(input.tx_id);
            }
            for (const auto &output : transaction.outputs) {
                utxo_pool[output.tx_id].push_back(output);
            }
        }
    }
    void create_block_candidates(){
        int number_of_block_candidates = 0;
        vector<Transaction> candidate_transactions = pending_transactions;
        while (candidate_transactions.size() > 0 && number_of_block_candidates<4) {

            random_device rd;
            mt19937 gen(rd());
            shuffle(candidate_transactions.begin(), candidate_transactions.end(), gen);
            size_t transactions_to_take = min<size_t>(100, candidate_transactions.size());
            vector<Transaction> transactions_for_candidates(candidate_transactions.end()-transactions_to_take, candidate_transactions.end());
            string prev_hash = chain.back().block_hash;
            block_candidates.push_back(Block(prev_hash, transactions_for_candidates, difficulty));
            number_of_block_candidates++;

            candidate_transactions.erase(candidate_transactions.end()-transactions_to_take, candidate_transactions.end());
        }
        pending_transactions = candidate_transactions;
    }
    void mine_block_candidates() {
        atomic<bool> block_mined{false};
        mutex mtx;
        vector<thread> threads;
        vector<Transaction> unused_transactions;

        auto mine_block = [&](Block& block) {
            string target(block.difficulty, '0');
            do{
                if (block_mined.load()) {
                    return;
                }
                block.block_hash = block.calculate_hash();
                if (block.block_hash.substr(0, block.difficulty) == target){
                    block.hash_found=true;
                }
                else{block.nonce++;}
            }while(block.hash_found == false);

            if (!block_mined.exchange(true)) {
                lock_guard<mutex> lock(mtx);
                block_mined = true;
                cout << "Block mined by: thread " << this_thread::get_id() << endl;

                for(const auto &returning_blocks : block_candidates){
                    if(returning_blocks.merkle_root != block.merkle_root){
                        pending_transactions.insert(pending_transactions.end(), returning_blocks.transactions.begin(), returning_blocks.transactions.end());
                    }
                }

                confirm_transactions(block.transactions);

                chain.push_back(block);
            }
        };

        for (Block& block : block_candidates) {
            threads.emplace_back(mine_block, ref(block));
        }
        for (auto& t : threads) {
            t.join();
        }
        if (!block_mined) {
            for(const auto &returning_blocks : block_candidates){
                pending_transactions.insert(pending_transactions.end(), returning_blocks.transactions.begin(), returning_blocks.transactions.end());
            }
        }
        block_candidates.clear();
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

    int get_user_balance(const string &public_key) const {
        int balance = 0;
        for (const auto &pair : utxo_pool) {
            if(!pair.second.empty()){
                const string validation = pair.second.at(0).owner;
                if (validation == public_key) {
                    for(const auto& utxo : pair.second){
                    balance += utxo.amount;
                    }
                }
            }
        }
        return balance;
    }
    int get_total_coin() const {
        int sum = 0;
        for (const auto &pair : utxo_pool) {
            for(const auto &utxo : pair.second){
                sum += utxo.amount;
            }
        }
        return sum;
    }
    void Launch(){
        while(pending_transactions.size() > 0){
             create_block_candidates();
             mine_block_candidates();
        }
    }
    unordered_map<string, vector<UTXO>> get_utxo_pool_copy(){
        return utxo_pool;
    }
    void set_utxo_pool(unordered_map<string, vector<UTXO>> &utxo_pool_copy){
        utxo_pool = utxo_pool_copy;
    }
};
class User {
private:
    string name;
public:
    string public_key;

    User(const string &name)
        : name(name), public_key(HashFunction(name)) {}

    int get_balance(const Blockchain &blockchain) const {
        return blockchain.get_user_balance(public_key);
    }
    vector<UTXO> get_user_utxos(Blockchain &blockchain) const {
        unordered_map<string, vector<UTXO>> utxo_pool_copy = blockchain.get_utxo_pool_copy();
        vector<UTXO> user_utxo;
        for (const auto &pair : utxo_pool_copy) {
            if(!pair.second.empty()){
                if (pair.second.at(0).owner == this->public_key) {
                    for(const auto Utxos: pair.second){user_utxo.push_back(Utxos);}
                }
            }
        }
        return user_utxo;
    }
};
 vector<User> generate_users(Blockchain &blockchain, int num_users) {
    unordered_map<string, vector<UTXO>> utxo_pool_copy = blockchain.get_utxo_pool_copy();
    vector<User> users;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(100, 1000000);

    for (int i = 0; i < num_users; ++i) {
        string name = "User" + to_string(i);
        User user(name);

        int balance = dist(gen);
        UTXO initial_utxo("utxo_" + user.public_key.substr(0, 6), balance, user.public_key);
        utxo_pool_copy[initial_utxo.tx_id].push_back(initial_utxo);

        users.push_back(user);
    }
    blockchain.set_utxo_pool(utxo_pool_copy);
    return users;
}
vector<Transaction> generate_transactions(Blockchain &blockchain, const vector<User> &users, int target_num_transactions) {
    vector<Transaction> transactions;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> user_dist(0, users.size() - 1);

    while (transactions.size() < target_num_transactions) {
        int sender_index = user_dist(gen);
        const User &sender = users[sender_index];

        vector<UTXO> sender_utxos = sender.get_user_utxos(blockchain);

        if (!sender_utxos.empty()) {
            int utxo_index = gen() % sender_utxos.size();
            UTXO utxo_to_spend = sender_utxos[utxo_index];

            int amount_to_send = gen() % utxo_to_spend.amount;
            while(amount_to_send == 0){
                gen() % utxo_to_spend.amount;
            }

            int receiver_index = user_dist(gen);
            while (receiver_index == sender_index) {
                receiver_index = user_dist(gen);
            }
            const User &receiver = users[receiver_index];

            UTXO receiver_utxo(utxo_to_spend.tx_id + "_to_" + receiver.public_key.substr(0, 6), amount_to_send, receiver.public_key);
            vector<UTXO> outputs = {receiver_utxo};

            Transaction new_tx({utxo_to_spend}, outputs);
            transactions.push_back(new_tx);
        }
    }
    return transactions;
}
Transaction make_transaction(User &sender,User &receiver, int amount, Blockchain &blockchain){
        vector<UTXO> inputs;
        vector<UTXO> outputs;
        int sender_coin = 0;
        vector<UTXO> sender_utxos = sender.get_user_utxos(blockchain);
        int locator;


        while (sender_coin < amount && !sender_utxos.empty()){
            UTXO filter;
            for(int i = 0; i<=sender_utxos.size()-1; i++){
                if(sender_utxos.at(i).amount > filter.amount){
                    filter = sender_utxos.at(i);
                    locator = i;
                }
            }
            sender_utxos.erase(sender_utxos.begin()+locator);
            sender_coin += filter.amount;
            inputs.push_back(filter);
        }
        if(sender_coin < amount){
            Transaction Result(inputs,outputs);
            Result.valid = false;
            return Result;
        }
        UTXO transaction_utxo(amount, receiver.public_key);
        outputs.push_back(transaction_utxo);
        Transaction Result(inputs,outputs);
        return Result;
}

int main() {
    // Blockchain blockchain(2);
    // vector<User> users = generate_users(blockchain, 1000);

    // cout << "Total Users Created: " << users.size() << endl;

    // vector<Transaction> transactions = generate_transactions(blockchain, users, 10000);
    // cout << "Total Transactions Created: " << transactions.size() << endl;

    // for (const auto &tx : transactions) {
    //     blockchain.add_transaction(tx);
    // }
    
    // blockchain.Launch();
    // blockchain.print_block(100);


    Blockchain local_blockchain(2);
    unordered_map<string, vector<UTXO>> utxo_pool_copy = local_blockchain.get_utxo_pool_copy();
    User Tom("Tom");
    UTXO initial_utxo("utxo_" + Tom.public_key.substr(0, 6), 2, Tom.public_key);
    utxo_pool_copy[initial_utxo.tx_id].push_back(initial_utxo);
    UTXO initial_utxo2("utxo_" + Tom.public_key.substr(0, 6), 3, Tom.public_key);
    utxo_pool_copy[initial_utxo2.tx_id].push_back(initial_utxo2);
    local_blockchain.set_utxo_pool(utxo_pool_copy);

    User Jerry("Jerry");
    Transaction Test = make_transaction(Tom, Jerry, 6, local_blockchain);
    Test.print_transaction();
    local_blockchain.add_transaction(Test);
    local_blockchain.Launch();
    local_blockchain.print_block(1);

    return 0;
}


