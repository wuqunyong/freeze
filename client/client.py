import struct
import socket
import errno
import os
import rsa
import weakref
import threading
import lz4.frame
from arc4 import ARC4

from proto import protocol_pb2
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
# 0|                           iSeqNum
# +---------------+---------------+---------------+---------------+
# 4| ..........R|C|   --iMagic--  |          ---iOpcode---
# +---------------+---------------+---------------+---------------+
# 8|                           iBodyLen
# +---------------+---------------+---------------+---------------+
# 12|          iCheckSum          |              Data
# +---------------+---------------+---------------+---------------+
# 16|                             Data
# +---------------+---------------+---------------+---------------+
# */

PH_COMPRESSED = 0x01
PH_CRYPTO = 0x02

class Protocol:
    def __init__(self):
        self.iSeqNum = 0
        self.iFlags = 0
        self.iMagic = 0
        self.iOpcode = 0
        self.iBodyLen = 0
        self.iCheckSum = 0
        self.bBody = b""

    def __str__(self):
        return f"iSeqNum:{self.iSeqNum},iFlags:{self.iFlags},iMagic:{self.iMagic},iOpcode:{self.iOpcode},iBodyLen:{self.iBodyLen},iCheckSum:{self.iCheckSum}"


def packToStreams(protocol):
    iBodyLen = len(protocol.bBody)
    protocol.iBodyLen = iBodyLen

    fmt = '<ibbhii%ds'%(iBodyLen,)
    bytes = struct.pack(fmt, protocol.iSeqNum, protocol.iFlags, protocol.iMagic, protocol.iOpcode, protocol.iBodyLen, protocol.iCheckSum,
                        protocol.bBody)
    print("bytes:", bytes)
    return bytes


def unpackFromStreams(buffer):
    iRecvLen = len(buffer)
    iHeadLen = 4 + 1 + 1 + 2 + 4 + 4
    if iRecvLen < iHeadLen:
        return

    bHead = buffer[:iHeadLen]
    iSeqNum, iFlags, iMagic, iOpcode, iBodyLen, iCheckSum = struct.unpack('<ibbhii', bHead)
    if iRecvLen != iHeadLen + iBodyLen:
        return

    protocol = Protocol()
    protocol.iSeqNum = iSeqNum
    protocol.iFlags = iFlags
    protocol.iMagic = iMagic
    protocol.iOpcode = iOpcode
    protocol.iBodyLen = iBodyLen
    protocol.iCheckSum = iCheckSum
    protocol.bBody = buffer[iHeadLen: iHeadLen+iBodyLen]

    return protocol

