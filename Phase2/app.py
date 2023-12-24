# app.py
from flask import Flask, render_template, Response
from pydub import AudioSegment
import cv2
import numpy as np

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
        _, buffer = cv2.imencode('.jpg', resized_frame, [int(cv2.IMWRITE_JPEG_QUALITY), 90])

        frame_bytes = buffer.tobytes()

        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n\r\n')

def generate_audio():
    audio_file_path = "./audio.mp3"
    audio = AudioSegment.from_file(audio_file_path, format="mp3")
    compressed_options = {"codec": "flac", "parameters": ["-compression_level", "8"]}
    compressed_audio = audio.export(format="flac", codec=compressed_options["codec"], parameters=compressed_options["parameters"])
    compressed_audio_data = compressed_audio.read(1024)
    while compressed_audio_data:
        yield compressed_audio_data
        compressed_audio_data = compressed_audio.read(1024)

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
