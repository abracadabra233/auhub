import asyncio
import wave

import pyaudio
import websockets

# 音频参数
FORMAT = pyaudio.paInt16  # 16-bit PCM
CHANNELS = 1  # 单声道
RATE = 16000  # 采样率
CHUNK = 1024  # 每次读取的帧数


async def send_audio():
    uri = "ws://localhost:8081/ws"
    async with websockets.connect(uri) as websocket:
        # 初始化 pyaudio
        p = pyaudio.PyAudio()

        # 打开麦克风流
        stream = p.open(format=FORMAT, channels=CHANNELS, rate=RATE, input=True, frames_per_buffer=CHUNK)

        print("开始录音并发送音频数据...")

        try:
            while True:
                # 从麦克风读取音频数据
                data = stream.read(CHUNK)
                # 发送音频数据到 WebSocket 服务器
                await websocket.send(data)
                print("Audio data sent.")
        except Exception as e:
            print(f"发生错误: {e}")
        finally:
            # 停止并关闭流
            stream.stop_stream()
            stream.close()
            p.terminate()


# 运行客户端
asyncio.get_event_loop().run_until_complete(send_audio())
