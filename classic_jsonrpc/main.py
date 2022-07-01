#!/bin/python3
import asyncio
from jsonrpc_websocket import Server
import base64

#create instance with http POST like: curl -vvv -X POST http://localhost:8888/classic/{devicename}
#in example below used classic_test device name...

async def routine():
    server = Server('ws://localhost:8888/classic/classic_test')

    try:
        await server.ws_connect()

        await server.Set_ConnectionURI("tcp://localhost:7778")
        typez = await server.Get_ConnectionType()
        print(typez)
        ret = await server.Connect()
        code = await server.Get_ResultCode()

        code_description = await server.Get_ResultCodeDescription()
        some_binary_data = base64.b64encode(b'kinda binary')
        await server.Set_BlockData(some_binary_data)
        print(code, code_description)

    finally:
        await server.close()

asyncio.get_event_loop().run_until_complete(routine())
