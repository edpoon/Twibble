// This will be an array of dicts to send to Pebble
var messages = [];

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

function getFollowedStreams(offset) {
    // Send at most 10 items for first request and at most 5 items for subsequent requests
    var limit = 5;
    if (offset === 0) {
        limit = 10;
    }

    // Fetch request for user's followed streams
    var req = new XMLHttpRequest();
    req.open('GET', 'https://api.twitch.tv/kraken/streams/followed?limit=' + limit + '&oauth_token=' + localStorage.getItem('oauth token') + '&offset=' + offset, false);
    req.send(null);
    var response = JSON.parse(req.responseText);

    // Number of messages to send
    var total = Math.min(response._total - offset, limit);
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
    sendMessage();
}

function getTopStreams(offset) {
    // Send at most 10 items for first request and at most 5 items for subsequent requests
    var limit = 5;
    if (offset === 0) {
        limit = 10;
    }

    // Fetch request for current top streams
    var req = new XMLHttpRequest();
    req.open('GET', 'https://api.twitch.tv/kraken/streams?limit=' + limit + '&offset=' + offset, false);
    req.send(null);
    var response = JSON.parse(req.responseText);

    // Number of messages to send
    var total = Math.min(response._total - offset, limit);
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

    sendMessage();
}

function getTopGames(offset) {
    var limit = 5;
    /*
    if (offset === 0) {
        limit = 10;
    }
     */

    var req = new XMLHttpRequest();
    req.open('GET', 'https://api.twitch.tv/kraken/games/top?limit=' + limit + '&offset=' + offset, false);
    req.send(null);
    var response = JSON.parse(req.responseText);

    var total = Math.min(response._total - offset, limit);
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

    sendMessage();
}

function getStreams(game, offset) {
    var limit = 5;
    var req = new XMLHttpRequest();
  req.open('GET', 'https://api.twitch.tv/kraken/streams?game=' + game + '&limit=' + limit, false);
    req.send(null);
    var response = JSON.parse(req.responseText);

    var total = Math.min(response.streams.length - offset, limit);
    for (var i = 0; i < total; i++) {
        // Issues with special characters
        var status = (response.streams[i].channel.status);
        var streamer = response.streams[i].channel.name;
        var viewers = response.streams[i].viewers.toString();

        console.log(status + streamer + viewers);
        var message = {
            TITLE_KEY: status,
            SUBTITLE1_KEY: streamer,
            SUBTITLE2_KEY: viewers
        };
        messages.push(message);
    }

    sendMessage();
}

/**
 * Deletes token from local storage.
 * At the watch end, this should result in
 * the user getting logged out.
 */
function removeToken() {
    if(localStorage.getItem('oauth token')) {
        localStorage.removeItem('oauth token');
        var message = {
            TOKEN_REMOVE_KEY: "Removed token"
        };
    }
    else {
        var message = {
            TOKEN_REMOVE_KEY: "Token already deleted"
        };
    }
    sendMessage();
}

/*
 * Retrieves user name from token
 * still need to handle error cases.
 */
function getUserName() {
  console.log("Getting user name");
    var req = new XMLHttpRequest();
    req.open('GET', 'https://api.twitch.tv/kraken?oauth_token=' + localStorage.getItem('oauth token'), false);
    req.send(null);
    var response = JSON.parse(req.responseText);
    var userName = response.token.user_name;
    console.log(userName);
    var message = {
        USERNAME_KEY: userName
    };
    messages.push(message);
    sendMessage();
}

// Configuration window
Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL('https://api.twitch.tv/kraken/oauth2/authorize?response_type=token&client_id=kqxn6nov00how5uom46vlxb7p32xvf6&redirect_uri=pebblejs://close&scope=user_read');
});

// Called when the configuration window is closed
Pebble.addEventListener("webviewclosed", function(e) {
    // Decode and parse config data as JSON
    var config_data = JSON.parse(decodeURIComponent(e.response));
    console.log('Config window returned:' + JSON.stringify(config_data));
});

// Called when Pebble first starts
Pebble.addEventListener("ready", function(e) {
    // Store a sample oauth token into local storage for now
    var oauth_token = '0z7aboxelw2npt53h9ax0vxcwutrox';
    localStorage.setItem('oauth token', oauth_token);
    // getUserName();
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
    case "Token":
        removeToken();
        break;
    case "User":
        getUserName();
        break;
    default:
        getStreams(e.payload.QUERY_KEY, e.payload.OFFSET_KEY);
        break;
    }
});
