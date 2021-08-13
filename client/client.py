import struct
import socket
import errno
import rsa
import weakref
import threading

from proto import rpc_login_pb2
from proto import login_msg_pb2

# /*
# Byte order:little-endian
# Native byte order is big-endian or little-endian, depending on the host system. For example, Intel x86 and AMD64 (x86-64) are little-endian;
# Byte
# /       0       |       1       |       2       |       3       |
# /               |               |               |               |
# |0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
# +---------------+---------------+---------------+---------------+
# 0| ..........R|C|   --iMagic--  |          ---iOpcode---
# +---------------+---------------+---------------+---------------+
# 4|                           iBodyLen
# +---------------+---------------+---------------+---------------+
# 4|          iCheckSum           |              Data
# +---------------+---------------+---------------+---------------+
# 4|                             Data
# +---------------+---------------+---------------+---------------+
# */

class Protocol:
    def __init__(self):
        self.iFlags = 0
        self.iMagic = 0
        self.iOpcode = 0
        self.iBodyLen = 0
        self.iCheckSum = 0
        self.bBody = b""

    def __str__(self):
        return f"iFlags:{self.iFlags},iMagic:{self.iMagic},iOpcode:{self.iOpcode},iBodyLen:{self.iBodyLen},iCheckSum:{self.iCheckSum}"


def packToStreams(protocol):
    iBodyLen = len(protocol.bBody)
    protocol.iBodyLen = iBodyLen

    fmt = '<bbhii%ds'%(iBodyLen,)
    bytes = struct.pack(fmt, protocol.iFlags, protocol.iMagic, protocol.iOpcode, protocol.iBodyLen, protocol.iCheckSum,
                        protocol.bBody)
    print("bytes:", bytes)
    return bytes


def unpackFromStreams(buffer):
    iRecvLen = len(buffer)
    iHeadLen = 1 + 1 + 2 + 4 + 4
    if iRecvLen < iHeadLen:
        return

    bHead = buffer[:iHeadLen]
    iFlags, iMagic, iOpcode, iBodyLen, iCheckSum = struct.unpack('<bbhii', bHead)
    if iRecvLen != iHeadLen + iBodyLen:
        return

    protocol = Protocol()
    protocol.iFlags = iFlags
    protocol.iMagic = iMagic
    protocol.iOpcode = iOpcode
    protocol.iBodyLen = iBodyLen
    protocol.iCheckSum = iCheckSum
    protocol.bBody = buffer[iHeadLen: iHeadLen+iBodyLen]

    return protocol
    
    

def testPack1():
    data = Protocol()
    data.iOpcode = 100
    data.bBody = b"hello world"
    data.iBodyLen = len(data.bBody)

    sPack = packToStreams(data)

    uppackData = unpackFromStreams(sPack)

def testPack2():
    data = Protocol()
    data.iFlags = 1
    data.iMagic = 2
    data.iOpcode = 100
    data.iCheckSum = 123456

    pbMsg = rpc_login_pb2.L2G_LoginPendingRequest()
    pbMsg.account_id = 123
    pbMsg.session_key = "hello world"
    print("pbMsg:", pbMsg)

    data.bBody = pbMsg.SerializeToString()
    print("data:", data)


    sPack = packToStreams(data)
    uppackData = unpackFromStreams(sPack)

    response = rpc_login_pb2.L2G_LoginPendingRequest()
    response.ParseFromString(uppackData.bBody)
    print("uppackData:", uppackData)
    print("resposne:", response)

