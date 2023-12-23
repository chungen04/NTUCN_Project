// script.js
document.addEventListener('DOMContentLoaded', function () {
    // Get references to the image and button
    var videoFrame = document.getElementById('videoFrame');
    var refreshButton = document.getElementById('refreshButton');

    // Function to refresh the image source
    function refreshVideo() {
        // Append a timestamp to the image source URL to force a refresh
        videoFrame.src = "video_feed?timestamp=" + new Date().getTime();
    }

    // Attach the refreshVideo function to the button click event
    refreshButton.addEventListener('click', refreshVideo);
});
