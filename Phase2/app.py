# app.py
from flask import Flask, render_template, Response
from pydub import AudioSegment
import cv2

app = Flask(__name__)

# Video streaming
def generate_video():
    video_path = "./video.mp4"  # Replace with the path to your video file
    cap = cv2.VideoCapture(video_path)
    while True:
        success, frame = cap.read()
        if not success:
            break
        resized_frame = cv2.resize(frame, (640, 480))
        _, buffer = cv2.imencode('.jpg', resized_frame)

        frame_bytes = buffer.tobytes()

        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n\r\n')

# Audio streaming
def generate_audio():
    # audio_file = "./audio.mp3"  # Replace with the path to your audio file
    # audio = AudioSegment.from_file(audio_file, format="mp3")

    # # Convert audio to raw data
    # audio_data = audio.raw_data

    # # Yield chunks of audio data
    # chunk_size = 1024
    # for i in range(0, len(audio_data), chunk_size):
    #     yield audio_data[i:i+chunk_size]
    
    with open("./audio.mp3", "rb") as fwav:
        data = fwav.read(1024)
        while data:
            yield data
            data = fwav.read(1024)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/video_feed')
def video_feed():
    return Response(generate_video(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/audio_feed')
def audio_feed():
    return Response(generate_audio(), mimetype='audio/mp3')

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000)