class Client(threading.Thread):
    def __init__(self, world):
        threading.Thread.__init__(self)

        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.addr = ("127.0.0.1", 16007)
        self.alive = True
        self.buff = b""
        self.registerCb = {}

        self.accountId = 0
        self.sessionKey = ""

        self.world = weakref.ref(world)

        self.init()

    def run(self):
        self.recv()

    def connect(self):
        try:
            self.client.connect(self.addr)
        except socket.timeout as e:
            print(e)

    def send(self, iOpcode, pbMsg):
        print("pbMsg:", pbMsg)

        data = Protocol()
        data.iFlags = 0
        data.iMagic = 0
        data.iOpcode = iOpcode
        data.iCheckSum = 0
        data.bBody = pbMsg.SerializeToString()
        sPack = packToStreams(data)
        self.client.sendall(sPack)
        print("send:", sPack)

    def recv(self):
        while self.alive:
            try:
                sBuff = self.client.recv(1024*8)
                self.buff += sBuff

                print("recv:", sBuff)

                protocolObj = unpackFromStreams(self.buff)
                iTotalLen = 12 + protocolObj.iBodyLen
                self.buff = self.buff[iTotalLen:]

                callback = self.getHandler(protocolObj.iOpcode)
                if callback is None:
                    print("unregister handler|iopcode:", protocolObj.iOpcode, "|", protocolObj)
                    continue
                callback(self, protocolObj.bBody)

                # response = login_msg_pb2.MSG_RESPONSE_ACCOUNT_LOGIN_L()
                # response.ParseFromString(protocolObj.bBody)
                # print("response:", response)
            except ConnectionResetError as e:
                print(e)
                raise e

    def registerHandler(self, iOpcode, callback):
        if iOpcode in self.registerCb:
            print("duplicate opcode:", iOpcode)
            return
        self.registerCb[iOpcode] = callback

    def getHandler(self, iOpcode):
        if iOpcode in self.registerCb:
            return self.registerCb[iOpcode]
        return None

    def init(self):
        self.registerHandler(1001, handle_MSG_RESPONSE_ACCOUNT_LOGIN_L)
        self.registerHandler(1007, handle_MSG_RESPONSE_HANDSHAKE_INIT)
        self.registerHandler(1009, handle_MSG_RESPONSE_HANDSHAKE_ESTABLISHED)
        self.registerHandler(1003, handle_MSG_RESPONSE_CLIENT_LOGIN)

    def sendLogin(self, id):
        pbMsg = login_msg_pb2.MSG_REQUEST_ACCOUNT_LOGIN_L()
        pbMsg.platform_id = "test"
        pbMsg.program_id = "hello world"
        pbMsg.version = 100
        pbMsg.account_id = id
        pbMsg.auth = "test"

        self.send(1000, pbMsg)

def handle_MSG_RESPONSE_ACCOUNT_LOGIN_L(clientObj, sBuff):
    response = login_msg_pb2.MSG_RESPONSE_ACCOUNT_LOGIN_L()
    response.ParseFromString(sBuff)
    print("response:", response)

    clientObj.accountId = response.account_id
    clientObj.sessionKey = response.session_key

    clientObj.client.close()

    clientObj.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    clientObj.addr = (response.ip, response.port)
    clientObj.connect()

    pbMsg = login_msg_pb2.MSG_REQUEST_HANDSHAKE_INIT()
    pbMsg.client_random = "client"
    clientObj.send(1006, pbMsg)

def handle_MSG_RESPONSE_HANDSHAKE_INIT(clientObj, sBuff):
    response = login_msg_pb2.MSG_RESPONSE_HANDSHAKE_INIT()
    response.ParseFromString(sBuff)
    print("response:", response)

    plainMsg = "client_key"
    pubKey = rsa.PublicKey.load_pkcs1_openssl_pem(response.public_key.encode())
    encryptedMsg = rsa.encrypt(plainMsg.encode(), pubKey)

    pbMsg = login_msg_pb2.MSG_REQUEST_HANDSHAKE_ESTABLISHED()
    pbMsg.encrypted_key = encryptedMsg
    clientObj.send(1008, pbMsg)

def handle_MSG_RESPONSE_HANDSHAKE_ESTABLISHED(clientObj, sBuff):
    response = login_msg_pb2.MSG_RESPONSE_HANDSHAKE_ESTABLISHED()
    response.ParseFromString(sBuff)
    print("response:", response)

    pbMsg = login_msg_pb2.MSG_REQUEST_CLIENT_LOGIN()
    pbMsg.user_id = clientObj.accountId
    pbMsg.session_key = clientObj.sessionKey
    clientObj.send(1002, pbMsg)

def handle_MSG_RESPONSE_CLIENT_LOGIN(clientObj, sBuff):
    response = login_msg_pb2.MSG_RESPONSE_CLIENT_LOGIN()
    response.ParseFromString(sBuff)
    print("response:", response)

    if response.status_code == 0:
        clientObj.world().accountId = response.user_id

class TestObj:
    def __init__(self):
        self.accountId = 0

testObj = TestObj()
def testPack3():

    clientObj = Client(testObj)
    clientObj.connect()
    clientObj.registerHandler(1001, handle_MSG_RESPONSE_ACCOUNT_LOGIN_L)
    clientObj.registerHandler(1007, handle_MSG_RESPONSE_HANDSHAKE_INIT)
    clientObj.registerHandler(1009, handle_MSG_RESPONSE_HANDSHAKE_ESTABLISHED)
    clientObj.registerHandler(1003, handle_MSG_RESPONSE_CLIENT_LOGIN)

    pbMsg = login_msg_pb2.MSG_REQUEST_ACCOUNT_LOGIN_L()
    pbMsg.platform_id = "123"
    pbMsg.program_id = "hello world"
    pbMsg.version = 100
    pbMsg.account_id = 369
    pbMsg.auth = "test"

    clientObj.send(1000, pbMsg)
    clientObj.start()
    clientObj.join()

# testPack1()
# testPack2()
# testPack3()