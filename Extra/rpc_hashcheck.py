import struct
from bitcoin.rpc import RawProxy
from hashlib import sha256

p = RawProxy()

block_hash = '' ##Input block hash here

block = p.getblock(block_hash)

version = block['version']
prev_block_hash = block['previousblockhash']
merkle_root = block['merkleroot']
timestamp = block['time']
bits = block['bits']
nonce = block['nonce']

header = (
    struct.pack("<L", version) +
    bytes.fromhex(prev_block_hash)[::-1] +
    bytes.fromhex(merkle_root)[::-1] +
    struct.pack("<L", timestamp) +
    struct.pack("<L", int(bits, 16)) +
    struct.pack("<L", nonce)
)

hash_calculated = sha256(sha256(header).digest()).digest()[::-1].hex()

if hash_calculated == block_hash:
    print("Bloko hash'as yra teisingai apskaičiuotas.")
else:
    print("Bloko hash'as yra neteisingas.")
