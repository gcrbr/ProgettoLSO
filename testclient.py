import socket
import sys
import time

s = socket.socket()
s.connect(('127.0.0.1', 8080))

packetId = (0).to_bytes()
content = b'ciao mondo'
packetSize = len(content).to_bytes(2, 'little')

s.send(packetId + packetSize + content)
data=s.recv(1024)
player_id = data[-1]
print('sei il giocatore #'+str(player_id))

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

i=0
j=0
data = b''
while True:
    data=s.recv(1024)
    if data:
        print(data)
        if len(sys.argv) > 1 and sys.argv[1] == 'create':
            if data[0] == 0x17:
                packetId = (4).to_bytes()
                content = b'\x10\x00' #b'\x00\x00' per rifiutare
                packetSize = len(content).to_bytes(2, 'little')
                print('invio accettazione', packetId + packetSize + content)
                s.send(packetId + packetSize + content)
            if data[0] == 0x18:
                if data[3] == 0x4: # è il mio turno (4=PLAYER1)
                    print('mio turno!')
                    s.send(b'\x05\x03\x00' + chr(i).encode('utf-8') + b'\x00\x00') # prova a vincere sulla prima riga
                    i += 1
        else:
            if data[0] == 0x18:
                if data[3] == 0x5: # (5=PLAYER2)
                    print('mio turno!')
                    s.send(b'\x05\x03\x00' + chr(j).encode('utf-8') + b'\x01\x00') # prova a vincere sulla seconda riga
                    j += 1
    else:
        break

s.close()