class Client(threading.Thread):
    def __init__(self, world):
        threading.Thread.__init__(self)

        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.addr = ("127.0.0.1", 16007)
        self.alive = True
        self.buff = b""
        self.registerCb = {}

        #压缩，加密
        self.flag = PH_COMPRESSED | PH_CRYPTO
        self.cipher = ''

        self.accountId = 0
        self.sessionKey = ""

        self.world = weakref.ref(world)

        self.init()

        self.iSeqNum = 0

    def setCipher(self, data):
        self.cipher = data

    def run(self):
        self.recv()
        print("run线程退出")

    def connect(self):
        try:
            self.client.connect(self.addr)
        except socket.timeout as e:
            print(e)

    def send(self, iOpcode, pbMsg):
        print("pbMsg:", pbMsg)

        self.iSeqNum += 1

        data = Protocol()
        data.iSeqNum = self.iSeqNum
        data.iFlags = self.flag
        data.iMagic = 0
        data.iOpcode = iOpcode
        data.iCheckSum = 0
        data.bBody = pbMsg.SerializeToString()

        if self.flag & PH_COMPRESSED:
            compressedData = lz4.frame.compress(data.bBody)
            data.bBody = compressedData

        if self.flag & PH_CRYPTO:
            if len(self.cipher) > 0:
                arc4 = ARC4(self.cipher)
                cipherText = arc4.encrypt(data.bBody)
                data.bBody = cipherText
            else:
                iFlag = ~PH_CRYPTO
                data.iFlags = data.iFlags & iFlag

        print("send Protocol:", data)
        sPack = packToStreams(data)
        self.client.sendall(sPack)
        print("send:", sPack)

    def passiveClose(self):
        self.alive = False

    def recv(self):
        while self.alive:
            try:
                sBuff = self.client.recv(1024*8)
                self.buff += sBuff

                print("recv:", sBuff)

                if len(sBuff) == 0:
                    self.passiveClose()
                    continue

                protocolObj = unpackFromStreams(self.buff)
                iTotalLen = 16 + protocolObj.iBodyLen
                self.buff = self.buff[iTotalLen:]

                callback = self.getHandler(protocolObj.iOpcode)
                if callback is None:
                    print("unregister handler|iopcode:", protocolObj.iOpcode, "|", protocolObj)
                    continue

                if protocolObj.iFlags & PH_CRYPTO:
                    arc4 = ARC4(self.cipher)
                    plainText = arc4.decrypt(protocolObj.bBody)
                    protocolObj.bBody = plainText

                if protocolObj.iFlags & PH_COMPRESSED:
                    outputData = lz4.frame.decompress(protocolObj.bBody)
                    protocolObj.bBody = outputData

                print("recv Protocol:", protocolObj)
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
        self.registerHandler(protocol_pb2.OP_ClientLoginRequest, handle_MSG_RESPONSE_CLIENT_LOGIN)

        self.registerHandler(1005, handle_MSG_RESPONSE_ECHO)

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

    cipher = "client" + response.server_random + plainMsg
    clientObj.setCipher(cipher)

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
        clientObj.world().player.ammo = response.ammo
        clientObj.world().player.start_ammo = response.ammo
        clientObj.world().player.grenades = response.grenades

        pbMsg = login_msg_pb2.MSG_REQUEST_ECHO()
        pbMsg.value1 = 123456
        pbMsg.value2 = "hello world"
        clientObj.send(1004, pbMsg)

def handle_MSG_RESPONSE_ECHO(clientObj, sBuff):
    response = login_msg_pb2.MSG_RESPONSE_ECHO()
    response.ParseFromString(sBuff)
    print("response:", response)


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

class TestPlayer:
    def __init__(self):
        self.ammo = 0
        self.start_ammo = 0
        self.grenades = 0

class TestWorld:
    def __init__(self):
        self.accountId = 0

testObj = TestWorld()
testObj.player = TestPlayer()

def testPack3():
    clientObj = Client(testObj)
    clientObj.connect()

    pbMsg = login_msg_pb2.MSG_REQUEST_ACCOUNT_LOGIN_L()
    pbMsg.platform_id = "123"
    pbMsg.program_id = "hello world"
    pbMsg.version = 100
    pbMsg.account_id = 369
    pbMsg.auth = "test"

    clientObj.send(1000, pbMsg)
    clientObj.start()
    clientObj.join()

def testLz4():
    # input_data = 20 * 128 * os.urandom(1024)  # Read 20 * 128kb
    #input_data = b'\n\x03123\x12\x0bhello world\x18d \xf1\x02*\x04test'
    input_data = "hello world".encode()
    #out b'\x1d\x00\x00\x00\xf0\x0e\n\x03123\x12\x0bhello world\x18d \xf1\x02*\x04test'

    compressed_data = lz4.frame.compress(input_data)
    output_data = lz4.frame.decompress(compressed_data)
    if input_data == output_data:
        print("success")
    else:
        print("failure")

def testRC4():
    key = "123"
    arc4 = ARC4(key)
    input_data = b'some plain text to encrypt'
    cipher = arc4.encrypt(input_data)

    arc4Recv = ARC4(key)
    plainText = arc4Recv.decrypt(cipher)
    if input_data == plainText:
        print("success")
    else:
        print("failure")

# testPack1()
# testPack2()
# testPack3()
# testLz4()
# testRC4()