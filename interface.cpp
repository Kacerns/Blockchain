#include "einsteinhash.hpp"

class UTXO {
public:
    string tx_id;
    double amount;
    string owner;

    UTXO(const string& tx_id, double amount, const string& owner)
        : tx_id(tx_id), amount(amount), owner(owner) {
    }

    string to_string() const {
        ostringstream oss;
        oss << "UTXO(tx_id=" << tx_id << ", amount=" << amount << ", owner=" << owner << ")";
        return oss.str();
    }
};

class Transaction {
private:
    vector<UTXO> inputs;
    vector<UTXO> outputs;
    string tx_id;
public:
    Transaction(const vector<UTXO>& inputs, const vector<UTXO>& outputs)
        : inputs(inputs), outputs(outputs) {
        tx_id = calculate_hash();
    }

    string calculate_hash(){
        string tx_data = "";
        for (const auto& utxo : inputs) {
            tx_data += utxo.tx_id;
        }
        for (const auto& utxo : outputs) {
            cout<<to_string(utxo.amount)<<endl;
            tx_data += to_string(utxo.amount);
        }
        return HashFunction(tx_data);
    }

    void print_transaction() const {
        cout << "Transaction ID: " << tx_id << endl;
        cout << "Inputs:" << endl;
        for (const auto& utxo : inputs) {
            cout << "  - " << utxo.to_string() << endl;
        }
        cout << "Outputs:" << endl;
        for (const auto& utxo : outputs) {
            cout << "  - " << utxo.to_string() << endl;
        }
    }
};
class User{
    string name;
    string public_key;
    string balance;
};

int main(int argc, const char *argv[]){

    UTXO utxo1("tx123", 50.0, "Alice");
    UTXO utxo2("tx124", 30.0, "Bob");

    vector<UTXO> inputs = { utxo1 };
    vector<UTXO> outputs = { utxo2 };

    Transaction transaction(inputs, outputs);
    transaction.print_transaction();

    return 0;
}
