import socket
import sys

s = socket.socket()
s.connect(('127.0.0.1', 8080))

packetId = (0).to_bytes()
content = b'ciao mondo'
packetSize = len(content).to_bytes(2, 'little')

s.send(packetId + packetSize + content)
data=s.recv(1024)
print('sei il giocatore #'+str(data[-1]))

#
if len(sys.argv) > 1 and sys.argv[1] == 'create':
    packetId = (1).to_bytes()
    content = b'creo partita'
    packetSize = len(content).to_bytes(2, 'little')

    s.send(packetId + packetSize + content)
    data=s.recv(1024)
    print(data)
else:
    packetId = (3).to_bytes()
    content = b'\x00'
    packetSize = len(content).to_bytes(2, 'little')

    s.send(packetId + packetSize + content)

data = b''
while True:
    data=s.recv(1024)
    if data:
        if data[0] == 0x17:
            print('invio accettazione')
            packetId = (4).to_bytes()
            content = b'\x00'
            packetSize = len(content).to_bytes(2, 'little')
            s.send(packetId + packetSize + content)
    else:
        break

s.close()