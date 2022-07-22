#!/bin/python3
import asyncio
import aiohttp
from jsonrpc_websocket import Server
import base64

#считаем что предварительно запустили сервер на 8080 порту console_test_fr_drv_ng run-ws-json-rpc-server 0.0.0.0:8080

async def routine(device_name):
    async with aiohttp.ClientSession() as session:
        async with session.post(f'ws://localhost:8080/classic/{device_name}') as response:
            if response.status != 201:
                print(f"unable to create instance {device_name}, code: {response.status}")
                return
            else:
                print(f"instance {device_name} created")
    server = Server(f'ws://localhost:8080/classic/{device_name}')

    try:
        await server.ws_connect()

        #await server.Set_ConnectionURI("tcp://192.168.137.111:7778?timeout=3000&plain_transfer=auto")
        await server.Set_ConnectionURI("tcp://10.222.222.99:7778?timeout=30000&plain_transfer=auto")

        typez = await server.Get_ConnectionType()

        ret = await server.Connect()
        ret = await server.GetECRStatus()
        code = await server.Get_ResultCode()

        code_description = await server.Get_ResultCodeDescription()
        some_binary_data = base64.b64encode(b'0123456789 abc ABC kinda binary')
        await server.Set_BlockData(some_binary_data.decode('utf-8'))
        print(code, code_description)

    finally:
        await server.close()
    async with aiohttp.ClientSession() as session:
        async with session.delete(f'ws://localhost:8080/classic/{device_name}') as response:
            if response.status == 200:
                print(f"instance {device_name} deleted")
            else:
                print(f"unable to remove instance {device_name}, code: {response.status}")



if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(routine("my_instance"))
