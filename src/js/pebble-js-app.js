// This will be an array of dicts to send to Pebble
var messages = [];
var MAX_ITEMS = 5;

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
    }
    else {
        reponse = "error";
    }
    return response;
}

function getFollowedStreams(offset) {
    console.log('OAuth Token: ' + localStorage.getItem('OAUTH_TOKEN'));
    var url = 'https://api.twitch.tv/kraken/streams/followed?limit=5&oauth_token=' + localStorage.getItem('OAUTH_TOKEN') + '&offset=' + offset;
    var response = sendDataRequest(url);
    if (response === "error" || !response || !response.hasOwnProperty('_total')) {
        var message = { ERROR_KEY: ":( Couldn't fetch followed streams. Log in via watch and check connection." };
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
                SUBTITLE1_KEY: game,
                SUBTITLE1_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

function getTopStreams(offset) {
    var url = 'https://api.twitch.tv/kraken/streams?limit=5&offset=' + offset;
    var response = sendDataRequest(url);
    if (!response || response === "error" || !response.hasOwnProperty('_total')) {
        var message = { ERROR_KEY: "Error retrieving top streams. Verify phone and internet connection." };
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
                SUBTITLE1_KEY: game,
                SUBTITLE2_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

function getTopGames(offset) {
    var url = 'https://api.twitch.tv/kraken/games/top?limit=&offset=' + offset;
    var response = sendDataRequest(url);
    if (!response || response === "error" || !response.hasOwnProperty('_total')) {
        var message = { ERROR_KEY: "Error retrieving top games. Verify phone and internet connection." };
        messages.push(message);
    } else {
        // Number of messages to send
        var total = Math.min(response.top.length - offset, MAX_ITEMS);
        for (var i = 0; i < total; i++) {
            var game = response.top[i].game.name;
            var channels = response.top[i].channels.toString() + ' Live Channels';
            var viewers = response.top[i].viewers.toString();
            var message = {
                TITLE_KEY: game,
                SUBTITLE1_KEY: channels,
                SUBTITLE2_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

function getStreams(game, offset) {
    var url = 'https://api.twitch.tv/kraken/streams?limit=5&game=' + game;
    var response = sendDataRequest(url);
    if (!response || response === "error" || !response.hasOwnProperty('_total')) {
        var message = { ERROR_KEY: "Error retrieving top streams. Verify phone and internet connection." };
        messages.push(message);
    } else {
        // Number of messages to send
        var total = Math.min(response.streams.length - offset, MAX_ITEMS);
        for (var i = 0; i < total; i++) {
            // Issues with special characters
            var status = response.streams[i].channel.status;
            var streamer = response.streams[i].channel.name;
            var viewers = response.streams[i].viewers.toString();
            var message = {
                TITLE_KEY: status,
                SUBTITLE1_KEY: streamer,
                SUBTITLE2_KEY: viewers
            };
            messages.push(message);
        }
    }
    sendMessage();
}

// Retrieves Twitch username using the locally stored oauth token
function getUserName() {
    var url = 'https://api.twitch.tv/kraken?oauth_token=' + localStorage.getItem('OAUTH_TOKEN');
    var response = sendDataRequest(url);
    var username = response.token.user_name;
    if (!username) {
        username = "Not Logged In";
    }
    var message = {
        USERNAME_KEY: username
    };
    messages.push(message);
    sendMessage();
}

Pebble.addEventListener("ready", function(e) {
    getUserName();
});

// Configuration window
Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL('https://api.twitch.tv/kraken/oauth2/authorize?response_type=token&client_id=kqxn6nov00how5uom46vlxb7p32xvf6&redirect_uri=pebblejs://close&scope=user_read&force_verify=true');
});

// Called when the configuration window is closed
Pebble.addEventListener("webviewclosed", function(e) {
    // Grab the token from the URL
    var OAUTH_TOKEN = e.response.slice(13, 43);
    localStorage.setItem("OAUTH_TOKEN", OAUTH_TOKEN);
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
        case "User":
            getUserName();
            break;
        default:
            getStreams(e.payload.QUERY_KEY, e.payload.OFFSET_KEY);
            break;
    }
});
