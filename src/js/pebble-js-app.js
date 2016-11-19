// This will be an array of dicts to send to Pebble
var messages = [];
var MAX_ITEMS = 5;
var CLIENT_ID='kqxn6nov00how5uom46vlxb7p32xvf6'

// Function to send a message to the Pebble using AppMessage API
function sendMessage() {
    if (messages.length === 0) {
        return;
    }
    var message = messages.shift();
    Pebble.sendAppMessage(message, ack, nack);

    // If Pebble receives the message, send the next one
    function ack() {
        sendMessage();
    }

    // If Pebble doesn't receive the message, send it again
    function nack() {
        messages.unshift(message);
        sendMessage();
    }
}

// Helper function to send fetch Twitch API data
function sendDataRequest(url) {
    var req = new XMLHttpRequest();
    var response;
    req.open('GET', url, false);
    req.send(null);
    if (req.readyState == 4 && req.status == 200) {
        response = JSON.parse(req.responseText);
    } else {
        response = "error";
    }
    return response;
}

function getFollowedStreams(offset) {
    var url = 'https://api.twitch.tv/kraken/streams/followed?limit=5&oauth_token=' + localStorage.getItem('OAUTH_TOKEN') + '&offset=' + offset;
    var response = sendDataRequest(url);
    if (response === "error" || !response || !response.hasOwnProperty('_total')) {
        var message = {
            ERROR_KEY: "LOGIN/CHECK DATA CONNECTION AND TRY AGAIN."
        };
        messages.push(message);
    } else {
        // Number of messages to send
        var total = Math.min(response._total - offset, MAX_ITEMS);
        for (var i = 0; i < total; i++) {
            var streamer = response.streams[i].channel.display_name;
            var game = response.streams[i].game;
            var viewers = response.streams[i].viewers.toString();
            // Add items to array in preparation to send
            var message = {
                TITLE_KEY: streamer,
                FIRST_SUBTITLE_KEY: game,
                SECOND_SUBTITLE_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

function getTopStreams(offset) {
    var url = 'https://api.twitch.tv/kraken/streams?client_id=' + CLIENT_ID + '&limit=5&offset=' + offset;
    var response = sendDataRequest(url);
    if (!response || response === "error" || !response.hasOwnProperty('_total')) {
        var message = {
            ERROR_KEY: "CHECK DATA CONNECTION AND TRY AGAIN."
        };
        messages.push(message);
    } else {
        // Number of messages to send
        var total = Math.min(response._total - offset, MAX_ITEMS);
        for (var i = 0; i < total; i++) {
            var streamer = response.streams[i].channel.display_name;
            var game = response.streams[i].game;
            var viewers = response.streams[i].viewers.toString();
            // Add items to array in preparation to send
            var message = {
                TITLE_KEY: streamer,
                FIRST_SUBTITLE_KEY: game,
                SECOND_SUBTITLE_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

function getTopGames(offset) {
    var url = 'https://api.twitch.tv/kraken/games/top?client_id=' + CLIENT_ID + '&limit=5&offset=' + offset;
    var response = sendDataRequest(url);
    if (!response || response === "error" || !response.hasOwnProperty('_total')) {
        var message = {
            ERROR_KEY: "CHECK DATA CONNECTION AND TRY AGAIN."
        };
        messages.push(message);
    } else {
        // Number of messages to send
        var total = Math.min(response._total - offset, MAX_ITEMS);
        for (var i = 0; i < total; i++) {
            var game = response.top[i].game.name;
            var channels = response.top[i].channels.toString() + ' Live Channels';
            var viewers = response.top[i].viewers.toString();
            var message = {
                TITLE_KEY: game,
                FIRST_SUBTITLE_KEY: channels,
                SECOND_SUBTITLE_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

function getStreams(game, offset) {
    var url = 'https://api.twitch.tv/kraken/streams?client_id=' + CLIENT_ID + '&limit=5&game=' + game;
    var response = sendDataRequest(url);
    if (!response || response === "error" || !response.hasOwnProperty('_total')) {
        var message = {
            ERROR_KEY: "Error retrieving top streams. Verify phone and internet connection."
        };
        messages.push(message);
    } else {
        // Number of messages to send
        var total = Math.min(response._total - offset, MAX_ITEMS);
        for (var i = 0; i < total; i++) {
            // Issues with special characters
            var status = response.streams[i].channel.status;
            var streamer = response.streams[i].channel.name;
            var viewers = response.streams[i].viewers.toString();
            var message = {
                TITLE_KEY: status,
                FIRST_SUBTITLE_KEY: streamer,
                SECOND_SUBTITLE_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

// Configuration window
Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL('https://api.twitch.tv/kraken/oauth2/authorize?response_type=token&client_id=' + CLIENT_ID + '&redirect_uri=pebblejs://close&scope=user_read&force_verify=true');
});

// Called when the configuration window is closed
Pebble.addEventListener("webviewclosed", function(e) {
    // Grab the token from the URL
    var OAUTH_TOKEN = e.response.slice(13, 43);
    localStorage.setItem('OAUTH_TOKEN', OAUTH_TOKEN);
});

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", function(e) {
    switch (e.payload.QUERY_KEY) {
        case "Channels":
            getTopStreams(e.payload.OFFSET_KEY);
            break;
        case "Games":
            getTopGames(e.payload.OFFSET_KEY);
            break;
        case "Following":
            getFollowedStreams(e.payload.OFFSET_KEY);
            break;
        default:
            getStreams(e.payload.QUERY_KEY, e.payload.OFFSET_KEY);
            break;
    }
});
