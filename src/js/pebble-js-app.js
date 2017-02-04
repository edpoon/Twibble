var Twibble = {
    messages: [],
    baseUrl: "https://api.twitch.tv/kraken",
    clientId: "kqxn6nov00how5uom46vlxb7p32xvf6",
    redirectUri: "pebblejs://close&scope=user_read&force_verify=true",
    token: localStorage.getItem("token")
};

// function to send message to Pebble using AppMessage API
Twibble.sendMessage = function() {
    if (Twibble.messages.length === 0) {
        return;
    }
    var message = Twibble.messages.shift();
    Pebble.sendAppMessage(message, ack, nack);

    // If Pebble receives the message, send the next one
    function ack() {
        Twibble.sendMessage();
    }

    // If Pebble doesn't receive the message, send it again
    function nack() {
        Twibble.messages.unshift(message);
        Twibble.sendMessage();
    }
};


/**
 * Given that the user has authenticated Twibble, it sends the live channels from the user's followed list on Twitch
 *
 * @param {number} offset - Object offset for pagination
 */
Twibble.getFollowedStreams = function(offset) {
    var url = `${Twibble.baseUrl}/streams/followed?limit=5&oauth_token=${Twibble.token}&offset=${offset}`;
    var request = new XMLHttpRequest();

    request.onload = function() {
        if (request.readyState === 4 && request.status === 200) {
            var response =  JSON.parse(request.responseText);
            for (var i = 0; i < response.streams.length; i++) {
                var message = {
                    TITLE_KEY: response.streams[i].channel.display_name,
                    FIRST_SUBTITLE_KEY: response.streams[i].game,
                    SECOND_SUBTITLE_KEY: response.streams[i].viewers.toString()
                };
                Twibble.messages.push(message);
            }
        } else {
            Twibble.messages.push({ERROR_KEY: "Login and check data connection and try again."});
        }
        Twibble.sendMessage();
    };
    request.open("GET", url, true);
    request.send();
};

/**
 * Sends the top streaming channels on Twitch to the Pebble application
 *
 * @param {number} offset - Object offset for pagination.
 */
Twibble.getTopStreams = function(offset) {
    var url = `${Twibble.baseUrl}/streams?client_id=${Twibble.clientId}&limit=5&offset=${offset}`;
    var request = new XMLHttpRequest();

    request.onload = function() {
        if (request.readyState === 4 && request.status === 200) {
            var response = JSON.parse(request.responseText);
            for (var i = 0; i < response.streams.length; i++) {
                var message = {
                    TITLE_KEY: response.streams[i].channel.display_name,
                    FIRST_SUBTITLE_KEY: response.streams[i].game,
                    SECOND_SUBTITLE_KEY: response.streams[i].viewers.toString()
                };
                Twibble.messages.push(message);
            }
        } else {
            Twibble.messages.push({ERROR_KEY: "Login and check data connection and try again."});
        }
        Twibble.sendMessage();
    };
    request.open("GET", url, true);
    request.send();
};

/**
 * Sends the top streaming games on Twitch to the Pebble application
 *
 * @param {number} offset - Object offset for pagination
 */
Twibble.getTopGames = function(offset) {
    var url = `${Twibble.baseUrl}/games/top?client_id=${Twibble.clientId}&limit=5&offset=${offset}`;
    var request = new XMLHttpRequest();

    request.onload = function() {
        if (request.readyState === 4 && request.status === 200) {
            var response = JSON.parse(request.responseText);
            for (var i = 0; i < response.top.length; i++) {
                var message = {
                    TITLE_KEY: response.top[i].game.name,
                    FIRST_SUBTITLE_KEY: `${response.top[i].channels.toString()} Live Channels`,
                    SECOND_SUBTITLE_KEY: response.top[i].viewers.toString()
                };
                Twibble.messages.push(message);
            }
        } else {
            Twibble.messages.push({ERROR_KEY: "Check data connection and try again."});
        }
        Twibble.sendMessage();
    };
    request.open("GET", url, true);
    request.send();
};

/**
 * Sends the top channels for a particular game on Twitch to the Pebble application
 *
 * @param {string} game - The name of the game whose channels we'll return
 * @param {number} offset - Object offset for pagination
 */
Twibble.getStreams = function(game, offset) {
    var url = `${Twibble.baseUrl}/streams?client_id=${Twibble.clientId}&limit=5&game=${game}`;
    var request = new XMLHttpRequest();

    request.onload = function() {
        if (request.readyState === 4 && request.status === 200) {
            var response = JSON.parse(request.responseText);
            for (var i = 0; i < response.streams.length; i++) {
                var message = {
                    TITLE_KEY: response.streams[i].channel.status,
                    FIRST_SUBTITLE_KEY: response.streams[i].channel.name,
                    SECOND_SUBTITLE_KEY: response.streams[i].viewers.toString()
                };
                Twibble.messages.push(message);
            }
        } else {
            Twibble.messages.push({ERROR_KEY: "Check data connection and try again."});
        }
        Twibble.sendMessage();
    };
    request.open("GET", url, true);
    request.send();
};

// Called when the configuration window is opened so that the user may authenticate Twibble
Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL(`${Twibble.baseUrl}/oauth2/authorize?response_type=token&client_id=${Twibble.clientId}&redirect_uri=${Twibble.redirectUri}`);
});

// Called when the configuration window is closed
Pebble.addEventListener("webviewclosed", function(e) {
    // Grab the token from the url and stores it into phone storage
    Twibble.token = e.response.slice(13, 43);
    localStorage.setItem("token", Twibble.token);
});

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", function(e) {
    switch (e.payload.QUERY_KEY) {
    case "Channels":
        Twibble.getTopStreams(e.payload.OFFSET_KEY);
        break;
    case "Games":
        Twibble.getTopGames(e.payload.OFFSET_KEY);
        break;
    case "Following":
        Twibble.getFollowedStreams(e.payload.OFFSET_KEY);
        break;
    default:
        Twibble.getStreams(e.payload.QUERY_KEY, e.payload.OFFSET_KEY);
    }
});
