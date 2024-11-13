from bitcoin.rpc import RawProxy

p = RawProxy()

txid = '' ##input transaction id

transaction = p.getrawtransaction(txid, True)

input_total = 0
for vin in transaction['vin']:
    in_txid = vin['txid']
    vout_index = vin['vout']
    
    in_transaction = p.getrawtransaction(in_txid, True)
    
    input_total += in_transaction['vout'][vout_index]['value'] * 100000000  # Satoshi

output_total = sum(vout['value'] * 100000000 for vout in transaction['vout'])  # Satoshi

fee = input_total - output_total

print(f"Transakcijos mokestis yra: {fee} satoshi")
