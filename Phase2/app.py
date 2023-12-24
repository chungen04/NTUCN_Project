# app.py
from flask import Flask, render_template, Response, redirect, url_for, session, request
from pydub import AudioSegment
import cv2
import numpy as np
from flask import Flask, render_template, redirect, url_for, flash
from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import DataRequired, EqualTo

app = Flask(__name__)

app.secret_key = 'g/I^+v(&Z@Y@:j('
def is_logged_in(session):
    return 'user_id' in session

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

@app.route('/video_feed')
def video_feed():
    if not is_logged_in(session):
        return redirect(url_for('login'))
    return Response(generate_video(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/audio_feed')
def audio_feed():
    if not is_logged_in(session):
        return redirect(url_for('login'))
    return Response(generate_audio(), mimetype='audio/mp3')

@app.route('/dashboard')
def dashboard():
    # Check if the user is logged in
    if not is_logged_in(session):
        return redirect(url_for('login'))
    user = User.query.get(session['user_id'])
    login_count = user.login_count
    username = user.username
    return render_template('dashboard.html', login_count = login_count, username = username)

# Define the LoginForm class
class LoginForm(FlaskForm):
    username = StringField('Username', validators=[DataRequired()])
    password = PasswordField('Password', validators=[DataRequired()])
    submit = SubmitField('Login')
    
class RegisterForm(FlaskForm):
    username = StringField('Username', validators=[DataRequired()])
    password = PasswordField('Password', validators=[DataRequired()])
    confirm_password = PasswordField('Confirm Password', validators=[DataRequired(), EqualTo('password')])
    submit = SubmitField('Register')

app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///users.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password_hash = db.Column(db.String(128), nullable=False)
    login_count = db.Column(db.Integer, default=0)  # New column for login count

@app.route('/')
def home():
    # Redirect to the login page
    return redirect(url_for('login'))

@app.route('/register', methods=['GET', 'POST'])
def register():
    form = RegisterForm()
    if request.method == 'POST' and form.validate_on_submit():
        username = form.username.data
        password = form.password.data
        # Check if the username already exists
        existing_user = User.query.filter_by(username=username).first()
        if existing_user:
            flash('Username already exists. Please choose another one.', 'error')
            return redirect(url_for('register'))
        
        # Hash the password before storing
        hashed_password = generate_password_hash(password)
        new_user = User(username=username, password_hash=hashed_password)
        db.session.add(new_user)
        db.session.commit()

        flash('Registration successful! Please log in.', 'success')
        return redirect(url_for('login'))
    else:
        print(form.errors)
    return render_template('register.html', form=form)

@app.route('/login', methods=['GET', 'POST'])
def login():
    form = LoginForm()

    if request.method == 'POST' and form.validate_on_submit():
        username = form.username.data
        password = form.password.data

        user = User.query.filter_by(username=username).first()

        if user and check_password_hash(user.password_hash, password):
            # Set a cookie to store user's information (insecure for demonstration purposes)
            user.login_count += 1
            db.session.commit()
            response = redirect(url_for('dashboard'))
            session['user_id'] = user.id
            response.set_cookie('user_id', str(user.id))
            return response
        else:
            flash('Login failed. Check your username and password.', 'error')

    return render_template('login.html', form=form)

@app.route('/logout')
def logout():
    flash('Logged out successfully.', 'info')
    # Clear the user_id cookie to log out
    session.pop('user_id', None)
    response = redirect(url_for('home'))
    response.delete_cookie('user_id')
    return response

if __name__ == '__main__':
    with app.app_context():
        db.create_all()    
    app.run(host="0.0.0.0", port=5000)