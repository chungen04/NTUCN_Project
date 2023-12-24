// static/script.js
document.addEventListener('DOMContentLoaded', function () {
    // Get references to the video and audio elements
    var videoFrame = document.getElementById('videoFrame');
    var refreshVideoButton = document.getElementById('refreshVideoButton');
    
    var audioPlayer = document.getElementById('audioPlayer');
    var refreshAudioButton = document.getElementById('refreshAudioButton');

    // Function to refresh the video source
    function refreshVideo() {
        // Append a timestamp to the video source URL to force a refresh
        videoFrame.src = "video_feed?timestamp=" + new Date().getTime();
    }

    // Function to refresh the audio source
    function refreshAudio() {
        // Reload the audio player
        audioPlayer.load();
    }

    // Attach the refreshVideo function to the video refresh button click event
    refreshVideoButton.addEventListener('click', refreshVideo);

    // Attach the refreshAudio function to the audio refresh button click event
    refreshAudioButton.addEventListener('click', refreshAudio);

    function onFormSubmit(event) {
        event.preventDefault();  // Prevent the default form submission
        const message = document.getElementById('messageInput').value;
        console.log(message);
        sendMessage(message);
    }

    // Attach the onFormSubmit function to the form submission event
    document.querySelector('form').addEventListener('submit', onFormSubmit);
});

// logout.js
function logout() {
    // You can use JavaScript to make an AJAX request or redirect to the logout route.
    // Here, I'm using a simple redirect.
    window.location.href = "/logout"; // Adjust the path as needed
}

// for message board
const socket = new WebSocket('ws://localhost:5001');  // WebSocket connection to the server

// Function to send a message to the server
function sendMessage(message) {
    // Check if the WebSocket connection is open
    if (socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify({ message: message }));
    } else {
        console.error('WebSocket is not open.');
    }
}