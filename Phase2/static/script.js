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
});

// logout.js
function logout() {
    // You can use JavaScript to make an AJAX request or redirect to the logout route.
    // Here, I'm using a simple redirect.
    window.location.href = "/logout"; // Adjust the path as needed
}
