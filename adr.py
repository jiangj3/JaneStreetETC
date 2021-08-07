def act(id, name, bs, price, size):
    return "ADD " + id + " "+ name + " " + bs + " " + price+ " "+size

def convert(id, name, bs, size):
    return "CONVERT " + id + " "+name+ " "+bs + " "+size

if valebuy < (valbzsell-buffer):
    act(id, "VALE", "BUY", valebuy, 2)
    convert(id, "VALE", "SELL", 2)
    act(id, "VALBZ", "SELL", valbzsell, 2)