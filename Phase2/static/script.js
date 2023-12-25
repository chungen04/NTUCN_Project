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

        // Get the username from the HTML
        const usernameElement = document.getElementById('username');
        const username = usernameElement ? usernameElement.textContent.trim() : '';

        sendMessage(message, username);
    }

    // Attach the onFormSubmit function to the form submission event
    document.querySelector('form').addEventListener('submit', onFormSubmit);
});

// Function to fetch messages from the backend
async function fetchMessages() {
    try {
        const response = await fetch('/get-messages');
        const messages = await response.json();

        // Clear the existing messages
        const messageList = document.getElementById('messageList');
        messageList.innerHTML = '';

        // Update the message container with the new messages
        messages.forEach(message => {
            const listItem = document.createElement('li');
            listItem.textContent = `${message.username}: ${message.message}`;
            messageList.appendChild(listItem);
        });
    } catch (error) {
        console.error('Error fetching messages:', error);
    }
}

// Call fetchMessages when the page loads and periodically refresh
fetchMessages();
setInterval(fetchMessages, 500); // Refresh every 5 seconds

// for message board
const socket = new WebSocket('ws://localhost:5001');  // WebSocket connection to the server

// Function to send a message to the server
function sendMessage(message, username) {
    // Check if the WebSocket connection is open
    if (socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify({ 
            message: message,
            username: username
        }));
    } else {
        console.error('WebSocket is not open.');
    }
}

// logout.js
function logout() {
    // You can use JavaScript to make an AJAX request or redirect to the logout route.
    // Here, I'm using a simple redirect.
    window.location.href = "/logout"; // Adjust the path as needed
}
