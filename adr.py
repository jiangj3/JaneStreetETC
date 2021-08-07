def act(id, name, bs, price, size):
    write_to_exchange(exchange, {"type": "add", "order_id": id, "symbol": name, "dir": bs, "price": price, "size": size})

def convert(id, name, bs, size):
    write_to_exchange(exchange, {"type": "convert", "order_id": id, "symbol": name, "dir": bs, "size": size})

if valebuy < (valbzsell-buffer):
    act(id, "VALE", "BUY", valebuy, 2)
    convert(id, "VALE", "SELL", 2)
    act(id, "VALBZ", "SELL", valbzsell, 2)

if valbzbuy < (valesell-buffer):
    act(id, "VALBZ", "BUY", valbzbuy, 2)
    convert(id, "VALE", "BUY", 2)
    act(id, "VALE", "BUY", valesell, 2)
