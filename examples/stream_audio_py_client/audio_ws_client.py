import asyncio
import wave

import websockets

# async def send_audio():
#     uri = "ws://localhost:18780/ws"
#     async with websockets.connect(uri) as websocket:
#         # 打开本地 WAV 文件
#         with wave.open("/home/wx/code/ros2/loong_exp/cpp_exp/audio_player/examples/test.wav", "rb") as wav_file:
#             # 读取 WAV 文件的全部数据
#             frames = wav_file.readframes(wav_file.getnframes())
#             # await websocket.send(frames)
#             for i in range(0, len(frames), 10000):
#                 await websocket.send(frames[i : i + 10000])
#                 print("Audio data sent.")

# if a=='1'

a = 2


async def send_audio():
    uri = "ws://localhost:8081/ws"
    if a == 1:
        data = [1] * 10000
        async with websockets.connect(uri) as websocket:
            while True:
                await websocket.send(bytes(data))
                print("Audio data sent.")
    else:
        async with websockets.connect(uri) as websocket:
            # 打开本地 WAV 文件
            with wave.open("/home/wx/code/loong/loong_exp/cpp_exp/audio_player/examples/sources/test.wav", "rb") as wav_file:
                # 读取 WAV 文件的全部数据
                frames = wav_file.readframes(wav_file.getnframes())
                # await websocket.send(frames)
                for i in range(0, len(frames), 10000):
                    await websocket.send(frames[i : i + 10000])
                    await asyncio.sleep(0.1)
                    print("Audio data sent.")


# 运行客户端
asyncio.get_event_loop().run_until_complete(send_audio())

# wav pcm
# sudo kill -9 $(sudo lsof -t -i :18780) 2>/dev/null
